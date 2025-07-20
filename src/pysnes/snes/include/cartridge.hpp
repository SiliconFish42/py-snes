#pragma once
#include <cstdint>
#include <string>
#include <vector>

class Cartridge {
  public:
    Cartridge(const std::string &rom_path);
    ~Cartridge();

    bool is_loaded();

    // Communication with the main CPU bus
    uint8_t cpu_read(uint16_t addr, bool bReadOnly = false);
    void cpu_write(uint16_t addr, uint8_t data);

    // Communication with the PPU bus
    bool ppu_read(uint16_t addr, uint8_t &data);
    bool ppu_write(uint16_t addr, uint8_t data);

    void reset();

  private:
    std::vector<uint8_t> rom_data;
    bool loaded = false;
};