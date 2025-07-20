#include "../include/cpu.hpp"
#include "../include/cpu_addressing.hpp"
#include "../include/cpu_instructions.hpp"
#include "../include/cpu_helpers.hpp"
#include "../include/bus.hpp"
#include <cstdio>

// Constructor
CPU::CPU() {
    reset();
}

// Destructor
CPU::~CPU() {
    // Nothing to clean up
}

// Connect to bus
void CPU::connect_bus(std::shared_ptr<Bus> b) {
    bus = b;
}

// Reset CPU state
void CPU::reset() {
    // Initialize registers to known state
    a = 0x0000;
    x = 0x0000;
    y = 0x0000;
    stkp = 0x01FD;
    pc = 0x8000;
    p = 0x34;  // Set initial flags
    d = 0x0000;
    pb = 0x00;
    db = 0x00;
    
    // Clear internal state
    addr_abs = 0;
    addr_rel = 0;
    fetched = 0;
    opcode = 0;
    cycles = 0;
}

// Execute one instruction
void CPU::step() {
    if (!bus) {
        printf("ERROR: No bus connected to CPU\n");
        return;
    }
    
    // Fetch opcode
    uint32_t full_addr = ((uint32_t)pb << 16) | pc;
    uint8_t opcode = bus->read(full_addr);
    pc++;
    
    // Reset cycles for this instruction
    cycles = 0;
    
    // Dispatch to appropriate instruction
    // This will be replaced with a function pointer table later
    switch (opcode) {
        // BRK - Break
        case 0x00:
            CPUInstructions::brk(this);
            break;
            
        // NOP - No Operation
        case 0xEA:
            CPUInstructions::nop(this);
            break;
            
        // SEI - Set Interrupt Disable
        case 0x78:
            CPUInstructions::sei(this);
            break;
            
        // CLI - Clear Interrupt Disable
        case 0x58:
            CPUInstructions::cli(this);
            break;
            
        // CLC - Clear Carry
        case 0x18:
            CPUInstructions::clc(this);
            break;
            
        // SEC - Set Carry
        case 0x38:
            CPUInstructions::sec(this);
            break;
            
        // CLD - Clear Decimal
        case 0xD8:
            CPUInstructions::cld(this);
            break;
            
        // SED - Set Decimal
        case 0xF8:
            CPUInstructions::sed(this);
            break;
            
        // CLV - Clear Overflow
        case 0xB8:
            CPUInstructions::clv(this);
            break;
            
        // JMP - Jump Instructions
        case 0x4C: // JMP Absolute
            CPUInstructions::jmp_absolute(this);
            break;
        case 0x5C: // JMP Absolute Long
            CPUInstructions::jmp_absolute_long(this);
            break;
        case 0x6C: // JMP Indirect
            CPUInstructions::jmp_absolute_indirect(this);
            break;
        case 0xDC: // JMP Indirect Long
            CPUInstructions::jmp_absolute_indirect_long(this);
            break;
        case 0x7C: // JMP Indexed Indirect
            CPUInstructions::jmp_absolute_indirect_x(this);
            break;
            
        // JSR - Jump to Subroutine
        case 0x20: // JSR Absolute
            CPUInstructions::jsr(this);
            break;
        case 0x22: // JSR Absolute Long
            CPUInstructions::jsr_absolute_long(this);
            break;
            
        // RTS/RTL - Return from Subroutine
        case 0x60: // RTS
            CPUInstructions::rts(this);
            break;
        case 0x6B: // RTL
            CPUInstructions::rtl(this);
            break;
            
        // RTI - Return from Interrupt
        case 0x40:
            CPUInstructions::rti(this);
            break;
            
        // LDA - Load Accumulator
        case 0xA9: // Immediate
            CPUInstructions::lda_immediate(this);
            break;
        case 0xA5: // Direct Page
            CPUInstructions::lda_direct_page(this);
            break;
        case 0xB5: // Direct Page, X
            CPUInstructions::lda_direct_page_x(this);
            break;
        case 0xAD: // Absolute
            CPUInstructions::lda_absolute(this);
            break;
        case 0xBD: // Absolute, X
            CPUInstructions::lda_absolute_x(this);
            break;
        case 0xB9: // Absolute, Y
            CPUInstructions::lda_absolute_y(this);
            break;
        case 0xA1: // (Direct Page, X)
            CPUInstructions::lda_dp_indirect_x(this);
            break;
        case 0xB1: // (Direct Page), Y
            CPUInstructions::lda_dp_indirect_y(this);
            break;
        case 0xB2: // (Direct Page)
            CPUInstructions::lda_dp_indirect(this);
            break;
        case 0xA7: // [Direct Page]
            CPUInstructions::lda_dp_indirect_long(this);
            break;
        case 0xB7: // [Direct Page], Y
            CPUInstructions::lda_dp_indirect_long_y(this);
            break;
        case 0xAF: // Absolute Long
            CPUInstructions::lda_absolute_long(this);
            break;
        case 0xBF: // Absolute Long, X
            CPUInstructions::lda_absolute_long_x(this);
            break;
        case 0xA3: // Stack Relative
            CPUInstructions::lda_stack_relative(this);
            break;
        case 0xB3: // Stack Relative Indirect, Y
            CPUInstructions::lda_stack_relative_indirect_y(this);
            break;
            
        // STA - Store Accumulator
        case 0x85: // Direct Page
            CPUInstructions::sta_direct_page(this);
            break;
        case 0x95: // Direct Page, X
            CPUInstructions::sta_direct_page_x(this);
            break;
        case 0x8D: // Absolute
            CPUInstructions::sta_absolute(this);
            break;
        case 0x9D: // Absolute, X
            CPUInstructions::sta_absolute_x(this);
            break;
        case 0x99: // Absolute, Y
            CPUInstructions::sta_absolute_y(this);
            break;
        case 0x81: // (Direct Page, X)
            CPUInstructions::sta_dp_indirect_x(this);
            break;
        case 0x91: // (Direct Page), Y
            CPUInstructions::sta_dp_indirect_y(this);
            break;
        case 0x92: // (Direct Page)
            CPUInstructions::sta_dp_indirect(this);
            break;
        case 0x87: // [Direct Page]
            CPUInstructions::sta_dp_indirect_long(this);
            break;
        case 0x97: // [Direct Page], Y
            CPUInstructions::sta_dp_indirect_long_y(this);
            break;
        case 0x8F: // Absolute Long
            CPUInstructions::sta_absolute_long(this);
            break;
        case 0x9F: // Absolute Long, X
            CPUInstructions::sta_absolute_long_x(this);
            break;
        case 0x83: // Stack Relative
            CPUInstructions::sta_stack_relative(this);
            break;
        case 0x93: // Stack Relative Indirect, Y
            CPUInstructions::sta_stack_relative_indirect_y(this);
            break;
            
        // Transfer Instructions
        case 0xAA: // TAX - Transfer Accumulator to X
            CPUInstructions::tax(this);
            break;
        case 0x8A: // TXA - Transfer X to Accumulator
            CPUInstructions::txa(this);
            break;
        case 0xA8: // TAY - Transfer Accumulator to Y
            CPUInstructions::tay(this);
            break;
        case 0x98: // TYA - Transfer Y to Accumulator
            CPUInstructions::tya(this);
            break;
        case 0xBA: // TSX - Transfer Stack Pointer to X
            CPUInstructions::tsx(this);
            break;
        case 0x9A: // TXS - Transfer X to Stack Pointer
            CPUInstructions::txs(this);
            break;
        case 0x9B: // TXY - Transfer X to Y
            CPUInstructions::txy(this);
            break;
        case 0xBB: // TYX - Transfer Y to X
            CPUInstructions::tyx(this);
            break;
        case 0x5B: // TCD - Transfer Accumulator to Direct Page
            CPUInstructions::tcd(this);
            break;
        case 0x7B: // TDC - Transfer Direct Page to Accumulator
            CPUInstructions::tdc(this);
            break;
        case 0x3B: // TSC - Transfer Stack Pointer to Accumulator
            CPUInstructions::tsc(this);
            break;
        case 0x1B: // TCS - Transfer Accumulator to Stack Pointer
            CPUInstructions::tcs(this);
            break;
        case 0xEB: // XBA - Exchange B and A
            CPUInstructions::xba(this);
            break;
        case 0xFB: // XCE - Exchange Carry and Emulation
            CPUInstructions::xce(this);
            break;
            
        // Stack Instructions
        case 0x48: // PHA - Push Accumulator
            CPUInstructions::pha(this);
            break;
        case 0x68: // PLA - Pull Accumulator
            CPUInstructions::pla(this);
            break;
        case 0xDA: // PHX - Push X
            CPUInstructions::phx(this);
            break;
        case 0xFA: // PLX - Pull X
            CPUInstructions::plx(this);
            break;
        case 0x5A: // PHY - Push Y
            CPUInstructions::phy(this);
            break;
        case 0x7A: // PLY - Pull Y
            CPUInstructions::ply(this);
            break;
        case 0x08: // PHP - Push Processor Status
            CPUInstructions::php(this);
            break;
        case 0x28: // PLP - Pull Processor Status
            CPUInstructions::plp(this);
            break;
        case 0x0B: // PHD - Push Direct Page
            CPUInstructions::phd(this);
            break;
        case 0x2B: // PLD - Pull Direct Page
            CPUInstructions::pld(this);
            break;
        case 0x4B: // PHK - Push Program Bank
            CPUInstructions::phk(this);
            break;
        case 0xAB: // PLK - Pull Program Bank
            CPUInstructions::plk(this);
            break;
        case 0xF4: // PEA - Push Effective Address
            CPUInstructions::pea(this);
            break;
        case 0xD4: // PEI - Push Effective Indirect Address
            CPUInstructions::pei(this);
            break;
        case 0x62: // PER - Push Effective PC Relative Address
            CPUInstructions::per(this);
            break;
            
        // ADC - Add with Carry
        case 0x69: // Immediate
            CPUInstructions::adc_immediate(this);
            break;
        case 0x65: // Direct Page
            CPUInstructions::adc_direct_page(this);
            break;
        case 0x75: // Direct Page, X
            CPUInstructions::adc_direct_page_x(this);
            break;
        case 0x6D: // Absolute
            CPUInstructions::adc_absolute(this);
            break;
        case 0x7D: // Absolute, X
            CPUInstructions::adc_absolute_x(this);
            break;
        case 0x79: // Absolute, Y
            CPUInstructions::adc_absolute_y(this);
            break;
        case 0x61: // (Direct Page, X)
            CPUInstructions::adc_dp_indirect_x(this);
            break;
        case 0x71: // (Direct Page), Y
            CPUInstructions::adc_dp_indirect_y(this);
            break;
        case 0x72: // (Direct Page)
            CPUInstructions::adc_dp_indirect(this);
            break;
        case 0x67: // [Direct Page]
            CPUInstructions::adc_dp_indirect_long(this);
            break;
        case 0x77: // [Direct Page], Y
            CPUInstructions::adc_dp_indirect_long_y(this);
            break;
            
        // SBC - Subtract with Carry
        case 0xE9: // Immediate
            CPUInstructions::sbc_immediate(this);
            break;
        case 0xE5: // Direct Page
            CPUInstructions::sbc_direct_page(this);
            break;
        case 0xF5: // Direct Page, X
            CPUInstructions::sbc_direct_page_x(this);
            break;
        case 0xED: // Absolute
            CPUInstructions::sbc_absolute(this);
            break;
        case 0xFD: // Absolute, X
            CPUInstructions::sbc_absolute_x(this);
            break;
        case 0xF9: // Absolute, Y
            CPUInstructions::sbc_absolute_y(this);
            break;
        case 0xE1: // (Direct Page, X)
            CPUInstructions::sbc_dp_indirect_x(this);
            break;
        case 0xF1: // (Direct Page), Y
            CPUInstructions::sbc_dp_indirect_y(this);
            break;
        case 0xF2: // (Direct Page)
            CPUInstructions::sbc_dp_indirect(this);
            break;
        case 0xE7: // [Direct Page]
            CPUInstructions::sbc_dp_indirect_long(this);
            break;
        case 0xF7: // [Direct Page], Y
            CPUInstructions::sbc_dp_indirect_long_y(this);
            break;
            
        // INC - Increment
        case 0x1A: // Accumulator
            CPUInstructions::inc_accumulator(this);
            break;
        case 0xE6: // Direct Page
            CPUInstructions::inc_direct_page(this);
            break;
        case 0xF6: // Direct Page, X
            CPUInstructions::inc_direct_page_x(this);
            break;
        case 0xEE: // Absolute
            CPUInstructions::inc_absolute(this);
            break;
        case 0xFE: // Absolute, X
            CPUInstructions::inc_absolute_x(this);
            break;
        case 0xE8: // INX
            CPUInstructions::inx(this);
            break;
        case 0xC8: // INY
            CPUInstructions::iny(this);
            break;
            
        // DEC - Decrement
        case 0x3A: // Accumulator
            CPUInstructions::dec_accumulator(this);
            break;
        case 0xC6: // Direct Page
            CPUInstructions::dec_direct_page(this);
            break;
        case 0xD6: // Direct Page, X
            CPUInstructions::dec_direct_page_x(this);
            break;
        case 0xCE: // Absolute
            CPUInstructions::dec_absolute(this);
            break;
        case 0xDE: // Absolute, X
            CPUInstructions::dec_absolute_x(this);
            break;
        case 0xCA: // DEX
            CPUInstructions::dex(this);
            break;
        case 0x88: // DEY
            CPUInstructions::dey(this);
            break;
            
        // CMP - Compare Accumulator
        case 0xC9: // Immediate
            CPUInstructions::cmp_immediate(this);
            break;
        case 0xC5: // Direct Page
            CPUInstructions::cmp_direct_page(this);
            break;
        case 0xD5: // Direct Page, X
            CPUInstructions::cmp_direct_page_x(this);
            break;
        case 0xCD: // Absolute
            CPUInstructions::cmp_absolute(this);
            break;
        case 0xDD: // Absolute, X
            CPUInstructions::cmp_absolute_x(this);
            break;
        case 0xD9: // Absolute, Y
            CPUInstructions::cmp_absolute_y(this);
            break;
        case 0xC1: // (Direct Page, X)
            CPUInstructions::cmp_dp_indirect_x(this);
            break;
        case 0xD1: // (Direct Page), Y
            CPUInstructions::cmp_dp_indirect_y(this);
            break;
        case 0xD2: // (Direct Page)
            CPUInstructions::cmp_dp_indirect(this);
            break;
        case 0xC7: // [Direct Page]
            CPUInstructions::cmp_dp_indirect_long(this);
            break;
        case 0xD7: // [Direct Page], Y
            CPUInstructions::cmp_dp_indirect_long_y(this);
            break;
        case 0xCF: // Absolute Long
            CPUInstructions::cmp_absolute_long(this);
            break;
        case 0xDF: // Absolute Long, X
            CPUInstructions::cmp_absolute_long_x(this);
            break;
        case 0xC3: // Stack Relative
            CPUInstructions::cmp_stack_relative(this);
            break;
        case 0xD3: // Stack Relative Indirect, Y
            CPUInstructions::cmp_stack_relative_indirect_y(this);
            break;
            
        // CPX - Compare X Register
        case 0xE0: // Immediate
            CPUInstructions::cpx_immediate(this);
            break;
        case 0xE4: // Direct Page
            CPUInstructions::cpx_direct_page(this);
            break;
        case 0xEC: // Absolute
            CPUInstructions::cpx_absolute(this);
            break;
            
        // CPY - Compare Y Register
        case 0xC0: // Immediate
            CPUInstructions::cpy_immediate(this);
            break;
        case 0xC4: // Direct Page
            CPUInstructions::cpy_direct_page(this);
            break;
        case 0xCC: // Absolute
            CPUInstructions::cpy_absolute(this);
            break;
            
        // AND - Logical AND
        case 0x29: // Immediate
            CPUInstructions::and_immediate(this);
            break;
        case 0x25: // Direct Page
            CPUInstructions::and_direct_page(this);
            break;
        case 0x35: // Direct Page, X
            CPUInstructions::and_direct_page_x(this);
            break;
        case 0x2D: // Absolute
            CPUInstructions::and_absolute(this);
            break;
        case 0x3D: // Absolute, X
            CPUInstructions::and_absolute_x(this);
            break;
        case 0x39: // Absolute, Y
            CPUInstructions::and_absolute_y(this);
            break;
        case 0x21: // (Direct Page, X)
            CPUInstructions::and_dp_indirect_x(this);
            break;
        case 0x31: // (Direct Page), Y
            CPUInstructions::and_dp_indirect_y(this);
            break;
        case 0x32: // (Direct Page)
            CPUInstructions::and_dp_indirect(this);
            break;
        case 0x27: // [Direct Page]
            CPUInstructions::and_dp_indirect_long(this);
            break;
        case 0x37: // [Direct Page], Y
            CPUInstructions::and_dp_indirect_long_y(this);
            break;
            
        // ORA - Logical OR
        case 0x09: // Immediate
            CPUInstructions::ora_immediate(this);
            break;
        case 0x05: // Direct Page
            CPUInstructions::ora_direct_page(this);
            break;
        case 0x15: // Direct Page, X
            CPUInstructions::ora_direct_page_x(this);
            break;
        case 0x0D: // Absolute
            CPUInstructions::ora_absolute(this);
            break;
        case 0x1D: // Absolute, X
            CPUInstructions::ora_absolute_x(this);
            break;
        case 0x19: // Absolute, Y
            CPUInstructions::ora_absolute_y(this);
            break;
        case 0x01: // (Direct Page, X)
            CPUInstructions::ora_dp_indirect_x(this);
            break;
        case 0x11: // (Direct Page), Y
            CPUInstructions::ora_dp_indirect_y(this);
            break;
        case 0x12: // (Direct Page)
            CPUInstructions::ora_dp_indirect(this);
            break;
        case 0x07: // [Direct Page]
            CPUInstructions::ora_dp_indirect_long(this);
            break;
        case 0x17: // [Direct Page], Y
            CPUInstructions::ora_dp_indirect_long_y(this);
            break;
            
        // EOR - Logical XOR
        case 0x49: // Immediate
            CPUInstructions::eor_immediate(this);
            break;
        case 0x45: // Direct Page
            CPUInstructions::eor_direct_page(this);
            break;
        case 0x55: // Direct Page, X
            CPUInstructions::eor_direct_page_x(this);
            break;
        case 0x4D: // Absolute
            CPUInstructions::eor_absolute(this);
            break;
        case 0x5D: // Absolute, X
            CPUInstructions::eor_absolute_x(this);
            break;
        case 0x59: // Absolute, Y
            CPUInstructions::eor_absolute_y(this);
            break;
        case 0x41: // (Direct Page, X)
            CPUInstructions::eor_dp_indirect_x(this);
            break;
        case 0x51: // (Direct Page), Y
            CPUInstructions::eor_dp_indirect_y(this);
            break;
        case 0x52: // (Direct Page)
            CPUInstructions::eor_dp_indirect(this);
            break;
        case 0x47: // [Direct Page]
            CPUInstructions::eor_dp_indirect_long(this);
            break;
        case 0x57: // [Direct Page], Y
            CPUInstructions::eor_dp_indirect_long_y(this);
            break;
            
        // Branch Instructions
        case 0x90: // BCC - Branch if Carry Clear
            CPUInstructions::bcc(this);
            break;
        case 0xB0: // BCS - Branch if Carry Set
            CPUInstructions::bcs(this);
            break;
        case 0xF0: // BEQ - Branch if Equal
            CPUInstructions::beq(this);
            break;
        case 0xD0: // BNE - Branch if Not Equal
            CPUInstructions::bne(this);
            break;
        case 0x30: // BMI - Branch if Minus
            CPUInstructions::bmi(this);
            break;
        case 0x10: // BPL - Branch if Plus
            CPUInstructions::bpl(this);
            break;
        case 0x50: // BVC - Branch if Overflow Clear
            CPUInstructions::bvc(this);
            break;
        case 0x70: // BVS - Branch if Overflow Set
            CPUInstructions::bvs(this);
            break;
        case 0x80: // BRA - Branch Always
            CPUInstructions::bra(this);
            break;
        case 0x82: // BRL - Branch Always Long
            CPUInstructions::brl(this);
            break;
            
        // Shift and Rotate Instructions
        // ASL - Arithmetic Shift Left
        case 0x0A: // Accumulator
            CPUInstructions::asl_accumulator(this);
            break;
        case 0x06: // Direct Page
            CPUInstructions::asl_direct_page(this);
            break;
        case 0x16: // Direct Page, X
            CPUInstructions::asl_direct_page_x(this);
            break;
        case 0x0E: // Absolute
            CPUInstructions::asl_absolute(this);
            break;
        case 0x1E: // Absolute, X
            CPUInstructions::asl_absolute_x(this);
            break;
            
        // LSR - Logical Shift Right
        case 0x4A: // Accumulator
            CPUInstructions::lsr_accumulator(this);
            break;
        case 0x46: // Direct Page
            CPUInstructions::lsr_direct_page(this);
            break;
        case 0x56: // Direct Page, X
            CPUInstructions::lsr_direct_page_x(this);
            break;
        case 0x4E: // Absolute
            CPUInstructions::lsr_absolute(this);
            break;
        case 0x5E: // Absolute, X
            CPUInstructions::lsr_absolute_x(this);
            break;
            
        // ROL - Rotate Left
        case 0x2A: // Accumulator
            CPUInstructions::rol_accumulator(this);
            break;
        case 0x26: // Direct Page
            CPUInstructions::rol_direct_page(this);
            break;
        case 0x36: // Direct Page, X
            CPUInstructions::rol_direct_page_x(this);
            break;
        case 0x2E: // Absolute
            CPUInstructions::rol_absolute(this);
            break;
        case 0x3E: // Absolute, X
            CPUInstructions::rol_absolute_x(this);
            break;
            
        // ROR - Rotate Right
        case 0x6A: // Accumulator
            CPUInstructions::ror_accumulator(this);
            break;
        case 0x66: // Direct Page
            CPUInstructions::ror_direct_page(this);
            break;
        case 0x76: // Direct Page, X
            CPUInstructions::ror_direct_page_x(this);
            break;
        case 0x6E: // Absolute
            CPUInstructions::ror_absolute(this);
            break;
        case 0x7E: // Absolute, X
            CPUInstructions::ror_absolute_x(this);
            break;
            
        // Bit Instructions
        case 0x89: // Immediate
            CPUInstructions::bit_immediate(this);
            break;
        case 0x24: // Direct Page
            CPUInstructions::bit_direct_page(this);
            break;
        case 0x2C: // Absolute
            CPUInstructions::bit_absolute(this);
            break;
        case 0x3C: // Absolute, X
            CPUInstructions::bit_absolute_x(this);
            break;
            
        // Block Move Instructions
        case 0x44: // MVP - Move Positive
            CPUInstructions::mvp(this);
            break;
        case 0x54: // MVN - Move Negative
            CPUInstructions::mvn(this);
            break;
            
        // Default case for unimplemented instructions
        default:
            cycles = 2; // Default cycle count
            // Don't increment PC for unimplemented instructions to avoid infinite loops
            // Instead, increment PC by 1 to move to next instruction
            pc++;
            break;
    }
}

// Interrupt request
void CPU::irq() {
    if (!get_flag(I)) {
        CPUHelpers::handle_irq(this);
    }
}

// Non-maskable interrupt
void CPU::nmi() {
    CPUHelpers::handle_nmi(this);
}

// Flag management
void CPU::set_flag(FLAGS f, bool v) {
    if (v) {
        p |= f;
    } else {
        p &= ~f;
    }
}

bool CPU::get_flag(FLAGS f) const {
    return (p & f) != 0;
}

// Helper functions
void CPU::setZN(uint16_t value, bool is16) {
    set_flag(Z, value == 0);
    set_flag(N, is16 ? (value & 0x8000) : (value & 0x80));
}

void CPU::validate_stack_pointer() {
    CPUHelpers::validate_stack_pointer(this);
}