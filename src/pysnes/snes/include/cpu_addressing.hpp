#pragma once
#include <cstdint>

// Forward declaration
class CPU;

// 65816 Addressing Modes
class CPUAddressing {
public:
    // Immediate Addressing
    static uint16_t immediate(CPU* cpu);
    static uint16_t immediate_16(CPU* cpu);
    
    // Zero Page Addressing (8-bit addresses)
    static uint16_t zero_page(CPU* cpu);
    static uint16_t zero_page_x(CPU* cpu);
    static uint16_t zero_page_y(CPU* cpu);
    
    // Absolute Addressing (16-bit addresses)
    static uint16_t absolute(CPU* cpu);
    static uint16_t absolute_x(CPU* cpu);
    static uint16_t absolute_y(CPU* cpu);
    static uint32_t absolute_long(CPU* cpu);
    static uint32_t absolute_long_x(CPU* cpu);
    
    // Direct Page Addressing (8-bit addresses with DP offset)
    static uint16_t direct_page(CPU* cpu);
    static uint16_t direct_page_x(CPU* cpu);
    static uint16_t direct_page_y(CPU* cpu);
    
    // Indirect Addressing
    static uint16_t direct_page_indexed_indirect_x(CPU* cpu);
    static uint16_t direct_page_indirect(CPU* cpu);
    static uint16_t direct_page_indirect_y(CPU* cpu);
    static uint32_t direct_page_indirect_long(CPU* cpu);
    static uint32_t direct_page_indirect_long_y(CPU* cpu);
    static uint16_t absolute_indirect(CPU* cpu);
    static uint32_t absolute_indirect_long(CPU* cpu);
    
    // Stack Relative Addressing
    static uint16_t stack_relative(CPU* cpu);
    static uint16_t stack_relative_indirect_y(CPU* cpu);
    
    // Relative Addressing (for branches)
    static uint16_t relative(CPU* cpu);
    static uint16_t relative_long(CPU* cpu);
    
    // Block Move Addressing
    static uint16_t block_move(CPU* cpu);
}; 