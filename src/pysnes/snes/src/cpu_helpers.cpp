#include "../include/cpu_helpers.hpp"
#include "../include/cpu.hpp"
#include "../include/bus.hpp"
#include <cstdio>

// Flag manipulation
void CPUHelpers::setZN(CPU* cpu, uint16_t value, bool is16) {
    cpu->set_flag(CPU::Z, value == 0);
    cpu->set_flag(CPU::N, is16 ? (value & 0x8000) : (value & 0x80));
}

void CPUHelpers::set_flag(CPU* cpu, uint8_t flag, bool value) {
    cpu->set_flag(static_cast<CPU::FLAGS>(flag), value);
}

bool CPUHelpers::get_flag(CPU* cpu, uint8_t flag) {
    return cpu->get_flag(static_cast<CPU::FLAGS>(flag));
}

// Stack operations
void CPUHelpers::push_8(CPU* cpu, uint8_t value) {
    cpu->stkp--;
    if (cpu->stkp < 0x0100) cpu->stkp = 0x01FF; // Wrap around if underflow
    uint32_t addr = 0x0100 + (cpu->stkp & 0xFF);
    cpu->bus->write(addr, value);
}

void CPUHelpers::push_16(CPU* cpu, uint16_t value) {
    push_8(cpu, (value >> 8) & 0xFF); // high byte first
    push_8(cpu, value & 0xFF);        // low byte second
}

uint8_t CPUHelpers::pop_8(CPU* cpu) {
    uint32_t addr = 0x0100 + (cpu->stkp & 0xFF);
    uint8_t value = cpu->bus->read(addr);
    cpu->stkp++;
    if (cpu->stkp > 0x01FF) cpu->stkp = 0x0100; // Wrap around if overflow
    return value;
}

uint16_t CPUHelpers::pop_16(CPU* cpu) {
    uint16_t lo = pop_8(cpu);
    uint16_t hi = pop_8(cpu);
    uint16_t result = (hi << 8) | lo;
    return result;
}

void CPUHelpers::validate_stack_pointer(CPU* cpu) {
    // Ensure stack pointer stays within bounds
    if (cpu->stkp < 0x0100) {
        cpu->stkp = 0x01FF;
    } else if (cpu->stkp > 0x01FF) {
        cpu->stkp = 0x0100;
    }
}

// Memory operations
uint8_t CPUHelpers::read_8(CPU* cpu, uint32_t address) {
    return cpu->bus->read(address & 0xFFFF);
}

uint16_t CPUHelpers::read_16(CPU* cpu, uint32_t address) {
    uint16_t lo = read_8(cpu, address);
    uint16_t hi = read_8(cpu, address + 1);
    return (hi << 8) | lo;
}

void CPUHelpers::write_8(CPU* cpu, uint32_t address, uint8_t value) {
    cpu->bus->write(address & 0xFFFF, value);
}

void CPUHelpers::write_16(CPU* cpu, uint32_t address, uint16_t value) {
    write_8(cpu, address, value & 0xFF);
    write_8(cpu, address + 1, (value >> 8) & 0xFF);
}

// Utility functions
void CPUHelpers::log_instruction(CPU* cpu, const char* instruction, uint32_t address) {
    printf("DEBUG: %s at PC %06X, Address %04X\n", instruction, cpu->pc, address);
}

void CPUHelpers::validate_address(CPU* cpu, uint32_t address) {
    // Basic address validation - could be expanded
    if (address > 0xFFFFFF) {
        printf("WARNING: Invalid address %06X\n", address);
    }
}

bool CPUHelpers::is_emulation_mode(CPU* cpu) {
    return cpu->get_flag(CPU::E);
}

bool CPUHelpers::is_16bit_accumulator(CPU* cpu) {
    return !cpu->get_flag(CPU::M);
}

bool CPUHelpers::is_16bit_index(CPU* cpu) {
    return !cpu->get_flag(CPU::X);
}

// Interrupt handling
void CPUHelpers::handle_interrupt(CPU* cpu, uint16_t vector_low, uint16_t vector_high) {
    // Push processor status and return address
    push_16(cpu, cpu->pc);
    push_8(cpu, cpu->p & 0xFF);
    
    // Set interrupt disable flag
    cpu->set_flag(CPU::I, true);
    
    // Jump to interrupt vector
    uint16_t lo = cpu->bus->read(vector_low);
    uint16_t hi = cpu->bus->read(vector_high);
    cpu->pc = (hi << 8) | lo;
    
    // Prevent infinite loops by checking if we're jumping to unmapped memory
    if (cpu->pc == 0x0000 || (lo == 0x00 && hi == 0x00)) {
        // If interrupt vector is invalid, jump to a safe location
        cpu->pc = 0x8000; // Reset to ROM start
    }
}

void CPUHelpers::handle_irq(CPU* cpu) {
    handle_interrupt(cpu, 0xFFFE, 0xFFFF);
}

void CPUHelpers::handle_nmi(CPU* cpu) {
    handle_interrupt(cpu, 0xFFFA, 0xFFFB);
}

void CPUHelpers::handle_reset(CPU* cpu) {
    uint16_t lo = cpu->bus->read(0xFFFC);
    uint16_t hi = cpu->bus->read(0xFFFD);
    cpu->pc = (hi << 8) | lo;
    
    // Prevent infinite loops
    if (cpu->pc == 0x0000 || (lo == 0x00 && hi == 0x00)) {
        cpu->pc = 0x8000;
    }
}

// Cycle counting
void CPUHelpers::add_cycles(CPU* cpu, uint8_t cycles) {
    cpu->cycles += cycles;
}

void CPUHelpers::add_page_cross_penalty(CPU* cpu, uint32_t old_addr, uint32_t new_addr) {
    // Add penalty if crossing page boundary
    if ((old_addr & 0xFF00) != (new_addr & 0xFF00)) {
        cpu->cycles++;
    }
} 