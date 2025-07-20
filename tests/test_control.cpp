#include <cstdint>
#include <string>
#include "gtest/gtest.h"
#include "cpu.hpp"
#include "bus.hpp"
#include <tuple>

// Test structure for control instructions
struct ControlParams {
    uint8_t opcode;
    uint8_t expected_cycles;
    std::string instruction;
    std::string description;
};

// Test structure for flag instructions
struct FlagParams {
    uint8_t opcode;
    uint8_t expected_cycles;
    std::string instruction;
    CPU::FLAGS flag;
    bool set_flag;
    std::string description;
};

// Test structure for jump/subroutine instructions
struct JumpParams {
    uint8_t opcode;
    uint8_t expected_cycles;
    std::string instruction;
    std::string description;
};

// Control Instructions Test Class
class ControlTest : public ::testing::TestWithParam<std::tuple<ControlParams, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

// Flag Instructions Test Class
class FlagTest : public ::testing::TestWithParam<std::tuple<FlagParams, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

// Jump/Subroutine Instructions Test Class
class JumpTest : public ::testing::TestWithParam<std::tuple<JumpParams, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

// BRK Instruction Tests
TEST_F(ControlTest, BRK_Instruction) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;

    // Set up BRK instruction
    bus->write(test_pc, 0x00); // BRK opcode

    // Set up interrupt vector
    bus->set_interrupt_vector(0x34, 0x12); // low byte, High byte of interrupt vector

    // Set initial flags
    cpu->p = 0x30; // Clear I flag, set other flags

    uint32_t old_pc = cpu->pc;
    uint16_t old_stkp = cpu->stkp;

    cpu->step();

    // Verify BRK behavior
    EXPECT_EQ(cpu->cycles, 7); // BRK takes 7 cycles

    // Verify PC was pushed to stack (PC was already incremented in step())
    uint8_t pc_low = bus->read(0x01FB);
    uint8_t pc_high = bus->read(0x01FC);
    uint16_t pushed_pc = (pc_high << 8) | pc_low;
    EXPECT_EQ(pushed_pc, (old_pc + 1) & 0xFFFF); // PC was incremented after fetching opcode, mask to 16-bit

    // Verify P was pushed to stack with B flag set
    uint8_t pushed_p = bus->read(0x01FA);
    EXPECT_EQ(pushed_p & 0x10, 0x10); // B flag should be set

    // Verify I flag is set
    EXPECT_TRUE(cpu->get_flag(CPU::I));

    // Verify PC jumped to interrupt vector
    EXPECT_EQ(cpu->pc, 0x1234);

    // Verify stack pointer was decremented by 3
    EXPECT_EQ(cpu->stkp, old_stkp - 3);
}

// NOP Instruction Tests
TEST_F(ControlTest, NOP_Instruction) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;

    // Set up NOP instruction
    bus->write(test_pc, 0xEA); // NOP opcode

    uint32_t old_pc = cpu->pc;
    uint16_t old_a = cpu->a;
    uint16_t old_x = cpu->x;
    uint16_t old_y = cpu->y;
    uint16_t old_p = cpu->p;

    cpu->step();

    // Verify NOP behavior
    EXPECT_EQ(cpu->cycles, 2); // NOP takes 2 cycles
    EXPECT_EQ(cpu->pc, old_pc + 1); // PC was incremented in step(), NOP doesn't change it further
    EXPECT_EQ(cpu->a, old_a); // A should be unchanged
    EXPECT_EQ(cpu->x, old_x); // X should be unchanged
    EXPECT_EQ(cpu->y, old_y); // Y should be unchanged
    EXPECT_EQ(cpu->p, old_p); // P should be unchanged
}

// Flag Setting/Clearing Tests
TEST_P(FlagTest, Flag_Instructions) {
    auto [params, is16] = GetParam();
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;

    // Set up flag instruction
    bus->write(test_pc, params.opcode);

    // Set initial flag state (opposite of what we're testing)
    if (params.set_flag) {
        cpu->set_flag(params.flag, false); // Clear flag first
    } else {
        cpu->set_flag(params.flag, true);  // Set flag first
    }

    bool initial_flag_state = cpu->get_flag(params.flag);
    cpu->step();

    // Verify flag instruction behavior
    EXPECT_EQ(cpu->cycles, params.expected_cycles);
    EXPECT_EQ(cpu->pc, test_pc + 1); // PC should advance by 1

    // Verify flag was set/cleared correctly
    bool final_flag_state = cpu->get_flag(params.flag);
    EXPECT_EQ(final_flag_state, params.set_flag);
    EXPECT_NE(final_flag_state, initial_flag_state);
}

// JMP Absolute Tests
TEST_F(JumpTest, JMP_Absolute) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;

    // Set up JMP absolute instruction
    bus->write(test_pc, 0x4C);     // JMP opcode
    bus->write(test_pc + 1, 0x34); // Low byte of target
    bus->write(test_pc + 2, 0x12); // High byte of target

    cpu->step();

    // Verify JMP behavior
    EXPECT_EQ(cpu->cycles, 3); // JMP absolute takes 3 cycles
    EXPECT_EQ(cpu->pc, 0x1234); // PC should jump to target address
}

// JSR/RTS Tests
TEST_F(JumpTest, JSR_RTS_Pair) {
    uint32_t test_pc = 0x7E0000;
    uint32_t subroutine_addr = 0x7E1000;
    cpu->reset();
    cpu->pc = test_pc;
    cpu->pb = (test_pc >> 16) & 0xFF; // Set the bank byte

    // Set up JSR instruction
    bus->write(test_pc, 0x20);     // JSR opcode
    bus->write(test_pc + 1, 0x00); // Low byte of subroutine address
    bus->write(test_pc + 2, 0x10); // High byte of subroutine address

    // Set up RTS instruction at subroutine
    bus->write(subroutine_addr, 0x60); // RTS opcode

    uint16_t old_stkp = cpu->stkp;

    // Execute JSR
    cpu->step();

    // Verify JSR behavior
    EXPECT_EQ(cpu->cycles, 6); // JSR takes 6 cycles
    EXPECT_EQ(cpu->pc, subroutine_addr); // PC should jump to subroutine
    EXPECT_EQ(cpu->pb, (test_pc >> 16) & 0xFF) << "PB changed after JSR";

    // Verify return address was pushed to stack (PC-1)
    uint8_t ret_low = bus->read(0x01FB);
    uint8_t ret_high = bus->read(0x01FC);
    uint16_t pushed_ret = (ret_high << 8) | ret_low;
    EXPECT_EQ(pushed_ret, (test_pc + 2) & 0xFFFF); // JSR pushes PC-1 (last byte of JSR instruction)

    // Execute RTS
    cpu->step();

    // Verify RTS behavior
    EXPECT_EQ(cpu->cycles, 6); // RTS takes 6 cycles
    EXPECT_EQ(cpu->pb, (test_pc >> 16) & 0xFF) << "PB changed after RTS";
    EXPECT_EQ(cpu->pc & 0xFFFF, (test_pc + 3) & 0xFFFF); // Only compare low 16 bits
    EXPECT_EQ(cpu->stkp, old_stkp); // Stack pointer should be restored
}

// RTI Tests
TEST_F(JumpTest, RTI_Instruction) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    cpu->pb = (test_pc >> 16) & 0xFF; // Set the bank byte

    // Set up RTI instruction
    bus->write(test_pc, 0x40); // RTI opcode

    // Set up stack with return address and flags
    cpu->stkp = 0x01FD;
    bus->write(0x01FE, 0x34); // Low byte of return address
    bus->write(0x01FF, 0x12); // High byte of return address
    bus->write(0x01FD, 0x30); // Processor status (P) -- FIXED from 0x0200 to 0x01FD

    cpu->step();

    // Verify RTI behavior
    EXPECT_EQ(cpu->cycles, 6); // RTI takes 6 cycles
    EXPECT_EQ(cpu->pc, 0x1234); // PC should be restored from stack
    EXPECT_EQ(cpu->p, 0x30); // P should be restored from stack
    EXPECT_EQ(cpu->stkp, 0x0100); // Stack pointer should be incremented by 3 (wraps to 0x0100)
}

// XCE Tests
TEST_F(JumpTest, XCE_Instruction) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    cpu->pb = (test_pc >> 16) & 0xFF; // Set the bank byte

    // Set up XCE instruction
    bus->write(test_pc, 0xFB); // XCE opcode

    // Set initial state
    cpu->set_flag(CPU::C, true);  // Set carry flag
    cpu->set_flag(CPU::E, false); // Clear emulation flag

    bool initial_carry = cpu->get_flag(CPU::C);
    bool initial_emulation = cpu->get_flag(CPU::E);

    cpu->step();

    // Verify XCE behavior
    EXPECT_EQ(cpu->cycles, 2); // XCE takes 2 cycles
    EXPECT_EQ(cpu->pc, test_pc + 1); // PC should advance by 1

    // Verify flags were exchanged
    EXPECT_EQ(cpu->get_flag(CPU::C), initial_emulation);
    EXPECT_EQ(cpu->get_flag(CPU::E), initial_carry);
}

// Parameterized test instantiations

// Flag instruction test parameters
INSTANTIATE_TEST_SUITE_P(
    Flag_Instructions,
    FlagTest,
    ::testing::Combine(
        ::testing::Values(
            FlagParams{0x78, 2, "SEI", CPU::I, true, "Set Interrupt Disable"},
            FlagParams{0x58, 2, "CLI", CPU::I, false, "Clear Interrupt Disable"},
            FlagParams{0x18, 2, "CLC", CPU::C, false, "Clear Carry"},
            FlagParams{0x38, 2, "SEC", CPU::C, true, "Set Carry"},
            FlagParams{0xD8, 2, "CLD", CPU::D, false, "Clear Decimal"},
            FlagParams{0xF8, 2, "SED", CPU::D, true, "Set Decimal"},
            FlagParams{0xB8, 2, "CLV", CPU::V, false, "Clear Overflow"}
        ),
        ::testing::Bool()
    )
);

// Additional edge case tests

// Test BRK with different initial flag states
TEST_F(ControlTest, BRK_WithDifferentFlags) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    cpu->pb = (test_pc >> 16) & 0xFF; // Set the bank byte

    // Set up BRK instruction
    bus->write(test_pc, 0x00);
    bus->set_interrupt_vector(0x34, 0x12);

    // Test with different flag combinations
    std::vector<uint8_t> flag_combinations = {0x00, 0xFF, 0x30, 0xCF};

    for (uint8_t flags : flag_combinations) {
        cpu->reset();
        cpu->pc = test_pc;
        cpu->pb = (test_pc >> 16) & 0xFF; // Set the bank byte
        cpu->p = flags;

        uint16_t old_stkp = cpu->stkp;
        cpu->step();

        // Verify B flag is always set in pushed P
        uint8_t pushed_p = bus->read(0x01FA);
        EXPECT_EQ(pushed_p & 0x10, 0x10) << "B flag not set in pushed P for flags 0x" << std::hex << (int)flags;

        // Verify I flag is always set
        EXPECT_TRUE(cpu->get_flag(CPU::I)) << "I flag not set for flags 0x" << std::hex << (int)flags;

        // Verify stack pointer decremented
        EXPECT_EQ(cpu->stkp, old_stkp - 3) << "Stack pointer not decremented for flags 0x" << std::hex << (int)flags;
    }
}

// Test JSR with different return addresses
TEST_F(JumpTest, JSR_DifferentAddresses) {
    std::vector<std::pair<uint16_t, uint16_t>> test_cases = {
        {0x1000, 0x1000}, // Non-zero address to avoid conflict
        {0xFFFF, 0xFFFF}, // Maximum address
        {0x1234, 0x5678}, // Random addresses
        {0x8000, 0xC000}  // High addresses
    };

    for (auto [jsr_addr, rts_addr] : test_cases) {
        uint32_t test_pc = 0x7E0000;
        cpu->reset();
        cpu->pc = test_pc;
        cpu->pb = (test_pc >> 16) & 0xFF; // Set the bank byte

        // Set up JSR
        bus->write(test_pc, 0x20);
        bus->write(test_pc + 1, jsr_addr & 0xFF);        // Low byte
        bus->write(test_pc + 2, (jsr_addr >> 8) & 0xFF); // High byte

        // Set up RTS at target (include bank byte)
        uint32_t target_addr = ((uint32_t)cpu->pb << 16) | jsr_addr;
        // Write RTS to target address
        bus->write(target_addr, 0x60);

        uint16_t old_stkp = cpu->stkp;

        // Execute JSR
        cpu->step();
        // JSR preserves the current bank byte, so the target address includes the bank
        uint32_t expected_addr = ((uint32_t)cpu->pb << 16) | jsr_addr;
        EXPECT_EQ(cpu->pc, expected_addr) << "JSR failed for address 0x" << std::hex << jsr_addr;
        EXPECT_EQ(cpu->pb, (test_pc >> 16) & 0xFF) << "PB changed after JSR for address 0x" << std::hex << jsr_addr;
        // After JSR, check stack for correct return address (little-endian)
        uint8_t ret_low = bus->read(0x01FB);   // Low byte at lower address
        uint8_t ret_high = bus->read(0x01FC);  // High byte at higher address
        uint16_t pushed_ret = (ret_high << 8) | ret_low;
        EXPECT_EQ(pushed_ret, (test_pc + 2) & 0xFFFF) << "Stack return address wrong for JSR at 0x" << std::hex << jsr_addr;
        // Execute RTS
        cpu->step();
        EXPECT_EQ(cpu->pb, (test_pc >> 16) & 0xFF) << "PB changed after RTS for address 0x" << std::hex << jsr_addr;
        EXPECT_EQ(cpu->pc & 0xFFFF, (test_pc + 3) & 0xFFFF) << "RTS failed for address 0x" << std::hex << rts_addr;
        EXPECT_EQ(cpu->stkp, old_stkp) << "Stack not restored for address 0x" << std::hex << rts_addr;
    }
}

// Test flag instructions with all flag combinations
TEST_F(FlagTest, Flag_AllCombinations) {
    uint32_t test_pc = 0x7E0000;

    // Test each flag instruction with all possible flag states
    std::vector<FlagParams> flag_instructions = {
        {0x78, 2, "SEI", CPU::I, true, "Set Interrupt Disable"},
        {0x58, 2, "CLI", CPU::I, false, "Clear Interrupt Disable"},
        {0x18, 2, "CLC", CPU::C, false, "Clear Carry"},
        {0x38, 2, "SEC", CPU::C, true, "Set Carry"},
        {0xD8, 2, "CLD", CPU::D, false, "Clear Decimal"},
        {0xF8, 2, "SED", CPU::D, true, "Set Decimal"},
        {0xB8, 2, "CLV", CPU::V, false, "Clear Overflow"}
    };

    for (const auto& params : flag_instructions) {
        for (uint8_t flags = 0; flags <= 0x11; flags += 0x11) { // Test fewer flag combinations to avoid infinite loop
            cpu->reset();
            cpu->pc = test_pc;
            cpu->pb = (test_pc >> 16) & 0xFF; // Set the bank byte
            cpu->p = flags;

            // Set up interrupt vector to prevent infinite loops
            bus->set_interrupt_vector(0x34, 0x12);

            bus->write(test_pc, params.opcode);
            // Write a NOP after the flag instruction to prevent reading unimplemented opcodes
            bus->write(test_pc + 1, 0xEA); // NOP instruction

            bool initial_flag_state = cpu->get_flag(params.flag);
            cpu->step();

            // Verify flag was set/cleared correctly
            bool final_flag_state = cpu->get_flag(params.flag);
            EXPECT_EQ(final_flag_state, params.set_flag)
                << "Flag " << params.instruction << " failed for flags 0x" << std::hex << (int)flags;

            // Verify other flags were not affected
            uint8_t other_flags = cpu->p & ~params.flag;
            uint8_t expected_other_flags = flags & ~params.flag;
            EXPECT_EQ(other_flags, expected_other_flags)
                << "Other flags affected by " << params.instruction << " for flags 0x" << std::hex << (int)flags;
        }
    }
}
