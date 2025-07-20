#include <cstdint>
#include <string>
#include "gtest/gtest.h"
#include "cpu.hpp"
#include "bus.hpp"
#include <tuple>

// Test structure for arithmetic instructions
struct ArithmeticParams {
    uint8_t opcode;
    uint8_t expected_cycles_8;
    uint8_t expected_cycles_16;
    std::string mode;
    std::string instruction;
    std::string description;
};

// ADC Test Class
class ADCTest : public ::testing::TestWithParam<std::tuple<ArithmeticParams, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

// SBC Test Class
class SBCTest : public ::testing::TestWithParam<std::tuple<ArithmeticParams, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

// ADC Tests
TEST_P(ADCTest, ADC_AllModes) {
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
    
    // Set carry flag for testing
    cpu->set_flag(CPU::C, true);
    
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
    
    // Calculate expected result
    uint32_t expected_result = initial_a + test_operand + 1; // +1 for carry flag
    if (is16) {
        expected_result &= 0xFFFF;
    } else {
        expected_result = (initial_a & 0xFF00) | (expected_result & 0xFF);
    }
    
    // Verify result
    EXPECT_EQ(cpu->a, expected_result) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    EXPECT_EQ(cpu->cycles, expected_cycles) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    
    // Verify flags
    bool expected_carry = (initial_a + test_operand + 1) > (is16 ? 0xFFFF : 0xFF);
    EXPECT_EQ(cpu->get_flag(CPU::C), expected_carry) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    
    // Verify zero flag
    uint16_t result_for_flags = is16 ? cpu->a : (cpu->a & 0xFF);
    EXPECT_EQ(cpu->get_flag(CPU::Z), result_for_flags == 0) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    
    // Verify negative flag
    bool expected_negative = is16 ? ((result_for_flags & 0x8000) != 0) : ((result_for_flags & 0x80) != 0);
    EXPECT_EQ(cpu->get_flag(CPU::N), expected_negative) << "Instruction: " << params.instruction << " Mode: " << params.mode;
}

// SBC Tests
TEST_P(SBCTest, SBC_AllModes) {
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
    uint16_t initial_a = is16 ? 0x5678 : 0x84;
    cpu->a = initial_a;
    
    // Set carry flag for testing (1 means no borrow)
    cpu->set_flag(CPU::C, true);
    
    // Write test operand value
    uint16_t test_operand = is16 ? 0x1234 : 0x42;
    
    // Setup for each addressing mode (same as ADC)
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
    
    // Calculate expected result (A - operand - (1 - carry))
    uint32_t expected_result = initial_a - test_operand - 0; // carry=1 means no borrow
    if (is16) {
        expected_result &= 0xFFFF;
    } else {
        expected_result = (initial_a & 0xFF00) | (expected_result & 0xFF);
    }
    
    // Verify result
    EXPECT_EQ(cpu->a, expected_result) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    EXPECT_EQ(cpu->cycles, expected_cycles) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    
    // Verify carry flag (1 if no borrow occurred)
    bool expected_carry = initial_a >= test_operand;
    EXPECT_EQ(cpu->get_flag(CPU::C), expected_carry) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    
    // Verify zero flag
    uint16_t result_for_flags = is16 ? cpu->a : (cpu->a & 0xFF);
    EXPECT_EQ(cpu->get_flag(CPU::Z), result_for_flags == 0) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    
    // Verify negative flag
    bool expected_negative = is16 ? ((result_for_flags & 0x8000) != 0) : ((result_for_flags & 0x80) != 0);
    EXPECT_EQ(cpu->get_flag(CPU::N), expected_negative) << "Instruction: " << params.instruction << " Mode: " << params.mode;
}

// ADC test parameters
std::vector<ArithmeticParams> adc_params = {
    {0x69, 2, 3, "Immediate", "ADC", "Add with Carry Immediate"},
    {0x65, 3, 4, "Direct Page", "ADC", "Add with Carry Direct Page"},
    {0x75, 4, 5, "Direct Page,X", "ADC", "Add with Carry Direct Page,X"},
    {0x6D, 4, 5, "Absolute", "ADC", "Add with Carry Absolute"},
    {0x7D, 5, 6, "Absolute,X", "ADC", "Add with Carry Absolute,X"},
    {0x79, 5, 6, "Absolute,Y", "ADC", "Add with Carry Absolute,Y"},
    {0x61, 6, 7, "(DP,X)", "ADC", "Add with Carry (DP,X)"},
    {0x71, 6, 7, "(DP),Y", "ADC", "Add with Carry (DP),Y"},
    {0x72, 5, 6, "(DP)", "ADC", "Add with Carry (DP)"},
    {0x67, 6, 7, "[DP]", "ADC", "Add with Carry [DP]"},
    {0x77, 7, 8, "[DP],Y", "ADC", "Add with Carry [DP],Y"}
};

// SBC test parameters
std::vector<ArithmeticParams> sbc_params = {
    {0xE9, 2, 3, "Immediate", "SBC", "Subtract with Carry Immediate"},
    {0xE5, 3, 4, "Direct Page", "SBC", "Subtract with Carry Direct Page"},
    {0xF5, 4, 5, "Direct Page,X", "SBC", "Subtract with Carry Direct Page,X"},
    {0xED, 4, 5, "Absolute", "SBC", "Subtract with Carry Absolute"},
    {0xFD, 5, 6, "Absolute,X", "SBC", "Subtract with Carry Absolute,X"},
    {0xF9, 5, 6, "Absolute,Y", "SBC", "Subtract with Carry Absolute,Y"},
    {0xE1, 6, 7, "(DP,X)", "SBC", "Subtract with Carry (DP,X)"},
    {0xF1, 6, 7, "(DP),Y", "SBC", "Subtract with Carry (DP),Y"},
    {0xF2, 5, 6, "(DP)", "SBC", "Subtract with Carry (DP)"},
    {0xE7, 6, 7, "[DP]", "SBC", "Subtract with Carry [DP]"},
    {0xF7, 7, 8, "[DP],Y", "SBC", "Subtract with Carry [DP],Y"}
};

// ADC test instantiation
INSTANTIATE_TEST_SUITE_P(
    ADC_AllModes,
    ADCTest,
    ::testing::Combine(
        ::testing::ValuesIn(adc_params),
        ::testing::Values(true, false) // 16-bit and 8-bit modes
    )
);

// SBC test instantiation
INSTANTIATE_TEST_SUITE_P(
    SBC_AllModes,
    SBCTest,
    ::testing::Combine(
        ::testing::ValuesIn(sbc_params),
        ::testing::Values(true, false) // 16-bit and 8-bit modes
    )
);

// Additional edge case tests
TEST_F(ADCTest, ADC_Overflow_Edge_Cases) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    // Test 8-bit overflow
    cpu->p |= CPU::M; // 8-bit mode
    cpu->a = 0x80; // -128
    cpu->set_flag(CPU::C, false);
    
    bus->write(test_pc, 0x69); // ADC immediate
    bus->write(test_pc + 1, 0x80); // Add -128
    
    cpu->step();
    
    EXPECT_EQ(cpu->a & 0xFF, 0x00); // Should be 0
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Should set carry
    EXPECT_TRUE(cpu->get_flag(CPU::Z)); // Should set zero
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Should clear negative
}

TEST_F(SBCTest, SBC_Borrow_Edge_Cases) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    // Test 8-bit borrow
    cpu->p |= CPU::M; // 8-bit mode
    cpu->a = 0x00; // 0
    cpu->set_flag(CPU::C, false); // No carry = borrow
    
    bus->write(test_pc, 0xE9); // SBC immediate
    bus->write(test_pc + 1, 0x01); // Subtract 1
    
    cpu->step();
    
    EXPECT_EQ(cpu->a & 0xFF, 0xFE); // Should be -2 (0xFE) since 0x00 - 0x01 - 1 = 0xFE
    EXPECT_FALSE(cpu->get_flag(CPU::C)); // Should clear carry (borrow occurred)
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Should clear zero
    EXPECT_TRUE(cpu->get_flag(CPU::N)); // Should set negative
}

TEST_F(ADCTest, ADC_16Bit_Overflow) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    // Test 16-bit overflow
    cpu->p &= ~CPU::M; // 16-bit mode
    cpu->a = 0x8000; // -32768
    cpu->set_flag(CPU::C, false);
    
    bus->write(test_pc, 0x69); // ADC immediate
    bus->write(test_pc + 1, 0x00); // Low byte
    bus->write(test_pc + 2, 0x80); // High byte (-32768)
    
    cpu->step();
    
    EXPECT_EQ(cpu->a, 0x0000); // Should be 0
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Should set carry
    EXPECT_TRUE(cpu->get_flag(CPU::Z)); // Should set zero
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Should clear negative
} 