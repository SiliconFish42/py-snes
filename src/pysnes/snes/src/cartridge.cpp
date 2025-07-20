#include <fstream>
#include "cartridge.hpp"

Cartridge::Cartridge(const std::string &rom_path) {
    std::ifstream rom_file(rom_path, std::ios::binary | std::ios::ate);
    if (rom_file.is_open()) {
        std::streampos size = rom_file.tellg();
        rom_data.resize(size);
        rom_file.seekg(0, std::ios::beg);
        rom_file.read((char *)rom_data.data(), size);
        rom_file.close();
        loaded = true;
    }
}

Cartridge::~Cartridge() {}

bool Cartridge::is_loaded() {
    return loaded;
}

// The CPU is asking to read from the cartridge
uint8_t Cartridge::cpu_read(uint16_t addr, bool bReadOnly) {
    // Basic LoROM mapping for now
    if (addr >= 0x8000 && addr <= 0xFFFF) {
        // The address is masked to wrap around the ROM size
        return rom_data[addr & (rom_data.size() - 1)];
    }
    return 0; // Return 0 for addresses outside the ROM range
}

void Cartridge::cpu_write(uint16_t addr, uint8_t data) {
    // For now, we don't support writing to the cartridge (SRAM, etc.)
}

bool Cartridge::ppu_read(uint16_t addr, uint8_t &data) {
    // TODO: PPU reads from CHR ROM
    return false;
}

bool Cartridge::ppu_write(uint16_t addr, uint8_t data) {
    // For now, we don't support writing to the cartridge (SRAM, etc.)
    return false;
}

void Cartridge::reset() {}
