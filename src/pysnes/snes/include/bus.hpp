#pragma once
#include <array>
#include <cstdint>
#include <memory>

#include "cartridge.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "controller.hpp"

class Bus : public std::enable_shared_from_this<Bus> {
  public:
    Bus();
    ~Bus();

    // Components
    std::shared_ptr<CPU> cpu;
    std::shared_ptr<PPU> ppu;
    std::shared_ptr<Cartridge> cartridge;
    std::shared_ptr<Controller> controller1; // <-- Add controller 1
    std::shared_ptr<Controller> controller2; // <-- Add controller 2

    // Fake RAM
    std::array<uint8_t, 64 * 1024> ram;

    // Bus Read and Write
    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr, bool bReadOnly = false);
};