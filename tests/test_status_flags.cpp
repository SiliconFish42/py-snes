#include <gtest/gtest.h>
#include <memory>
#include "cpu.hpp"
#include "bus.hpp"

class StatusFlagTest : public ::testing::Test {
protected:
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }

    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
};

// Test CLC (Clear Carry Flag)
TEST_F(StatusFlagTest, CLC) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->set_flag(CPU::C, true); // Set carry flag
    bus->write(cpu->pc, 0x18); // CLC opcode
    cpu->step();
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_FALSE(cpu->get_flag(CPU::C)); // Carry flag should be cleared
    EXPECT_EQ(cpu->pc, 0x7E0001);
}

// Test SEC (Set Carry Flag)
TEST_F(StatusFlagTest, SEC) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->set_flag(CPU::C, false); // Clear carry flag
    bus->write(cpu->pc, 0x38); // SEC opcode
    cpu->step();
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Carry flag should be set
    EXPECT_EQ(cpu->pc, 0x7E0001);
}

// Test CLD (Clear Decimal Flag)
TEST_F(StatusFlagTest, CLD) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->set_flag(CPU::D, true); // Set decimal flag
    bus->write(cpu->pc, 0xD8); // CLD opcode
    cpu->step();
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_FALSE(cpu->get_flag(CPU::D)); // Decimal flag should be cleared
    EXPECT_EQ(cpu->pc, 0x7E0001);
}

// Test SED (Set Decimal Flag)
TEST_F(StatusFlagTest, SED) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->set_flag(CPU::D, false); // Clear decimal flag
    bus->write(cpu->pc, 0xF8); // SED opcode
    cpu->step();
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_TRUE(cpu->get_flag(CPU::D)); // Decimal flag should be set
    EXPECT_EQ(cpu->pc, 0x7E0001);
}

// Test CLI (Clear Interrupt Disable Flag)
TEST_F(StatusFlagTest, CLI) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->set_flag(CPU::I, true); // Set interrupt disable flag
    bus->write(cpu->pc, 0x58); // CLI opcode
    cpu->step();
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_FALSE(cpu->get_flag(CPU::I)); // Interrupt disable flag should be cleared
    EXPECT_EQ(cpu->pc, 0x7E0001);
}

// Test SEI (Set Interrupt Disable Flag)
TEST_F(StatusFlagTest, SEI) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->set_flag(CPU::I, false); // Clear interrupt disable flag
    bus->write(cpu->pc, 0x78); // SEI opcode
    cpu->step();
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_TRUE(cpu->get_flag(CPU::I)); // Interrupt disable flag should be set
    EXPECT_EQ(cpu->pc, 0x7E0001);
}

// Test CLV (Clear Overflow Flag)
TEST_F(StatusFlagTest, CLV) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->set_flag(CPU::V, true); // Set overflow flag
    bus->write(cpu->pc, 0xB8); // CLV opcode
    cpu->step();
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_FALSE(cpu->get_flag(CPU::V)); // Overflow flag should be cleared
    EXPECT_EQ(cpu->pc, 0x7E0001);
}

// Test CMP Immediate - Equal values
TEST_F(StatusFlagTest, CMP_Immediate_Equal) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->a = 0x42;
    bus->write(cpu->pc, 0xC9); // CMP immediate opcode
    bus->write(cpu->pc + 1, 0x42);
    cpu->step();
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_TRUE(cpu->get_flag(CPU::Z)); // Zero flag should be set (equal)
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Carry flag should be set (A >= M)
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Negative flag should be clear
    EXPECT_EQ(cpu->pc, 0x7E0002);
}

// Test CMP Immediate - A greater than M
TEST_F(StatusFlagTest, CMP_Immediate_AGreater) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->a = 0x80;
    bus->write(cpu->pc, 0xC9); // CMP immediate opcode
    bus->write(cpu->pc + 1, 0x40);
    cpu->step();
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Zero flag should be clear (not equal)
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Carry flag should be set (A >= M)
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Negative flag should be clear (result positive)
    EXPECT_EQ(cpu->pc, 0x7E0002);
}

// Test CMP Immediate - A less than M
TEST_F(StatusFlagTest, CMP_Immediate_ALess) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->a = 0x20;
    bus->write(cpu->pc, 0xC9); // CMP immediate opcode
    bus->write(cpu->pc + 1, 0x80);
    cpu->step();
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Zero flag should be clear (not equal)
    EXPECT_FALSE(cpu->get_flag(CPU::C)); // Carry flag should be clear (A < M)
    EXPECT_TRUE(cpu->get_flag(CPU::N)); // Negative flag should be set (result negative)
    EXPECT_EQ(cpu->pc, 0x7E0002);
}

// Test CMP Zero Page
TEST_F(StatusFlagTest, CMP_ZeroPage) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->a = 0x50;
    bus->write(cpu->pc, 0xC5); // CMP zero page opcode
    bus->write(cpu->pc + 1, 0x42);
    bus->write(0x0042, 0x30);
    cpu->step();
    EXPECT_EQ(cpu->cycles, 3);
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Zero flag should be clear
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Carry flag should be set (A >= M)
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Negative flag should be clear
    EXPECT_EQ(cpu->pc, 0x7E0002);
}

// Test CMP Absolute
TEST_F(StatusFlagTest, CMP_Absolute) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->a = 0xFF;
    bus->write(cpu->pc, 0xCD); // CMP absolute opcode
    bus->write(cpu->pc + 1, 0x34);
    bus->write(cpu->pc + 2, 0x12);
    bus->write(0x1234, 0xFE);
    cpu->step();
    EXPECT_EQ(cpu->cycles, 4);
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Zero flag should be clear
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Carry flag should be set (A >= M)
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Negative flag should be clear
    EXPECT_EQ(cpu->pc, 0x7E0003);
}

// Test CMP Absolute Long
TEST_F(StatusFlagTest, CMP_AbsoluteLong) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->a = 0x00;
    bus->write(cpu->pc, 0xCF); // CMP absolute long opcode
    bus->write(cpu->pc + 1, 0x34);
    bus->write(cpu->pc + 2, 0x12);
    bus->write(cpu->pc + 3, 0x00);
    bus->write(0x001234, 0x01);
    cpu->step();
    EXPECT_EQ(cpu->cycles, 5);
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Zero flag should be clear
    EXPECT_FALSE(cpu->get_flag(CPU::C)); // Carry flag should be clear (A < M)
    EXPECT_TRUE(cpu->get_flag(CPU::N)); // Negative flag should be set
    EXPECT_EQ(cpu->pc, 0x7E0004);
}

// Test CMP Indirect X
TEST_F(StatusFlagTest, CMP_IndirectX) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->a = 0xAA;
    cpu->x = 0x02;
    bus->write(cpu->pc, 0xC1); // CMP indirect X opcode
    bus->write(cpu->pc + 1, 0x40);
    bus->write(0x0042, 0x34); // Low byte of address
    bus->write(0x0043, 0x12); // High byte of address
    bus->write(0x1234, 0xAA);
    cpu->step();
    EXPECT_EQ(cpu->cycles, 6);
    EXPECT_TRUE(cpu->get_flag(CPU::Z)); // Zero flag should be set (equal)
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Carry flag should be set (A >= M)
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Negative flag should be clear
    EXPECT_EQ(cpu->pc, 0x7E0002);
}

// Test CMP Indirect Y
TEST_F(StatusFlagTest, CMP_IndirectY) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->a = 0x55;
    cpu->y = 0x01;
    bus->write(cpu->pc, 0xD1); // CMP indirect Y opcode
    bus->write(cpu->pc + 1, 0x40);
    bus->write(0x0040, 0x33); // Low byte of address
    bus->write(0x0041, 0x12); // High byte of address
    bus->write(0x1234, 0x54);
    cpu->step();
    EXPECT_EQ(cpu->cycles, 6);
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Zero flag should be clear
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Carry flag should be set (A >= M)
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Negative flag should be clear
    EXPECT_EQ(cpu->pc, 0x7E0002);
}

// Test CMP Zero Page X
TEST_F(StatusFlagTest, CMP_ZeroPageX) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->a = 0x80;
    cpu->x = 0x03;
    bus->write(cpu->pc, 0xD5); // CMP zero page X opcode
    bus->write(cpu->pc + 1, 0x40);
    bus->write(0x0043, 0x7F);
    cpu->step();
    EXPECT_EQ(cpu->cycles, 4);
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Zero flag should be clear
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Carry flag should be set (A >= M)
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Negative flag should be clear
    EXPECT_EQ(cpu->pc, 0x7E0002);
}

// Test CMP Absolute Y
TEST_F(StatusFlagTest, CMP_AbsoluteY) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->a = 0x00;
    cpu->y = 0x02;
    bus->write(cpu->pc, 0xD9); // CMP absolute Y opcode
    bus->write(cpu->pc + 1, 0x32);
    bus->write(cpu->pc + 2, 0x12);
    bus->write(0x1234, 0x00);
    cpu->step();
    EXPECT_EQ(cpu->cycles, 5);
    EXPECT_TRUE(cpu->get_flag(CPU::Z)); // Zero flag should be set (equal)
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Carry flag should be set (A >= M)
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Negative flag should be clear
    EXPECT_EQ(cpu->pc, 0x7E0003);
}

// Test CMP Absolute X
TEST_F(StatusFlagTest, CMP_AbsoluteX) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->a = 0xFF;
    cpu->x = 0x01;
    bus->write(cpu->pc, 0xDD); // CMP absolute X opcode
    bus->write(cpu->pc + 1, 0x33);
    bus->write(cpu->pc + 2, 0x12);
    bus->write(0x1234, 0xFE);
    cpu->step();
    EXPECT_EQ(cpu->cycles, 5);
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Zero flag should be clear
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Carry flag should be set (A >= M)
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Negative flag should be clear
    EXPECT_EQ(cpu->pc, 0x7E0003);
}

// Test CMP Absolute Long X
TEST_F(StatusFlagTest, CMP_AbsoluteLongX) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->a = 0x10;
    cpu->x = 0x01;
    bus->write(cpu->pc, 0xDF); // CMP absolute long X opcode
    bus->write(cpu->pc + 1, 0x33);
    bus->write(cpu->pc + 2, 0x12);
    bus->write(cpu->pc + 3, 0x00);
    bus->write(0x001234, 0x0F);
    cpu->step();
    EXPECT_EQ(cpu->cycles, 5);
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Zero flag should be clear
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Carry flag should be set (A >= M)
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Negative flag should be clear
    EXPECT_EQ(cpu->pc, 0x7E0004);
}

// Test CMP Stack Relative
TEST_F(StatusFlagTest, CMP_StackRelative) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->a = 0x42;
    cpu->stkp = 0x01FD;
    bus->write(cpu->pc, 0xC3); // CMP stack relative opcode
    bus->write(cpu->pc + 1, 0x02);
    bus->write(0x0001FF, 0x42);
    cpu->step();
    EXPECT_EQ(cpu->cycles, 4);
    EXPECT_TRUE(cpu->get_flag(CPU::Z)); // Zero flag should be set (equal)
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Carry flag should be set (A >= M)
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Negative flag should be clear
    EXPECT_EQ(cpu->pc, 0x7E0002);
}

// Test CMP Stack Relative Indirect Y
TEST_F(StatusFlagTest, CMP_StackRelativeIndirectY) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->a = 0x55;
    cpu->y = 0x01;
    cpu->stkp = 0x01FD;
    bus->write(cpu->pc, 0xD3); // CMP stack relative indirect Y opcode
    bus->write(cpu->pc + 1, 0x02);
    bus->write(0x0001FF, 0x33); // Low byte of address
    bus->write(0x000200, 0x12); // High byte of address
    bus->write(0x1234, 0x54);
    cpu->step();
    EXPECT_EQ(cpu->cycles, 7);
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Zero flag should be clear
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Carry flag should be set (A >= M)
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Negative flag should be clear
    EXPECT_EQ(cpu->pc, 0x7E0002);
}

// Test multiple flag operations in sequence
TEST_F(StatusFlagTest, MultipleFlagOperations) {
    cpu->reset();
    cpu->pc = 0x7E0000;

    // Set all flags initially
    cpu->set_flag(CPU::C, true);
    cpu->set_flag(CPU::D, true);
    cpu->set_flag(CPU::I, true);
    cpu->set_flag(CPU::V, true);

    // Clear all flags
    bus->write(cpu->pc, 0x18); // CLC
    bus->write(cpu->pc + 1, 0xD8); // CLD
    bus->write(cpu->pc + 2, 0x58); // CLI
    bus->write(cpu->pc + 3, 0xB8); // CLV

    cpu->step(); // CLC
    EXPECT_FALSE(cpu->get_flag(CPU::C));
    EXPECT_EQ(cpu->cycles, 2);

    cpu->step(); // CLD
    EXPECT_FALSE(cpu->get_flag(CPU::D));
    EXPECT_EQ(cpu->cycles, 2);

    cpu->step(); // CLI
    EXPECT_FALSE(cpu->get_flag(CPU::I));
    EXPECT_EQ(cpu->cycles, 2);

    cpu->step(); // CLV
    EXPECT_FALSE(cpu->get_flag(CPU::V));
    EXPECT_EQ(cpu->cycles, 2);

    EXPECT_EQ(cpu->pc, 0x7E0004);
}

// Test flag operations with accumulator operations
TEST_F(StatusFlagTest, FlagOperationsWithAccumulator) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->a = 0x80;

    // Set carry flag
    bus->write(cpu->pc, 0x38); // SEC
    bus->write(cpu->pc + 1, 0xC9); // CMP immediate
    bus->write(cpu->pc + 2, 0x80);

    cpu->step(); // SEC
    EXPECT_TRUE(cpu->get_flag(CPU::C));
    EXPECT_EQ(cpu->cycles, 2);

    cpu->step(); // CMP
    EXPECT_TRUE(cpu->get_flag(CPU::Z)); // Equal
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // A >= M
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Result not negative
    EXPECT_EQ(cpu->cycles, 2);

    EXPECT_EQ(cpu->pc, 0x7E0003);
}

// Test edge cases for CMP
TEST_F(StatusFlagTest, CMP_EdgeCases) {
    cpu->reset();
    cpu->pc = 0x7E0000;

    // Test with zero values
    cpu->a = 0x00;
    bus->write(cpu->pc, 0xC9); // CMP immediate
    bus->write(cpu->pc + 1, 0x00);
    cpu->step();
    EXPECT_TRUE(cpu->get_flag(CPU::Z));
    EXPECT_TRUE(cpu->get_flag(CPU::C));
    EXPECT_FALSE(cpu->get_flag(CPU::N));
    EXPECT_EQ(cpu->cycles, 2);

    // Test with maximum values
    cpu->pc = 0x7E0000;
    cpu->a = 0xFF;
    bus->write(cpu->pc, 0xC9); // CMP immediate
    bus->write(cpu->pc + 1, 0xFF);
    cpu->step();
    EXPECT_TRUE(cpu->get_flag(CPU::Z));
    EXPECT_TRUE(cpu->get_flag(CPU::C));
    EXPECT_FALSE(cpu->get_flag(CPU::N));
    EXPECT_EQ(cpu->cycles, 2);

    // Test with negative comparison
    cpu->pc = 0x7E0000;
    cpu->a = 0x00;
    bus->write(cpu->pc, 0xC9); // CMP immediate
    bus->write(cpu->pc + 1, 0x80);
    cpu->step();
    EXPECT_FALSE(cpu->get_flag(CPU::Z));
    EXPECT_FALSE(cpu->get_flag(CPU::C));
    EXPECT_TRUE(cpu->get_flag(CPU::N));
    EXPECT_EQ(cpu->cycles, 2);
}
