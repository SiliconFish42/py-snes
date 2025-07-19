#include "bus.hpp"

Bus::Bus() {
    cpu_ram.fill(0);
}

Bus::~Bus() = default;

uint8_t Bus::read(uint16_t addr) {
    if (addr >= 0x0000 && addr <= 0x1FFF) {
        // CPU RAM (mirrored every 2KB)
        return cpu_ram[addr & 0x07FF];
    } else if (addr >= 0x2000 && addr <= 0x3FFF) {
        // PPU Registers (mirrored every 8 bytes)
        return ppu->read_register(addr & 0x0007);
    } else if (addr == 0x4016) {
        return controller1->read();
    } else if (addr == 0x4017) {
        return controller2->read();
    } else if (addr >= 0x4020 && addr <= 0xFFFF) {
        // Cartridge space
        return cartridge->read(addr);
    }
    return 0x00;
}

void Bus::write(uint16_t addr, uint8_t data) {
    if (addr >= 0x0000 && addr <= 0x1FFF) {
        // CPU RAM (mirrored every 2KB)
        cpu_ram[addr & 0x07FF] = data;
    } else if (addr >= 0x2000 && addr <= 0x3FFF) {
        // PPU Registers (mirrored every 8 bytes)
        ppu->write_register(addr & 0x0007, data);
    } else if (addr == 0x4014) {
        // OAM DMA
        ppu->start_dma(data);
    } else if (addr == 0x4016) {
        controller1->strobe(data);
        controller2->strobe(data);
    }
    else if (addr >= 0x4020 && addr <= 0xFFFF) {
        // Cartridge space
        cartridge->write(addr, data);
    }
}

void Bus::connect_cpu(std::shared_ptr<CPU> n_cpu) {
    this->cpu = n_cpu;
    this->cpu->connect_bus(this);
}

void Bus::connect_ppu(std::shared_ptr<PPU> n_ppu) {
    this->ppu = n_ppu;
}

void Bus::connect_cartridge(std::shared_ptr<Cartridge> n_cartridge) {
    this->cartridge = n_cartridge;
    this->ppu->connect_cartridge(n_cartridge);
}

void Bus::connect_controller1(std::shared_ptr<Controller> n_controller) {
    this->controller1 = n_controller;
}

void Bus::connect_controller2(std::shared_ptr<Controller> n_controller) {
    this->controller2 = n_controller;
}
