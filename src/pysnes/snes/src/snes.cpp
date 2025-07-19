#include "snes.hpp"
#include "bus.hpp"
#include "cartridge.hpp"
#include "cpu.hpp"
#include "ppu.hpp"       // <-- Add PPU include
#include "controller.hpp" // <-- Add Controller include

struct SNES::Impl {
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    std::shared_ptr<Cartridge> cartridge;
    std::shared_ptr<PPU> ppu;

    Impl() {
        bus = std::make_shared<Bus>();
        cpu = bus->cpu;
        ppu = bus->ppu;
    }
};

SNES::SNES() : pimpl(std::make_unique<Impl>()) {}
SNES::~SNES() = default;

void SNES::insert_cartridge(const std::string &rom_path) {
    pimpl->cartridge = std::make_shared<Cartridge>(rom_path);
    pimpl->bus->cartridge = pimpl->cartridge;
}

void SNES::power_on() {
    pimpl->cpu->connect_bus(pimpl->bus);
    pimpl->cpu->power_on();
    pimpl->ppu->power_on();
}

void SNES::reset() {
    pimpl->cpu->reset();
}

void SNES::step() {
    pimpl->cpu->step();
}

std::vector<uint32_t>& SNES::get_screen() {
    return pimpl->bus->ppu->get_screen();
}

void SNES::set_controller_state(int controller_num, uint8_t state) {
    if (controller_num == 1) {
        pimpl->bus->controller1->buttons = state;
    } else if (controller_num == 2) {
        pimpl->bus->controller2->buttons = state;
    }
}