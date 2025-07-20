#include <cstdint>
#include <string>
#include "gtest/gtest.h"
#include "cpu.hpp"
#include "bus.hpp"
#include <tuple>

// Test structure for logical instructions
struct LogicalParams {
    uint8_t opcode;
    uint8_t expected_cycles_8;
    uint8_t expected_cycles_16;
    std::string mode;
    std::string instruction;
    std::string description;
};

// AND Test Class
class ANDTest : public ::testing::TestWithParam<std::tuple<LogicalParams, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

// ORA Test Class
class ORATest : public ::testing::TestWithParam<std::tuple<LogicalParams, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

// EOR Test Class
class EORTest : public ::testing::TestWithParam<std::tuple<LogicalParams, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

// AND Tests
TEST_P(ANDTest, AND_AllModes) {
    auto [params, is16] = GetParam();
    uint8_t opcode = params.opcode;
    uint8_t expected_cycles = is16 ? params.expected_cycles_16 : params.expected_cycles_8;
    uint32_t test_pc = 0x7E0000;

    cpu->reset();
    cpu->pc = test_pc;

    // Set accumulator size flag
    if (is16) {
        cpu->p &= ~CPU::M;
    } else {
        cpu->p |= CPU::M;
    }

    // Set initial accumulator value
    uint16_t initial_a = is16 ? 0x1234 : 0x42;
    cpu->a = initial_a;

    // Write test operand value
    uint16_t test_operand = is16 ? 0x5678 : 0x84;

    // Setup for each addressing mode
    if (params.mode == "Immediate") {
        bus->write(test_pc, opcode);
        if (is16) {
            bus->write(test_pc + 1, test_operand & 0xFF);
            bus->write(test_pc + 2, (test_operand >> 8) & 0xFF);
        } else {
            bus->write(test_pc + 1, test_operand & 0xFF);
        }
    } else if (params.mode == "Direct Page") {
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        bus->write(0xF0, test_operand & 0xFF);
        if (is16) bus->write(0xF1, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "Direct Page,X") {
        cpu->x = 0x02;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        uint8_t addr = (0xF0 + 0x02) & 0xFF;
        bus->write(addr, test_operand & 0xFF);
        if (is16) bus->write((addr + 1) & 0xFF, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "Absolute") {
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F0, test_operand & 0xFF);
        if (is16) bus->write(0x7E00F1, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "Absolute,X") {
        cpu->x = 0x01;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F1, test_operand & 0xFF);
        if (is16) bus->write(0x7E00F2, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "Absolute,Y") {
        cpu->y = 0x01;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F1, test_operand & 0xFF);
        if (is16) bus->write(0x7E00F2, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "(DP,X)") {
        cpu->x = 0x02;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0x10);
        bus->write(0x12, 0xF0);
        bus->write(0x13, 0x00);
        bus->write(0xF0, test_operand & 0xFF);
        if (is16) bus->write(0xF1, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "(DP),Y") {
        cpu->y = 0x01;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0x10);
        bus->write(0x10, 0xF0);
        bus->write(0x11, 0x00);
        bus->write(0xF1, test_operand & 0xFF);
        if (is16) bus->write(0xF2, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "(DP)") {
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0x20);
        bus->write(0x20, 0xF0);
        bus->write(0x21, 0x00);
        bus->write(0xF0, test_operand & 0xFF);
        if (is16) bus->write(0xF1, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "[DP]") {
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0x20);
        bus->write(0x20, 0xF0);
        bus->write(0x21, 0x00);
        bus->write(0x22, 0x7E);
        bus->write(0x7E00F0, test_operand & 0xFF);
        if (is16) bus->write(0x7E00F1, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "[DP],Y") {
        cpu->y = 0x01;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0x20);
        bus->write(0x20, 0xF0);
        bus->write(0x21, 0x00);
        bus->write(0x22, 0x7E);
        bus->write(0x7E00F1, test_operand & 0xFF);
        if (is16) bus->write(0x7E00F2, (test_operand >> 8) & 0xFF);
    }

    cpu->step();

    // Calculate expected result (A & operand)
    uint16_t expected_result = initial_a & test_operand;
    if (is16) {
        expected_result &= 0xFFFF;
    } else {
        expected_result &= 0xFF;
    }

    EXPECT_EQ(cpu->cycles, expected_cycles) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    EXPECT_EQ(cpu->a, expected_result) << "Instruction: " << params.instruction << " Mode: " << params.mode;

    // Verify zero flag
    uint16_t result_for_flags = is16 ? expected_result : (expected_result & 0xFF);
    EXPECT_EQ(cpu->get_flag(CPU::Z), result_for_flags == 0) << "Instruction: " << params.instruction << " Mode: " << params.mode;

    // Verify negative flag
    bool expected_negative = is16 ? ((result_for_flags & 0x8000) != 0) : ((result_for_flags & 0x80) != 0);
    EXPECT_EQ(cpu->get_flag(CPU::N), expected_negative) << "Instruction: " << params.instruction << " Mode: " << params.mode;
}

// ORA Tests
TEST_P(ORATest, ORA_AllModes) {
    auto [params, is16] = GetParam();
    uint8_t opcode = params.opcode;
    uint8_t expected_cycles = is16 ? params.expected_cycles_16 : params.expected_cycles_8;
    uint32_t test_pc = 0x7E0000;

    cpu->reset();
    cpu->pc = test_pc;

    // Set accumulator size flag
    if (is16) {
        cpu->p &= ~CPU::M;
    } else {
        cpu->p |= CPU::M;
    }

    // Set initial accumulator value
    uint16_t initial_a = is16 ? 0x1234 : 0x42;
    cpu->a = initial_a;

    // Write test operand value
    uint16_t test_operand = is16 ? 0x5678 : 0x84;

    // Setup for each addressing mode
    if (params.mode == "Immediate") {
        bus->write(test_pc, opcode);
        if (is16) {
            bus->write(test_pc + 1, test_operand & 0xFF);
            bus->write(test_pc + 2, (test_operand >> 8) & 0xFF);
        } else {
            bus->write(test_pc + 1, test_operand & 0xFF);
        }
    } else if (params.mode == "Direct Page") {
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        bus->write(0xF0, test_operand & 0xFF);
        if (is16) bus->write(0xF1, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "Direct Page,X") {
        cpu->x = 0x02;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        uint8_t addr = (0xF0 + 0x02) & 0xFF;
        bus->write(addr, test_operand & 0xFF);
        if (is16) bus->write((addr + 1) & 0xFF, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "Absolute") {
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F0, test_operand & 0xFF);
        if (is16) bus->write(0x7E00F1, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "Absolute,X") {
        cpu->x = 0x01;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F1, test_operand & 0xFF);
        if (is16) bus->write(0x7E00F2, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "Absolute,Y") {
        cpu->y = 0x01;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F1, test_operand & 0xFF);
        if (is16) bus->write(0x7E00F2, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "(DP,X)") {
        cpu->x = 0x02;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0x10);
        bus->write(0x12, 0xF0);
        bus->write(0x13, 0x00);
        bus->write(0xF0, test_operand & 0xFF);
        if (is16) bus->write(0xF1, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "(DP),Y") {
        cpu->y = 0x01;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0x10);
        bus->write(0x10, 0xF0);
        bus->write(0x11, 0x00);
        bus->write(0xF1, test_operand & 0xFF);
        if (is16) bus->write(0xF2, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "(DP)") {
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0x20);
        bus->write(0x20, 0xF0);
        bus->write(0x21, 0x00);
        bus->write(0xF0, test_operand & 0xFF);
        if (is16) bus->write(0xF1, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "[DP]") {
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0x20);
        bus->write(0x20, 0xF0);
        bus->write(0x21, 0x00);
        bus->write(0x22, 0x7E);
        bus->write(0x7E00F0, test_operand & 0xFF);
        if (is16) bus->write(0x7E00F1, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "[DP],Y") {
        cpu->y = 0x01;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0x20);
        bus->write(0x20, 0xF0);
        bus->write(0x21, 0x00);
        bus->write(0x22, 0x7E);
        bus->write(0x7E00F1, test_operand & 0xFF);
        if (is16) bus->write(0x7E00F2, (test_operand >> 8) & 0xFF);
    }

    cpu->step();

    // Calculate expected result (A | operand)
    uint16_t expected_result = initial_a | test_operand;
    if (is16) {
        expected_result &= 0xFFFF;
    } else {
        expected_result &= 0xFF;
    }

    EXPECT_EQ(cpu->cycles, expected_cycles) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    EXPECT_EQ(cpu->a, expected_result) << "Instruction: " << params.instruction << " Mode: " << params.mode;

    // Verify zero flag
    uint16_t result_for_flags = is16 ? expected_result : (expected_result & 0xFF);
    EXPECT_EQ(cpu->get_flag(CPU::Z), result_for_flags == 0) << "Instruction: " << params.instruction << " Mode: " << params.mode;

    // Verify negative flag
    bool expected_negative = is16 ? ((result_for_flags & 0x8000) != 0) : ((result_for_flags & 0x80) != 0);
    EXPECT_EQ(cpu->get_flag(CPU::N), expected_negative) << "Instruction: " << params.instruction << " Mode: " << params.mode;
}

// EOR Tests
TEST_P(EORTest, EOR_AllModes) {
    auto [params, is16] = GetParam();
    uint8_t opcode = params.opcode;
    uint8_t expected_cycles = is16 ? params.expected_cycles_16 : params.expected_cycles_8;
    uint32_t test_pc = 0x7E0000;

    cpu->reset();
    cpu->pc = test_pc;

    // Set accumulator size flag
    if (is16) {
        cpu->p &= ~CPU::M;
    } else {
        cpu->p |= CPU::M;
    }

    // Set initial accumulator value
    uint16_t initial_a = is16 ? 0x1234 : 0x42;
    cpu->a = initial_a;

    // Write test operand value
    uint16_t test_operand = is16 ? 0x5678 : 0x84;

    // Setup for each addressing mode
    if (params.mode == "Immediate") {
        bus->write(test_pc, opcode);
        if (is16) {
            bus->write(test_pc + 1, test_operand & 0xFF);
            bus->write(test_pc + 2, (test_operand >> 8) & 0xFF);
        } else {
            bus->write(test_pc + 1, test_operand & 0xFF);
        }
    } else if (params.mode == "Direct Page") {
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        bus->write(0xF0, test_operand & 0xFF);
        if (is16) bus->write(0xF1, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "Direct Page,X") {
        cpu->x = 0x02;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        uint8_t addr = (0xF0 + 0x02) & 0xFF;
        bus->write(addr, test_operand & 0xFF);
        if (is16) bus->write((addr + 1) & 0xFF, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "Absolute") {
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F0, test_operand & 0xFF);
        if (is16) bus->write(0x7E00F1, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "Absolute,X") {
        cpu->x = 0x01;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F1, test_operand & 0xFF);
        if (is16) bus->write(0x7E00F2, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "Absolute,Y") {
        cpu->y = 0x01;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F1, test_operand & 0xFF);
        if (is16) bus->write(0x7E00F2, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "(DP,X)") {
        cpu->x = 0x02;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0x10);
        bus->write(0x12, 0xF0);
        bus->write(0x13, 0x00);
        bus->write(0xF0, test_operand & 0xFF);
        if (is16) bus->write(0xF1, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "(DP),Y") {
        cpu->y = 0x01;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0x10);
        bus->write(0x10, 0xF0);
        bus->write(0x11, 0x00);
        bus->write(0xF1, test_operand & 0xFF);
        if (is16) bus->write(0xF2, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "(DP)") {
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0x20);
        bus->write(0x20, 0xF0);
        bus->write(0x21, 0x00);
        bus->write(0xF0, test_operand & 0xFF);
        if (is16) bus->write(0xF1, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "[DP]") {
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0x20);
        bus->write(0x20, 0xF0);
        bus->write(0x21, 0x00);
        bus->write(0x22, 0x7E);
        bus->write(0x7E00F0, test_operand & 0xFF);
        if (is16) bus->write(0x7E00F1, (test_operand >> 8) & 0xFF);
    } else if (params.mode == "[DP],Y") {
        cpu->y = 0x01;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0x20);
        bus->write(0x20, 0xF0);
        bus->write(0x21, 0x00);
        bus->write(0x22, 0x7E);
        bus->write(0x7E00F1, test_operand & 0xFF);
        if (is16) bus->write(0x7E00F2, (test_operand >> 8) & 0xFF);
    }

    cpu->step();

    // Calculate expected result (A ^ operand)
    uint16_t expected_result = initial_a ^ test_operand;
    if (is16) {
        expected_result &= 0xFFFF;
    } else {
        expected_result &= 0xFF;
    }

    EXPECT_EQ(cpu->cycles, expected_cycles) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    EXPECT_EQ(cpu->a, expected_result) << "Instruction: " << params.instruction << " Mode: " << params.mode;

    // Verify zero flag
    uint16_t result_for_flags = is16 ? expected_result : (expected_result & 0xFF);
    EXPECT_EQ(cpu->get_flag(CPU::Z), result_for_flags == 0) << "Instruction: " << params.instruction << " Mode: " << params.mode;

    // Verify negative flag
    bool expected_negative = is16 ? ((result_for_flags & 0x8000) != 0) : ((result_for_flags & 0x80) != 0);
    EXPECT_EQ(cpu->get_flag(CPU::N), expected_negative) << "Instruction: " << params.instruction << " Mode: " << params.mode;
}

// AND test parameters
std::vector<LogicalParams> and_params = {
    {0x29, 2, 3, "Immediate", "AND", "Logical AND Immediate"},
    {0x25, 3, 4, "Direct Page", "AND", "Logical AND Direct Page"},
    {0x35, 4, 5, "Direct Page,X", "AND", "Logical AND Direct Page,X"},
    {0x2D, 4, 5, "Absolute", "AND", "Logical AND Absolute"},
    {0x3D, 5, 6, "Absolute,X", "AND", "Logical AND Absolute,X"},
    {0x39, 5, 6, "Absolute,Y", "AND", "Logical AND Absolute,Y"},
    {0x21, 6, 7, "(DP,X)", "AND", "Logical AND (DP,X)"},
    {0x31, 6, 7, "(DP),Y", "AND", "Logical AND (DP),Y"},
    {0x32, 5, 6, "(DP)", "AND", "Logical AND (DP)"},
    {0x27, 6, 7, "[DP]", "AND", "Logical AND [DP]"},
    {0x37, 7, 8, "[DP],Y", "AND", "Logical AND [DP],Y"}
};

// ORA test parameters
std::vector<LogicalParams> ora_params = {
    {0x09, 2, 3, "Immediate", "ORA", "Logical ORA Immediate"},
    {0x05, 3, 4, "Direct Page", "ORA", "Logical ORA Direct Page"},
    {0x15, 4, 5, "Direct Page,X", "ORA", "Logical ORA Direct Page,X"},
    {0x0D, 4, 5, "Absolute", "ORA", "Logical ORA Absolute"},
    {0x1D, 5, 6, "Absolute,X", "ORA", "Logical ORA Absolute,X"},
    {0x19, 5, 6, "Absolute,Y", "ORA", "Logical ORA Absolute,Y"},
    {0x01, 6, 7, "(DP,X)", "ORA", "Logical ORA (DP,X)"},
    {0x11, 6, 7, "(DP),Y", "ORA", "Logical ORA (DP),Y"},
    {0x12, 5, 6, "(DP)", "ORA", "Logical ORA (DP)"},
    {0x07, 6, 7, "[DP]", "ORA", "Logical ORA [DP]"},
    {0x17, 7, 8, "[DP],Y", "ORA", "Logical ORA [DP],Y"}
};

// EOR test parameters
std::vector<LogicalParams> eor_params = {
    {0x49, 2, 3, "Immediate", "EOR", "Logical EOR Immediate"},
    {0x45, 3, 4, "Direct Page", "EOR", "Logical EOR Direct Page"},
    {0x55, 4, 5, "Direct Page,X", "EOR", "Logical EOR Direct Page,X"},
    {0x4D, 4, 5, "Absolute", "EOR", "Logical EOR Absolute"},
    {0x5D, 5, 6, "Absolute,X", "EOR", "Logical EOR Absolute,X"},
    {0x59, 5, 6, "Absolute,Y", "EOR", "Logical EOR Absolute,Y"},
    {0x41, 6, 7, "(DP,X)", "EOR", "Logical EOR (DP,X)"},
    {0x51, 6, 7, "(DP),Y", "EOR", "Logical EOR (DP),Y"},
    {0x52, 5, 6, "(DP)", "EOR", "Logical EOR (DP)"},
    {0x47, 6, 7, "[DP]", "EOR", "Logical EOR [DP]"},
    {0x57, 7, 8, "[DP],Y", "EOR", "Logical EOR [DP],Y"}
};

// AND test instantiation
INSTANTIATE_TEST_SUITE_P(
    AND_AllModes,
    ANDTest,
    ::testing::Combine(
        ::testing::ValuesIn(and_params),
        ::testing::Values(true, false) // 16-bit and 8-bit modes
    )
);

// ORA test instantiation
INSTANTIATE_TEST_SUITE_P(
    ORA_AllModes,
    ORATest,
    ::testing::Combine(
        ::testing::ValuesIn(ora_params),
        ::testing::Values(true, false) // 16-bit and 8-bit modes
    )
);

// EOR test instantiation
INSTANTIATE_TEST_SUITE_P(
    EOR_AllModes,
    EORTest,
    ::testing::Combine(
        ::testing::ValuesIn(eor_params),
        ::testing::Values(true, false) // 16-bit and 8-bit modes
    )
);

// Additional edge case tests
TEST_F(ANDTest, AND_Zero_Result) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;

    // Test 8-bit AND that results in zero
    cpu->p |= CPU::M; // 8-bit mode
    cpu->a = 0x42; // 01000010
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, true);

    bus->write(test_pc, 0x29); // AND immediate
    bus->write(test_pc + 1, 0x81); // 10000001 (no bits in common)

    cpu->step();

    EXPECT_EQ(cpu->a, 0x00); // Should be zero
    EXPECT_TRUE(cpu->get_flag(CPU::Z)); // Should set zero
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Should clear negative
}

TEST_F(ORATest, ORA_All_Ones) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;

    // Test 8-bit ORA that results in all ones
    cpu->p |= CPU::M; // 8-bit mode
    cpu->a = 0x42; // 01000010
    cpu->set_flag(CPU::Z, true);
    cpu->set_flag(CPU::N, false);

    bus->write(test_pc, 0x09); // ORA immediate
    bus->write(test_pc + 1, 0xBD); // 10111101

    cpu->step();

    EXPECT_EQ(cpu->a, 0xFF); // Should be all ones
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Should clear zero
    EXPECT_TRUE(cpu->get_flag(CPU::N)); // Should set negative
}

TEST_F(EORTest, EOR_Toggle_Bits) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;

    // Test 8-bit EOR that toggles bits
    cpu->p |= CPU::M; // 8-bit mode
    cpu->a = 0x55; // 01010101
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);

    bus->write(test_pc, 0x49); // EOR immediate
    bus->write(test_pc + 1, 0xAA); // 10101010

    cpu->step();

    EXPECT_EQ(cpu->a, 0xFF); // Should be all ones (01010101 ^ 10101010 = 11111111)
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Should clear zero
    EXPECT_TRUE(cpu->get_flag(CPU::N)); // Should set negative
}

TEST_F(ANDTest, AND_16Bit_Operation) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;

    // Test 16-bit AND operation
    cpu->p &= ~CPU::M; // 16-bit mode
    cpu->a = 0x1234; // 00010010 00110100
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);

    bus->write(test_pc, 0x29); // AND immediate
    bus->write(test_pc + 1, 0x34); // Low byte
    bus->write(test_pc + 2, 0x12); // High byte (0x1234)

    cpu->step();

    EXPECT_EQ(cpu->a, 0x1234); // Should be unchanged (A & A = A)
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Should clear zero
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Should clear negative
}
