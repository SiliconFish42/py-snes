#include "bus.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "cartridge.hpp"
#include "controller.hpp"
#include <cstring>

Bus::Bus() {
    wram.fill(0);
    controllers.fill(nullptr);
}

Bus::~Bus() {}

void Bus::connect_cpu(std::shared_ptr<CPU> cpu_) { cpu = cpu_; }
void Bus::connect_ppu(std::shared_ptr<PPU> ppu_) { 
    ppu = ppu_; 
    if (ppu) ppu->set_bus(this);
}
void Bus::connect_cartridge(std::shared_ptr<Cartridge> cart_) { cart = cart_; }
void Bus::connect_controller(int port, std::shared_ptr<Controller> ctrl_) {
    if (port >= 0 && port < 2) controllers[port] = ctrl_;
}

void Bus::reset() {
    wram.fill(0);
    if (cpu) cpu->reset();
    if (ppu) ppu->reset();
    if (cart) cart->reset();
    for (auto &c : controllers) if (c) c->reset();
}

// 24-bit address space read
uint8_t Bus::read(uint32_t addr, bool readonly) {
    // Mirror $0000-$1FFF to WRAM (bank 0)
    if (addr < 0x2000) {
        return wram[addr];
    }
    // WRAM: $7E:0000–$7F:FFFF (128KB, mirrored)
    if ((addr >= 0x7E0000 && addr <= 0x7FFFFF)) {
        return wram[addr - 0x7E0000];
    }
    // PPU registers: $2100–$213F (mirrored every 0x10000)
    if ((addr & 0xFFFF) >= 0x2100 && (addr & 0xFFFF) <= 0x213F) {
        if (ppu) return ppu->cpu_read(addr & 0xFFFF);
        return 0x00;
    }
    // Cartridge ROM/RAM: $8000–$FFFF (LoROM/HiROM mapping simplified)
    if (cart && (addr & 0xFFFF) >= 0x8000) {
        // Mask to 16 bits for now; TODO: support full 24-bit mapping
        return cart->cpu_read(addr & 0xFFFF, readonly);
    }
    // Controller ports: $4016, $4017
    if ((addr & 0xFFFF) == 0x4016 && controllers[0]) return controllers[0]->read();
    if ((addr & 0xFFFF) == 0x4017 && controllers[1]) return controllers[1]->read();
    // Interrupt vectors: $FFFE-$FFFF (IRQ/BRK vector)
    if ((addr & 0xFFFF) == 0xFFFE) return interrupt_vector_low;
    if ((addr & 0xFFFF) == 0xFFFF) return interrupt_vector_high;
    // Open bus for unmapped
    return 0x00;
}

// 24-bit address space write
void Bus::write(uint32_t addr, uint8_t data) {
    // Mirror $0000-$1FFF to WRAM (bank 0)
    if (addr < 0x2000) {
        wram[addr] = data;
        return;
    }
    // WRAM: $7E:0000–$7F:FFFF (128KB, mirrored)
    if ((addr >= 0x7E0000 && addr <= 0x7FFFFF)) {
        wram[addr - 0x7E0000] = data;
        return;
    }
    // PPU registers: $2100–$213F (mirrored every 0x10000)
    if ((addr & 0xFFFF) >= 0x2100 && (addr & 0xFFFF) <= 0x213F) {
        if (ppu) ppu->cpu_write(addr & 0xFFFF, data);
        return;
    }
    // Cartridge ROM/RAM: $8000–$FFFF (LoROM/HiROM mapping simplified)
    if (cart && (addr & 0xFFFF) >= 0x8000) {
        // Mask to 16 bits for now; TODO: support full 24-bit mapping
        cart->cpu_write(addr & 0xFFFF, data);
        return;
    }
    // Controller ports: $4016, $4017
    if ((addr & 0xFFFF) == 0x4016 && controllers[0]) { controllers[0]->write(data); return; }
    if ((addr & 0xFFFF) == 0x4017 && controllers[1]) { controllers[1]->write(data); return; }
    // Ignore writes to unmapped
}

// If you add new device types or features, add stubs here for future expansion.
// Example: DMA, APU, etc.
//
// void Bus::connect_dma(std::shared_ptr<DMA> dma_) { /* TODO: implement DMA connection */ }
// void Bus::connect_apu(std::shared_ptr<APU> apu_) { /* TODO: implement APU connection */ }
//
// Add more stubs as needed for full SNES hardware emulation.
