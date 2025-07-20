#include <cstdint>
#include <string>
#include "gtest/gtest.h"
#include "cpu.hpp"
#include "bus.hpp"
#include <tuple>
#include "cpu_helpers.hpp"

// Test structure for stack instructions
struct StackParams {
    uint8_t opcode;
    uint8_t expected_cycles_8;
    uint8_t expected_cycles_16;
    std::string instruction;
    std::string description;
};

// Base Stack Test Class
class StackTestBase : public ::testing::Test {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

// Individual Stack Test Classes
class PHATest : public StackTestBase {};
class PLATest : public StackTestBase {};
class PHXTest : public StackTestBase {};
class PLXTest : public StackTestBase {};
class PHYTest : public StackTestBase {};
class PLYTest : public StackTestBase {};
class PHPTest : public StackTestBase {};
class PLPTest : public StackTestBase {};
class PHDTest : public StackTestBase {};
class PLDTest : public StackTestBase {};
class PHKTest : public StackTestBase {};
class PLKTest : public StackTestBase {};
class PEATest : public StackTestBase {};
class PEITest : public StackTestBase {};
class PERTest : public StackTestBase {};

// PHA Tests
TEST_F(PHATest, PHA_8Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p |= CPU::M; // 8-bit accumulator
    cpu->a = 0x42;
    cpu->stkp = 0x01FD; // Initial stack pointer
    
    bus->write(test_pc, 0x48); // PHA
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 3);
    EXPECT_EQ(cpu->stkp, 0x01FC); // Stack pointer decremented by 1
    EXPECT_EQ(bus->read(0x0001FC), 0x42); // Value pushed to stack
}

TEST_F(PHATest, PHA_16Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p &= ~CPU::M; // 16-bit accumulator
    cpu->a = 0x1234;
    cpu->stkp = 0x01FD; // Initial stack pointer
    
    bus->write(test_pc, 0x48); // PHA
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 4);
    EXPECT_EQ(cpu->stkp, 0x01FB); // Stack pointer decremented by 2
    EXPECT_EQ(bus->read(0x0001FC), 0x12); // High byte pushed first
    EXPECT_EQ(bus->read(0x0001FB), 0x34); // Low byte pushed second
}

// PLA Tests
TEST_F(PLATest, PLA_8Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p |= CPU::M; // 8-bit accumulator
    cpu->a = 0x00; // Clear accumulator
    cpu->stkp = 0x01FC; // Stack pointer pointing to pushed value
    bus->write(0x0001FC, 0x42); // Value on stack (at stack address)
    
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x68); // PLA
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 4);
    EXPECT_EQ(cpu->stkp, 0x01FD); // Stack pointer incremented by 1
    EXPECT_EQ(cpu->a, 0x42); // Value pulled from stack
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Not zero
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Not negative
}

TEST_F(PLATest, PLA_16Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p &= ~CPU::M; // 16-bit accumulator
    cpu->a = 0x0000; // Clear accumulator
    cpu->stkp = 0x01FB; // Stack pointer pointing to pushed values
    bus->write(0x0001FC, 0x12); // High byte on stack
    bus->write(0x0001FB, 0x34); // Low byte on stack
    
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x68); // PLA
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 5);
    EXPECT_EQ(cpu->stkp, 0x01FD); // Stack pointer incremented by 2
    EXPECT_EQ(cpu->a, 0x1234); // Value pulled from stack
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Not zero
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Not negative
}

// PHX Tests
TEST_F(PHXTest, PHX_8Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p |= CPU::X; // 8-bit index registers
    cpu->x = 0x42;
    cpu->stkp = 0x01FD; // Initial stack pointer
    
    bus->write(test_pc, 0xDA); // PHX
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 3);
    EXPECT_EQ(cpu->stkp, 0x01FC); // Stack pointer decremented by 1
    EXPECT_EQ(bus->read(0x0001FC), 0x42); // Value pushed to stack
}

TEST_F(PHXTest, PHX_16Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p &= ~CPU::X; // 16-bit index registers
    cpu->x = 0x1234;
    cpu->stkp = 0x01FD; // Initial stack pointer
    
    bus->write(test_pc, 0xDA); // PHX
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 4);
    EXPECT_EQ(cpu->stkp, 0x01FB); // Stack pointer decremented by 2
    EXPECT_EQ(bus->read(0x0001FC), 0x12); // High byte pushed first
    EXPECT_EQ(bus->read(0x0001FB), 0x34); // Low byte pushed second
}

// PLX Tests
TEST_F(PLXTest, PLX_8Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p |= CPU::X; // 8-bit index registers
    cpu->x = 0x00; // Clear X register
    cpu->stkp = 0x01FC; // Stack pointer pointing to pushed value
    bus->write(0x0001FC, 0x42); // Value on stack (at stack address)
    
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0xFA); // PLX
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 4);
    EXPECT_EQ(cpu->stkp, 0x01FD); // Stack pointer incremented by 1
    EXPECT_EQ(cpu->x, 0x42); // Value pulled from stack
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Not zero
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Not negative
}

TEST_F(PLXTest, PLX_16Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p &= ~CPU::X; // 16-bit index registers
    cpu->x = 0x0000; // Clear X register
    cpu->stkp = 0x01FB; // Stack pointer pointing to pushed values
    bus->write(0x0001FC, 0x12); // High byte on stack
    bus->write(0x0001FB, 0x34); // Low byte on stack
    
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0xFA); // PLX
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 5);
    EXPECT_EQ(cpu->stkp, 0x01FD); // Stack pointer incremented by 2
    EXPECT_EQ(cpu->x, 0x1234); // Value pulled from stack
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Not zero
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Not negative
}

// PHY Tests
TEST_F(PHYTest, PHY_8Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p |= CPU::X; // 8-bit index registers
    cpu->y = 0x42;
    cpu->stkp = 0x01FD; // Initial stack pointer
    
    bus->write(test_pc, 0x5A); // PHY
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 3);
    EXPECT_EQ(cpu->stkp, 0x01FC); // Stack pointer decremented by 1
    EXPECT_EQ(bus->read(0x0001FC), 0x42); // Value pushed to stack
}

TEST_F(PHYTest, PHY_16Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p &= ~CPU::X; // 16-bit index registers
    cpu->y = 0x1234;
    cpu->stkp = 0x01FD; // Initial stack pointer
    
    bus->write(test_pc, 0x5A); // PHY
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 4);
    EXPECT_EQ(cpu->stkp, 0x01FB); // Stack pointer decremented by 2
    EXPECT_EQ(bus->read(0x0001FC), 0x12); // High byte pushed first
    EXPECT_EQ(bus->read(0x0001FB), 0x34); // Low byte pushed second
}

// PLY Tests
TEST_F(PLYTest, PLY_8Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p |= CPU::X; // 8-bit index registers
    cpu->y = 0x00; // Clear Y register
    cpu->stkp = 0x01FC; // Stack pointer pointing to pushed value
    bus->write(0x0001FC, 0x42); // Value on stack (at stack address)
    
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x7A); // PLY
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 4);
    EXPECT_EQ(cpu->stkp, 0x01FD); // Stack pointer incremented by 1
    EXPECT_EQ(cpu->y, 0x42); // Value pulled from stack
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Not zero
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Not negative
}

TEST_F(PLYTest, PLY_16Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p &= ~CPU::X; // 16-bit index registers
    cpu->y = 0x0000; // Clear Y register
    cpu->stkp = 0x01FB; // Stack pointer pointing to pushed values
    bus->write(0x0001FC, 0x12); // High byte on stack
    bus->write(0x0001FB, 0x34); // Low byte on stack
    
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x7A); // PLY
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 5);
    EXPECT_EQ(cpu->stkp, 0x01FD); // Stack pointer incremented by 2
    EXPECT_EQ(cpu->y, 0x1234); // Value pulled from stack
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Not zero
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Not negative
}

// PHP Tests
TEST_F(PHPTest, PHP_Push_Status) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p = 0x1234; // Set various flags
    cpu->stkp = 0x01FD; // Initial stack pointer
    
    bus->write(test_pc, 0x08); // PHP
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 3);
    EXPECT_EQ(cpu->stkp, 0x01FC); // Stack pointer decremented by 1
    uint8_t pushed_status = bus->read(0x0001FC);
    EXPECT_EQ(pushed_status, 0x34); // Low byte of status register
    EXPECT_TRUE(pushed_status & 0x10); // B flag should be set
}

// PLP Tests
TEST_F(PLPTest, PLP_Pull_Status) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p = 0x0000; // Clear status register
    cpu->stkp = 0x01FC; // Stack pointer pointing to pushed value
    bus->write(0x0001FC, 0x42); // Status value on stack (at stack address)
    
    bus->write(test_pc, 0x28); // PLP
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 4);
    EXPECT_EQ(cpu->stkp, 0x01FD); // Stack pointer incremented by 1
    EXPECT_EQ(cpu->p & 0xFF, 0x42); // Low byte of status register updated
}

// PHD Tests
TEST_F(PHDTest, PHD_Push_Direct_Page) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->d = 0x1234; // Direct page register
    cpu->stkp = 0x01FD; // Initial stack pointer
    
    bus->write(test_pc, 0x0B); // PHD
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 4);
    EXPECT_EQ(cpu->stkp, 0x01FB); // Stack pointer decremented by 2
    EXPECT_EQ(bus->read(0x0001FC), 0x12); // High byte pushed first
    EXPECT_EQ(bus->read(0x0001FB), 0x34); // Low byte pushed second
}

// PLD Tests
TEST_F(PLDTest, PLD_Pull_Direct_Page) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->d = 0x0000; // Clear direct page register
    cpu->stkp = 0x01FB; // Stack pointer pointing to pushed values
    bus->write(0x0001FC, 0x12); // High byte on stack
    bus->write(0x0001FB, 0x34); // Low byte on stack
    
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x2B); // PLD
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 5);
    EXPECT_EQ(cpu->stkp, 0x01FD); // Stack pointer incremented by 2
    EXPECT_EQ(cpu->d, 0x1234); // Value pulled from stack
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Not zero
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Not negative
}

// PHK Tests
TEST_F(PHKTest, PHK_Push_Program_Bank) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->pb = 0x42; // Program bank register
    cpu->stkp = 0x01FD; // Initial stack pointer
    
    bus->write(test_pc, 0x4B); // PHK
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 3);
    EXPECT_EQ(cpu->stkp, 0x01FC); // Stack pointer decremented by 1
    EXPECT_EQ(bus->read(0x0001FC), 0x42); // Program bank pushed to stack
}

// PLK Tests
TEST_F(PLKTest, PLK_Pull_Program_Bank) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->pb = 0x00; // Clear program bank register
    cpu->stkp = 0x01FC; // Stack pointer pointing to pushed value
    bus->write(0x0001FC, 0x42); // Program bank value on stack (at stack address)
    
    bus->write(test_pc, 0xAB); // PLK
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 4);
    EXPECT_EQ(cpu->stkp, 0x01FD); // Stack pointer incremented by 1
    EXPECT_EQ(cpu->pb, 0x42); // Value pulled from stack
}

// PEA Tests
TEST_F(PEATest, PEA_Push_Effective_Address) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->stkp = 0x01FD; // Initial stack pointer
    
    // Write the 16-bit address operand
    bus->write(test_pc, 0xF4); // PEA opcode
    bus->write(test_pc + 1, 0x34); // Low byte
    bus->write(test_pc + 2, 0x12); // High byte
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 5);
    EXPECT_EQ(cpu->stkp, 0x01FB); // Stack pointer decremented by 2
    EXPECT_EQ(bus->read(0x0001FC), 0x12); // High byte pushed first
    EXPECT_EQ(bus->read(0x0001FB), 0x34); // Low byte pushed second
    EXPECT_EQ(cpu->pc, test_pc + 3); // PC incremented by 3
}

// PEI Tests
TEST_F(PEITest, PEI_Push_Effective_Indirect_Address) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->d = 0x0000; // Direct page register
    cpu->stkp = 0x01FD; // Initial stack pointer
    
    // Write the direct page offset
    bus->write(test_pc, 0xD4); // PEI opcode
    bus->write(test_pc + 1, 0x42); // Direct page offset
    
    // Write the 16-bit address at the direct page location
    bus->write(0x000042, 0x12); // High byte
    bus->write(0x000043, 0x34); // Low byte
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 6);
    EXPECT_EQ(cpu->stkp, 0x01FB); // Stack pointer decremented by 2
    EXPECT_EQ(bus->read(0x0001FC), 0x34); // Low byte pushed first
    EXPECT_EQ(bus->read(0x0001FB), 0x12); // High byte pushed second
    EXPECT_EQ(cpu->pc, test_pc + 2); // PC incremented by 2
}

// PER Tests
TEST_F(PERTest, PER_Push_Effective_PC_Relative_Address) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->stkp = 0x01FD; // Initial stack pointer
    
    // Write the 16-bit PC-relative offset
    bus->write(test_pc, 0x62); // PER opcode
    bus->write(test_pc + 1, 0x34); // Low byte of offset
    bus->write(test_pc + 2, 0x12); // High byte of offset
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 6);
    EXPECT_EQ(cpu->stkp, 0x01FB); // Stack pointer decremented by 2
    
    // Calculate expected target address
    uint16_t offset = 0x1234;
    uint16_t target = (test_pc + 3) + offset; // PC after instruction + offset
    
    EXPECT_EQ(bus->read(0x0001FC), (target >> 8) & 0xFF);
    EXPECT_EQ(bus->read(0x0001FB), target & 0xFF); 
    EXPECT_EQ(cpu->pc, test_pc + 3); // PC incremented by 3
}

// Additional edge case tests
TEST_F(PLATest, PLA_Zero_Result) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p |= CPU::M; // 8-bit accumulator
    cpu->a = 0x42; // Non-zero value
    cpu->stkp = 0x01FC; // Stack pointer pointing to pushed value
    bus->write(0x0001FD, 0x00); // Zero value on stack
    
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, true);
    
    bus->write(test_pc, 0x68); // PLA
    
    cpu->step();
    
    EXPECT_EQ(cpu->a, 0x00); // Should be zero
    EXPECT_TRUE(cpu->get_flag(CPU::Z)); // Should set zero
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Should clear negative
}

TEST_F(PLATest, PLA_Negative_Result) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p |= CPU::M; // 8-bit accumulator
    cpu->a = 0x42; // Non-zero value
    cpu->stkp = 0x01FC; // Stack pointer pointing to pushed value
    bus->write(0x0001FC, 0x80); // Negative value on stack (at stack address)
    
    cpu->set_flag(CPU::Z, true);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x68); // PLA
    
    cpu->step();
    
    EXPECT_EQ(cpu->a, 0x80); // Should be negative value
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Should clear zero
    EXPECT_TRUE(cpu->get_flag(CPU::N)); // Should set negative
}

TEST_F(PHPTest, PHP_Sets_B_Flag) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p = 0x0000; // Clear all flags
    cpu->stkp = 0x01FD; // Initial stack pointer
    
    bus->write(test_pc, 0x08); // PHP
    
    cpu->step();
    
    uint8_t pushed_status = bus->read(0x0001FC);
    EXPECT_TRUE(pushed_status & 0x10); // B flag should be set when pushing
}

TEST_F(PLDTest, PLD_Zero_Result) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->d = 0x1234; // Non-zero value
    cpu->stkp = 0x01FB; // Stack pointer pointing to pushed values
    bus->write(0x0001FC, 0x00); // Low byte zero
    bus->write(0x0001FB, 0x00); // High byte zero
    
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, true);
    
    bus->write(test_pc, 0x2B); // PLD
    
    cpu->step();
    
    EXPECT_EQ(cpu->d, 0x0000); // Should be zero
    EXPECT_TRUE(cpu->get_flag(CPU::Z)); // Should set zero
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Should clear negative
}