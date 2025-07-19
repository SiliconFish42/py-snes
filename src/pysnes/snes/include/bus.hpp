#ifndef BUS_HPP
#define BUS_HPP

#include <cstdint>
#include <array>
#include <memory>

#include "cpu.hpp"
#include "ppu.hpp"
#include "cartridge.hpp"
#include "controller.hpp"

class Bus {
public:
    Bus();
    ~Bus();

    // Read and Write
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);

    // Component Connections
    void connect_cpu(std::shared_ptr<CPU> n_cpu);
    void connect_ppu(std::shared_ptr<PPU> n_ppu);
    void connect_cartridge(std::shared_ptr<Cartridge> n_cartridge);
    void connect_controller1(std::shared_ptr<Controller> n_controller);
    void connect_controller2(std::shared_ptr<Controller> n_controller);


private:
    std::shared_ptr<CPU> cpu;
    std::shared_ptr<PPU> ppu;
    std::shared_ptr<Cartridge> cartridge;
    std::shared_ptr<Controller> controller1;
    std::shared_ptr<Controller> controller2;


    // CPU RAM
    std::array<uint8_t, 2048> cpu_ram;
};

#endif // BUS_HPP
