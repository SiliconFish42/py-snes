#include <cstdint>
#include <string>
#include "gtest/gtest.h"
#include "cpu.hpp"
#include "bus.hpp"
#include <tuple>

// Test structure for transfer instructions
struct TransferParams {
    uint8_t opcode;
    uint8_t expected_cycles_8;
    uint8_t expected_cycles_16;
    std::string instruction;
    std::string description;
};

// Base Transfer Test Class
class TransferTestBase : public ::testing::Test {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

// Individual Transfer Test Classes
class TAXTest : public TransferTestBase {};
class TAYTest : public TransferTestBase {};
class TXATest : public TransferTestBase {};
class TYATest : public TransferTestBase {};
class TXYTest : public TransferTestBase {};
class TYXTest : public TransferTestBase {};
class TCDTest : public TransferTestBase {};
class TDCTest : public TransferTestBase {};
class TCSTest : public TransferTestBase {};
class TSCTest : public TransferTestBase {};
class XBATest : public TransferTestBase {};

// TAX Tests
TEST_F(TAXTest, TAX_8Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p |= CPU::M; // 8-bit accumulator
    cpu->p |= CPU::X; // 8-bit index registers
    cpu->a = 0x42;
    cpu->x = 0x84;
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0xAA); // TAX
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->x, 0x42);
    EXPECT_FALSE(cpu->get_flag(CPU::Z));
    EXPECT_FALSE(cpu->get_flag(CPU::N));
}

TEST_F(TAXTest, TAX_16Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p &= ~CPU::M; // 16-bit accumulator
    cpu->p &= ~CPU::X; // 16-bit index registers
    cpu->a = 0x1234;
    cpu->x = 0x5678;
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0xAA); // TAX
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->x, 0x1234);
    EXPECT_FALSE(cpu->get_flag(CPU::Z));
    EXPECT_FALSE(cpu->get_flag(CPU::N));
}

// TAY Tests
TEST_F(TAYTest, TAY_8Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p |= CPU::M; // 8-bit accumulator
    cpu->p |= CPU::X; // 8-bit index registers
    cpu->a = 0x42;
    cpu->y = 0x84;
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0xA8); // TAY
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->y, 0x42);
    EXPECT_FALSE(cpu->get_flag(CPU::Z));
    EXPECT_FALSE(cpu->get_flag(CPU::N));
}

TEST_F(TAYTest, TAY_16Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p &= ~CPU::M; // 16-bit accumulator
    cpu->p &= ~CPU::X; // 16-bit index registers
    cpu->a = 0x1234;
    cpu->y = 0x5678;
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0xA8); // TAY
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->y, 0x1234);
    EXPECT_FALSE(cpu->get_flag(CPU::Z));
    EXPECT_FALSE(cpu->get_flag(CPU::N));
}

// TXA Tests
TEST_F(TXATest, TXA_8Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p |= CPU::M; // 8-bit accumulator
    cpu->p |= CPU::X; // 8-bit index registers
    cpu->a = 0x42;
    cpu->x = 0x84;
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x8A); // TXA
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->a, 0x84);
    EXPECT_FALSE(cpu->get_flag(CPU::Z));
    EXPECT_TRUE(cpu->get_flag(CPU::N)); // 0x84 is negative
}

TEST_F(TXATest, TXA_16Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p &= ~CPU::M; // 16-bit accumulator
    cpu->p &= ~CPU::X; // 16-bit index registers
    cpu->a = 0x1234;
    cpu->x = 0x5678;
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x8A); // TXA
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->a, 0x5678);
    EXPECT_FALSE(cpu->get_flag(CPU::Z));
    EXPECT_FALSE(cpu->get_flag(CPU::N));
}

// TYA Tests
TEST_F(TYATest, TYA_8Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p |= CPU::M; // 8-bit accumulator
    cpu->p |= CPU::X; // 8-bit index registers
    cpu->a = 0x42;
    cpu->y = 0x84;
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x98); // TYA
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->a, 0x84);
    EXPECT_FALSE(cpu->get_flag(CPU::Z));
    EXPECT_TRUE(cpu->get_flag(CPU::N)); // 0x84 is negative
}

TEST_F(TYATest, TYA_16Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p &= ~CPU::M; // 16-bit accumulator
    cpu->p &= ~CPU::X; // 16-bit index registers
    cpu->a = 0x1234;
    cpu->y = 0x5678;
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x98); // TYA
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->a, 0x5678);
    EXPECT_FALSE(cpu->get_flag(CPU::Z));
    EXPECT_FALSE(cpu->get_flag(CPU::N));
}

// TXY Tests
TEST_F(TXYTest, TXY_8Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p |= CPU::M; // 8-bit accumulator
    cpu->p |= CPU::X; // 8-bit index registers
    cpu->x = 0x42;
    cpu->y = 0x84;
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x9B); // TXY
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->y, 0x42);
    EXPECT_FALSE(cpu->get_flag(CPU::Z));
    EXPECT_FALSE(cpu->get_flag(CPU::N));
}

TEST_F(TXYTest, TXY_16Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p &= ~CPU::M; // 16-bit accumulator
    cpu->p &= ~CPU::X; // 16-bit index registers
    cpu->x = 0x1234;
    cpu->y = 0x5678;
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x9B); // TXY
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->y, 0x1234);
    EXPECT_FALSE(cpu->get_flag(CPU::Z));
    EXPECT_FALSE(cpu->get_flag(CPU::N));
}

// TYX Tests
TEST_F(TYXTest, TYX_8Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p |= CPU::M; // 8-bit accumulator
    cpu->p |= CPU::X; // 8-bit index registers
    cpu->x = 0x42;
    cpu->y = 0x84;
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0xBB); // TYX
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->x, 0x84);
    EXPECT_FALSE(cpu->get_flag(CPU::Z));
    EXPECT_TRUE(cpu->get_flag(CPU::N)); // 0x84 is negative
}

TEST_F(TYXTest, TYX_16Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p &= ~CPU::M; // 16-bit accumulator
    cpu->p &= ~CPU::X; // 16-bit index registers
    cpu->x = 0x1234;
    cpu->y = 0x5678;
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0xBB); // TYX
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->x, 0x5678);
    EXPECT_FALSE(cpu->get_flag(CPU::Z));
    EXPECT_FALSE(cpu->get_flag(CPU::N));
}

// TCD Tests
TEST_F(TCDTest, TCD_8Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p |= CPU::M; // 8-bit accumulator
    cpu->a = 0x42;
    cpu->d = 0x84;
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x5B); // TCD
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->d, 0x42);
    // TCD doesn't affect flags
    EXPECT_FALSE(cpu->get_flag(CPU::Z));
    EXPECT_FALSE(cpu->get_flag(CPU::N));
}

TEST_F(TCDTest, TCD_16Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p &= ~CPU::M; // 16-bit accumulator
    cpu->a = 0x1234;
    cpu->d = 0x5678;
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x5B); // TCD
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->d, 0x1234);
    // TCD doesn't affect flags
    EXPECT_FALSE(cpu->get_flag(CPU::Z));
    EXPECT_FALSE(cpu->get_flag(CPU::N));
}

// TDC Tests
TEST_F(TDCTest, TDC_8Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p |= CPU::M; // 8-bit accumulator
    cpu->a = 0x42;
    cpu->d = 0x84;
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x7B); // TDC
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->a, 0x84);
    EXPECT_FALSE(cpu->get_flag(CPU::Z));
    EXPECT_TRUE(cpu->get_flag(CPU::N)); // 0x84 is negative
}

TEST_F(TDCTest, TDC_16Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p &= ~CPU::M; // 16-bit accumulator
    cpu->a = 0x1234;
    cpu->d = 0x5678;
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x7B); // TDC
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->a, 0x5678);
    EXPECT_FALSE(cpu->get_flag(CPU::Z));
    EXPECT_FALSE(cpu->get_flag(CPU::N));
}

// TCS Tests
TEST_F(TCSTest, TCS_8Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p |= CPU::X; // 8-bit index registers
    cpu->a = 0x42;
    cpu->stkp = 0x84;
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x1B); // TCS
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->stkp, 0x42);
    // TCS doesn't affect flags
    EXPECT_FALSE(cpu->get_flag(CPU::Z));
    EXPECT_FALSE(cpu->get_flag(CPU::N));
}

TEST_F(TCSTest, TCS_16Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p &= ~CPU::X; // 16-bit index registers
    cpu->a = 0x1234;
    cpu->stkp = 0x5678;
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x1B); // TCS
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->stkp, 0x1234);
    // TCS doesn't affect flags
    EXPECT_FALSE(cpu->get_flag(CPU::Z));
    EXPECT_FALSE(cpu->get_flag(CPU::N));
}

// TSC Tests
TEST_F(TSCTest, TSC_8Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p |= CPU::M; // 8-bit accumulator
    cpu->a = 0x42;
    cpu->stkp = 0x84;
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x3B); // TSC
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->a, 0x84);
    EXPECT_FALSE(cpu->get_flag(CPU::Z));
    EXPECT_TRUE(cpu->get_flag(CPU::N)); // 0x84 is negative
}

TEST_F(TSCTest, TSC_16Bit) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p &= ~CPU::M; // 16-bit accumulator
    cpu->a = 0x1234;
    cpu->stkp = 0x5678;
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x3B); // TSC
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->a, 0x5678);
    EXPECT_FALSE(cpu->get_flag(CPU::Z));
    EXPECT_FALSE(cpu->get_flag(CPU::N));
}

// XBA Tests
TEST_F(XBATest, XBA_Exchange) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->a = 0x1234; // High byte = 0x12, Low byte = 0x34
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0xEB); // XBA
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 3);
    EXPECT_EQ(cpu->a, 0x3412); // Should exchange bytes
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Low byte = 0x12
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Low byte = 0x12
}

// Additional edge case tests
TEST_F(TAXTest, TAX_Zero_Result) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p |= CPU::M; // 8-bit accumulator
    cpu->p |= CPU::X; // 8-bit index registers
    cpu->a = 0x00; // Zero
    cpu->x = 0x42; // Non-zero
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, true);
    
    bus->write(test_pc, 0xAA); // TAX
    
    cpu->step();
    
    EXPECT_EQ(cpu->x, 0x00); // Should be zero
    EXPECT_TRUE(cpu->get_flag(CPU::Z)); // Should set zero
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Should clear negative
}

TEST_F(TYATest, TYA_Negative_Result) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->p |= CPU::M; // 8-bit accumulator
    cpu->p |= CPU::X; // 8-bit index registers
    cpu->a = 0x42; // Non-zero
    cpu->y = 0x80; // Negative value
    cpu->set_flag(CPU::Z, true);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x98); // TYA
    
    cpu->step();
    
    EXPECT_EQ(cpu->a, 0x80); // Should be negative value
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Should clear zero
    EXPECT_TRUE(cpu->get_flag(CPU::N)); // Should set negative
}

TEST_F(XBATest, XBA_Zero_Low_Byte) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    cpu->a = 0x0012; // High byte = 0x00, Low byte = 0x12
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0xEB); // XBA
    
    cpu->step();
    
    EXPECT_EQ(cpu->cycles, 3);
    EXPECT_EQ(cpu->a, 0x1200); // Should exchange bytes
    EXPECT_TRUE(cpu->get_flag(CPU::Z)); // Low byte = 0x00
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Low byte = 0x00
} 