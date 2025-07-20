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
        cpu = std::make_shared<CPU>();
        ppu = std::make_shared<PPU>();
        bus->connect_cpu(cpu);
        bus->connect_ppu(ppu);
    }
};

SNES::SNES() : pimpl(std::make_unique<Impl>()) {}
SNES::~SNES() = default;

void SNES::insert_cartridge(const std::string &rom_path) {
    pimpl->cartridge = std::make_shared<Cartridge>(rom_path);
    pimpl->bus->connect_cartridge(pimpl->cartridge);
}

void SNES::power_on() {
    pimpl->cpu->connect_bus(pimpl->bus);
    // No power_on() method in CPU/PPU, so just reset
    pimpl->cpu->reset();
    pimpl->ppu->reset();
}

void SNES::reset() {
    pimpl->cpu->reset();
    pimpl->ppu->reset();
    if (pimpl->cartridge) pimpl->cartridge->reset();
    pimpl->bus->reset();
}

void SNES::step() {
    pimpl->cpu->step();
}

std::vector<uint32_t>& SNES::get_screen() {
    // PPU framebuffer is uint8_t, but API expects uint32_t. Cast for now.
    // This will need a real conversion for a real emulator.
    static std::vector<uint32_t> fake_screen;
    // auto& fb = pimpl->ppu->get_screen();
    // fake_screen.resize(fb.size() / 3);
    // for (size_t i = 0; i < fake_screen.size(); ++i) {
    //     fake_screen[i] = (fb[i*3] << 16) | (fb[i*3+1] << 8) | fb[i*3+2];
    // }
    return fake_screen;
}

void SNES::set_controller_state(int controller_num, uint8_t state) {
    auto ctrl = pimpl->bus->get_controller(controller_num - 1);
    if (ctrl) ctrl->buttons = state;
}
