#include "snes.hpp"
#include "bus.hpp"

SNES::SNES() {
    // Create components
    cpu = std::make_shared<CPU>();
    ppu = std::make_shared<PPU>();
    cartridge = nullptr; // No cartridge inserted initially
    bus = std::make_shared<Bus>();
    controller1 = std::make_shared<Controller>();
    controller2 = std::make_shared<Controller>();


    // Connect components to the bus
    bus->connect_cpu(cpu);
    bus->connect_ppu(ppu);
    bus->connect_controller1(controller1);
    bus->connect_controller2(controller2);
}

SNES::~SNES() = default;

void SNES::insert_cartridge(const std::string& rom_path) {
    cartridge = std::make_shared<Cartridge>(rom_path);
    if (cartridge->is_loaded()) {
        bus->connect_cartridge(cartridge);
        reset();
    }
}

void SNES::power_on() {
    cpu->power_on();
    ppu->power_on();
    total_cycles = 0;
}

void SNES::reset() {
    cpu->reset();
    ppu->reset();
    total_cycles = 0;
}

void SNES::step() {
    // The PPU runs 3 times as fast as the CPU
    ppu->step();
    ppu->step();
    ppu->step();

    cpu->step();
}

