#include "../include/cpu_instructions.hpp"
#include "../include/cpu.hpp"
#include "../include/cpu_helpers.hpp"
#include "../include/cpu_addressing.hpp"
#include "../include/bus.hpp"
#include <cstdio>

// Control Instructions
void CPUInstructions::brk(CPU* cpu) {
    // Push PC and P to stack (PC was already incremented after fetching opcode)
    CPUHelpers::push_16(cpu, cpu->pc);
    CPUHelpers::push_8(cpu, (cpu->p | 0x10) & 0xFF); // Set B flag
    
    // Set I flag and jump to interrupt vector
    CPUHelpers::set_flag(cpu, CPU::I, true);
    uint16_t lo = cpu->bus->read(0xFFFE);
    uint16_t hi = cpu->bus->read(0xFFFF);
    cpu->pc = (hi << 8) | lo;
    
    // Prevent infinite BRK loops by checking if we're jumping to unmapped memory
    if (cpu->pc == 0x0000 || (lo == 0x00 && hi == 0x00)) {
        // If interrupt vector is invalid, jump to a safe location
        cpu->pc = 0x8004; // Reset to ROM start + 4 to avoid infinite loop
    }
    
    cpu->cycles = 7;
}

void CPUInstructions::nop(CPU* cpu) {
    cpu->cycles = 2;
}

void CPUInstructions::jmp_absolute(CPU* cpu) {
    uint16_t lo = cpu->bus->read(cpu->pc++);
    uint16_t hi = cpu->bus->read(cpu->pc++);
    cpu->pc = (hi << 8) | lo;
    cpu->cycles = 3;
}

void CPUInstructions::jmp_absolute_long(CPU* cpu) {
    uint16_t lo = cpu->bus->read(cpu->pc++);
    uint16_t hi = cpu->bus->read(cpu->pc++);
    uint8_t bank = cpu->bus->read(cpu->pc++);
    cpu->pc = ((uint32_t)bank << 16) | (hi << 8) | lo;
    cpu->cycles = 4;
}

void CPUInstructions::jmp_absolute_indirect(CPU* cpu) {
    uint16_t lo = cpu->bus->read(cpu->pc++);
    uint16_t hi = cpu->bus->read(cpu->pc++);
    uint16_t ptr = (hi << 8) | lo;
    uint16_t addr_lo = cpu->bus->read(ptr);
    uint16_t addr_hi = cpu->bus->read((ptr + 1) & 0xFFFF);
    cpu->pc = (addr_hi << 8) | addr_lo;
    cpu->cycles = 5;
}

void CPUInstructions::jmp_absolute_indirect_long(CPU* cpu) {
    uint16_t lo = cpu->bus->read(cpu->pc++);
    uint16_t hi = cpu->bus->read(cpu->pc++);
    uint16_t ptr = (hi << 8) | lo;
    uint16_t addr_lo = cpu->bus->read(ptr);
    uint16_t addr_hi = cpu->bus->read((ptr + 1) & 0xFFFF);
    uint8_t addr_bank = cpu->bus->read((ptr + 2) & 0xFFFF);
    cpu->pc = ((uint32_t)addr_bank << 16) | (addr_hi << 8) | addr_lo;
    cpu->cycles = 6;
}

void CPUInstructions::jmp_absolute_indirect_x(CPU* cpu) {
    uint16_t lo = cpu->bus->read(cpu->pc++);
    uint16_t hi = cpu->bus->read(cpu->pc++);
    uint16_t ptr = ((hi << 8) | lo) + cpu->x;
    uint16_t addr_lo = cpu->bus->read(ptr);
    uint16_t addr_hi = cpu->bus->read((ptr + 1) & 0xFFFF);
    cpu->pc = (addr_hi << 8) | addr_lo;
    cpu->cycles = 6;
}

void CPUInstructions::jsr(CPU* cpu) {
    uint16_t lo = cpu->bus->read(cpu->pc++);
    uint16_t hi = cpu->bus->read(cpu->pc++);
    uint16_t ret_addr = cpu->pc - 1;
    printf("[JSR] Pushing return address: %04X (PC before JSR: %06X, after fetch: %06X)\n", ret_addr, cpu->pc - 3, cpu->pc);
    printf("[JSR] Stack pointer before push: %04X\n", cpu->stkp);
    CPUHelpers::push_16(cpu, ret_addr);
    printf("[JSR] Stack pointer after push: %04X\n", cpu->stkp);
    uint32_t target_addr = ((uint32_t)cpu->pb << 16) | (hi << 8) | lo;
    cpu->pc = target_addr;
    printf("[JSR] Jumping to target address: %06X\n", cpu->pc);
    cpu->cycles = 6;
}

void CPUInstructions::jsr_absolute_long(CPU* cpu) {
    uint16_t lo = cpu->bus->read(cpu->pc++);
    uint16_t hi = cpu->bus->read(cpu->pc++);
    uint8_t bank = cpu->bus->read(cpu->pc++);
    uint16_t ret_addr = cpu->pc - 1;
    uint8_t ret_bank = cpu->pb;
    CPUHelpers::push_16(cpu, ret_addr);
    CPUHelpers::push_8(cpu, ret_bank);
    cpu->pb = bank;
    cpu->pc = ((uint32_t)bank << 16) | (hi << 8) | lo;
    cpu->cycles = 8;
}

void CPUInstructions::rts(CPU* cpu) {
    uint16_t return_addr = CPUHelpers::pop_16(cpu);
    cpu->pc = ((uint32_t)cpu->pb << 16) | ((return_addr + 1) & 0xFFFF);
    cpu->cycles = 6;
}

void CPUInstructions::rtl(CPU* cpu) {
    uint8_t return_bank = CPUHelpers::pop_8(cpu);
    uint16_t return_addr = CPUHelpers::pop_16(cpu);
    cpu->pb = return_bank;
    cpu->pc = ((uint32_t)return_bank << 16) | (return_addr + 1);
    cpu->cycles = 6;
}

void CPUInstructions::rti(CPU* cpu) {
    uint8_t status = CPUHelpers::pop_8(cpu);
    uint16_t return_addr = CPUHelpers::pop_16(cpu);
    cpu->p = (cpu->p & 0xFF00) | status;
    cpu->pc = return_addr;
    cpu->cycles = 6;
}

void CPUInstructions::wai(CPU* cpu) {
    // Wait for interrupt - simplified implementation
    cpu->cycles = 3;
}

void CPUInstructions::stp(CPU* cpu) {
    // Stop processor - simplified implementation
    cpu->cycles = 3;
}

// Flag Instructions
void CPUInstructions::sei(CPU* cpu) {
    CPUHelpers::set_flag(cpu, CPU::I, true);
    cpu->cycles = 2;
}

void CPUInstructions::cli(CPU* cpu) {
    CPUHelpers::set_flag(cpu, CPU::I, false);
    cpu->cycles = 2;
}

void CPUInstructions::sec(CPU* cpu) {
    CPUHelpers::set_flag(cpu, CPU::C, true);
    cpu->cycles = 2;
}

void CPUInstructions::clc(CPU* cpu) {
    CPUHelpers::set_flag(cpu, CPU::C, false);
    cpu->cycles = 2;
}

void CPUInstructions::sed(CPU* cpu) {
    CPUHelpers::set_flag(cpu, CPU::D, true);
    cpu->cycles = 2;
}

void CPUInstructions::cld(CPU* cpu) {
    CPUHelpers::set_flag(cpu, CPU::D, false);
    cpu->cycles = 2;
}

void CPUInstructions::clv(CPU* cpu) {
    CPUHelpers::set_flag(cpu, CPU::V, false);
    cpu->cycles = 2;
}

void CPUInstructions::xce(CPU* cpu) {
    // Exchange Carry and Emulation flags
    bool carry = cpu->get_flag(CPU::C);
    bool emulation = cpu->get_flag(CPU::E);
    cpu->set_flag(CPU::C, emulation);
    cpu->set_flag(CPU::E, carry);
    cpu->cycles = 2;
}

// Load Instructions
void CPUInstructions::lda_immediate(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(cpu->pc++);
        uint16_t hi = cpu->bus->read(cpu->pc++);
        cpu->a = (hi << 8) | lo;
        cpu->cycles = 3;
    } else {
        cpu->a = cpu->bus->read(cpu->pc++);
        cpu->cycles = 2;
    }
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::lda_direct_page(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        cpu->a = (hi << 8) | lo;
        cpu->cycles = 4;
    } else {
        cpu->a = cpu->bus->read(addr);
        cpu->cycles = 3;
    }
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::lda_direct_page_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        cpu->a = (hi << 8) | lo;
        cpu->cycles = 5;
    } else {
        cpu->a = cpu->bus->read(addr);
        cpu->cycles = 4;
    }
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::lda_absolute(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        cpu->a = (hi << 8) | lo;
        cpu->cycles = 5;
    } else {
        cpu->a = cpu->bus->read(addr);
        cpu->cycles = 4;
    }
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::lda_absolute_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        cpu->a = (hi << 8) | lo;
        cpu->cycles = 5;
    } else {
        cpu->a = cpu->bus->read(addr);
        cpu->cycles = 4;
    }
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::lda_absolute_y(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        cpu->a = (hi << 8) | lo;
        cpu->cycles = 5;
    } else {
        cpu->a = cpu->bus->read(addr);
        cpu->cycles = 4;
    }
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::lda_dp_indirect_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indexed_indirect_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        cpu->a = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        cpu->a = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::lda_dp_indirect_y(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indirect_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        cpu->a = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        cpu->a = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::lda_dp_indirect(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indirect(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        cpu->a = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        cpu->a = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::lda_dp_indirect_long(CPU* cpu) {
    uint32_t addr = CPUAddressing::direct_page_indirect_long(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFFFF);
        cpu->a = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        cpu->a = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::lda_dp_indirect_long_y(CPU* cpu) {
    uint32_t addr = CPUAddressing::direct_page_indirect_long_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFFFF);
        cpu->a = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        cpu->a = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::lda_absolute_long(CPU* cpu) {
    uint32_t addr = CPUAddressing::absolute_long(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFFFF);
        cpu->a = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        cpu->a = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::lda_absolute_long_x(CPU* cpu) {
    uint32_t addr = CPUAddressing::absolute_long_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFFFF);
        cpu->a = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        cpu->a = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::lda_stack_relative(CPU* cpu) {
    uint16_t addr = CPUAddressing::stack_relative(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        cpu->a = (hi << 8) | lo;
        cpu->cycles = 5;
    } else {
        cpu->a = cpu->bus->read(addr);
        cpu->cycles = 4;
    }
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::lda_stack_relative_indirect_y(CPU* cpu) {
    uint16_t addr = CPUAddressing::stack_relative_indirect_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        cpu->a = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        cpu->a = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    cpu->setZN(cpu->a, is16);
}

// Store Instructions
void CPUInstructions::sta_direct_page(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFF, (cpu->a >> 8) & 0xFF);
        cpu->cycles = 4;
    } else {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->cycles = 3;
    }
}

void CPUInstructions::sta_direct_page_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFF, (cpu->a >> 8) & 0xFF);
        cpu->cycles = 5;
    } else {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->cycles = 4;
    }
}

void CPUInstructions::sta_absolute(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFF, (cpu->a >> 8) & 0xFF);
        cpu->cycles = 5;
    } else {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->cycles = 4;
    }
}

void CPUInstructions::sta_absolute_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFF, (cpu->a >> 8) & 0xFF);
        cpu->cycles = 5;
    } else {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->cycles = 4;
    }
}

void CPUInstructions::sta_absolute_y(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFF, (cpu->a >> 8) & 0xFF);
        cpu->cycles = 5;
    } else {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->cycles = 4;
    }
}

void CPUInstructions::sta_dp_indirect_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indexed_indirect_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFF, (cpu->a >> 8) & 0xFF);
        cpu->cycles = 7;
    } else {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->cycles = 6;
    }
}

void CPUInstructions::sta_dp_indirect_y(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indirect_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFF, (cpu->a >> 8) & 0xFF);
        cpu->cycles = 6;
    } else {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->cycles = 5;
    }
}

void CPUInstructions::sta_dp_indirect(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indirect(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFF, (cpu->a >> 8) & 0xFF);
        cpu->cycles = 6;
    } else {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->cycles = 5;
    }
}

void CPUInstructions::sta_dp_indirect_long(CPU* cpu) {
    uint32_t addr = CPUAddressing::direct_page_indirect_long(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFFFF, (cpu->a >> 8) & 0xFF);
        cpu->cycles = 7;
    } else {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->cycles = 6;
    }
}

void CPUInstructions::sta_dp_indirect_long_y(CPU* cpu) {
    uint32_t addr = CPUAddressing::direct_page_indirect_long_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFFFF, (cpu->a >> 8) & 0xFF);
        cpu->cycles = 7;
    } else {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->cycles = 6;
    }
}

void CPUInstructions::sta_absolute_long(CPU* cpu) {
    uint32_t addr = CPUAddressing::absolute_long(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFFFF, (cpu->a >> 8) & 0xFF);
        cpu->cycles = 6;
    } else {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->cycles = 5;
    }
}

void CPUInstructions::sta_absolute_long_x(CPU* cpu) {
    uint32_t addr = CPUAddressing::absolute_long_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFFFF, (cpu->a >> 8) & 0xFF);
        cpu->cycles = 6;
    } else {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->cycles = 5;
    }
}

void CPUInstructions::sta_stack_relative(CPU* cpu) {
    uint16_t addr = CPUAddressing::stack_relative(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFF, (cpu->a >> 8) & 0xFF);
        cpu->cycles = 5;
    } else {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->cycles = 4;
    }
}

void CPUInstructions::sta_stack_relative_indirect_y(CPU* cpu) {
    uint16_t addr = CPUAddressing::stack_relative_indirect_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFF, (cpu->a >> 8) & 0xFF);
        cpu->cycles = 7;
    } else {
        cpu->bus->write(addr, cpu->a & 0xFF);
        cpu->cycles = 6;
    }
}

// Transfer Instructions
void CPUInstructions::tax(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::X);
    if (is16) {
        cpu->x = cpu->a;
        cpu->setZN(cpu->x, true);
    } else {
        cpu->x = (cpu->x & 0xFF00) | (cpu->a & 0xFF);
        cpu->setZN(cpu->x & 0xFF, false);
    }
    cpu->cycles = 2;
}

void CPUInstructions::txa(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->a = cpu->x;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (cpu->x & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
    cpu->cycles = 2;
}

void CPUInstructions::tay(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::X);
    if (is16) {
        cpu->y = cpu->a;
        cpu->setZN(cpu->y, true);
    } else {
        cpu->y = (cpu->y & 0xFF00) | (cpu->a & 0xFF);
        cpu->setZN(cpu->y & 0xFF, false);
    }
    cpu->cycles = 2;
}

void CPUInstructions::tya(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->a = cpu->y;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (cpu->y & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
    cpu->cycles = 2;
}

void CPUInstructions::tsx(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::X);
    if (is16) {
        cpu->x = cpu->stkp;
        cpu->setZN(cpu->x, true);
    } else {
        cpu->x = (cpu->x & 0xFF00) | (cpu->stkp & 0xFF);
        cpu->setZN(cpu->x & 0xFF, false);
    }
    cpu->cycles = 2;
}

void CPUInstructions::txs(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::X);
    if (is16) {
        cpu->stkp = cpu->x;
    } else {
        cpu->stkp = (cpu->stkp & 0xFF00) | (cpu->x & 0xFF);
    }
    cpu->cycles = 2;
}

void CPUInstructions::txy(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::X);
    if (is16) {
        cpu->y = cpu->x;
        cpu->setZN(cpu->y, true);
    } else {
        cpu->y = (cpu->y & 0xFF00) | (cpu->x & 0xFF);
        cpu->setZN(cpu->y & 0xFF, false);
    }
    cpu->cycles = 2;
}

void CPUInstructions::tyx(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::X);
    if (is16) {
        cpu->x = cpu->y;
        cpu->setZN(cpu->x, true);
    } else {
        cpu->x = (cpu->x & 0xFF00) | (cpu->y & 0xFF);
        cpu->setZN(cpu->x & 0xFF, false);
    }
    cpu->cycles = 2;
}

void CPUInstructions::tcd(CPU* cpu) {
    cpu->d = cpu->a;
    cpu->setZN(cpu->d, true);
    cpu->cycles = 2;
}

void CPUInstructions::tdc(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->a = cpu->d;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (cpu->d & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
    cpu->cycles = 2;
}

void CPUInstructions::tsc(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->a = cpu->stkp;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (cpu->stkp & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
    cpu->cycles = 2;
}

void CPUInstructions::tcs(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::X);
    if (is16) {
        cpu->stkp = cpu->a;
    } else {
        cpu->stkp = (cpu->stkp & 0xFF00) | (cpu->a & 0xFF);
    }
    cpu->cycles = 2;
}

void CPUInstructions::xba(CPU* cpu) {
    // Exchange B and A (high and low bytes of accumulator)
    uint8_t temp = (cpu->a >> 8) & 0xFF;
    cpu->a = ((cpu->a & 0xFF) << 8) | temp;
    cpu->setZN(cpu->a & 0xFF, false);
    cpu->cycles = 3;
}

// Stack Instructions
void CPUInstructions::pha(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        CPUHelpers::push_16(cpu, cpu->a);
        cpu->cycles = 4;
    } else {
        CPUHelpers::push_8(cpu, cpu->a & 0xFF);
        cpu->cycles = 3;
    }
}

void CPUInstructions::pla(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->a = CPUHelpers::pop_16(cpu);
        cpu->setZN(cpu->a, true);
        cpu->cycles = 5;
    } else {
        cpu->a = (cpu->a & 0xFF00) | CPUHelpers::pop_8(cpu);
        cpu->setZN(cpu->a & 0xFF, false);
        cpu->cycles = 4;
    }
}

void CPUInstructions::phx(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::X);
    if (is16) {
        CPUHelpers::push_16(cpu, cpu->x);
        cpu->cycles = 4;
    } else {
        CPUHelpers::push_8(cpu, cpu->x & 0xFF);
        cpu->cycles = 3;
    }
}

void CPUInstructions::plx(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::X);
    if (is16) {
        cpu->x = CPUHelpers::pop_16(cpu);
        cpu->setZN(cpu->x, true);
        cpu->cycles = 5;
    } else {
        cpu->x = (cpu->x & 0xFF00) | CPUHelpers::pop_8(cpu);
        cpu->setZN(cpu->x & 0xFF, false);
        cpu->cycles = 4;
    }
}

void CPUInstructions::phy(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::X);
    if (is16) {
        CPUHelpers::push_16(cpu, cpu->y);
        cpu->cycles = 4;
    } else {
        CPUHelpers::push_8(cpu, cpu->y & 0xFF);
        cpu->cycles = 3;
    }
}

void CPUInstructions::ply(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::X);
    if (is16) {
        cpu->y = CPUHelpers::pop_16(cpu);
        cpu->setZN(cpu->y, true);
        cpu->cycles = 5;
    } else {
        cpu->y = (cpu->y & 0xFF00) | CPUHelpers::pop_8(cpu);
        cpu->setZN(cpu->y & 0xFF, false);
        cpu->cycles = 4;
    }
}

void CPUInstructions::php(CPU* cpu) {
    // When pushing processor status, set B flag (bit 4) and clear E flag (bit 8)
    uint8_t status = (cpu->p & 0xFF) | 0x10; // Set B flag
    CPUHelpers::push_8(cpu, status);
    cpu->cycles = 3;
}

void CPUInstructions::plp(CPU* cpu) {
    uint8_t status = CPUHelpers::pop_8(cpu);
    cpu->p = (cpu->p & 0xFF00) | status;
    cpu->cycles = 4;
}

void CPUInstructions::phd(CPU* cpu) {
    CPUHelpers::push_16(cpu, cpu->d);
    cpu->cycles = 4;
}

void CPUInstructions::pld(CPU* cpu) {
    cpu->d = CPUHelpers::pop_16(cpu);
    cpu->setZN(cpu->d, true);
    cpu->cycles = 5;
}

void CPUInstructions::phk(CPU* cpu) {
    CPUHelpers::push_8(cpu, cpu->pb);
    cpu->cycles = 3;
}

void CPUInstructions::plk(CPU* cpu) {
    cpu->pb = CPUHelpers::pop_8(cpu);
    cpu->cycles = 4;
}

void CPUInstructions::pea(CPU* cpu) {
    uint16_t lo = cpu->bus->read(cpu->pc++);
    uint16_t hi = cpu->bus->read(cpu->pc++);
    CPUHelpers::push_16(cpu, (hi << 8) | lo);
    cpu->cycles = 5;
}

void CPUInstructions::pei(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page(cpu);
    uint16_t lo = cpu->bus->read(addr);
    uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
    CPUHelpers::push_16(cpu, (hi << 8) | lo);
    cpu->cycles = 6;
}

void CPUInstructions::per(CPU* cpu) {
    uint16_t offset = CPUAddressing::relative_long(cpu);
    uint16_t target = (cpu->pc + offset) & 0xFFFF;;
    CPUHelpers::push_16(cpu, target);
    cpu->cycles = 6;
} 

// ADC - Add with Carry
void CPUInstructions::adc_immediate(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(cpu->pc++);
        uint16_t hi = cpu->bus->read(cpu->pc++);
        operand = (hi << 8) | lo;
        cpu->cycles = 3;
    } else {
        operand = cpu->bus->read(cpu->pc++);
        cpu->cycles = 2;
    }
    
    uint32_t result = cpu->a + operand + (cpu->get_flag(CPU::C) ? 1 : 0);
    
    // Set flags
    cpu->set_flag(CPU::C, result > (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
}

void CPUInstructions::adc_direct_page(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 4;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 3;
    }
    
    uint32_t result = cpu->a + operand + (cpu->get_flag(CPU::C) ? 1 : 0);
    
    // Set flags
    cpu->set_flag(CPU::C, result > (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
}

void CPUInstructions::adc_direct_page_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 5;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 4;
    }
    
    uint32_t result = cpu->a + operand + (cpu->get_flag(CPU::C) ? 1 : 0);
    
    // Set flags
    cpu->set_flag(CPU::C, result > (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
}

void CPUInstructions::adc_absolute(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 5;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 4;
    }
    
    uint32_t result = cpu->a + operand + (cpu->get_flag(CPU::C) ? 1 : 0);
    
    // Set flags
    cpu->set_flag(CPU::C, result > (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
}

void CPUInstructions::adc_absolute_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    
    uint32_t result = cpu->a + operand + (cpu->get_flag(CPU::C) ? 1 : 0);
    
    // Set flags
    cpu->set_flag(CPU::C, result > (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
}

void CPUInstructions::adc_absolute_y(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    
    uint32_t result = cpu->a + operand + (cpu->get_flag(CPU::C) ? 1 : 0);
    
    // Set flags
    cpu->set_flag(CPU::C, result > (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
}

void CPUInstructions::adc_dp_indirect_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indexed_indirect_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    
    uint32_t result = cpu->a + operand + (cpu->get_flag(CPU::C) ? 1 : 0);
    
    // Set flags
    cpu->set_flag(CPU::C, result > (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
}

void CPUInstructions::adc_dp_indirect_y(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indirect_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    
    uint32_t result = cpu->a + operand + (cpu->get_flag(CPU::C) ? 1 : 0);
    
    // Set flags
    cpu->set_flag(CPU::C, result > (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
}

void CPUInstructions::adc_dp_indirect(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indirect(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    
    uint32_t result = cpu->a + operand + (cpu->get_flag(CPU::C) ? 1 : 0);
    
    // Set flags
    cpu->set_flag(CPU::C, result > (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
}

void CPUInstructions::adc_dp_indirect_long(CPU* cpu) {
    uint32_t addr = CPUAddressing::direct_page_indirect_long(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    
    uint32_t result = cpu->a + operand + (cpu->get_flag(CPU::C) ? 1 : 0);
    
    // Set flags
    cpu->set_flag(CPU::C, result > (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
}

void CPUInstructions::adc_dp_indirect_long_y(CPU* cpu) {
    uint32_t addr = CPUAddressing::direct_page_indirect_long_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 8;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 7;
    }
    
    uint32_t result = cpu->a + operand + (cpu->get_flag(CPU::C) ? 1 : 0);
    
    // Set flags
    cpu->set_flag(CPU::C, result > (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
} 

// SBC - Subtract with Carry
void CPUInstructions::sbc_immediate(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(cpu->pc++);
        uint16_t hi = cpu->bus->read(cpu->pc++);
        operand = (hi << 8) | lo;
        cpu->cycles = 3;
    } else {
        operand = cpu->bus->read(cpu->pc++);
        cpu->cycles = 2;
    }
    
    uint32_t result = cpu->a - operand - (cpu->get_flag(CPU::C) ? 0 : 1);
    
    // Set flags
    cpu->set_flag(CPU::C, result <= (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
}

void CPUInstructions::sbc_direct_page(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 4;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 3;
    }
    
    uint32_t result = cpu->a - operand - (cpu->get_flag(CPU::C) ? 0 : 1);
    
    // Set flags
    cpu->set_flag(CPU::C, result <= (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
}

void CPUInstructions::sbc_direct_page_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 5;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 4;
    }
    
    uint32_t result = cpu->a - operand - (cpu->get_flag(CPU::C) ? 0 : 1);
    
    // Set flags
    cpu->set_flag(CPU::C, result <= (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
}

void CPUInstructions::sbc_absolute(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 5;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 4;
    }
    
    uint32_t result = cpu->a - operand - (cpu->get_flag(CPU::C) ? 0 : 1);
    
    // Set flags
    cpu->set_flag(CPU::C, result <= (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
}

void CPUInstructions::sbc_absolute_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    
    uint32_t result = cpu->a - operand - (cpu->get_flag(CPU::C) ? 0 : 1);
    
    // Set flags
    cpu->set_flag(CPU::C, result <= (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
}

void CPUInstructions::sbc_absolute_y(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    
    uint32_t result = cpu->a - operand - (cpu->get_flag(CPU::C) ? 0 : 1);
    
    // Set flags
    cpu->set_flag(CPU::C, result <= (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
}

void CPUInstructions::sbc_dp_indirect_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indexed_indirect_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    
    uint32_t result = cpu->a - operand - (cpu->get_flag(CPU::C) ? 0 : 1);
    
    // Set flags
    cpu->set_flag(CPU::C, result <= (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
}

void CPUInstructions::sbc_dp_indirect_y(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indirect_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    
    uint32_t result = cpu->a - operand - (cpu->get_flag(CPU::C) ? 0 : 1);
    
    // Set flags
    cpu->set_flag(CPU::C, result <= (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
}

void CPUInstructions::sbc_dp_indirect(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indirect(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    
    uint32_t result = cpu->a - operand - (cpu->get_flag(CPU::C) ? 0 : 1);
    
    // Set flags
    cpu->set_flag(CPU::C, result <= (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
}

void CPUInstructions::sbc_dp_indirect_long(CPU* cpu) {
    uint32_t addr = CPUAddressing::direct_page_indirect_long(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    
    uint32_t result = cpu->a - operand - (cpu->get_flag(CPU::C) ? 0 : 1);
    
    // Set flags
    cpu->set_flag(CPU::C, result <= (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
}

void CPUInstructions::sbc_dp_indirect_long_y(CPU* cpu) {
    uint32_t addr = CPUAddressing::direct_page_indirect_long_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 8;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 7;
    }
    
    uint32_t result = cpu->a - operand - (cpu->get_flag(CPU::C) ? 0 : 1);
    
    // Set flags
    cpu->set_flag(CPU::C, result <= (is16 ? 0xFFFF : 0xFF));
    cpu->set_flag(CPU::V, ((cpu->a ^ result) & (operand ^ result) & (is16 ? 0x8000 : 0x80)) != 0);
    
    if (is16) {
        cpu->a = result & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        cpu->a = (cpu->a & 0xFF00) | (result & 0xFF);
        cpu->setZN(cpu->a & 0xFF, false);
    }
} 

// INC - Increment
void CPUInstructions::inc_accumulator(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->a = (cpu->a + 1) & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        uint8_t low_byte = (cpu->a & 0xFF) + 1;
        cpu->a = (cpu->a & 0xFF00) | low_byte;
        cpu->setZN(low_byte, false);
    }
    cpu->cycles = 2;
}

void CPUInstructions::inc_direct_page(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        uint16_t value = (hi << 8) | lo;
        value = (value + 1) & 0xFFFF;
        cpu->bus->write(addr, value & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFF, (value >> 8) & 0xFF);
        cpu->setZN(value, true);
        cpu->cycles = 6;
    } else {
        uint8_t value = cpu->bus->read(addr);
        value = (value + 1) & 0xFF;
        cpu->bus->write(addr, value);
        cpu->setZN(value, false);
        cpu->cycles = 5;
    }
}

void CPUInstructions::inc_direct_page_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        uint16_t value = (hi << 8) | lo;
        value = (value + 1) & 0xFFFF;
        cpu->bus->write(addr, value & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFF, (value >> 8) & 0xFF);
        cpu->setZN(value, true);
        cpu->cycles = 7;
    } else {
        uint8_t value = cpu->bus->read(addr);
        value = (value + 1) & 0xFF;
        cpu->bus->write(addr, value);
        cpu->setZN(value, false);
        cpu->cycles = 6;
    }
}

void CPUInstructions::inc_absolute(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        uint16_t value = (hi << 8) | lo;
        value = (value + 1) & 0xFFFF;
        cpu->bus->write(addr, value & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFF, (value >> 8) & 0xFF);
        cpu->setZN(value, true);
        cpu->cycles = 7;
    } else {
        uint8_t value = cpu->bus->read(addr);
        value = (value + 1) & 0xFF;
        cpu->bus->write(addr, value);
        cpu->setZN(value, false);
        cpu->cycles = 6;
    }
}

void CPUInstructions::inc_absolute_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        uint16_t value = (hi << 8) | lo;
        value = (value + 1) & 0xFFFF;
        cpu->bus->write(addr, value & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFF, (value >> 8) & 0xFF);
        cpu->setZN(value, true);
        cpu->cycles = 8;
    } else {
        uint8_t value = cpu->bus->read(addr);
        value = (value + 1) & 0xFF;
        cpu->bus->write(addr, value);
        cpu->setZN(value, false);
        cpu->cycles = 7;
    }
}

void CPUInstructions::inx(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::X);
    if (is16) {
        cpu->x = (cpu->x + 1) & 0xFFFF;
        cpu->setZN(cpu->x, true);
    } else {
        uint8_t low_byte = (cpu->x & 0xFF) + 1;
        cpu->x = (cpu->x & 0xFF00) | low_byte;
        cpu->setZN(low_byte, false);
    }
    cpu->cycles = 2;
}

void CPUInstructions::iny(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::X);
    if (is16) {
        cpu->y = (cpu->y + 1) & 0xFFFF;
        cpu->setZN(cpu->y, true);
    } else {
        uint8_t low_byte = (cpu->y & 0xFF) + 1;
        cpu->y = (cpu->y & 0xFF00) | low_byte;
        cpu->setZN(low_byte, false);
    }
    cpu->cycles = 2;
} 

// DEC - Decrement
void CPUInstructions::dec_accumulator(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->a = (cpu->a - 1) & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        uint8_t low_byte = (cpu->a & 0xFF) - 1;
        cpu->a = (cpu->a & 0xFF00) | low_byte;
        cpu->setZN(low_byte, false);
    }
    cpu->cycles = 2;
}

void CPUInstructions::dec_direct_page(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        uint16_t value = (hi << 8) | lo;
        value = (value - 1) & 0xFFFF;
        cpu->bus->write(addr, value & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFF, (value >> 8) & 0xFF);
        cpu->setZN(value, true);
        cpu->cycles = 6;
    } else {
        uint8_t value = cpu->bus->read(addr);
        value = (value - 1) & 0xFF;
        cpu->bus->write(addr, value);
        cpu->setZN(value, false);
        cpu->cycles = 5;
    }
}

void CPUInstructions::dec_direct_page_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        uint16_t value = (hi << 8) | lo;
        value = (value - 1) & 0xFFFF;
        cpu->bus->write(addr, value & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFF, (value >> 8) & 0xFF);
        cpu->setZN(value, true);
        cpu->cycles = 7;
    } else {
        uint8_t value = cpu->bus->read(addr);
        value = (value - 1) & 0xFF;
        cpu->bus->write(addr, value);
        cpu->setZN(value, false);
        cpu->cycles = 6;
    }
}

void CPUInstructions::dec_absolute(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        uint16_t value = (hi << 8) | lo;
        value = (value - 1) & 0xFFFF;
        cpu->bus->write(addr, value & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFF, (value >> 8) & 0xFF);
        cpu->setZN(value, true);
        cpu->cycles = 7;
    } else {
        uint8_t value = cpu->bus->read(addr);
        value = (value - 1) & 0xFF;
        cpu->bus->write(addr, value);
        cpu->setZN(value, false);
        cpu->cycles = 6;
    }
}

void CPUInstructions::dec_absolute_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        uint16_t value = (hi << 8) | lo;
        value = (value - 1) & 0xFFFF;
        cpu->bus->write(addr, value & 0xFF);
        cpu->bus->write((addr + 1) & 0xFFFF, (value >> 8) & 0xFF);
        cpu->setZN(value, true);
        cpu->cycles = 8;
    } else {
        uint8_t value = cpu->bus->read(addr);
        value = (value - 1) & 0xFF;
        cpu->bus->write(addr, value);
        cpu->setZN(value, false);
        cpu->cycles = 7;
    }
}

void CPUInstructions::dex(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::X);
    if (is16) {
        cpu->x = (cpu->x - 1) & 0xFFFF;
        cpu->setZN(cpu->x, true);
    } else {
        uint8_t low_byte = (cpu->x & 0xFF) - 1;
        cpu->x = (cpu->x & 0xFF00) | low_byte;
        cpu->setZN(low_byte, false);
    }
    cpu->cycles = 2;
}

void CPUInstructions::dey(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::X);
    if (is16) {
        cpu->y = (cpu->y - 1) & 0xFFFF;
        cpu->setZN(cpu->y, true);
    } else {
        uint8_t low_byte = (cpu->y & 0xFF) - 1;
        cpu->y = (cpu->y & 0xFF00) | low_byte;
        cpu->setZN(low_byte, false);
    }
    cpu->cycles = 2;
}

// CMP - Compare Accumulator
void CPUInstructions::cmp_immediate(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(cpu->pc++);
        uint16_t hi = cpu->bus->read(cpu->pc++);
        operand = (hi << 8) | lo;
        cpu->cycles = 3;
    } else {
        operand = cpu->bus->read(cpu->pc++);
        cpu->cycles = 2;
    }
    
    uint16_t result = cpu->a - operand;
    cpu->set_flag(CPU::C, cpu->a >= operand);
    cpu->setZN(result, is16);
}

void CPUInstructions::cmp_direct_page(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 4;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 3;
    }
    
    uint16_t result = cpu->a - operand;
    cpu->set_flag(CPU::C, cpu->a >= operand);
    cpu->setZN(result, is16);
}

void CPUInstructions::cmp_direct_page_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 5;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 4;
    }
    
    uint16_t result = cpu->a - operand;
    cpu->set_flag(CPU::C, cpu->a >= operand);
    cpu->setZN(result, is16);
}

void CPUInstructions::cmp_absolute(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 5;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 4;
    }
    
    uint16_t result = cpu->a - operand;
    cpu->set_flag(CPU::C, cpu->a >= operand);
    cpu->setZN(result, is16);
}

void CPUInstructions::cmp_absolute_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    
    uint16_t result = cpu->a - operand;
    cpu->set_flag(CPU::C, cpu->a >= operand);
    cpu->setZN(result, is16);
}

void CPUInstructions::cmp_absolute_y(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    
    uint16_t result = cpu->a - operand;
    cpu->set_flag(CPU::C, cpu->a >= operand);
    cpu->setZN(result, is16);
}

void CPUInstructions::cmp_dp_indirect_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indexed_indirect_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    
    uint16_t result = cpu->a - operand;
    cpu->set_flag(CPU::C, cpu->a >= operand);
    cpu->setZN(result, is16);
}

void CPUInstructions::cmp_dp_indirect_y(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indirect_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    
    uint16_t result = cpu->a - operand;
    cpu->set_flag(CPU::C, cpu->a >= operand);
    cpu->setZN(result, is16);
}

void CPUInstructions::cmp_dp_indirect(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indirect(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    
    uint16_t result = cpu->a - operand;
    cpu->set_flag(CPU::C, cpu->a >= operand);
    cpu->setZN(result, is16);
}

void CPUInstructions::cmp_dp_indirect_long(CPU* cpu) {
    uint32_t addr = CPUAddressing::direct_page_indirect_long(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    
    uint16_t result = cpu->a - operand;
    cpu->set_flag(CPU::C, cpu->a >= operand);
    cpu->setZN(result, is16);
}

void CPUInstructions::cmp_dp_indirect_long_y(CPU* cpu) {
    uint32_t addr = CPUAddressing::direct_page_indirect_long_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 8;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 7;
    }
    
    uint16_t result = cpu->a - operand;
    cpu->set_flag(CPU::C, cpu->a >= operand);
    cpu->setZN(result, is16);
}

void CPUInstructions::cmp_absolute_long(CPU* cpu) {
    uint32_t addr = CPUAddressing::absolute_long(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 5;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    
    uint16_t result = cpu->a - operand;
    cpu->set_flag(CPU::C, cpu->a >= operand);
    cpu->setZN(result, is16);
}

void CPUInstructions::cmp_absolute_long_x(CPU* cpu) {
    uint32_t addr = CPUAddressing::absolute_long_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 5;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    
    uint16_t result = cpu->a - operand;
    cpu->set_flag(CPU::C, cpu->a >= operand);
    cpu->setZN(result, is16);
}

void CPUInstructions::cmp_stack_relative(CPU* cpu) {
    uint16_t addr = CPUAddressing::stack_relative(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 4;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 4;
    }
    
    uint16_t result = cpu->a - operand;
    cpu->set_flag(CPU::C, cpu->a >= operand);
    cpu->setZN(result, is16);
}

void CPUInstructions::cmp_stack_relative_indirect_y(CPU* cpu) {
    uint16_t addr = CPUAddressing::stack_relative_indirect_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 7;
    }
    
    uint16_t result = cpu->a - operand;
    cpu->set_flag(CPU::C, cpu->a >= operand);
    cpu->setZN(result, is16);
}

// CPX - Compare X Register
void CPUInstructions::cpx_immediate(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::X);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(cpu->pc++);
        uint16_t hi = cpu->bus->read(cpu->pc++);
        operand = (hi << 8) | lo;
        cpu->cycles = 3;
    } else {
        operand = cpu->bus->read(cpu->pc++);
        cpu->cycles = 2;
    }
    
    uint16_t result = cpu->x - operand;
    cpu->set_flag(CPU::C, cpu->x >= operand);
    cpu->setZN(result, is16);
}

void CPUInstructions::cpx_direct_page(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page(cpu);
    bool is16 = !cpu->get_flag(CPU::X);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 4;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 3;
    }
    
    uint16_t result = cpu->x - operand;
    cpu->set_flag(CPU::C, cpu->x >= operand);
    cpu->setZN(result, is16);
}

void CPUInstructions::cpx_absolute(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute(cpu);
    bool is16 = !cpu->get_flag(CPU::X);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 5;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 4;
    }
    
    uint16_t result = cpu->x - operand;
    cpu->set_flag(CPU::C, cpu->x >= operand);
    cpu->setZN(result, is16);
}

// CPY - Compare Y Register
void CPUInstructions::cpy_immediate(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::X);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(cpu->pc++);
        uint16_t hi = cpu->bus->read(cpu->pc++);
        operand = (hi << 8) | lo;
        cpu->cycles = 3;
    } else {
        operand = cpu->bus->read(cpu->pc++);
        cpu->cycles = 2;
    }
    
    uint16_t result = cpu->y - operand;
    cpu->set_flag(CPU::C, cpu->y >= operand);
    cpu->setZN(result, is16);
}

void CPUInstructions::cpy_direct_page(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page(cpu);
    bool is16 = !cpu->get_flag(CPU::X);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 4;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 3;
    }
    
    uint16_t result = cpu->y - operand;
    cpu->set_flag(CPU::C, cpu->y >= operand);
    cpu->setZN(result, is16);
}

void CPUInstructions::cpy_absolute(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute(cpu);
    bool is16 = !cpu->get_flag(CPU::X);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 5;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 4;
    }
    
    uint16_t result = cpu->y - operand;
    cpu->set_flag(CPU::C, cpu->y >= operand);
    cpu->setZN(result, is16);
}

// AND - Logical AND
void CPUInstructions::and_immediate(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(cpu->pc++);
        uint16_t hi = cpu->bus->read(cpu->pc++);
        operand = (hi << 8) | lo;
        cpu->cycles = 3;
    } else {
        operand = cpu->bus->read(cpu->pc++);
        cpu->cycles = 2;
    }
    
    cpu->a &= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::and_direct_page(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 4;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 3;
    }
    
    cpu->a &= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::and_direct_page_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 5;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 4;
    }
    
    cpu->a &= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::and_absolute(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 5;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 4;
    }
    
    cpu->a &= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::and_absolute_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    
    cpu->a &= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::and_absolute_y(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    
    cpu->a &= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::and_dp_indirect_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indexed_indirect_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    
    cpu->a &= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::and_dp_indirect_y(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indirect_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    
    cpu->a &= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::and_dp_indirect(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indirect(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    
    cpu->a &= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::and_dp_indirect_long(CPU* cpu) {
    uint32_t addr = CPUAddressing::direct_page_indirect_long(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    
    cpu->a &= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::and_dp_indirect_long_y(CPU* cpu) {
    uint32_t addr = CPUAddressing::direct_page_indirect_long_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 8;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 7;
    }
    
    cpu->a &= operand;
    cpu->setZN(cpu->a, is16);
}

// ORA - Logical OR
void CPUInstructions::ora_immediate(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(cpu->pc++);
        uint16_t hi = cpu->bus->read(cpu->pc++);
        operand = (hi << 8) | lo;
        cpu->cycles = 3;
    } else {
        operand = cpu->bus->read(cpu->pc++);
        cpu->cycles = 2;
    }
    
    cpu->a |= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::ora_direct_page(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 4;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 3;
    }
    
    cpu->a |= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::ora_direct_page_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 5;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 4;
    }
    
    cpu->a |= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::ora_absolute(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 5;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 4;
    }
    
    cpu->a |= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::ora_absolute_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    
    cpu->a |= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::ora_absolute_y(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    
    cpu->a |= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::ora_dp_indirect_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indexed_indirect_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    
    cpu->a |= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::ora_dp_indirect_y(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indirect_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    
    cpu->a |= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::ora_dp_indirect(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indirect(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    
    cpu->a |= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::ora_dp_indirect_long(CPU* cpu) {
    uint32_t addr = CPUAddressing::direct_page_indirect_long(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    
    cpu->a |= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::ora_dp_indirect_long_y(CPU* cpu) {
    uint32_t addr = CPUAddressing::direct_page_indirect_long_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 8;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 7;
    }
    
    cpu->a |= operand;
    cpu->setZN(cpu->a, is16);
}

// EOR - Logical XOR
void CPUInstructions::eor_immediate(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(cpu->pc++);
        uint16_t hi = cpu->bus->read(cpu->pc++);
        operand = (hi << 8) | lo;
        cpu->cycles = 3;
    } else {
        operand = cpu->bus->read(cpu->pc++);
        cpu->cycles = 2;
    }
    
    cpu->a ^= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::eor_direct_page(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 4;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 3;
    }
    
    cpu->a ^= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::eor_direct_page_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 5;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 4;
    }
    
    cpu->a ^= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::eor_absolute(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 5;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 4;
    }
    
    cpu->a ^= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::eor_absolute_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    
    cpu->a ^= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::eor_absolute_y(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    
    cpu->a ^= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::eor_dp_indirect_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indexed_indirect_x(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    
    cpu->a ^= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::eor_dp_indirect_y(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indirect_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    
    cpu->a ^= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::eor_dp_indirect(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_indirect(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 6;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 5;
    }
    
    cpu->a ^= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::eor_dp_indirect_long(CPU* cpu) {
    uint32_t addr = CPUAddressing::direct_page_indirect_long(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 7;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 6;
    }
    
    cpu->a ^= operand;
    cpu->setZN(cpu->a, is16);
}

void CPUInstructions::eor_dp_indirect_long_y(CPU* cpu) {
    uint32_t addr = CPUAddressing::direct_page_indirect_long_y(cpu);
    bool is16 = !cpu->get_flag(CPU::M);
    uint16_t operand;
    if (is16) {
        uint16_t lo = cpu->bus->read(addr);
        uint16_t hi = cpu->bus->read((addr + 1) & 0xFFFFFF);
        operand = (hi << 8) | lo;
        cpu->cycles = 8;
    } else {
        operand = cpu->bus->read(addr);
        cpu->cycles = 7;
    }
    
    cpu->a ^= operand;
    cpu->setZN(cpu->a, is16);
}

// Branch Instructions
void CPUInstructions::bcc(CPU* cpu) {
    int8_t offset = cpu->bus->read(cpu->pc++);
    if (!cpu->get_flag(CPU::C)) {
        uint32_t old_pc = cpu->pc;
        cpu->pc += offset;
        cpu->cycles = 3;
        // Add extra cycle if page boundary crossed
        if ((old_pc & 0xFF00) != (cpu->pc & 0xFF00)) {
            cpu->cycles++;
        }
    } else {
        cpu->cycles = 2;
    }
}

void CPUInstructions::bcs(CPU* cpu) {
    int8_t offset = cpu->bus->read(cpu->pc++);
    if (cpu->get_flag(CPU::C)) {
        uint32_t old_pc = cpu->pc;
        cpu->pc += offset;
        cpu->cycles = 3;
        // Add extra cycle if page boundary crossed
        if ((old_pc & 0xFF00) != (cpu->pc & 0xFF00)) {
            cpu->cycles++;
        }
    } else {
        cpu->cycles = 2;
    }
}

void CPUInstructions::beq(CPU* cpu) {
    int8_t offset = cpu->bus->read(cpu->pc++);
    if (cpu->get_flag(CPU::Z)) {
        uint32_t old_pc = cpu->pc;
        cpu->pc += offset;
        cpu->cycles = 3;
        // Add extra cycle if page boundary crossed
        if ((old_pc & 0xFF00) != (cpu->pc & 0xFF00)) {
            cpu->cycles++;
        }
    } else {
        cpu->cycles = 2;
    }
}

void CPUInstructions::bne(CPU* cpu) {
    int8_t offset = cpu->bus->read(cpu->pc++);
    if (!cpu->get_flag(CPU::Z)) {
        uint32_t old_pc = cpu->pc;
        cpu->pc += offset;
        cpu->cycles = 3;
        // Add extra cycle if page boundary crossed
        if ((old_pc & 0xFF00) != (cpu->pc & 0xFF00)) {
            cpu->cycles++;
        }
    } else {
        cpu->cycles = 2;
    }
}

void CPUInstructions::bmi(CPU* cpu) {
    int8_t offset = cpu->bus->read(cpu->pc++);
    if (cpu->get_flag(CPU::N)) {
        uint32_t old_pc = cpu->pc;
        cpu->pc += offset;
        cpu->cycles = 3;
        // Add extra cycle if page boundary crossed
        if ((old_pc & 0xFF00) != (cpu->pc & 0xFF00)) {
            cpu->cycles++;
        }
    } else {
        cpu->cycles = 2;
    }
}

void CPUInstructions::bpl(CPU* cpu) {
    int8_t offset = cpu->bus->read(cpu->pc++);
    if (!cpu->get_flag(CPU::N)) {
        uint32_t old_pc = cpu->pc;
        cpu->pc += offset;
        cpu->cycles = 3;
        // Add extra cycle if page boundary crossed
        if ((old_pc & 0xFF00) != (cpu->pc & 0xFF00)) {
            cpu->cycles++;
        }
    } else {
        cpu->cycles = 2;
    }
}

void CPUInstructions::bvc(CPU* cpu) {
    int8_t offset = cpu->bus->read(cpu->pc++);
    if (!cpu->get_flag(CPU::V)) {
        uint32_t old_pc = cpu->pc;
        cpu->pc += offset;
        cpu->cycles = 3;
        // Add extra cycle if page boundary crossed
        if ((old_pc & 0xFF00) != (cpu->pc & 0xFF00)) {
            cpu->cycles++;
        }
    } else {
        cpu->cycles = 2;
    }
}

void CPUInstructions::bvs(CPU* cpu) {
    int8_t offset = cpu->bus->read(cpu->pc++);
    if (cpu->get_flag(CPU::V)) {
        uint32_t old_pc = cpu->pc;
        cpu->pc += offset;
        cpu->cycles = 3;
        // Add extra cycle if page boundary crossed
        if ((old_pc & 0xFF00) != (cpu->pc & 0xFF00)) {
            cpu->cycles++;
        }
    } else {
        cpu->cycles = 2;
    }
}

void CPUInstructions::bra(CPU* cpu) {
    int8_t offset = cpu->bus->read(cpu->pc++);
    uint32_t old_pc = cpu->pc;
    cpu->pc += offset;
    cpu->cycles = 3;
    // Add extra cycle if page boundary crossed
    if ((old_pc & 0xFF00) != (cpu->pc & 0xFF00)) {
        cpu->cycles++;
    }
}

void CPUInstructions::brl(CPU* cpu) {
    int16_t offset = cpu->bus->read(cpu->pc++);
    offset |= (cpu->bus->read(cpu->pc++) << 8);
    cpu->pc += offset;
    cpu->cycles = 4;
}

// Shift and Rotate Instructions
void CPUInstructions::asl_accumulator(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->set_flag(CPU::C, (cpu->a & 0x8000) != 0);
        cpu->a = (cpu->a << 1) & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        uint8_t low_byte = cpu->a & 0xFF;
        cpu->set_flag(CPU::C, (low_byte & 0x80) != 0);
        low_byte = (low_byte << 1) & 0xFF;
        cpu->a = (cpu->a & 0xFF00) | low_byte;
        cpu->setZN(low_byte, false);
    }
    cpu->cycles = 2;
}

void CPUInstructions::asl_direct_page(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page(cpu);
    uint8_t value = cpu->bus->read(addr);
    cpu->set_flag(CPU::C, (value & 0x80) != 0);
    value = (value << 1) & 0xFF;
    cpu->bus->write(addr, value);
    cpu->setZN(value, false);
    cpu->cycles = 5;
}

void CPUInstructions::asl_direct_page_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_x(cpu);
    uint8_t value = cpu->bus->read(addr);
    cpu->set_flag(CPU::C, (value & 0x80) != 0);
    value = (value << 1) & 0xFF;
    cpu->bus->write(addr, value);
    cpu->setZN(value, false);
    cpu->cycles = 6;
}

void CPUInstructions::asl_absolute(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute(cpu);
    uint8_t value = cpu->bus->read(addr);
    cpu->set_flag(CPU::C, (value & 0x80) != 0);
    value = (value << 1) & 0xFF;
    cpu->bus->write(addr, value);
    cpu->setZN(value, false);
    cpu->cycles = 6;
}

void CPUInstructions::asl_absolute_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_x(cpu);
    uint8_t value = cpu->bus->read(addr);
    cpu->set_flag(CPU::C, (value & 0x80) != 0);
    value = (value << 1) & 0xFF;
    cpu->bus->write(addr, value);
    cpu->setZN(value, false);
    cpu->cycles = 7;
}

void CPUInstructions::lsr_accumulator(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        cpu->set_flag(CPU::C, (cpu->a & 0x0001) != 0);
        cpu->a = (cpu->a >> 1) & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        uint8_t low_byte = cpu->a & 0xFF;
        cpu->set_flag(CPU::C, (low_byte & 0x01) != 0);
        low_byte = (low_byte >> 1) & 0xFF;
        cpu->a = (cpu->a & 0xFF00) | low_byte;
        cpu->setZN(low_byte, false);
    }
    cpu->cycles = 2;
}

void CPUInstructions::lsr_direct_page(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page(cpu);
    uint8_t value = cpu->bus->read(addr);
    cpu->set_flag(CPU::C, (value & 0x01) != 0);
    value = (value >> 1) & 0xFF;
    cpu->bus->write(addr, value);
    cpu->setZN(value, false);
    cpu->cycles = 5;
}

void CPUInstructions::lsr_direct_page_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_x(cpu);
    uint8_t value = cpu->bus->read(addr);
    cpu->set_flag(CPU::C, (value & 0x01) != 0);
    value = (value >> 1) & 0xFF;
    cpu->bus->write(addr, value);
    cpu->setZN(value, false);
    cpu->cycles = 6;
}

void CPUInstructions::lsr_absolute(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute(cpu);
    uint8_t value = cpu->bus->read(addr);
    cpu->set_flag(CPU::C, (value & 0x01) != 0);
    value = (value >> 1) & 0xFF;
    cpu->bus->write(addr, value);
    cpu->setZN(value, false);
    cpu->cycles = 6;
}

void CPUInstructions::lsr_absolute_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_x(cpu);
    uint8_t value = cpu->bus->read(addr);
    cpu->set_flag(CPU::C, (value & 0x01) != 0);
    value = (value >> 1) & 0xFF;
    cpu->bus->write(addr, value);
    cpu->setZN(value, false);
    cpu->cycles = 7;
}

void CPUInstructions::rol_accumulator(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        bool old_carry = cpu->get_flag(CPU::C);
        cpu->set_flag(CPU::C, (cpu->a & 0x8000) != 0);
        cpu->a = ((cpu->a << 1) | (old_carry ? 1 : 0)) & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        uint8_t low_byte = cpu->a & 0xFF;
        bool old_carry = cpu->get_flag(CPU::C);
        cpu->set_flag(CPU::C, (low_byte & 0x80) != 0);
        low_byte = ((low_byte << 1) | (old_carry ? 1 : 0)) & 0xFF;
        cpu->a = (cpu->a & 0xFF00) | low_byte;
        cpu->setZN(low_byte, false);
    }
    cpu->cycles = 2;
}

void CPUInstructions::rol_direct_page(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page(cpu);
    uint8_t value = cpu->bus->read(addr);
    bool old_carry = cpu->get_flag(CPU::C);
    cpu->set_flag(CPU::C, (value & 0x80) != 0);
    value = ((value << 1) | (old_carry ? 1 : 0)) & 0xFF;
    cpu->bus->write(addr, value);
    cpu->setZN(value, false);
    cpu->cycles = 5;
}

void CPUInstructions::rol_direct_page_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_x(cpu);
    uint8_t value = cpu->bus->read(addr);
    bool old_carry = cpu->get_flag(CPU::C);
    cpu->set_flag(CPU::C, (value & 0x80) != 0);
    value = ((value << 1) | (old_carry ? 1 : 0)) & 0xFF;
    cpu->bus->write(addr, value);
    cpu->setZN(value, false);
    cpu->cycles = 6;
}

void CPUInstructions::rol_absolute(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute(cpu);
    uint8_t value = cpu->bus->read(addr);
    bool old_carry = cpu->get_flag(CPU::C);
    cpu->set_flag(CPU::C, (value & 0x80) != 0);
    value = ((value << 1) | (old_carry ? 1 : 0)) & 0xFF;
    cpu->bus->write(addr, value);
    cpu->setZN(value, false);
    cpu->cycles = 6;
}

void CPUInstructions::rol_absolute_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_x(cpu);
    uint8_t value = cpu->bus->read(addr);
    bool old_carry = cpu->get_flag(CPU::C);
    cpu->set_flag(CPU::C, (value & 0x80) != 0);
    value = ((value << 1) | (old_carry ? 1 : 0)) & 0xFF;
    cpu->bus->write(addr, value);
    cpu->setZN(value, false);
    cpu->cycles = 7;
}

void CPUInstructions::ror_accumulator(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        bool old_carry = cpu->get_flag(CPU::C);
        cpu->set_flag(CPU::C, (cpu->a & 0x0001) != 0);
        cpu->a = ((cpu->a >> 1) | (old_carry ? 0x8000 : 0)) & 0xFFFF;
        cpu->setZN(cpu->a, true);
    } else {
        uint8_t low_byte = cpu->a & 0xFF;
        bool old_carry = cpu->get_flag(CPU::C);
        cpu->set_flag(CPU::C, (low_byte & 0x01) != 0);
        low_byte = ((low_byte >> 1) | (old_carry ? 0x80 : 0)) & 0xFF;
        cpu->a = (cpu->a & 0xFF00) | low_byte;
        cpu->setZN(low_byte, false);
    }
    cpu->cycles = 2;
}

void CPUInstructions::ror_direct_page(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page(cpu);
    uint8_t value = cpu->bus->read(addr);
    bool old_carry = cpu->get_flag(CPU::C);
    cpu->set_flag(CPU::C, (value & 0x01) != 0);
    value = ((value >> 1) | (old_carry ? 0x80 : 0)) & 0xFF;
    cpu->bus->write(addr, value);
    cpu->setZN(value, false);
    cpu->cycles = 5;
}

void CPUInstructions::ror_direct_page_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page_x(cpu);
    uint8_t value = cpu->bus->read(addr);
    bool old_carry = cpu->get_flag(CPU::C);
    cpu->set_flag(CPU::C, (value & 0x01) != 0);
    value = ((value >> 1) | (old_carry ? 0x80 : 0)) & 0xFF;
    cpu->bus->write(addr, value);
    cpu->setZN(value, false);
    cpu->cycles = 6;
}

void CPUInstructions::ror_absolute(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute(cpu);
    uint8_t value = cpu->bus->read(addr);
    bool old_carry = cpu->get_flag(CPU::C);
    cpu->set_flag(CPU::C, (value & 0x01) != 0);
    value = ((value >> 1) | (old_carry ? 0x80 : 0)) & 0xFF;
    cpu->bus->write(addr, value);
    cpu->setZN(value, false);
    cpu->cycles = 6;
}

void CPUInstructions::ror_absolute_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_x(cpu);
    uint8_t value = cpu->bus->read(addr);
    bool old_carry = cpu->get_flag(CPU::C);
    cpu->set_flag(CPU::C, (value & 0x01) != 0);
    value = ((value >> 1) | (old_carry ? 0x80 : 0)) & 0xFF;
    cpu->bus->write(addr, value);
    cpu->setZN(value, false);
    cpu->cycles = 7;
}

// Bit Instructions
void CPUInstructions::bit_immediate(CPU* cpu) {
    bool is16 = !cpu->get_flag(CPU::M);
    if (is16) {
        uint16_t operand;
        uint16_t lo = cpu->bus->read(cpu->pc++);
        uint16_t hi = cpu->bus->read(cpu->pc++);
        operand = (hi << 8) | lo;
        uint16_t result = cpu->a & operand;
        cpu->set_flag(CPU::Z, result == 0);
        cpu->set_flag(CPU::N, (operand & 0x8000) != 0);
        cpu->set_flag(CPU::V, (operand & 0x4000) != 0);
        cpu->cycles = 3;
    } else {
        uint8_t operand = cpu->bus->read(cpu->pc++);
        uint8_t result = (cpu->a & 0xFF) & operand;
        cpu->set_flag(CPU::Z, result == 0);
        cpu->set_flag(CPU::N, (operand & 0x80) != 0);
        cpu->set_flag(CPU::V, (operand & 0x40) != 0);
        cpu->cycles = 2;
    }
}

void CPUInstructions::bit_direct_page(CPU* cpu) {
    uint16_t addr = CPUAddressing::direct_page(cpu);
    uint8_t operand = cpu->bus->read(addr);
    uint8_t result = (cpu->a & 0xFF) & operand;
    cpu->set_flag(CPU::Z, result == 0);
    cpu->set_flag(CPU::N, (operand & 0x80) != 0);
    cpu->set_flag(CPU::V, (operand & 0x40) != 0);
    cpu->cycles = 3;
}

void CPUInstructions::bit_absolute(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute(cpu);
    uint8_t operand = cpu->bus->read(addr);
    uint8_t result = (cpu->a & 0xFF) & operand;
    cpu->set_flag(CPU::Z, result == 0);
    cpu->set_flag(CPU::N, (operand & 0x80) != 0);
    cpu->set_flag(CPU::V, (operand & 0x40) != 0);
    cpu->cycles = 4;
}

void CPUInstructions::bit_absolute_x(CPU* cpu) {
    uint16_t addr = CPUAddressing::absolute_x(cpu);
    uint8_t operand = cpu->bus->read(addr);
    uint8_t result = (cpu->a & 0xFF) & operand;
    cpu->set_flag(CPU::Z, result == 0);
    cpu->set_flag(CPU::N, (operand & 0x80) != 0);
    cpu->set_flag(CPU::V, (operand & 0x40) != 0);
    cpu->cycles = 4;
}

// Block Move Instructions
void CPUInstructions::mvp(CPU* cpu) {
    uint8_t src_bank = cpu->bus->read(cpu->pc++);
    uint8_t dst_bank = cpu->bus->read(cpu->pc++);
    
    // Move one byte from source to destination
    uint32_t src_addr = ((uint32_t)src_bank << 16) | cpu->x;
    uint32_t dst_addr = ((uint32_t)dst_bank << 16) | cpu->y;
    
    uint8_t value = cpu->bus->read(src_addr);
    cpu->bus->write(dst_addr, value);
    
    // Decrement counters
    cpu->x--;
    cpu->y--;
    cpu->a--;
    
    // If A is not 0xFFFF, repeat the instruction
    if (cpu->a != 0xFFFF) {
        cpu->pc -= 3; // Repeat instruction
        cpu->cycles = 7;
    } else {
        cpu->cycles = 6;
    }
}

void CPUInstructions::mvn(CPU* cpu) {
    uint8_t src_bank = cpu->bus->read(cpu->pc++);
    uint8_t dst_bank = cpu->bus->read(cpu->pc++);
    
    // Move one byte from source to destination
    uint32_t src_addr = ((uint32_t)src_bank << 16) | cpu->x;
    uint32_t dst_addr = ((uint32_t)dst_bank << 16) | cpu->y;
    
    uint8_t value = cpu->bus->read(src_addr);
    cpu->bus->write(dst_addr, value);
    
    // Increment counters
    cpu->x++;
    cpu->y++;
    cpu->a--;
    
    // If A is not 0xFFFF, repeat the instruction
    if (cpu->a != 0xFFFF) {
        cpu->pc -= 3; // Repeat instruction
        cpu->cycles = 7;
    } else {
        cpu->cycles = 6;
    }
}