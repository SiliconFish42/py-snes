#include <cstdint>
#include <string>
#include "gtest/gtest.h"
#include "cpu.hpp"
#include "bus.hpp"
#include <tuple>

// Test structure for comparison instructions
struct ComparisonParams {
    uint8_t opcode;
    uint8_t expected_cycles_8;
    uint8_t expected_cycles_16;
    std::string mode;
    std::string instruction;
    std::string description;
};

// CMP Test Class
class CMPTest : public ::testing::TestWithParam<std::tuple<ComparisonParams, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

// CPX Test Class
class CPXTest : public ::testing::TestWithParam<std::tuple<ComparisonParams, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

// CPY Test Class
class CPYTest : public ::testing::TestWithParam<std::tuple<ComparisonParams, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

// CMP Tests
TEST_P(CMPTest, CMP_AllModes) {
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

    // Calculate expected result (A - operand)
    uint16_t expected_result = initial_a - test_operand;
    if (is16) {
        expected_result &= 0xFFFF;
    } else {
        expected_result &= 0xFF;
    }

    EXPECT_EQ(cpu->cycles, expected_cycles) << "Instruction: " << params.instruction << " Mode: " << params.mode;

    // Verify carry flag (1 if A >= operand)
    bool expected_carry = initial_a >= test_operand;
    EXPECT_EQ(cpu->get_flag(CPU::C), expected_carry) << "Instruction: " << params.instruction << " Mode: " << params.mode;

    // Verify zero flag
    uint16_t result_for_flags = is16 ? expected_result : (expected_result & 0xFF);
    EXPECT_EQ(cpu->get_flag(CPU::Z), result_for_flags == 0) << "Instruction: " << params.instruction << " Mode: " << params.mode;

    // Verify negative flag
    bool expected_negative = is16 ? ((result_for_flags & 0x8000) != 0) : ((result_for_flags & 0x80) != 0);
    EXPECT_EQ(cpu->get_flag(CPU::N), expected_negative) << "Instruction: " << params.instruction << " Mode: " << params.mode;
}

// CPX Tests
TEST_P(CPXTest, CPX_AllModes) {
    auto [params, is16] = GetParam();
    uint8_t opcode = params.opcode;
    uint8_t expected_cycles = is16 ? params.expected_cycles_16 : params.expected_cycles_8;
    uint32_t test_pc = 0x7E0000;

    cpu->reset();
    cpu->pc = test_pc;

    // Set index register size flag
    if (is16) {
        cpu->p &= ~CPU::X;
    } else {
        cpu->p |= CPU::X;
    }

    // Set initial X register value
    uint16_t initial_x = is16 ? 0x1234 : 0x42;
    cpu->x = initial_x;

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
    } else if (params.mode == "Absolute") {
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F0, test_operand & 0xFF);
        if (is16) bus->write(0x7E00F1, (test_operand >> 8) & 0xFF);
    }

    cpu->step();

    // Calculate expected result (X - operand)
    uint16_t expected_result = initial_x - test_operand;
    if (is16) {
        expected_result &= 0xFFFF;
    } else {
        expected_result &= 0xFF;
    }

    EXPECT_EQ(cpu->cycles, expected_cycles) << "Instruction: " << params.instruction << " Mode: " << params.mode;

    // Verify carry flag (1 if X >= operand)
    bool expected_carry = initial_x >= test_operand;
    EXPECT_EQ(cpu->get_flag(CPU::C), expected_carry) << "Instruction: " << params.instruction << " Mode: " << params.mode;

    // Verify zero flag
    uint16_t result_for_flags = is16 ? expected_result : (expected_result & 0xFF);
    EXPECT_EQ(cpu->get_flag(CPU::Z), result_for_flags == 0) << "Instruction: " << params.instruction << " Mode: " << params.mode;

    // Verify negative flag
    bool expected_negative = is16 ? ((result_for_flags & 0x8000) != 0) : ((result_for_flags & 0x80) != 0);
    EXPECT_EQ(cpu->get_flag(CPU::N), expected_negative) << "Instruction: " << params.instruction << " Mode: " << params.mode;
}

// CPY Tests
TEST_P(CPYTest, CPY_AllModes) {
    auto [params, is16] = GetParam();
    uint8_t opcode = params.opcode;
    uint8_t expected_cycles = is16 ? params.expected_cycles_16 : params.expected_cycles_8;
    uint32_t test_pc = 0x7E0000;

    cpu->reset();
    cpu->pc = test_pc;

    // Set index register size flag
    if (is16) {
        cpu->p &= ~CPU::X;
    } else {
        cpu->p |= CPU::X;
    }

    // Set initial Y register value
    uint16_t initial_y = is16 ? 0x1234 : 0x42;
    cpu->y = initial_y;

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
    } else if (params.mode == "Absolute") {
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F0, test_operand & 0xFF);
        if (is16) bus->write(0x7E00F1, (test_operand >> 8) & 0xFF);
    }

    cpu->step();

    // Calculate expected result (Y - operand)
    uint16_t expected_result = initial_y - test_operand;
    if (is16) {
        expected_result &= 0xFFFF;
    } else {
        expected_result &= 0xFF;
    }

    EXPECT_EQ(cpu->cycles, expected_cycles) << "Instruction: " << params.instruction << " Mode: " << params.mode;

    // Verify carry flag (1 if Y >= operand)
    bool expected_carry = initial_y >= test_operand;
    EXPECT_EQ(cpu->get_flag(CPU::C), expected_carry) << "Instruction: " << params.instruction << " Mode: " << params.mode;

    // Verify zero flag
    uint16_t result_for_flags = is16 ? expected_result : (expected_result & 0xFF);
    EXPECT_EQ(cpu->get_flag(CPU::Z), result_for_flags == 0) << "Instruction: " << params.instruction << " Mode: " << params.mode;

    // Verify negative flag
    bool expected_negative = is16 ? ((result_for_flags & 0x8000) != 0) : ((result_for_flags & 0x80) != 0);
    EXPECT_EQ(cpu->get_flag(CPU::N), expected_negative) << "Instruction: " << params.instruction << " Mode: " << params.mode;
}

// CMP test parameters
std::vector<ComparisonParams> cmp_params = {
    {0xC9, 2, 3, "Immediate", "CMP", "Compare Accumulator Immediate"},
    {0xC5, 3, 4, "Direct Page", "CMP", "Compare Accumulator Direct Page"},
    {0xD5, 4, 5, "Direct Page,X", "CMP", "Compare Accumulator Direct Page,X"},
    {0xCD, 4, 5, "Absolute", "CMP", "Compare Accumulator Absolute"},
    {0xDD, 5, 6, "Absolute,X", "CMP", "Compare Accumulator Absolute,X"},
    {0xD9, 5, 6, "Absolute,Y", "CMP", "Compare Accumulator Absolute,Y"},
    {0xC1, 6, 7, "(DP,X)", "CMP", "Compare Accumulator (DP,X)"},
    {0xD1, 6, 7, "(DP),Y", "CMP", "Compare Accumulator (DP),Y"},
    {0xD2, 5, 6, "(DP)", "CMP", "Compare Accumulator (DP)"},
    {0xC7, 6, 7, "[DP]", "CMP", "Compare Accumulator [DP]"},
    {0xD7, 7, 8, "[DP],Y", "CMP", "Compare Accumulator [DP],Y"}
};

// CPX test parameters
std::vector<ComparisonParams> cpx_params = {
    {0xE0, 2, 3, "Immediate", "CPX", "Compare X Immediate"},
    {0xE4, 3, 4, "Direct Page", "CPX", "Compare X Direct Page"},
    {0xEC, 4, 5, "Absolute", "CPX", "Compare X Absolute"}
};

// CPY test parameters
std::vector<ComparisonParams> cpy_params = {
    {0xC0, 2, 3, "Immediate", "CPY", "Compare Y Immediate"},
    {0xC4, 3, 4, "Direct Page", "CPY", "Compare Y Direct Page"},
    {0xCC, 4, 5, "Absolute", "CPY", "Compare Y Absolute"}
};

// CMP test instantiation
INSTANTIATE_TEST_SUITE_P(
    CMP_AllModes,
    CMPTest,
    ::testing::Combine(
        ::testing::ValuesIn(cmp_params),
        ::testing::Values(true, false) // 16-bit and 8-bit modes
    )
);

// CPX test instantiation
INSTANTIATE_TEST_SUITE_P(
    CPX_AllModes,
    CPXTest,
    ::testing::Combine(
        ::testing::ValuesIn(cpx_params),
        ::testing::Values(true, false) // 16-bit and 8-bit modes
    )
);

// CPY test instantiation
INSTANTIATE_TEST_SUITE_P(
    CPY_AllModes,
    CPYTest,
    ::testing::Combine(
        ::testing::ValuesIn(cpy_params),
        ::testing::Values(true, false) // 16-bit and 8-bit modes
    )
);

// Additional edge case tests
TEST_F(CMPTest, CMP_Equal_Values) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;

    // Test 8-bit equal values
    cpu->p |= CPU::M; // 8-bit mode
    cpu->a = 0x42; // 66
    cpu->set_flag(CPU::C, false);

    bus->write(test_pc, 0xC9); // CMP immediate
    bus->write(test_pc + 1, 0x42); // Compare with 66

    cpu->step();

    EXPECT_TRUE(cpu->get_flag(CPU::Z)); // Should set zero (equal)
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Should set carry (A >= operand)
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Should clear negative
}

TEST_F(CMPTest, CMP_Greater_Than) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;

    // Test 8-bit greater than
    cpu->p |= CPU::M; // 8-bit mode
    cpu->a = 0x84; // 132
    cpu->set_flag(CPU::C, false);

    bus->write(test_pc, 0xC9); // CMP immediate
    bus->write(test_pc + 1, 0x42); // Compare with 66

    cpu->step();

    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Should clear zero (not equal)
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Should set carry (A >= operand)
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Should clear negative (positive result)
}

TEST_F(CMPTest, CMP_Less_Than) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;

    // Test 8-bit less than
    cpu->p |= CPU::M; // 8-bit mode
    cpu->a = 0x42; // 66
    cpu->set_flag(CPU::C, true);

    bus->write(test_pc, 0xC9); // CMP immediate
    bus->write(test_pc + 1, 0x84); // Compare with 132

    cpu->step();

    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Should clear zero (not equal)
    EXPECT_FALSE(cpu->get_flag(CPU::C)); // Should clear carry (A < operand)
    EXPECT_TRUE(cpu->get_flag(CPU::N)); // Should set negative (negative result)
}

TEST_F(CPXTest, CPX_16Bit_Comparison) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;

    // Test 16-bit comparison
    cpu->p &= ~CPU::X; // 16-bit mode
    cpu->x = 0x1234; // 4660
    cpu->set_flag(CPU::C, false);

    bus->write(test_pc, 0xE0); // CPX immediate
    bus->write(test_pc + 1, 0x34); // Low byte
    bus->write(test_pc + 2, 0x12); // High byte (4660)

    cpu->step();

    EXPECT_TRUE(cpu->get_flag(CPU::Z)); // Should set zero (equal)
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Should set carry (X >= operand)
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Should clear negative
}
