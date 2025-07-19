#ifndef SNES_HPP
#define SNES_HPP

#include <cstdint>
#include <memory>
#include <string>

#include "cpu.hpp"
#include "ppu.hpp"
#include "cartridge.hpp"
#include "controller.hpp"

class Bus;

class SNES {
public:
    SNES();
    ~SNES();

    void insert_cartridge(const std::string& rom_path);
    void power_on();
    void reset();
    void step();

    // Components
    std::shared_ptr<CPU> cpu;
    std::shared_ptr<PPU> ppu;
    std::shared_ptr<Cartridge> cartridge;
    std::shared_ptr<Bus> bus;
    std::shared_ptr<Controller> controller1;
    std::shared_ptr<Controller> controller2;


private:
    uint64_t total_cycles = 0;
};

#endif // SNES_HPP
