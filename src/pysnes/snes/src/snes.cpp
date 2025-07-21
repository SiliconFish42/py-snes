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
    std::array<std::shared_ptr<Controller>, 2> controllers;

    Impl() {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        ppu = std::make_shared<PPU>();
        for (int i = 0; i < 2; ++i) {
            controllers[i] = std::make_shared<Controller>();
            bus->connect_controller(i, controllers[i]);
        }
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
    for (int i = 0; i < 4; ++i) {
        pimpl->ppu->step_dot();
    }
}

std::vector<uint32_t>& SNES::get_screen() {
    // Convert PPU framebuffer (uint16_t) to uint32_t RGBA8888
    static std::vector<uint32_t> framebuffer32;
    constexpr int height = PPU::kScreenHeight;
    constexpr int width = PPU::kScreenWidth;
    framebuffer32.resize(height * width);
    for (int y = 0; y < height; ++y) {
        const uint16_t* row = pimpl->ppu->get_framebuffer_row(y);
        for (int x = 0; x < width; ++x) {
            uint16_t color = row[x];
            // SNES color: 15-bit BGR (0bbbbbgggggrrrrr)
            uint8_t r = (color & 0x1F) << 3;
            uint8_t g = ((color >> 5) & 0x1F) << 3;
            uint8_t b = ((color >> 10) & 0x1F) << 3;
            framebuffer32[y * width + x] = (0xFF << 24) | (b << 16) | (g << 8) | r; // ARGB
        }
    }
    return framebuffer32;
}

std::vector<uint8_t> SNES::get_framebuffer_rgb() {
    return pimpl->ppu->get_framebuffer_rgb();
}

void SNES::set_controller_state(int controller_num, uint8_t state) {
    if (controller_num >= 1 && controller_num <= 2) {
        auto ctrl = pimpl->controllers[controller_num - 1];
        if (ctrl) ctrl->buttons = state;
    }
}
