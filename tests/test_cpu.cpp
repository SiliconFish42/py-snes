#include <cstdint>
#include <string>
#include "gtest/gtest.h"
#include "cpu.hpp"
#include "bus.hpp"
#include <tuple>

struct LDAParams {
    uint8_t opcode;
    uint8_t expected_cycles_8;
    uint8_t expected_cycles_16;
    std::string mode;
};

struct STAParams {
    uint8_t opcode;
    uint8_t expected_cycles_8;
    uint8_t expected_cycles_16;
    std::string mode;
};

class LDATest : public ::testing::TestWithParam<std::tuple<LDAParams, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

TEST_P(LDATest, LDA_AllModes) {
    auto [params, is16] = GetParam();
    uint8_t opcode = params.opcode;
    uint8_t expected_cycles = is16 ? params.expected_cycles_16 : params.expected_cycles_8;
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    if (is16) cpu->p &= ~CPU::M; else cpu->p |= CPU::M;
    // Write test value (0x42 for 8-bit, 0x1234 for 16-bit)
    uint16_t test_val = is16 ? 0x1234 : 0x42;
    // Setup for each addressing mode
    if (params.mode == "Immediate") {
        bus->write(test_pc, opcode);
        if (is16) { bus->write(test_pc + 1, 0x34); bus->write(test_pc + 2, 0x12); }
        else { bus->write(test_pc + 1, 0x42); }
    } else if (params.mode == "Direct Page") {
        bus->write(test_pc, opcode); bus->write(test_pc + 1, 0xF0);
        bus->write(0xF0, test_val & 0xFF); if (is16) bus->write(0xF1, (test_val >> 8) & 0xFF);
    } else if (params.mode == "Direct Page,X") {
        cpu->x = 0x02; bus->write(test_pc, opcode); bus->write(test_pc + 1, 0xF0);
        uint8_t addr = (0xF0 + 0x02) & 0xFF;
        bus->write(addr, test_val & 0xFF); if (is16) bus->write((addr + 1) & 0xFF, (test_val >> 8) & 0xFF);
    } else if (params.mode == "Direct Page,Y") {
        cpu->y = 0x03; bus->write(test_pc, opcode); bus->write(test_pc + 1, 0xF0);
        uint8_t addr = (0xF0 + 0x03) & 0xFF;
        bus->write(addr, test_val & 0xFF); if (is16) bus->write((addr + 1) & 0xFF, (test_val >> 8) & 0xFF);
    } else if (params.mode == "Absolute") {
        bus->write(test_pc, opcode); bus->write(test_pc + 1, 0xF0); bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F0, test_val & 0xFF); if (is16) bus->write(0x7E00F1, (test_val >> 8) & 0xFF);
    } else if (params.mode == "Absolute,X") {
        cpu->x = 0x01; bus->write(test_pc, opcode); bus->write(test_pc + 1, 0xF0); bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F1, test_val & 0xFF); if (is16) bus->write(0x7E00F2, (test_val >> 8) & 0xFF);
    } else if (params.mode == "Absolute,Y") {
        cpu->y = 0x01; bus->write(test_pc, opcode); bus->write(test_pc + 1, 0xF0); bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F1, test_val & 0xFF); if (is16) bus->write(0x7E00F2, (test_val >> 8) & 0xFF);
    } else if (params.mode == "Absolute Long") {
        bus->write(test_pc, opcode); bus->write(test_pc + 1, 0xF0); bus->write(test_pc + 2, 0x00); bus->write(test_pc + 3, 0x7E);
        bus->write(0x7E00F0, test_val & 0xFF); if (is16) bus->write(0x7E00F1, (test_val >> 8) & 0xFF);
    } else if (params.mode == "Absolute Long,X") {
        cpu->x = 0x01; bus->write(test_pc, opcode); bus->write(test_pc + 1, 0xF0); bus->write(test_pc + 2, 0x00); bus->write(test_pc + 3, 0x7E);
        bus->write(0x7E00F1, test_val & 0xFF); if (is16) bus->write(0x7E00F2, (test_val >> 8) & 0xFF);
    } else if (params.mode == "(DP,X)") {
        cpu->x = 0x02; bus->write(test_pc, opcode); bus->write(test_pc + 1, 0x10);
        bus->write(0x12, 0xF0); bus->write(0x13, 0x00);
        bus->write(0xF0, test_val & 0xFF); if (is16) bus->write(0xF1, (test_val >> 8) & 0xFF);
    } else if (params.mode == "(DP),Y") {
        cpu->y = 0x01; bus->write(test_pc, opcode); bus->write(test_pc + 1, 0x10);
        bus->write(0x10, 0xF0); bus->write(0x11, 0x00);
        bus->write(0xF1, test_val & 0xFF); if (is16) bus->write(0xF2, (test_val >> 8) & 0xFF);
    } else if (params.mode == "(DP)") {
        bus->write(test_pc, opcode); bus->write(test_pc + 1, 0x20);
        bus->write(0x20, 0xF0); bus->write(0x21, 0x00);
        bus->write(0xF0, test_val & 0xFF); if (is16) bus->write(0xF1, (test_val >> 8) & 0xFF);
    } else if (params.mode == "[DP] Indirect Long") {
        bus->write(test_pc, opcode); bus->write(test_pc + 1, 0x20);
        bus->write(0x20, 0xF0); bus->write(0x21, 0x00); bus->write(0x22, 0x7E);
        bus->write(0x7E00F0, test_val & 0xFF); if (is16) bus->write(0x7E00F1, (test_val >> 8) & 0xFF);
    } else if (params.mode == "[DP],Y Indirect Long") {
        cpu->y = 0x01; bus->write(test_pc, opcode); bus->write(test_pc + 1, 0x20);
        bus->write(0x20, 0xF0); bus->write(0x21, 0x00); bus->write(0x22, 0x7E);
        bus->write(0x7E00F1, test_val & 0xFF); if (is16) bus->write(0x7E00F2, (test_val >> 8) & 0xFF);
    } else if (params.mode == "Stack Relative") {
        cpu->stkp = 0x10; bus->write(test_pc, opcode); bus->write(test_pc + 1, 0x05);
        bus->write(0x15, test_val & 0xFF); if (is16) bus->write(0x16, (test_val >> 8) & 0xFF);
    } else if (params.mode == "(Stack Relative),Y Indirect") {
        cpu->stkp = 0x10; cpu->y = 0x01; bus->write(test_pc, opcode); bus->write(test_pc + 1, 0x05);
        bus->write(0x15, 0xF0); bus->write(0x16, 0x00);
        bus->write(0xF1, test_val & 0xFF); if (is16) bus->write(0xF2, (test_val >> 8) & 0xFF);
    }
    cpu->step();
    EXPECT_EQ(cpu->a, test_val);
    EXPECT_EQ(cpu->cycles, expected_cycles);
    EXPECT_EQ(cpu->get_flag(CPU::Z), test_val == 0);
    EXPECT_EQ(cpu->get_flag(CPU::N), is16 ? ((test_val & 0x8000) != 0) : ((test_val & 0x80) != 0));
}

INSTANTIATE_TEST_SUITE_P(
    LDA_AllModes,
    LDATest,
    ::testing::Combine(
        ::testing::Values(
            LDAParams{0xA9, 2, 3, "Immediate"},
            LDAParams{0xA5, 3, 4, "Direct Page"},
            LDAParams{0xB5, 4, 5, "Direct Page,X"},
            LDAParams{0xB9, 4, 5, "Absolute,Y"},
            LDAParams{0xA1, 6, 7, "(DP,X)"},
            LDAParams{0xB1, 5, 6, "(DP),Y"},
            LDAParams{0xB2, 5, 6, "(DP)"},
            LDAParams{0xA7, 6, 7, "[DP] Indirect Long"},
            LDAParams{0xB7, 6, 7, "[DP],Y Indirect Long"},
            LDAParams{0xAD, 4, 5, "Absolute"},
            LDAParams{0xBD, 4, 5, "Absolute,X"},
            LDAParams{0xAF, 5, 6, "Absolute Long"},
            LDAParams{0xBF, 5, 6, "Absolute Long,X"},
            LDAParams{0xA3, 4, 5, "Stack Relative"},
            LDAParams{0xB3, 6, 7, "(Stack Relative),Y Indirect"}
        ),
        ::testing::Bool()
    )
);

class STATest : public ::testing::TestWithParam<std::tuple<STAParams, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

TEST_P(STATest, STA_AllModes) {
    auto [params, is16] = GetParam();
    uint8_t opcode = params.opcode;
    uint8_t expected_cycles = is16 ? params.expected_cycles_16 : params.expected_cycles_8;
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    if (is16) { cpu->p &= ~CPU::M; cpu->a = 0x1234; } else { cpu->p |= CPU::M; cpu->a = 0x42; }
    uint16_t test_val = cpu->a;
    uint32_t expect_addr = 0;
    if (params.mode == "Direct Page") {
        bus->write(test_pc, opcode); bus->write(test_pc + 1, 0xF0); expect_addr = 0xF0;
    } else if (params.mode == "Direct Page,X") {
        cpu->x = 0x02; bus->write(test_pc, opcode); bus->write(test_pc + 1, 0xF0); expect_addr = (0xF0 + 0x02) & 0xFF;
    } else if (params.mode == "Absolute") {
        bus->write(test_pc, opcode); bus->write(test_pc + 1, 0xF0); bus->write(test_pc + 2, 0x00); expect_addr = 0x7E00F0;
    } else if (params.mode == "Absolute,X") {
        cpu->x = 0x01; bus->write(test_pc, opcode); bus->write(test_pc + 1, 0xF0); bus->write(test_pc + 2, 0x00); expect_addr = 0x7E00F1;
    } else if (params.mode == "Absolute Long") {
        bus->write(test_pc, opcode); bus->write(test_pc + 1, 0xF0); bus->write(test_pc + 2, 0x00); bus->write(test_pc + 3, 0x7E); expect_addr = 0x7E00F0;
    } else if (params.mode == "Absolute Long,X") {
        cpu->x = 0x01; bus->write(test_pc, opcode); bus->write(test_pc + 1, 0xF0); bus->write(test_pc + 2, 0x00); bus->write(test_pc + 3, 0x7E); expect_addr = 0x7E00F1;
    } else if (params.mode == "(DP,X)") {
        cpu->x = 0x02; bus->write(test_pc, opcode); bus->write(test_pc + 1, 0x10); bus->write(0x12, 0xF0); bus->write(0x13, 0x00); expect_addr = 0xF0;
    } else if (params.mode == "(DP),Y") {
        cpu->y = 0x01; bus->write(test_pc, opcode); bus->write(test_pc + 1, 0x10); bus->write(0x10, 0xF0); bus->write(0x11, 0x00); expect_addr = 0xF1;
    } else if (params.mode == "(DP)") {
        bus->write(test_pc, opcode); bus->write(test_pc + 1, 0x20); bus->write(0x20, 0xF0); bus->write(0x21, 0x00); expect_addr = 0xF0;
    } else if (params.mode == "[DP] Indirect Long") {
        bus->write(test_pc, opcode); bus->write(test_pc + 1, 0x20); bus->write(0x20, 0xF0); bus->write(0x21, 0x00); bus->write(0x22, 0x7E); expect_addr = 0x7E00F0;
    } else if (params.mode == "[DP],Y Indirect Long") {
        cpu->y = 0x01; bus->write(test_pc, opcode); bus->write(test_pc + 1, 0x20); bus->write(0x20, 0xF0); bus->write(0x21, 0x00); bus->write(0x22, 0x7E); expect_addr = 0x7E00F1;
    } else if (params.mode == "Stack Relative") {
        cpu->stkp = 0x10; bus->write(test_pc, opcode); bus->write(test_pc + 1, 0x05); expect_addr = 0x15;
    } else if (params.mode == "(Stack Relative),Y Indirect") {
        cpu->stkp = 0x10; cpu->y = 0x01; bus->write(test_pc, opcode); bus->write(test_pc + 1, 0x05); bus->write(0x15, 0xF0); bus->write(0x16, 0x00); expect_addr = 0xF1;
    }
    cpu->step();
    EXPECT_EQ(bus->read(expect_addr), test_val & 0xFF);
    if (is16) EXPECT_EQ(bus->read((expect_addr + 1) & 0xFFFFFF), (test_val >> 8) & 0xFF);
    EXPECT_EQ(cpu->cycles, expected_cycles);
}

INSTANTIATE_TEST_SUITE_P(
    STA_AllModes,
    STATest,
    ::testing::Combine(
        ::testing::Values(
            STAParams{0x85, 3, 4, "Direct Page"},
            STAParams{0x95, 4, 5, "Direct Page,X"},
            STAParams{0x8D, 4, 5, "Absolute"},
            STAParams{0x9D, 4, 5, "Absolute,X"},
            STAParams{0x8F, 5, 6, "Absolute Long"},
            STAParams{0x9F, 5, 6, "Absolute Long,X"},
            STAParams{0x81, 6, 7, "(DP,X)"},
            STAParams{0x91, 5, 6, "(DP),Y"},
            STAParams{0x92, 5, 6, "(DP)"},
            STAParams{0x87, 6, 7, "[DP] Indirect Long"},
            STAParams{0x97, 6, 7, "[DP],Y Indirect Long"},
            STAParams{0x83, 4, 5, "Stack Relative"},
            STAParams{0x93, 6, 7, "(Stack Relative),Y Indirect"}
        ),
        ::testing::Bool()
    )
);

class TransferTest : public ::testing::TestWithParam<std::tuple<std::string, uint8_t, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

TEST_P(TransferTest, TransferInstructions) {
    auto [instr, opcode, is16] = GetParam();
    uint32_t test_pc = 0x7E1000;
    cpu->reset();
    cpu->pc = test_pc;
    if (is16) { cpu->p &= ~CPU::M; cpu->p &= ~CPU::X; } else { cpu->p |= CPU::M; cpu->p |= CPU::X; }
    // Set up registers for each instruction
    if (instr == "TAX") {
        cpu->a = is16 ? 0x1234 : 0x42;
        cpu->x = 0;
        bus->write(test_pc, opcode);
        cpu->step();
        EXPECT_EQ(cpu->x, is16 ? 0x1234 : 0x42);
        EXPECT_EQ(cpu->get_flag(CPU::Z), 0);
        EXPECT_EQ(cpu->get_flag(CPU::N), is16 ? 0 : 0);
    } else if (instr == "TXA") {
        cpu->x = is16 ? 0x1234 : 0x42;
        cpu->a = 0;
        bus->write(test_pc, opcode);
        cpu->step();
        EXPECT_EQ(cpu->a, is16 ? 0x1234 : 0x42);
        EXPECT_EQ(cpu->get_flag(CPU::Z), 0);
        EXPECT_EQ(cpu->get_flag(CPU::N), is16 ? 0 : 0);
    } else if (instr == "TAY") {
        cpu->a = is16 ? 0x1234 : 0x42;
        cpu->y = 0;
        bus->write(test_pc, opcode);
        cpu->step();
        EXPECT_EQ(cpu->y, is16 ? 0x1234 : 0x42);
    } else if (instr == "TYA") {
        cpu->y = is16 ? 0x1234 : 0x42;
        cpu->a = 0;
        bus->write(test_pc, opcode);
        cpu->step();
        EXPECT_EQ(cpu->a, is16 ? 0x1234 : 0x42);
    } else if (instr == "TSX") {
        cpu->stkp = is16 ? 0x1234 : 0x42; // 16-bit or 8-bit value
        cpu->x = 0;
        bus->write(test_pc, opcode);
        cpu->step();
        EXPECT_EQ(cpu->x, is16 ? 0x1234 : 0x42);
    } else if (instr == "TXS") {
        cpu->x = is16 ? 0x1234 : 0x42;
        cpu->stkp = 0;
        bus->write(test_pc, opcode);
        cpu->step();
        EXPECT_EQ(cpu->stkp, is16 ? 0x1234 : 0x42);
    } else if (instr == "TXY") {
        cpu->x = is16 ? 0x1234 : 0x42;
        cpu->y = 0;
        bus->write(test_pc, opcode);
        cpu->step();
        EXPECT_EQ(cpu->y, is16 ? 0x1234 : 0x42);
    } else if (instr == "TYX") {
        cpu->y = is16 ? 0x1234 : 0x42;
        cpu->x = 0;
        bus->write(test_pc, opcode);
        cpu->step();
        EXPECT_EQ(cpu->x, is16 ? 0x1234 : 0x42);
    } else if (instr == "TSC") {
        cpu->stkp = 0x1234;
        cpu->a = 0;
        bus->write(test_pc, opcode);
        cpu->step();
        EXPECT_EQ(cpu->a, 0x1234);
    } else if (instr == "TCS") {
        cpu->a = 0x1234;
        cpu->stkp = 0;
        bus->write(test_pc, opcode);
        cpu->step();
        EXPECT_EQ(cpu->stkp, 0x1234); // Full 16-bit A
    } else if (instr == "XBA") {
        cpu->a = 0x1234;
        bus->write(test_pc, opcode);
        cpu->step();
        EXPECT_EQ(cpu->a, 0x3412);
    } else if (instr == "XCE") {
        cpu->p = 0;
        cpu->set_flag(CPU::C, true);
        cpu->set_flag(CPU::E, false);
        bus->write(test_pc, opcode);
        cpu->step();
        EXPECT_EQ(cpu->get_flag(CPU::C), false);
        EXPECT_EQ(cpu->get_flag(CPU::E), true);
    }
}

INSTANTIATE_TEST_SUITE_P(
    TransferInstructions,
    TransferTest,
    ::testing::Values(
        std::make_tuple("TAX", 0xAA, false),
        std::make_tuple("TAX", 0xAA, true),
        std::make_tuple("TXA", 0x8A, false),
        std::make_tuple("TXA", 0x8A, true),
        std::make_tuple("TAY", 0xA8, false),
        std::make_tuple("TAY", 0xA8, true),
        std::make_tuple("TYA", 0x98, false),
        std::make_tuple("TYA", 0x98, true),
        std::make_tuple("TSX", 0xBA, false),
        std::make_tuple("TSX", 0xBA, true),
        std::make_tuple("TXS", 0x9A, false),
        std::make_tuple("TXS", 0x9A, true),
        std::make_tuple("TXY", 0x9B, false),
        std::make_tuple("TXY", 0x9B, true),
        std::make_tuple("TYX", 0xBB, false),
        std::make_tuple("TYX", 0xBB, true),
        std::make_tuple("TSC", 0x3B, true),
        std::make_tuple("TCS", 0x1B, true),
        std::make_tuple("XBA", 0xEB, true),
        std::make_tuple("XCE", 0xFB, true)
    )
);

// Stack Operations Tests
struct StackParams {
    std::string instruction;
    uint8_t opcode;
    uint8_t expected_cycles_8;
    uint8_t expected_cycles_16;
    std::string description;
};

class StackTest : public ::testing::TestWithParam<std::tuple<StackParams, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

TEST_P(StackTest, StackOperations) {
    auto [params, is16] = GetParam();
    uint8_t opcode = params.opcode;
    uint8_t expected_cycles = is16 ? params.expected_cycles_16 : params.expected_cycles_8;
    uint32_t test_pc = 0x7E0000;

    cpu->reset();
    cpu->pc = test_pc;

    // Set accumulator/index size flags
    if (is16) {
        cpu->p &= ~CPU::M;
        cpu->p &= ~CPU::X;
    } else {
        cpu->p |= CPU::M;
        cpu->p |= CPU::X;
    }

    // Setup initial values
    cpu->a = is16 ? 0x1234 : 0x42;
    cpu->x = is16 ? 0x5678 : 0x84;
    cpu->y = is16 ? 0x9ABC : 0xC6;
    cpu->d = 0xDEAD;
    cpu->pb = 0x7E;
    cpu->stkp = 0x01FD;

    // Write opcode
    bus->write(test_pc, opcode);

    // Setup additional data for specific instructions
    if (params.instruction == "PEA") {
        bus->write(test_pc + 1, 0x34); // Low byte
        bus->write(test_pc + 2, 0x12); // High byte
    } else if (params.instruction == "PEI") {
        bus->write(test_pc + 1, 0xF0); // Direct page address
        bus->write(0xF0, 0x78); // Low byte at DP
        bus->write(0xF1, 0x56); // High byte at DP+1
    } else if (params.instruction == "PER") {
        bus->write(test_pc + 1, 0xFE); // Relative offset low
        bus->write(test_pc + 2, 0xFF); // Relative offset high (-2)
    }

    // Execute instruction
    cpu->step();

    // Verify cycles
    EXPECT_EQ(cpu->cycles, expected_cycles) << "Instruction: " << params.instruction << " (16-bit: " << is16 << ")";

    // Verify stack operations
    if (params.instruction == "PHA") {
        if (is16) {
            EXPECT_EQ(bus->read(0x01FC), 0x12); // High byte
            EXPECT_EQ(bus->read(0x01FB), 0x34); // Low byte
            EXPECT_EQ(cpu->stkp, 0x01FB);
        } else {
            EXPECT_EQ(bus->read(0x01FC), 0x42); // Low byte only
            EXPECT_EQ(cpu->stkp, 0x01FC);
        }
    } else if (params.instruction == "PLA") {
        // First push some data to stack
        if (is16) {
            bus->write(0x01FC, 0x12);
            bus->write(0x01FB, 0x34);
        } else {
            bus->write(0x01FC, 0x42);
        }

        // Reset and execute PLA
        cpu->reset();
        cpu->pc = test_pc;
        cpu->connect_bus(bus);
        if (is16) {
            cpu->p &= ~CPU::M;
            cpu->stkp = 0x01FB; // Set stack pointer after reset
        } else {
            cpu->p |= CPU::M;
            cpu->stkp = 0x01FC; // Set stack pointer after reset
        }
        bus->write(test_pc, opcode);
        cpu->step();

        if (is16) {
            EXPECT_EQ(cpu->a, 0x1234);
            EXPECT_EQ(cpu->stkp, 0x01FD);
        } else {
            EXPECT_EQ(cpu->a & 0xFF, 0x42);
            EXPECT_EQ(cpu->stkp, 0x01FD);
        }
    } else if (params.instruction == "PHX") {
        if (is16) {
            EXPECT_EQ(bus->read(0x01FC), 0x56); // High byte
            EXPECT_EQ(bus->read(0x01FB), 0x78); // Low byte
            EXPECT_EQ(cpu->stkp, 0x01FB);
        } else {
            EXPECT_EQ(bus->read(0x01FC), 0x84); // Low byte only
            EXPECT_EQ(cpu->stkp, 0x01FC);
        }
    } else if (params.instruction == "PLX") {
        // Setup stack data
        if (is16) {
            bus->write(0x01FC, 0x56);
            bus->write(0x01FB, 0x78);
        } else {
            bus->write(0x01FC, 0x84);
        }

        // Reset and execute PLX
        cpu->reset();
        cpu->pc = test_pc;
        cpu->connect_bus(bus);
        if (is16) {
            cpu->p &= ~CPU::X;
            cpu->stkp = 0x01FB; // Set stack pointer after reset
        } else {
            cpu->p |= CPU::X;
            cpu->stkp = 0x01FC; // Set stack pointer after reset
        }
        bus->write(test_pc, opcode);
        cpu->step();

        if (is16) {
            EXPECT_EQ(cpu->x, 0x5678);
            EXPECT_EQ(cpu->stkp, 0x01FD);
        } else {
            EXPECT_EQ(cpu->x & 0xFF, 0x84);
            EXPECT_EQ(cpu->stkp, 0x01FD);
        }
    } else if (params.instruction == "PHY") {
        if (is16) {
            EXPECT_EQ(bus->read(0x01FC), 0x9A); // High byte
            EXPECT_EQ(bus->read(0x01FB), 0xBC); // Low byte
            EXPECT_EQ(cpu->stkp, 0x01FB);
        } else {
            EXPECT_EQ(bus->read(0x01FC), 0xC6); // Low byte only
            EXPECT_EQ(cpu->stkp, 0x01FC);
        }
    } else if (params.instruction == "PLY") {
        // Setup stack data
        if (is16) {
            bus->write(0x01FC, 0x9A);
            bus->write(0x01FB, 0xBC);
        } else {
            bus->write(0x01FC, 0xC6);
        }

        // Reset and execute PLY
        cpu->reset();
        cpu->pc = test_pc;
        cpu->connect_bus(bus);
        if (is16) {
            cpu->p &= ~CPU::X;
            cpu->stkp = 0x01FB; // Set stack pointer after reset
        } else {
            cpu->p |= CPU::X;
            cpu->stkp = 0x01FC; // Set stack pointer after reset
        }
        bus->write(test_pc, opcode);
        cpu->step();

        if (is16) {
            EXPECT_EQ(cpu->y, 0x9ABC);
            EXPECT_EQ(cpu->stkp, 0x01FD);
        } else {
            EXPECT_EQ(cpu->y & 0xFF, 0xC6);
            EXPECT_EQ(cpu->stkp, 0x01FD);
        }
    } else if (params.instruction == "PHP") {
        EXPECT_EQ(bus->read(0x01FC), (cpu->p & 0xFF) | 0x10); // PHP sets B flag when pushing
        EXPECT_EQ(cpu->stkp, 0x01FC);
    } else if (params.instruction == "PLP") {
        // Setup stack data
        bus->write(0x01FC, 0x34);

        // Reset and execute PLP
        cpu->reset();
        cpu->pc = test_pc;
        cpu->connect_bus(bus);
        cpu->stkp = 0x01FC; // Set stack pointer after reset
        bus->write(test_pc, opcode);
        cpu->step();

        EXPECT_EQ(cpu->p & 0xFF, 0x34);
        EXPECT_EQ(cpu->stkp, 0x01FD);
    } else if (params.instruction == "PHD") {
        EXPECT_EQ(bus->read(0x01FC), 0xDE); // High byte
        EXPECT_EQ(bus->read(0x01FB), 0xAD); // Low byte
        EXPECT_EQ(cpu->stkp, 0x01FB);
    } else if (params.instruction == "PLD") {
        // Setup stack data
        bus->write(0x01FC, 0xDE);
        bus->write(0x01FB, 0xAD);

        // Reset and execute PLD
        cpu->reset();
        cpu->pc = test_pc;
        cpu->connect_bus(bus);
        cpu->stkp = 0x01FB; // Set stack pointer after reset
        bus->write(test_pc, opcode);
        cpu->step();

        EXPECT_EQ(cpu->d, 0xDEAD);
        EXPECT_EQ(cpu->stkp, 0x01FD);
    } else if (params.instruction == "PHK") {
        EXPECT_EQ(bus->read(0x01FC), 0x7E);
        EXPECT_EQ(cpu->stkp, 0x01FC);
    } else if (params.instruction == "PLK") {
        // Setup stack data
        bus->write(0x01FC, 0x7E);

        // Reset and execute PLK
        cpu->reset();
        cpu->pc = test_pc;
        cpu->connect_bus(bus);
        cpu->stkp = 0x01FC; // Set stack pointer after reset
        bus->write(test_pc, opcode);
        cpu->step();

        EXPECT_EQ(cpu->pb, 0x7E);
        EXPECT_EQ(cpu->stkp, 0x01FD);
    } else if (params.instruction == "PEA") {
        EXPECT_EQ(bus->read(0x01FC), 0x12); // High byte
        EXPECT_EQ(bus->read(0x01FB), 0x34); // Low byte
        EXPECT_EQ(cpu->stkp, 0x01FB);
        EXPECT_EQ(cpu->pc, test_pc + 3); // PC advanced by 3
    } else if (params.instruction == "PEI") {
        EXPECT_EQ(bus->read(0x01FC), 0x56); // High byte from DP indirect
        EXPECT_EQ(bus->read(0x01FB), 0x78); // Low byte from DP indirect
        EXPECT_EQ(cpu->stkp, 0x01FB);
        EXPECT_EQ(cpu->pc, test_pc + 2); // PC advanced by 2
    } else if (params.instruction == "PER") {
        // Setup relative address data (-2 as 16-bit little-endian)
        bus->write(test_pc + 1, 0xFE); // Low byte of -2
        bus->write(test_pc + 2, 0xFF); // High byte of -2 (sign-extended)

        // Calculate expected target (PC + 2 + (-2) = PC, where PC is after reading opcode)
        uint16_t expected_target = (test_pc + 1) & 0xFFFF; // PC after reading opcode
        EXPECT_EQ(bus->read(0x01FC), (expected_target >> 8) & 0xFF);
        EXPECT_EQ(bus->read(0x01FB), expected_target & 0xFF);
        EXPECT_EQ(cpu->stkp, 0x01FB);
        EXPECT_EQ(cpu->pc, test_pc + 3); // PC advanced by 3 (opcode + 2 bytes)
    }
}

// Stack operation test parameters
std::vector<StackParams> stack_params = {
    {"PHA", 0x48, 3, 4, "Push Accumulator"},
    {"PLA", 0x68, 4, 5, "Pull Accumulator"},
    {"PHX", 0xDA, 3, 4, "Push X Register"},
    {"PLX", 0xFA, 4, 5, "Pull X Register"},
    {"PHY", 0x5A, 3, 4, "Push Y Register"},
    {"PLY", 0x7A, 4, 5, "Pull Y Register"},
    {"PHP", 0x08, 3, 3, "Push Processor Status"},
    {"PLP", 0x28, 4, 4, "Pull Processor Status"},
    {"PHD", 0x0B, 4, 4, "Push Direct Page Register"},
    {"PLD", 0x2B, 5, 5, "Pull Direct Page Register"},
    {"PHK", 0x4B, 3, 3, "Push Program Bank Register"},
    {"PLK", 0xAB, 4, 4, "Pull Program Bank Register"},
    {"PEA", 0xF4, 5, 5, "Push Effective Address"},
    {"PEI", 0xD4, 6, 6, "Push Effective Indirect Address"},
    {"PER", 0x62, 6, 6, "Push Program Counter Relative"}
};

INSTANTIATE_TEST_SUITE_P(
    StackOperations,
    StackTest,
    ::testing::Combine(
        ::testing::ValuesIn(stack_params),
        ::testing::Values(true, false) // 16-bit and 8-bit modes
    )
);

// Direct unit tests for CPUHelpers stack operations
#include "cpu_helpers.hpp"

class CPUHelpersStackTest : public ::testing::Test {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

TEST_F(CPUHelpersStackTest, Push8_WrapsFrom0100To01FF) {
    cpu->stkp = 0x0100;
    CPUHelpers::push_8(cpu.get(), 0xAB);
    EXPECT_EQ(cpu->stkp, 0x01FF);
    EXPECT_EQ(bus->read(0x01FF), 0xAB);
}

TEST_F(CPUHelpersStackTest, Pop8_WrapsFrom01FFTo0100) {
    cpu->stkp = 0x01FF;
    bus->write(0x01FF, 0xCD);
    uint8_t val = CPUHelpers::pop_8(cpu.get());
    EXPECT_EQ(val, 0xCD);
    EXPECT_EQ(cpu->stkp, 0x0100);
}

TEST_F(CPUHelpersStackTest, Push16_WrapsCorrectlyAt0100) {
    cpu->stkp = 0x0100;
    CPUHelpers::push_16(cpu.get(), 0xBEEF);
    // push_16 does high then low: push_8 (0xBE), push_8 (0xEF)
    EXPECT_EQ(cpu->stkp, 0x01FE);
    EXPECT_EQ(bus->read(0x01FF), 0xBE);
    EXPECT_EQ(bus->read(0x01FE), 0xEF);
}

TEST_F(CPUHelpersStackTest, Pop16_WrapsCorrectlyAt01FF) {
    cpu->stkp = 0x01FF;
    // Set up stack: pop_8 will read 0x01FF (lo), then 0x0100 (hi)
    bus->write(0x01FF, 0x34);
    bus->write(0x0100, 0x12);
    uint16_t val = CPUHelpers::pop_16(cpu.get());
    EXPECT_EQ(val, 0x1234);
    EXPECT_EQ(cpu->stkp, 0x0101);
}

TEST_F(CPUHelpersStackTest, PushPop8_SequenceMaintainsStackPointer) {
    cpu->stkp = 0x01FD;
    CPUHelpers::push_8(cpu.get(), 0x55);
    EXPECT_EQ(cpu->stkp, 0x01FC);
    uint8_t val = CPUHelpers::pop_8(cpu.get());
    EXPECT_EQ(val, 0x55);
    EXPECT_EQ(cpu->stkp, 0x01FD);
}

TEST_F(CPUHelpersStackTest, PushPop16_SequenceMaintainsStackPointer) {
    cpu->stkp = 0x01FD;
    CPUHelpers::push_16(cpu.get(), 0xCAFE);
    EXPECT_EQ(cpu->stkp, 0x01FB);
    // Set up expected stack values for pop
    // push_16: push_8(hi=0xCA) at 0x01FC, push_8(lo=0xFE) at 0x01FB
    // So pop_8 at 0x01FB (lo), pop_8 at 0x01FC (hi)
    bus->write(0x01FB, 0xFE);
    bus->write(0x01FC, 0xCA);
    cpu->stkp = 0x01FB;
    uint16_t val = CPUHelpers::pop_16(cpu.get());
    EXPECT_EQ(val, 0xCAFE);
    EXPECT_EQ(cpu->stkp, 0x01FD);
}

// --- Coverage Gap Stubs ---
TEST_F(LDATest, IllegalOpcodeHandling) {
    GTEST_SKIP() << "Not yet implemented: illegal opcode handling test stub.";
}

TEST_F(LDATest, DecimalModeADC) {
    GTEST_SKIP() << "Not yet implemented: ADC in decimal mode test stub.";
}

TEST_F(LDATest, DecimalModeSBC) {
    GTEST_SKIP() << "Not yet implemented: SBC in decimal mode test stub.";
}

TEST_F(LDATest, InterruptHandlingNMI) {
    GTEST_SKIP() << "Not yet implemented: NMI interrupt handling test stub.";
}

TEST_F(LDATest, InterruptHandlingIRQ) {
    GTEST_SKIP() << "Not yet implemented: IRQ interrupt handling test stub.";
}

TEST_F(LDATest, InterruptHandlingRESET) {
    GTEST_SKIP() << "Not yet implemented: RESET interrupt handling test stub.";
}

TEST_F(LDATest, WAI_STP_Instruction) {
    GTEST_SKIP() << "Not yet implemented: WAI/STP instruction test stub.";
}

TEST_F(LDATest, XCE_EmulationNativeTransition) {
    GTEST_SKIP() << "Not yet implemented: XCE emulation/native mode transition test stub.";
}

TEST_F(LDATest, EmulationModeEdgeCases) {
    GTEST_SKIP() << "Not yet implemented: emulation mode edge case test stub.";
}
