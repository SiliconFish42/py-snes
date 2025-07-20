#include <cstdint>
#include <string>
#include "gtest/gtest.h"
#include "cpu.hpp"
#include "bus.hpp"
#include <tuple>

// Test structure for shift/rotate instructions
struct ShiftRotateParams {
    uint8_t opcode;
    uint8_t expected_cycles_8;
    uint8_t expected_cycles_16;
    std::string mode;
    std::string instruction;
    std::string description;
};

// ASL Test Class
class ASLTest : public ::testing::TestWithParam<std::tuple<ShiftRotateParams, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

// LSR Test Class
class LSRTest : public ::testing::TestWithParam<std::tuple<ShiftRotateParams, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

// ROL Test Class
class ROLTest : public ::testing::TestWithParam<std::tuple<ShiftRotateParams, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

// ROR Test Class
class RORTest : public ::testing::TestWithParam<std::tuple<ShiftRotateParams, bool>> {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }
};

// ASL Tests
TEST_P(ASLTest, ASL_AllModes) {
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
    
    // Set initial carry flag
    cpu->set_flag(CPU::C, false);
    
    // Setup for each addressing mode
    if (params.mode == "Accumulator") {
        bus->write(test_pc, opcode);
    } else if (params.mode == "Direct Page") {
        bus->write(test_pc, opcode); 
        bus->write(test_pc + 1, 0xF0);
        bus->write(0xF0, initial_a & 0xFF); 
        if (is16) bus->write(0xF1, (initial_a >> 8) & 0xFF);
    } else if (params.mode == "Direct Page,X") {
        cpu->x = 0x02; 
        bus->write(test_pc, opcode); 
        bus->write(test_pc + 1, 0xF0);
        uint8_t addr = (0xF0 + 0x02) & 0xFF;
        bus->write(addr, initial_a & 0xFF); 
        if (is16) bus->write((addr + 1) & 0xFF, (initial_a >> 8) & 0xFF);
    } else if (params.mode == "Absolute") {
        bus->write(test_pc, opcode); 
        bus->write(test_pc + 1, 0xF0); 
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F0, initial_a & 0xFF); 
        if (is16) bus->write(0x7E00F1, (initial_a >> 8) & 0xFF);
    } else if (params.mode == "Absolute,X") {
        cpu->x = 0x01; 
        bus->write(test_pc, opcode); 
        bus->write(test_pc + 1, 0xF0); 
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F1, initial_a & 0xFF); 
        if (is16) bus->write(0x7E00F2, (initial_a >> 8) & 0xFF);
    }
    
    cpu->step();
    
    // Calculate expected result (A << 1)
    uint16_t expected_result = initial_a << 1;
    if (is16) {
        expected_result &= 0xFFFF;
    } else {
        expected_result &= 0xFF;
    }
    
    EXPECT_EQ(cpu->cycles, expected_cycles) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    
    // For accumulator mode, check accumulator result
    if (params.mode == "Accumulator") {
        EXPECT_EQ(cpu->a, expected_result) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    }
    
    // Verify carry flag (bit 7/15 of original value)
    bool expected_carry = is16 ? ((initial_a & 0x8000) != 0) : ((initial_a & 0x80) != 0);
    EXPECT_EQ(cpu->get_flag(CPU::C), expected_carry) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    
    // Verify zero flag
    uint16_t result_for_flags = is16 ? expected_result : (expected_result & 0xFF);
    EXPECT_EQ(cpu->get_flag(CPU::Z), result_for_flags == 0) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    
    // Verify negative flag
    bool expected_negative = is16 ? ((result_for_flags & 0x8000) != 0) : ((result_for_flags & 0x80) != 0);
    EXPECT_EQ(cpu->get_flag(CPU::N), expected_negative) << "Instruction: " << params.instruction << " Mode: " << params.mode;
}

// LSR Tests
TEST_P(LSRTest, LSR_AllModes) {
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
    
    // Set initial carry flag
    cpu->set_flag(CPU::C, false);
    
    // Setup for each addressing mode
    if (params.mode == "Accumulator") {
        bus->write(test_pc, opcode);
    } else if (params.mode == "Direct Page") {
        bus->write(test_pc, opcode); 
        bus->write(test_pc + 1, 0xF0);
        bus->write(0xF0, initial_a & 0xFF); 
        if (is16) bus->write(0xF1, (initial_a >> 8) & 0xFF);
    } else if (params.mode == "Direct Page,X") {
        cpu->x = 0x02; 
        bus->write(test_pc, opcode); 
        bus->write(test_pc + 1, 0xF0);
        uint8_t addr = (0xF0 + 0x02) & 0xFF;
        bus->write(addr, initial_a & 0xFF); 
        if (is16) bus->write((addr + 1) & 0xFF, (initial_a >> 8) & 0xFF);
    } else if (params.mode == "Absolute") {
        bus->write(test_pc, opcode); 
        bus->write(test_pc + 1, 0xF0); 
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F0, initial_a & 0xFF); 
        if (is16) bus->write(0x7E00F1, (initial_a >> 8) & 0xFF);
    } else if (params.mode == "Absolute,X") {
        cpu->x = 0x01; 
        bus->write(test_pc, opcode); 
        bus->write(test_pc + 1, 0xF0); 
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F1, initial_a & 0xFF); 
        if (is16) bus->write(0x7E00F2, (initial_a >> 8) & 0xFF);
    }
    
    cpu->step();
    
    // Calculate expected result (A >> 1)
    uint16_t expected_result = initial_a >> 1;
    if (is16) {
        expected_result &= 0xFFFF;
    } else {
        expected_result &= 0xFF;
    }
    
    EXPECT_EQ(cpu->cycles, expected_cycles) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    
    // For accumulator mode, check accumulator result
    if (params.mode == "Accumulator") {
        EXPECT_EQ(cpu->a, expected_result) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    }
    
    // Verify carry flag (bit 0 of original value)
    bool expected_carry = (initial_a & 0x01) != 0;
    EXPECT_EQ(cpu->get_flag(CPU::C), expected_carry) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    
    // Verify zero flag
    uint16_t result_for_flags = is16 ? expected_result : (expected_result & 0xFF);
    EXPECT_EQ(cpu->get_flag(CPU::Z), result_for_flags == 0) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    
    // Verify negative flag (should always be false for LSR)
    EXPECT_FALSE(cpu->get_flag(CPU::N)) << "Instruction: " << params.instruction << " Mode: " << params.mode;
}

// ROL Tests
TEST_P(ROLTest, ROL_AllModes) {
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
    
    // Set initial carry flag
    cpu->set_flag(CPU::C, true);
    
    // Setup for each addressing mode
    if (params.mode == "Accumulator") {
        bus->write(test_pc, opcode);
    } else if (params.mode == "Direct Page") {
        bus->write(test_pc, opcode); 
        bus->write(test_pc + 1, 0xF0);
        bus->write(0xF0, initial_a & 0xFF); 
        if (is16) bus->write(0xF1, (initial_a >> 8) & 0xFF);
    } else if (params.mode == "Direct Page,X") {
        cpu->x = 0x02; 
        bus->write(test_pc, opcode); 
        bus->write(test_pc + 1, 0xF0);
        uint8_t addr = (0xF0 + 0x02) & 0xFF;
        bus->write(addr, initial_a & 0xFF); 
        if (is16) bus->write((addr + 1) & 0xFF, (initial_a >> 8) & 0xFF);
    } else if (params.mode == "Absolute") {
        bus->write(test_pc, opcode); 
        bus->write(test_pc + 1, 0xF0); 
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F0, initial_a & 0xFF); 
        if (is16) bus->write(0x7E00F1, (initial_a >> 8) & 0xFF);
    } else if (params.mode == "Absolute,X") {
        cpu->x = 0x01; 
        bus->write(test_pc, opcode); 
        bus->write(test_pc + 1, 0xF0); 
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F1, initial_a & 0xFF); 
        if (is16) bus->write(0x7E00F2, (initial_a >> 8) & 0xFF);
    }
    
    cpu->step();
    
    // Calculate expected result (A << 1) | carry
    uint16_t expected_result = (initial_a << 1) | 0x01; // Carry was set to true
    if (is16) {
        expected_result &= 0xFFFF;
    } else {
        expected_result &= 0xFF;
    }
    
    EXPECT_EQ(cpu->cycles, expected_cycles) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    
    // For accumulator mode, check accumulator result
    if (params.mode == "Accumulator") {
        EXPECT_EQ(cpu->a, expected_result) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    }
    
    // Verify carry flag (bit 7/15 of original value)
    bool expected_carry = is16 ? ((initial_a & 0x8000) != 0) : ((initial_a & 0x80) != 0);
    EXPECT_EQ(cpu->get_flag(CPU::C), expected_carry) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    
    // Verify zero flag
    uint16_t result_for_flags = is16 ? expected_result : (expected_result & 0xFF);
    EXPECT_EQ(cpu->get_flag(CPU::Z), result_for_flags == 0) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    
    // Verify negative flag
    bool expected_negative = is16 ? ((result_for_flags & 0x8000) != 0) : ((result_for_flags & 0x80) != 0);
    EXPECT_EQ(cpu->get_flag(CPU::N), expected_negative) << "Instruction: " << params.instruction << " Mode: " << params.mode;
}

// ROR Tests
TEST_P(RORTest, ROR_AllModes) {
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
    
    // Set initial carry flag
    cpu->set_flag(CPU::C, true);
    
    // Setup for each addressing mode
    if (params.mode == "Accumulator") {
        bus->write(test_pc, opcode);
    } else if (params.mode == "Direct Page") {
        bus->write(test_pc, opcode); 
        bus->write(test_pc + 1, 0xF0);
        bus->write(0xF0, initial_a & 0xFF); 
        if (is16) bus->write(0xF1, (initial_a >> 8) & 0xFF);
    } else if (params.mode == "Direct Page,X") {
        cpu->x = 0x02; 
        bus->write(test_pc, opcode); 
        bus->write(test_pc + 1, 0xF0);
        uint8_t addr = (0xF0 + 0x02) & 0xFF;
        bus->write(addr, initial_a & 0xFF); 
        if (is16) bus->write((addr + 1) & 0xFF, (initial_a >> 8) & 0xFF);
    } else if (params.mode == "Absolute") {
        bus->write(test_pc, opcode); 
        bus->write(test_pc + 1, 0xF0); 
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F0, initial_a & 0xFF); 
        if (is16) bus->write(0x7E00F1, (initial_a >> 8) & 0xFF);
    } else if (params.mode == "Absolute,X") {
        cpu->x = 0x01; 
        bus->write(test_pc, opcode); 
        bus->write(test_pc + 1, 0xF0); 
        bus->write(test_pc + 2, 0x00);
        bus->write(0x7E00F1, initial_a & 0xFF); 
        if (is16) bus->write(0x7E00F2, (initial_a >> 8) & 0xFF);
    }
    
    cpu->step();
    
    // Calculate expected result (A >> 1) | (carry << 7/15)
    uint16_t expected_result = (initial_a >> 1) | (is16 ? 0x8000 : 0x80); // Carry was set to true
    if (is16) {
        expected_result &= 0xFFFF;
    } else {
        expected_result &= 0xFF;
    }
    
    EXPECT_EQ(cpu->cycles, expected_cycles) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    
    // For accumulator mode, check accumulator result
    if (params.mode == "Accumulator") {
        EXPECT_EQ(cpu->a, expected_result) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    }
    
    // Verify carry flag (bit 0 of original value)
    bool expected_carry = (initial_a & 0x01) != 0;
    EXPECT_EQ(cpu->get_flag(CPU::C), expected_carry) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    
    // Verify zero flag
    uint16_t result_for_flags = is16 ? expected_result : (expected_result & 0xFF);
    EXPECT_EQ(cpu->get_flag(CPU::Z), result_for_flags == 0) << "Instruction: " << params.instruction << " Mode: " << params.mode;
    
    // Verify negative flag
    bool expected_negative = is16 ? ((result_for_flags & 0x8000) != 0) : ((result_for_flags & 0x80) != 0);
    EXPECT_EQ(cpu->get_flag(CPU::N), expected_negative) << "Instruction: " << params.instruction << " Mode: " << params.mode;
}

// ASL test parameters
std::vector<ShiftRotateParams> asl_params = {
    {0x0A, 2, 2, "Accumulator", "ASL", "Arithmetic Shift Left Accumulator"},
    {0x06, 5, 5, "Direct Page", "ASL", "Arithmetic Shift Left Direct Page"},
    {0x16, 6, 6, "Direct Page,X", "ASL", "Arithmetic Shift Left Direct Page,X"},
    {0x0E, 6, 6, "Absolute", "ASL", "Arithmetic Shift Left Absolute"},
    {0x1E, 7, 7, "Absolute,X", "ASL", "Arithmetic Shift Left Absolute,X"}
};

// LSR test parameters
std::vector<ShiftRotateParams> lsr_params = {
    {0x4A, 2, 2, "Accumulator", "LSR", "Logical Shift Right Accumulator"},
    {0x46, 5, 5, "Direct Page", "LSR", "Logical Shift Right Direct Page"},
    {0x56, 6, 6, "Direct Page,X", "LSR", "Logical Shift Right Direct Page,X"},
    {0x4E, 6, 6, "Absolute", "LSR", "Logical Shift Right Absolute"},
    {0x5E, 7, 7, "Absolute,X", "LSR", "Logical Shift Right Absolute,X"}
};

// ROL test parameters
std::vector<ShiftRotateParams> rol_params = {
    {0x2A, 2, 2, "Accumulator", "ROL", "Rotate Left Accumulator"},
    {0x26, 5, 5, "Direct Page", "ROL", "Rotate Left Direct Page"},
    {0x36, 6, 6, "Direct Page,X", "ROL", "Rotate Left Direct Page,X"},
    {0x2E, 6, 6, "Absolute", "ROL", "Rotate Left Absolute"},
    {0x3E, 7, 7, "Absolute,X", "ROL", "Rotate Left Absolute,X"}
};

// ROR test parameters
std::vector<ShiftRotateParams> ror_params = {
    {0x6A, 2, 2, "Accumulator", "ROR", "Rotate Right Accumulator"},
    {0x66, 5, 5, "Direct Page", "ROR", "Rotate Right Direct Page"},
    {0x76, 6, 6, "Direct Page,X", "ROR", "Rotate Right Direct Page,X"},
    {0x6E, 6, 6, "Absolute", "ROR", "Rotate Right Absolute"},
    {0x7E, 7, 7, "Absolute,X", "ROR", "Rotate Right Absolute,X"}
};

// ASL test instantiation
INSTANTIATE_TEST_SUITE_P(
    ASL_AllModes,
    ASLTest,
    ::testing::Combine(
        ::testing::ValuesIn(asl_params),
        ::testing::Values(true, false) // 16-bit and 8-bit modes
    )
);

// LSR test instantiation
INSTANTIATE_TEST_SUITE_P(
    LSR_AllModes,
    LSRTest,
    ::testing::Combine(
        ::testing::ValuesIn(lsr_params),
        ::testing::Values(true, false) // 16-bit and 8-bit modes
    )
);

// ROL test instantiation
INSTANTIATE_TEST_SUITE_P(
    ROL_AllModes,
    ROLTest,
    ::testing::Combine(
        ::testing::ValuesIn(rol_params),
        ::testing::Values(true, false) // 16-bit and 8-bit modes
    )
);

// ROR test instantiation
INSTANTIATE_TEST_SUITE_P(
    ROR_AllModes,
    RORTest,
    ::testing::Combine(
        ::testing::ValuesIn(ror_params),
        ::testing::Values(true, false) // 16-bit and 8-bit modes
    )
);

// Additional edge case tests
TEST_F(ASLTest, ASL_Zero_Result) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    // Test 8-bit ASL that results in zero
    cpu->p |= CPU::M; // 8-bit mode
    cpu->a = 0x00; // Zero
    cpu->set_flag(CPU::C, false);
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, true);
    
    bus->write(test_pc, 0x0A); // ASL accumulator
    
    cpu->step();
    
    EXPECT_EQ(cpu->a, 0x00); // Should remain zero
    EXPECT_TRUE(cpu->get_flag(CPU::Z)); // Should set zero
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Should clear negative
    EXPECT_FALSE(cpu->get_flag(CPU::C)); // Should clear carry
}

TEST_F(LSRTest, LSR_Carry_Set) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    // Test 8-bit LSR that sets carry
    cpu->p |= CPU::M; // 8-bit mode
    cpu->a = 0x81; // 10000001
    cpu->set_flag(CPU::C, false);
    cpu->set_flag(CPU::Z, true);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x4A); // LSR accumulator
    
    cpu->step();
    
    EXPECT_EQ(cpu->a, 0x40); // Should be 01000000
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Should clear zero
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Should clear negative
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Should set carry
}

TEST_F(ROLTest, ROL_With_Carry) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    // Test 8-bit ROL with carry set
    cpu->p |= CPU::M; // 8-bit mode
    cpu->a = 0x80; // 10000000
    cpu->set_flag(CPU::C, true);
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x2A); // ROL accumulator
    
    cpu->step();
    
    EXPECT_EQ(cpu->a, 0x01); // Should be 00000001 (carry becomes bit 0)
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Should clear zero
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Should clear negative
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Should set carry (original bit 7)
}

TEST_F(RORTest, ROR_16Bit_Operation) {
    uint32_t test_pc = 0x7E0000;
    cpu->reset();
    cpu->pc = test_pc;
    
    // Test 16-bit ROR operation
    cpu->p &= ~CPU::M; // 16-bit mode
    cpu->a = 0x8001; // 10000000 00000001
    cpu->set_flag(CPU::C, false);
    cpu->set_flag(CPU::Z, false);
    cpu->set_flag(CPU::N, false);
    
    bus->write(test_pc, 0x6A); // ROR accumulator
    
    cpu->step();
    
    EXPECT_EQ(cpu->a, 0x4000); // Should be 01000000 00000000
    EXPECT_FALSE(cpu->get_flag(CPU::Z)); // Should clear zero
    EXPECT_FALSE(cpu->get_flag(CPU::N)); // Should clear negative
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Should set carry (original bit 0)
} 