#include <cstdint>
#include <string>
#include "gtest/gtest.h"
#include "cpu.hpp"
#include "bus.hpp"
#include <tuple>

// Test structure for increment/decrement instructions
struct IncDecParams {
    uint8_t opcode;
    uint8_t expected_cycles_8;
    uint8_t expected_cycles_16;
    std::string mode;
    std::string instruction;
    std::string description;
};

// INC Test Class
class INCTest : public ::testing::TestWithParam<std::tuple<IncDecParams, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

// DEC Test Class
class DECTest : public ::testing::TestWithParam<std::tuple<IncDecParams, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

// Register INC/DEC Test Class
class RegisterIncDecTest : public ::testing::TestWithParam<std::tuple<IncDecParams, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

// INC Tests
TEST_P(INCTest, INC_AllModes) {
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

    // Set initial value
    uint16_t initial_value = is16 ? 0x1234 : 0x42;

    // Setup for each addressing mode
    if (params.mode == "Accumulator") {
        cpu->a = initial_value;
        bus->write(test_pc, opcode);
    } else if (params.mode == "Direct Page") {
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        bus->write(0xF0, initial_value & 0xFF);
        if (is16) bus->write(0xF1, (initial_value >> 8) & 0xFF);
    } else if (params.mode == "Direct Page,X") {
        cpu->x = 0x02;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        uint8_t addr = (0xF0 + 0x02) & 0xFF;
        bus->write(addr, initial_value & 0xFF);
        if (is16) bus->write((addr + 1) & 0xFF, (initial_value >> 8) & 0xFF);
    } else if (params.mode == "Absolute") {
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F0, initial_value & 0xFF);
        if (is16) bus->write(0x7E00F1, (initial_value >> 8) & 0xFF);
    } else if (params.mode == "Absolute,X") {
        cpu->x = 0x01;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F1, initial_value & 0xFF);
        if (is16) bus->write(0x7E00F2, (initial_value >> 8) & 0xFF);
    }

    cpu->step();

    // Calculate expected result
    uint16_t expected_result = (initial_value + 1) & (is16 ? 0xFFFF : 0xFF);

    // Verify result based on addressing mode
    if (params.mode == "Accumulator") {
        EXPECT_EQ(cpu->a, expected_result) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    } else if (params.mode == "Direct Page") {
        uint16_t result = bus->read(0xF0);
        if (is16) result |= (bus->read(0xF1) << 8);
        EXPECT_EQ(result, expected_result) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    } else if (params.mode == "Direct Page,X") {
        uint8_t addr = (0xF0 + 0x02) & 0xFF;
        uint16_t result = bus->read(addr);
        if (is16) result |= (bus->read((addr + 1) & 0xFF) << 8);
        EXPECT_EQ(result, expected_result) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    } else if (params.mode == "Absolute") {
        uint16_t result = bus->read(0x7E00F0);
        if (is16) result |= (bus->read(0x7E00F1) << 8);
        EXPECT_EQ(result, expected_result) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    } else if (params.mode == "Absolute,X") {
        uint16_t result = bus->read(0x7E00F1);
        if (is16) result |= (bus->read(0x7E00F2) << 8);
        EXPECT_EQ(result, expected_result) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    }

    EXPECT_EQ(cpu->cycles, expected_cycles) << "Instruction: " << params.instruction << " Mode: " << params.mode;

    // Verify zero flag
    uint16_t result_for_flags = is16 ? expected_result : (expected_result & 0xFF);
    EXPECT_EQ(cpu->get_flag(CPU::Z), result_for_flags == 0) << "Instruction: " << params.instruction << " Mode: " << params.mode;

    // Verify negative flag
    bool expected_negative = is16 ? ((result_for_flags & 0x8000) != 0) : ((result_for_flags & 0x80) != 0);
    EXPECT_EQ(cpu->get_flag(CPU::N), expected_negative) << "Instruction: " << params.instruction << " Mode: " << params.mode;
}

// DEC Tests
TEST_P(DECTest, DEC_AllModes) {
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

    // Set initial value
    uint16_t initial_value = is16 ? 0x1234 : 0x42;

    // Setup for each addressing mode
    if (params.mode == "Accumulator") {
        cpu->a = initial_value;
        bus->write(test_pc, opcode);
    } else if (params.mode == "Direct Page") {
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        bus->write(0xF0, initial_value & 0xFF);
        if (is16) bus->write(0xF1, (initial_value >> 8) & 0xFF);
    } else if (params.mode == "Direct Page,X") {
        cpu->x = 0x02;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        uint8_t addr = (0xF0 + 0x02) & 0xFF;
        bus->write(addr, initial_value & 0xFF);
        if (is16) bus->write((addr + 1) & 0xFF, (initial_value >> 8) & 0xFF);
    } else if (params.mode == "Absolute") {
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F0, initial_value & 0xFF);
        if (is16) bus->write(0x7E00F1, (initial_value >> 8) & 0xFF);
    } else if (params.mode == "Absolute,X") {
        cpu->x = 0x01;
        bus->write(test_pc, opcode);
        bus->write(test_pc + 1, 0xF0);
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F1, initial_value & 0xFF);
        if (is16) bus->write(0x7E00F2, (initial_value >> 8) & 0xFF);
    }

    cpu->step();

    // Calculate expected result
    uint16_t expected_result = (initial_value - 1) & (is16 ? 0xFFFF : 0xFF);

    // Verify result based on addressing mode
    if (params.mode == "Accumulator") {
        EXPECT_EQ(cpu->a, expected_result) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    } else if (params.mode == "Direct Page") {
        uint16_t result = bus->read(0xF0);
        if (is16) result |= (bus->read(0xF1) << 8);
        EXPECT_EQ(result, expected_result) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    } else if (params.mode == "Direct Page,X") {
        uint8_t addr = (0xF0 + 0x02) & 0xFF;
        uint16_t result = bus->read(addr);
        if (is16) result |= (bus->read((addr + 1) & 0xFF) << 8);
        EXPECT_EQ(result, expected_result) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    } else if (params.mode == "Absolute") {
        uint16_t result = bus->read(0x7E00F0);
        if (is16) result |= (bus->read(0x7E00F1) << 8);
        EXPECT_EQ(result, expected_result) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    } else if (params.mode == "Absolute,X") {
        uint16_t result = bus->read(0x7E00F1);
        if (is16) result |= (bus->read(0x7E00F2) << 8);
        EXPECT_EQ(result, expected_result) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    }

    EXPECT_EQ(cpu->cycles, expected_cycles) << "Instruction: " << params.instruction << " Mode: " << params.mode;

    // Verify zero flag
    uint16_t result_for_flags = is16 ? expected_result : (expected_result & 0xFF);
    EXPECT_EQ(cpu->get_flag(CPU::Z), result_for_flags == 0) << "Instruction: " << params.instruction << " Mode: " << params.mode;

    // Verify negative flag
    bool expected_negative = is16 ? ((result_for_flags & 0x8000) != 0) : ((result_for_flags & 0x80) != 0);
    EXPECT_EQ(cpu->get_flag(CPU::N), expected_negative) << "Instruction: " << params.instruction << " Mode: " << params.mode;
}

// Register INC/DEC Tests
TEST_P(RegisterIncDecTest, RegisterIncDec_AllModes) {
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

    // Set initial values
    uint16_t initial_x = is16 ? 0x1234 : 0x42;
    uint16_t initial_y = is16 ? 0x5678 : 0x84;
    cpu->x = initial_x;
    cpu->y = initial_y;

    // Write opcode
    bus->write(test_pc, opcode);

    cpu->step();

    // Calculate expected result based on instruction
    uint16_t expected_result = 0;
    if (params.instruction == "INX") {
        expected_result = (initial_x + 1) & (is16 ? 0xFFFF : 0xFF);
        EXPECT_EQ(cpu->x, expected_result) << "Instruction: " << params.instruction;
    } else if (params.instruction == "INY") {
        expected_result = (initial_y + 1) & (is16 ? 0xFFFF : 0xFF);
        EXPECT_EQ(cpu->y, expected_result) << "Instruction: " << params.instruction;
    } else if (params.instruction == "DEX") {
        expected_result = (initial_x - 1) & (is16 ? 0xFFFF : 0xFF);
        EXPECT_EQ(cpu->x, expected_result) << "Instruction: " << params.instruction;
    } else if (params.instruction == "DEY") {
        expected_result = (initial_y - 1) & (is16 ? 0xFFFF : 0xFF);
        EXPECT_EQ(cpu->y, expected_result) << "Instruction: " << params.instruction;
    }

    EXPECT_EQ(cpu->cycles, expected_cycles) << "Instruction: " << params.instruction;

    // Verify zero flag
    uint16_t result_for_flags = is16 ? expected_result : (expected_result & 0xFF);
    EXPECT_EQ(cpu->get_flag(CPU::Z), result_for_flags == 0) << "Instruction: " << params.instruction;

    // Verify negative flag
    bool expected_negative = is16 ? ((result_for_flags & 0x8000) != 0) : ((result_for_flags & 0x80) != 0);
    EXPECT_EQ(cpu->get_flag(CPU::N), expected_negative) << "Instruction: " << params.instruction;
}

// INC test parameters
std::vector<IncDecParams> inc_params = {
    {0x1A, 2, 2, "Accumulator", "INC", "Increment Accumulator"},
    {0xE6, 5, 6, "Direct Page", "INC", "Increment Direct Page"},
    {0xF6, 6, 7, "Direct Page,X", "INC", "Increment Direct Page,X"},
    {0xEE, 6, 7, "Absolute", "INC", "Increment Absolute"},
    {0xFE, 7, 8, "Absolute,X", "INC", "Increment Absolute,X"}
};

// DEC test parameters
std::vector<IncDecParams> dec_params = {
    {0x3A, 2, 2, "Accumulator", "DEC", "Decrement Accumulator"},
    {0xC6, 5, 6, "Direct Page", "DEC", "Decrement Direct Page"},
    {0xD6, 6, 7, "Direct Page,X", "DEC", "Decrement Direct Page,X"},
    {0xCE, 6, 7, "Absolute", "DEC", "Decrement Absolute"},
    {0xDE, 7, 8, "Absolute,X", "DEC", "Decrement Absolute,X"}
};

// Register INC/DEC test parameters
std::vector<IncDecParams> reg_incdec_params = {
    {0xE8, 2, 2, "Register", "INX", "Increment X"},
    {0xC8, 2, 2, "Register", "INY", "Increment Y"},
    {0xCA, 2, 2, "Register", "DEX", "Decrement X"},
    {0x88, 2, 2, "Register", "DEY", "Decrement Y"}
};

// INC test instantiation
INSTANTIATE_TEST_SUITE_P(
    INC_AllModes,
    INCTest,
    ::testing::Combine(
        ::testing::ValuesIn(inc_params),
        ::testing::Values(true, false) // 16-bit and 8-bit modes
    )
);

// DEC test instantiation
INSTANTIATE_TEST_SUITE_P(
    DEC_AllModes,
    DECTest,
    ::testing::Combine(
        ::testing::ValuesIn(dec_params),
        ::testing::Values(true, false) // 16-bit and 8-bit modes
    )
);

// Register INC/DEC test instantiation
INSTANTIATE_TEST_SUITE_P(
    RegisterIncDec_AllModes,
    RegisterIncDecTest,
    ::testing::Combine(
        ::testing::ValuesIn(reg_incdec_params),
        ::testing::Values(true, false) // 16-bit and 8-bit modes
    )
);

// Additional edge case tests
TEST_F(INCTest, INC_Overflow_Edge_Cases) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;

    // Test 8-bit overflow
    cpu->p |= CPU::M; // 8-bit mode
    cpu->a = 0xFF; // 255

    bus->write(test_pc, 0x1A); // INC accumulator

    cpu->step();

    EXPECT_EQ(cpu->a & 0xFF, 0x00); // Should wrap to 0
    EXPECT_TRUE(cpu->get_flag(CPU::Z)); // Should set zero
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Should clear negative
}

TEST_F(DECTest, DEC_Underflow_Edge_Cases) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;

    // Test 8-bit underflow
    cpu->p |= CPU::M; // 8-bit mode
    cpu->a = 0x00; // 0

    bus->write(test_pc, 0x3A); // DEC accumulator

    cpu->step();

    EXPECT_EQ(cpu->a & 0xFF, 0xFF); // Should wrap to 255
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Should clear zero
    EXPECT_TRUE(cpu->get_flag(CPU::N)); // Should set negative
}

TEST_F(RegisterIncDecTest, Register_16Bit_Overflow) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;

    // Test 16-bit overflow
    cpu->p &= ~CPU::X; // 16-bit mode
    cpu->x = 0xFFFF; // 65535

    bus->write(test_pc, 0xE8); // INX

    cpu->step();

    EXPECT_EQ(cpu->x, 0x0000); // Should wrap to 0
    EXPECT_TRUE(cpu->get_flag(CPU::Z)); // Should set zero
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Should clear negative
}
