#pragma once
#include <cstdint>
#include <array>
#include <memory>

class CPU;
class PPU;
class Cartridge;
class Controller;

// SNES Bus: connects CPU, PPU, WRAM, Cartridge, Controllers, etc.
class Bus {
public:
    Bus();
    ~Bus();

    // 24-bit address space read/write
    uint8_t read(uint32_t addr, bool readonly = false);
    void write(uint32_t addr, uint8_t data);

    // Connect devices
    void connect_cpu(std::shared_ptr<CPU> cpu_);
    void connect_ppu(std::shared_ptr<PPU> ppu_);
    void connect_cartridge(std::shared_ptr<Cartridge> cart_);
    void connect_controller(int port, std::shared_ptr<Controller> ctrl_);

    // Reset bus and all devices
    void reset();

    // Add public getters for devices
    std::shared_ptr<CPU> get_cpu() const { return cpu; }
    std::shared_ptr<PPU> get_ppu() const { return ppu; }
    std::shared_ptr<Cartridge> get_cartridge() const { return cart; }
    std::shared_ptr<Controller> get_controller(int port) const { return (port >= 0 && port < 2) ? controllers[port] : nullptr; }

    // Interrupt vector setters for testing
    void set_interrupt_vector(uint8_t low, uint8_t high) {
        interrupt_vector_low = low;
        interrupt_vector_high = high;
    }

private:
    // 128KB Work RAM (WRAM)
    std::array<uint8_t, 128 * 1024> wram;

    // Devices
    std::shared_ptr<CPU> cpu;
    std::shared_ptr<PPU> ppu;
    std::shared_ptr<Cartridge> cart;
    std::array<std::shared_ptr<Controller>, 2> controllers;

    // Interrupt vectors
    uint8_t interrupt_vector_low = 0x00;
    uint8_t interrupt_vector_high = 0x00;

    // TODO: Add DMA, APU, etc.
};
