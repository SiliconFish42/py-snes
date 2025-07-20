#pragma once

#include <cstdint>

// Forward declaration
class CPU;

// 65816 CPU Helper Functions
class CPUHelpers {
public:
    // Flag manipulation
    static void setZN(CPU* cpu, uint16_t value, bool is16);
    static void set_flag(CPU* cpu, uint8_t flag, bool value);
    static bool get_flag(CPU* cpu, uint8_t flag);

    // Stack operations
    static void push_8(CPU* cpu, uint8_t value);
    static void push_16(CPU* cpu, uint16_t value);
    static uint8_t pop_8(CPU* cpu);
    static uint16_t pop_16(CPU* cpu);
    static void validate_stack_pointer(CPU* cpu);

    // Memory operations
    static uint8_t read_8(CPU* cpu, uint32_t address);
    static uint16_t read_16(CPU* cpu, uint32_t address);
    static void write_8(CPU* cpu, uint32_t address, uint8_t value);
    static void write_16(CPU* cpu, uint32_t address, uint16_t value);

    // Utility functions
    static void log_instruction(CPU* cpu, const char* instruction, uint32_t address);
    static void validate_address(CPU* cpu, uint32_t address);
    static bool is_emulation_mode(CPU* cpu);
    static bool is_16bit_accumulator(CPU* cpu);
    static bool is_16bit_index(CPU* cpu);

    // Interrupt handling
    static void handle_interrupt(CPU* cpu, uint16_t vector_low, uint16_t vector_high);
    static void handle_irq(CPU* cpu);
    static void handle_nmi(CPU* cpu);
    static void handle_reset(CPU* cpu);

    // Cycle counting
    static void add_cycles(CPU* cpu, uint8_t cycles);
    static void add_page_cross_penalty(CPU* cpu, uint32_t old_addr, uint32_t new_addr);
};
