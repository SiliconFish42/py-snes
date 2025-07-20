#include <gtest/gtest.h>
#include <memory>
#include <tuple>
#include "cpu.hpp"
#include "bus.hpp"

class BranchTest : public ::testing::Test {
protected:
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }

    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
};

// BCC - Branch if Carry Clear
class BCCTest : public BranchTest {};

TEST_F(BCCTest, BCC_Taken) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->p &= ~CPU::C; // Clear carry flag
    bus->write(cpu->pc, 0x90); // BCC opcode
    bus->write(cpu->pc + 1, 0x10); // Branch offset +16
    cpu->step();
    EXPECT_EQ(cpu->cycles, 3);
    EXPECT_EQ(cpu->pc, 0x7E0012); // 0x7E0000 + 2 + 0x10
}

TEST_F(BCCTest, BCC_NotTaken) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->p |= CPU::C; // Set carry flag
    bus->write(cpu->pc, 0x90); // BCC opcode
    bus->write(cpu->pc + 1, 0x10); // Branch offset +16
    cpu->step();
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->pc, 0x7E0002); // No branch, just increment PC
}

TEST_F(BCCTest, BCC_Backward) {
    cpu->reset();
    cpu->pc = 0x7E0010;
    cpu->p &= ~CPU::C; // Clear carry flag
    bus->write(cpu->pc, 0x90); // BCC opcode
    bus->write(cpu->pc + 1, 0xF0); // Branch offset -16 (signed)
    cpu->step();
    EXPECT_EQ(cpu->cycles, 3);
    EXPECT_EQ(cpu->pc, 0x7E0002); // 0x7E0010 + 2 - 16
}

// BCS - Branch if Carry Set
class BCSTest : public BranchTest {};

TEST_F(BCSTest, BCS_Taken) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->p |= CPU::C; // Set carry flag
    bus->write(cpu->pc, 0xB0); // BCS opcode
    bus->write(cpu->pc + 1, 0x10); // Branch offset +16
    cpu->step();
    EXPECT_EQ(cpu->cycles, 3);
    EXPECT_EQ(cpu->pc, 0x7E0012);
}

TEST_F(BCSTest, BCS_NotTaken) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->p &= ~CPU::C; // Clear carry flag
    bus->write(cpu->pc, 0xB0); // BCS opcode
    bus->write(cpu->pc + 1, 0x10); // Branch offset +16
    cpu->step();
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->pc, 0x7E0002);
}

// BEQ - Branch if Equal (Zero flag set)
class BEQTest : public BranchTest {};

TEST_F(BEQTest, BEQ_Taken) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->p |= CPU::Z; // Set zero flag
    bus->write(cpu->pc, 0xF0); // BEQ opcode
    bus->write(cpu->pc + 1, 0x10); // Branch offset +16
    cpu->step();
    EXPECT_EQ(cpu->cycles, 3);
    EXPECT_EQ(cpu->pc, 0x7E0012);
}

TEST_F(BEQTest, BEQ_NotTaken) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->p &= ~CPU::Z; // Clear zero flag
    bus->write(cpu->pc, 0xF0); // BEQ opcode
    bus->write(cpu->pc + 1, 0x10); // Branch offset +16
    cpu->step();
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->pc, 0x7E0002);
}

// BMI - Branch if Minus (Negative flag set)
class BMITest : public BranchTest {};

TEST_F(BMITest, BMI_Taken) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->p |= CPU::N; // Set negative flag
    bus->write(cpu->pc, 0x30); // BMI opcode
    bus->write(cpu->pc + 1, 0x10); // Branch offset +16
    cpu->step();
    EXPECT_EQ(cpu->cycles, 3);
    EXPECT_EQ(cpu->pc, 0x7E0012);
}

TEST_F(BMITest, BMI_NotTaken) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->p &= ~CPU::N; // Clear negative flag
    bus->write(cpu->pc, 0x30); // BMI opcode
    bus->write(cpu->pc + 1, 0x10); // Branch offset +16
    cpu->step();
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->pc, 0x7E0002);
}

// BNE - Branch if Not Equal (Zero flag clear)
class BNETest : public BranchTest {};

TEST_F(BNETest, BNE_Taken) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->p &= ~CPU::Z; // Clear zero flag
    bus->write(cpu->pc, 0xD0); // BNE opcode
    bus->write(cpu->pc + 1, 0x10); // Branch offset +16
    cpu->step();
    EXPECT_EQ(cpu->cycles, 3);
    EXPECT_EQ(cpu->pc, 0x7E0012);
}

TEST_F(BNETest, BNE_NotTaken) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->p |= CPU::Z; // Set zero flag
    bus->write(cpu->pc, 0xD0); // BNE opcode
    bus->write(cpu->pc + 1, 0x10); // Branch offset +16
    cpu->step();
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->pc, 0x7E0002);
}

// BPL - Branch if Plus (Negative flag clear)
class BPLTest : public BranchTest {};

TEST_F(BPLTest, BPL_Taken) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->p &= ~CPU::N; // Clear negative flag
    bus->write(cpu->pc, 0x10); // BPL opcode
    bus->write(cpu->pc + 1, 0x10); // Branch offset +16
    cpu->step();
    EXPECT_EQ(cpu->cycles, 3);
    EXPECT_EQ(cpu->pc, 0x7E0012);
}

TEST_F(BPLTest, BPL_NotTaken) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->p |= CPU::N; // Set negative flag
    bus->write(cpu->pc, 0x10); // BPL opcode
    bus->write(cpu->pc + 1, 0x10); // Branch offset +16
    cpu->step();
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->pc, 0x7E0002);
}

// BVC - Branch if Overflow Clear
class BVCTest : public BranchTest {};

TEST_F(BVCTest, BVC_Taken) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->p &= ~CPU::V; // Clear overflow flag
    bus->write(cpu->pc, 0x50); // BVC opcode
    bus->write(cpu->pc + 1, 0x10); // Branch offset +16
    cpu->step();
    EXPECT_EQ(cpu->cycles, 3);
    EXPECT_EQ(cpu->pc, 0x7E0012);
}

TEST_F(BVCTest, BVC_NotTaken) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->p |= CPU::V; // Set overflow flag
    bus->write(cpu->pc, 0x50); // BVC opcode
    bus->write(cpu->pc + 1, 0x10); // Branch offset +16
    cpu->step();
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->pc, 0x7E0002);
}

// BVS - Branch if Overflow Set
class BVSTest : public BranchTest {};

TEST_F(BVSTest, BVS_Taken) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->p |= CPU::V; // Set overflow flag
    bus->write(cpu->pc, 0x70); // BVS opcode
    bus->write(cpu->pc + 1, 0x10); // Branch offset +16
    cpu->step();
    EXPECT_EQ(cpu->cycles, 3);
    EXPECT_EQ(cpu->pc, 0x7E0012);
}

TEST_F(BVSTest, BVS_NotTaken) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->p &= ~CPU::V; // Clear overflow flag
    bus->write(cpu->pc, 0x70); // BVS opcode
    bus->write(cpu->pc + 1, 0x10); // Branch offset +16
    cpu->step();
    EXPECT_EQ(cpu->cycles, 2);
    EXPECT_EQ(cpu->pc, 0x7E0002);
}

// BRA - Branch Always (unconditional)
class BRATest : public BranchTest {};

TEST_F(BRATest, BRA_Forward) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    bus->write(cpu->pc, 0x80); // BRA opcode
    bus->write(cpu->pc + 1, 0x10); // Branch offset +16
    cpu->step();
    EXPECT_EQ(cpu->cycles, 3);
    EXPECT_EQ(cpu->pc, 0x7E0012);
}

TEST_F(BRATest, BRA_Backward) {
    cpu->reset();
    cpu->pc = 0x7E0010;
    bus->write(cpu->pc, 0x80); // BRA opcode
    bus->write(cpu->pc + 1, 0xF0); // Branch offset -16 (signed)
    cpu->step();
    EXPECT_EQ(cpu->cycles, 3);
    EXPECT_EQ(cpu->pc, 0x7E0002);
}

// JMP - Jump (unconditional)
class JMPTest : public BranchTest {};

TEST_F(JMPTest, JMP_Absolute) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    bus->write(cpu->pc, 0x4C); // JMP absolute opcode
    bus->write(cpu->pc + 1, 0x34); // Low byte of address
    bus->write(cpu->pc + 2, 0x12); // High byte of address
    cpu->step();
    EXPECT_EQ(cpu->cycles, 3);
    EXPECT_EQ(cpu->pc, 0x1234);
}

TEST_F(JMPTest, JMP_AbsoluteLong) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    bus->write(cpu->pc, 0x5C); // JMP absolute long opcode
    bus->write(cpu->pc + 1, 0x34); // Low byte of address
    bus->write(cpu->pc + 2, 0x12); // High byte of address
    bus->write(cpu->pc + 3, 0x56); // Bank byte of address
    cpu->step();
    EXPECT_EQ(cpu->cycles, 4);
    EXPECT_EQ(cpu->pc, 0x561234);
}

TEST_F(JMPTest, JMP_Indirect) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    bus->write(cpu->pc, 0x6C); // JMP indirect opcode
    bus->write(cpu->pc + 1, 0x00); // Low byte of indirect address
    bus->write(cpu->pc + 2, 0x10); // High byte of indirect address
    bus->write(0x1000, 0x78); // Low byte of target address
    bus->write(0x1001, 0x56); // High byte of target address
    cpu->step();
    EXPECT_EQ(cpu->cycles, 5);
    EXPECT_EQ(cpu->pc, 0x5678);
}

TEST_F(JMPTest, JMP_IndirectLong) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    bus->write(cpu->pc, 0xDC); // JMP indirect long opcode
    bus->write(cpu->pc + 1, 0x00); // Low byte of indirect address
    bus->write(cpu->pc + 2, 0x10); // High byte of indirect address
    bus->write(0x1000, 0x78); // Low byte of target address
    bus->write(0x1001, 0x56); // High byte of target address
    bus->write(0x1002, 0x34); // Bank byte of target address
    cpu->step();
    EXPECT_EQ(cpu->cycles, 6);
    EXPECT_EQ(cpu->pc, 0x345678);
}

TEST_F(JMPTest, JMP_IndexedIndirect) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->x = 0x02;
    bus->write(cpu->pc, 0x7C); // JMP indexed indirect opcode
    bus->write(cpu->pc + 1, 0xFE); // Low byte of base address
    bus->write(cpu->pc + 2, 0x10); // High byte of base address
    bus->write(0x1100, 0x78); // Low byte of target address (0x10FE + 0x02)
    bus->write(0x1101, 0x56); // High byte of target address
    cpu->step();
    EXPECT_EQ(cpu->cycles, 6);
    EXPECT_EQ(cpu->pc, 0x5678);
}

// Test page boundary crossing for branches
class BranchPageBoundaryTest : public BranchTest {};

TEST_F(BranchPageBoundaryTest, BCC_SamePage) {
    cpu->reset();
    cpu->pc = 0x7E00FE; // Near end of page
    cpu->p &= ~CPU::C; // Clear carry flag
    bus->write(cpu->pc, 0x90); // BCC opcode
    bus->write(cpu->pc + 1, 0x10); // Branch offset +16
    cpu->step();
    EXPECT_EQ(cpu->cycles, 3); // No extra cycle (same page)
    EXPECT_EQ(cpu->pc, 0x7E0110); // Same page
}

TEST_F(BranchPageBoundaryTest, BRA_SamePage) {
    cpu->reset();
    cpu->pc = 0x7E00FE; // Near end of page
    bus->write(cpu->pc, 0x80); // BRA opcode
    bus->write(cpu->pc + 1, 0x10); // Branch offset +16
    cpu->step();
    EXPECT_EQ(cpu->cycles, 3); // No extra cycle (same page)
    EXPECT_EQ(cpu->pc, 0x7E0110); // Same page
}

// Test branch that crosses page boundary (extra cycle)
TEST_F(BranchPageBoundaryTest, BCC_PageCross) {
    cpu->reset();
    cpu->pc = 0x7E00F0; // Well within page
    cpu->p &= ~CPU::C; // Clear carry flag
    bus->write(cpu->pc, 0x90); // BCC opcode
    bus->write(cpu->pc + 1, 0x10); // Branch offset +16
    cpu->step();
    EXPECT_EQ(cpu->cycles, 4); // Extra cycle for page cross
    EXPECT_EQ(cpu->pc, 0x7E0102); // Crossed to next page
}
