#include "../include/cpu_addressing.hpp"
#include "../include/cpu.hpp"
#include "../include/bus.hpp"
#include <cstdint>

// Immediate Addressing
uint16_t CPUAddressing::immediate(CPU* cpu) {
    return cpu->bus->read(cpu->pc++);
}

uint16_t CPUAddressing::immediate_16(CPU* cpu) {
    uint16_t lo = cpu->bus->read(cpu->pc++);
    uint16_t hi = cpu->bus->read(cpu->pc++);
    return (hi << 8) | lo;
}

// Zero Page Addressing (8-bit addresses)
uint16_t CPUAddressing::zero_page(CPU* cpu) {
    return cpu->bus->read(cpu->pc++);
}

uint16_t CPUAddressing::zero_page_x(CPU* cpu) {
    return (cpu->bus->read(cpu->pc++) + cpu->x) & 0xFF;
}

uint16_t CPUAddressing::zero_page_y(CPU* cpu) {
    return (cpu->bus->read(cpu->pc++) + cpu->y) & 0xFF;
}

// Absolute Addressing (16-bit addresses)
uint16_t CPUAddressing::absolute(CPU* cpu) {
    uint16_t lo = cpu->bus->read(cpu->pc++);
    uint16_t hi = cpu->bus->read(cpu->pc++);
    return (hi << 8) | lo;
}

uint16_t CPUAddressing::absolute_x(CPU* cpu) {
    uint16_t lo = cpu->bus->read(cpu->pc++);
    uint16_t hi = cpu->bus->read(cpu->pc++);
    return (((hi << 8) | lo) + cpu->x) & 0xFFFF;
}

uint16_t CPUAddressing::absolute_y(CPU* cpu) {
    uint16_t lo = cpu->bus->read(cpu->pc++);
    uint16_t hi = cpu->bus->read(cpu->pc++);
    return (((hi << 8) | lo) + cpu->y) & 0xFFFF;
}

uint32_t CPUAddressing::absolute_long(CPU* cpu) {
    uint16_t lo = cpu->bus->read(cpu->pc++);
    uint16_t hi = cpu->bus->read(cpu->pc++);
    uint8_t bank = cpu->bus->read(cpu->pc++);
    return ((uint32_t)bank << 16) | (hi << 8) | lo;
}

uint32_t CPUAddressing::absolute_long_x(CPU* cpu) {
    uint16_t lo = cpu->bus->read(cpu->pc++);
    uint16_t hi = cpu->bus->read(cpu->pc++);
    uint8_t bank = cpu->bus->read(cpu->pc++);
    return (((uint32_t)bank << 16) | (hi << 8) | lo) + cpu->x;
}

// Direct Page Addressing (8-bit addresses with DP offset)
uint16_t CPUAddressing::direct_page(CPU* cpu) {
    return cpu->bus->read(cpu->pc++);
}

uint16_t CPUAddressing::direct_page_x(CPU* cpu) {
    return (cpu->bus->read(cpu->pc++) + cpu->x) & 0xFF;
}

uint16_t CPUAddressing::direct_page_y(CPU* cpu) {
    return (cpu->bus->read(cpu->pc++) + cpu->y) & 0xFF;
}

// Indirect Addressing
uint16_t CPUAddressing::direct_page_indexed_indirect_x(CPU* cpu) {
    uint8_t dp = cpu->bus->read(cpu->pc++);
    uint8_t ptr = (dp + cpu->x) & 0xFF;
    uint16_t lo = cpu->bus->read(ptr);
    uint16_t hi = cpu->bus->read((ptr + 1) & 0xFF);
    return (hi << 8) | lo;
}

uint16_t CPUAddressing::direct_page_indirect(CPU* cpu) {
    uint8_t dp = cpu->bus->read(cpu->pc++);
    uint16_t lo = cpu->bus->read(dp);
    uint16_t hi = cpu->bus->read((dp + 1) & 0xFF);
    return (hi << 8) | lo;
}

uint16_t CPUAddressing::direct_page_indirect_y(CPU* cpu) {
    uint8_t dp = cpu->bus->read(cpu->pc++);
    uint16_t lo = cpu->bus->read(dp);
    uint16_t hi = cpu->bus->read((dp + 1) & 0xFF);
    return (((hi << 8) | lo) + cpu->y) & 0xFFFF;
}

uint32_t CPUAddressing::direct_page_indirect_long(CPU* cpu) {
    uint8_t dp = cpu->bus->read(cpu->pc++);
    uint16_t ptr = dp;
    uint16_t lo = cpu->bus->read(ptr);
    uint16_t hi = cpu->bus->read((ptr + 1) & 0xFF);
    uint8_t bank = cpu->bus->read((ptr + 2) & 0xFF);
    return ((uint32_t)bank << 16) | (hi << 8) | lo;
}

uint32_t CPUAddressing::direct_page_indirect_long_y(CPU* cpu) {
    uint8_t dp = cpu->bus->read(cpu->pc++);
    uint16_t ptr = dp;
    uint16_t lo = cpu->bus->read(ptr);
    uint16_t hi = cpu->bus->read((ptr + 1) & 0xFF);
    uint8_t bank = cpu->bus->read((ptr + 2) & 0xFF);
    return (((uint32_t)bank << 16) | (hi << 8) | lo) + cpu->y;
}

uint16_t CPUAddressing::absolute_indirect(CPU* cpu) {
    uint16_t lo = cpu->bus->read(cpu->pc++);
    uint16_t hi = cpu->bus->read(cpu->pc++);
    uint16_t ptr = (hi << 8) | lo;
    uint16_t addr_lo = cpu->bus->read(ptr);
    uint16_t addr_hi = cpu->bus->read((ptr + 1) & 0xFFFF);
    return (addr_hi << 8) | addr_lo;
}

uint32_t CPUAddressing::absolute_indirect_long(CPU* cpu) {
    uint16_t lo = cpu->bus->read(cpu->pc++);
    uint16_t hi = cpu->bus->read(cpu->pc++);
    uint16_t ptr = (hi << 8) | lo;
    uint16_t addr_lo = cpu->bus->read(ptr);
    uint16_t addr_hi = cpu->bus->read((ptr + 1) & 0xFFFF);
    uint8_t addr_bank = cpu->bus->read((ptr + 2) & 0xFFFF);
    return ((uint32_t)addr_bank << 16) | (addr_hi << 8) | addr_lo;
}

// Stack Relative Addressing
uint16_t CPUAddressing::stack_relative(CPU* cpu) {
    uint8_t rel = cpu->bus->read(cpu->pc++);
    return (cpu->stkp + rel) & 0xFFFF;
}

uint16_t CPUAddressing::stack_relative_indirect_y(CPU* cpu) {
    uint8_t rel = cpu->bus->read(cpu->pc++);
    uint16_t ptr = (cpu->stkp + rel) & 0xFFFF;
    uint16_t lo = cpu->bus->read(ptr);
    uint16_t hi = cpu->bus->read((ptr + 1) & 0xFFFF);
    return (((hi << 8) | lo) + cpu->y) & 0xFFFF;
}

// Relative Addressing (for branches)
uint16_t CPUAddressing::relative(CPU* cpu) {
    uint8_t rel = cpu->bus->read(cpu->pc++);
    if (rel & 0x80) {
        rel |= 0xFF00; // Sign extend
    }
    return rel;
}

uint16_t CPUAddressing::relative_long(CPU* cpu) {
    uint8_t offset_lo = cpu->bus->read(cpu->pc++);
    uint8_t offset_hi = cpu->bus->read(cpu->pc++);
    uint16_t offset = (offset_hi << 8) | offset_lo;

    return offset;
}

// Block Move Addressing
uint16_t CPUAddressing::block_move(CPU* cpu) {
    // For MVP/MVN instructions
    uint8_t src_bank = cpu->bus->read(cpu->pc++);
    uint8_t dst_bank = cpu->bus->read(cpu->pc++);
    return (dst_bank << 8) | src_bank;
} 