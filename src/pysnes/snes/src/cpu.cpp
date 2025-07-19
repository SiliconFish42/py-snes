#include "cpu.hpp"
#include "bus.hpp"

// The constructor is now empty as we no longer initialize the lookup table
CPU::CPU() {}
CPU::~CPU() {}

void CPU::connect_bus(std::shared_ptr<Bus> b) { bus = b; }

void CPU::power_on() {
    reset();
}

void CPU::step() {
    opcode = bus->read(pc++);
    cycles = 0; // Reset cycle count for the new instruction

    // The giant switch statement handles dispatching.
    // This is much simpler for the linker to understand.
    switch (opcode) {
    case 0x00: BRK(); break;
    case 0x01: IZX(); ORA(); break;
    case 0x05: ZP0(); ORA(); break;
    case 0x06: ZP0(); ASL(); break;
    case 0x08: PHP(); break;
    case 0x09: IMM(); ORA(); break;
    case 0x0A: IMM(); ASL(); break;
    case 0x0D: ABS(); ORA(); break;
    case 0x0E: ABS(); ASL(); break;
    case 0x10: REL(); BPL(); break;
    case 0x11: IZY(); ORA(); break;
    case 0x15: ZPX(); ORA(); break;
    case 0x16: ZPX(); ASL(); break;
    case 0x18: CLC(); break;
    case 0x19: ABY(); ORA(); break;
    case 0x1D: ABX(); ORA(); break;
    case 0x1E: ABX(); ASL(); break;
    case 0x20: ABS(); JSR(); break;
    case 0x21: IZX(); AND(); break;
    case 0x24: ZP0(); BIT(); break;
    case 0x25: ZP0(); AND(); break;
    case 0x26: ZP0(); ROL(); break;
    case 0x28: PLP(); break;
    case 0x29: IMM(); AND(); break;
    case 0x2A: IMM(); ROL(); break;
    case 0x2C: ABS(); BIT(); break;
    case 0x2D: ABS(); AND(); break;
    case 0x2E: ABS(); ROL(); break;
    case 0x30: REL(); BMI(); break;
    case 0x31: IZY(); AND(); break;
    case 0x35: ZPX(); AND(); break;
    case 0x36: ZPX(); ROL(); break;
    case 0x38: SEC(); break;
    case 0x39: ABY(); AND(); break;
    case 0x3D: ABX(); AND(); break;
    case 0x3E: ABX(); ROL(); break;
    case 0x40: RTI(); break;
    case 0x41: IZX(); EOR(); break;
    case 0x45: ZP0(); EOR(); break;
    case 0x46: ZP0(); LSR(); break;
    case 0x48: PHA(); break;
    case 0x49: IMM(); EOR(); break;
    case 0x4A: IMM(); LSR(); break;
    case 0x4C: ABS(); JMP(); break;
    case 0x4D: ABS(); EOR(); break;
    case 0x4E: ABS(); LSR(); break;
    case 0x50: REL(); BVC(); break;
    case 0x51: IZY(); EOR(); break;
    case 0x55: ZPX(); EOR(); break;
    case 0x56: ZPX(); LSR(); break;
    case 0x58: CLI(); break;
    case 0x59: ABY(); EOR(); break;
    case 0x5D: ABX(); EOR(); break;
    case 0x5E: ABX(); LSR(); break;
    case 0x60: RTS(); break;
    case 0x61: IZX(); ADC(); break;
    case 0x65: ZP0(); ADC(); break;
    case 0x66: ZP0(); ROR(); break;
    case 0x68: PLA(); break;
    case 0x69: IMM(); ADC(); break;
    case 0x6A: IMM(); ROR(); break;
    case 0x6C: IND(); JMP(); break;
    case 0x6D: ABS(); ADC(); break;
    case 0x6E: ABS(); ROR(); break;
    case 0x70: REL(); BVS(); break;
    case 0x71: IZY(); ADC(); break;
    case 0x75: ZPX(); ADC(); break;
    case 0x76: ZPX(); ROR(); break;
    case 0x78: SEI(); break;
    case 0x79: ABY(); ADC(); break;
    case 0x7D: ABX(); ADC(); break;
    case 0x7E: ABX(); ROR(); break;
    case 0x81: IZX(); STA(); break;
    case 0x84: ZP0(); STY(); break;
    case 0x85: ZP0(); STA(); break;
    case 0x86: ZP0(); STX(); break;
    case 0x88: DEY(); break;
    case 0x8A: TXA(); break;
    case 0x8C: ABS(); STY(); break;
    case 0x8D: ABS(); STA(); break;
    case 0x8E: ABS(); STX(); break;
    case 0x90: REL(); BCC(); break;
    case 0x91: IZY(); STA(); break;
    case 0x94: ZPX(); STY(); break;
    case 0x95: ZPX(); STA(); break;
    case 0x96: ZPY(); STX(); break;
    case 0x98: TYA(); break;
    case 0x99: ABY(); STA(); break;
    case 0x9A: TXS(); break;
    case 0x9D: ABX(); STA(); break;
    case 0xA0: IMM(); LDY(); break;
    case 0xA1: IZX(); LDA(); break;
    case 0xA2: IMM(); LDX(); break;
    case 0xA4: ZP0(); LDY(); break;
    case 0xA5: ZP0(); LDA(); break;
    case 0xA6: ZP0(); LDX(); break;
    case 0xA8: TAY(); break;
    case 0xA9: IMM(); LDA(); break;
    case 0xAA: TAX(); break;
    case 0xAC: ABS(); LDY(); break;
    case 0xAD: ABS(); LDA(); break;
    case 0xAE: ABS(); LDX(); break;
    case 0xB0: REL(); BCS(); break;
    case 0xB1: IZY(); LDA(); break;
    case 0xB4: ZPX(); LDY(); break;
    case 0xB5: ZPX(); LDA(); break;
    case 0xB6: ZPY(); LDX(); break;
    case 0xB8: CLV(); break;
    case 0xB9: ABY(); LDA(); break;
    case 0xBA: TSX(); break;
    case 0xBC: ABX(); LDY(); break;
    case 0xBD: ABX(); LDA(); break;
    case 0xBE: ABY(); LDX(); break;
    case 0xC0: IMM(); CPY(); break;
    case 0xC1: IZX(); CMP(); break;
    case 0xC4: ZP0(); CPY(); break;
    case 0xC5: ZP0(); CMP(); break;
    case 0xC6: ZP0(); DEC(); break;
    case 0xC8: INY(); break;
    case 0xC9: IMM(); CMP(); break;
    case 0xCA: DEX(); break;
    case 0xCC: ABS(); CPY(); break;
    case 0xCD: ABS(); CMP(); break;
    case 0xCE: ABS(); DEC(); break;
    case 0xD0: REL(); BNE(); break;
    case 0xD1: IZY(); CMP(); break;
    case 0xD5: ZPX(); CMP(); break;
    case 0xD6: ZPX(); DEC(); break;
    case 0xD8: CLD(); break;
    case 0xD9: ABY(); CMP(); break;
    case 0xDD: ABX(); CMP(); break;
    case 0xDE: ABX(); DEC(); break;
    case 0xE0: IMM(); CPX(); break;
    case 0xE1: IZX(); SBC(); break;
    case 0xE4: ZP0(); CPX(); break;
    case 0xE5: ZP0(); SBC(); break;
    case 0xE6: ZP0(); INC(); break;
    case 0xE8: INX(); break;
    case 0xE9: IMM(); SBC(); break;
    case 0xEA: NOP(); break;
    case 0xEC: ABS(); CPX(); break;
    case 0xED: ABS(); SBC(); break;
    case 0xEE: ABS(); INC(); break;
    case 0xF0: REL(); BEQ(); break;
    case 0xF1: IZY(); SBC(); break;
    case 0xF5: ZPX(); SBC(); break;
    case 0xF6: ZPX(); INC(); break;
    case 0xF8: SED(); break;
    case 0xF9: ABY(); SBC(); break;
    case 0xFD: ABX(); SBC(); break;
    case 0xFE: ABX(); INC(); break;
    default: XXX(); break; // Handle illegal opcodes
    }
}

// All stub implementations remain the same. This is where you will fill in logic.
uint8_t CPU::IMM() { addr_abs = pc++; return 0; }
uint8_t CPU::ABS() { uint16_t lo = bus->read(pc++); uint16_t hi = bus->read(pc++); addr_abs = (hi << 8) | lo; return 0; }
uint8_t CPU::ZP0() { addr_abs = bus->read(pc++); addr_abs &= 0x00FF; return 0; }
uint8_t CPU::ZPX() { addr_abs = (bus->read(pc++) + x); addr_abs &= 0x00FF; return 0; }
uint8_t CPU::ZPY() { addr_abs = (bus->read(pc++) + y); addr_abs &= 0x00FF; return 0; }
uint8_t CPU::ABX() { uint16_t lo = bus->read(pc++); uint16_t hi = bus->read(pc++); addr_abs = (hi << 8) | lo; addr_abs += x; if ((addr_abs & 0xFF00) != (hi << 8)) cycles++; return 0; }
uint8_t CPU::ABY() { uint16_t lo = bus->read(pc++); uint16_t hi = bus->read(pc++); addr_abs = (hi << 8) | lo; addr_abs += y; if ((addr_abs & 0xFF00) != (hi << 8)) cycles++; return 0; }
uint8_t CPU::IND() { uint16_t ptr_lo = bus->read(pc++); uint16_t ptr_hi = bus->read(pc++); uint16_t ptr = (ptr_hi << 8) | ptr_lo; if (ptr_lo == 0x00FF) { addr_abs = (bus->read(ptr & 0xFF00) << 8) | bus->read(ptr + 0); } else { addr_abs = (bus->read(ptr + 1) << 8) | bus->read(ptr + 0); } return 0; }
uint8_t CPU::IZX() { uint16_t t = bus->read(pc++); uint16_t lo = bus->read((uint16_t)(t + x) & 0x00FF); uint16_t hi = bus->read((uint16_t)(t + x + 1) & 0x00FF); addr_abs = (hi << 8) | lo; return 0; }
uint8_t CPU::IZY() { uint16_t t = bus->read(pc++); uint16_t lo = bus->read(t & 0x00FF); uint16_t hi = bus->read((t + 1) & 0x00FF); addr_abs = (hi << 8) | lo; addr_abs += y; if ((addr_abs & 0xFF00) != (hi << 8)) cycles++; return 0; }
uint8_t CPU::REL() { addr_rel = bus->read(pc++); if (addr_rel & 0x80) addr_rel |= 0xFF00; return 0; }
uint8_t CPU::ADC() { return 0; }
uint8_t CPU::AND() { return 0; }
uint8_t CPU::ASL() { return 0; }
uint8_t CPU::BCC() { return 0; }
uint8_t CPU::BCS() { return 0; }
uint8_t CPU::BEQ() { return 0; }
uint8_t CPU::BIT() { return 0; }
uint8_t CPU::BMI() { return 0; }
uint8_t CPU::BNE() { return 0; }
uint8_t CPU::BPL() { return 0; }
uint8_t CPU::BRK() { return 0; }
uint8_t CPU::BVC() { return 0; }
uint8_t CPU::BVS() { return 0; }
uint8_t CPU::CLC() { set_flag(C, false); return 0; }
uint8_t CPU::CLD() { set_flag(D, false); return 0; }
uint8_t CPU::CLI() { set_flag(I, false); return 0; }
uint8_t CPU::CLV() { set_flag(V, false); return 0; }
uint8_t CPU::CMP() { return 0; }
uint8_t CPU::CPX() { return 0; }
uint8_t CPU::CPY() { return 0; }
uint8_t CPU::DEC() { return 0; }
uint8_t CPU::DEX() { x--; set_flag(Z, x == 0x00); set_flag(N, x & 0x80); return 0; }
uint8_t CPU::DEY() { y--; set_flag(Z, y == 0x00); set_flag(N, y & 0x80); return 0; }
uint8_t CPU::EOR() { return 0; }
uint8_t CPU::INC() { return 0; }
uint8_t CPU::INX() { x++; set_flag(Z, x == 0x00); set_flag(N, x & 0x80); return 0; }
uint8_t CPU::INY() { y++; set_flag(Z, y == 0x00); set_flag(N, y & 0x80); return 0; }
uint8_t CPU::JMP() { pc = addr_abs; return 0; }
uint8_t CPU::JSR() { return 0; }
uint8_t CPU::LDA() { fetched = bus->read(addr_abs); a = fetched; set_flag(Z, a == 0x00); set_flag(N, a & 0x80); return 0; }
uint8_t CPU::LDX() { fetched = bus->read(addr_abs); x = fetched; set_flag(Z, x == 0x00); set_flag(N, x & 0x80); return 0; }
uint8_t CPU::LDY() { fetched = bus->read(addr_abs); y = fetched; set_flag(Z, y == 0x00); set_flag(N, y & 0x80); return 0; }
uint8_t CPU::LSR() { return 0; }
uint8_t CPU::NOP() { return 0; }
uint8_t CPU::ORA() { return 0; }
uint8_t CPU::PHA() { return 0; }
uint8_t CPU::PHP() { return 0; }
uint8_t CPU::PLA() { return 0; }
uint8_t CPU::PLP() { return 0; }
uint8_t CPU::ROL() { return 0; }
uint8_t CPU::ROR() { return 0; }
uint8_t CPU::RTI() { return 0; }
uint8_t CPU::RTS() { return 0; }
uint8_t CPU::SBC() { return 0; }
uint8_t CPU::SEC() { set_flag(C, true); return 0; }
uint8_t CPU::SED() { set_flag(D, true); return 0; }
uint8_t CPU::SEI() { set_flag(I, true); return 0; }
uint8_t CPU::STA() { bus->write(addr_abs, a); return 0; }
uint8_t CPU::STX() { bus->write(addr_abs, x); return 0; }
uint8_t CPU::STY() { bus->write(addr_abs, y); return 0; }
uint8_t CPU::TAX() { x = a; set_flag(Z, x == 0x00); set_flag(N, x & 0x80); return 0; }
uint8_t CPU::TAY() { y = a; set_flag(Z, y == 0x00); set_flag(N, y & 0x80); return 0; }
uint8_t CPU::TSX() { x = stkp; set_flag(Z, x == 0x00); set_flag(N, x & 0x80); return 0; }
uint8_t CPU::TXA() { a = x; set_flag(Z, a == 0x00); set_flag(N, a & 0x80); return 0; }
uint8_t CPU::TXS() { stkp = x; return 0; }
uint8_t CPU::TYA() { a = y; set_flag(Z, a == 0x00); set_flag(N, a & 0x80); return 0; }
uint8_t CPU::XXX() { return 0; }
void CPU::clock() {}
void CPU::reset() {}
void CPU::irq() {}
void CPU::nmi() {}
uint8_t CPU::get_flag(FLAGS f) { return (status & f) > 0 ? 1 : 0; }
void CPU::set_flag(FLAGS f, bool v) { if (v) status |= f; else status &= ~f; }