#ifndef CARTRIDGE_HPP
#define CARTRIDGE_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

class Cartridge {
public:
    Cartridge(const std::string& rom_path);
    ~Cartridge();

    bool is_loaded() const;

    // Read and Write from CPU bus
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);

    // Read and Write from PPU bus
    bool ppu_read(uint16_t addr, uint8_t& data);
    bool ppu_write(uint16_t addr, uint8_t data);

private:
    bool loaded = false;

    std::vector<uint8_t> prg_memory; // Program ROM
    std::vector<uint8_t> chr_memory; // Character ROM (pattern tables)

    uint8_t mapper_id = 0;
    uint8_t prg_banks = 0;
    uint8_t chr_banks = 0;

    // A pointer to the current mapper would go here
    // std::shared_ptr<Mapper> mapper;
};

#endif // CARTRIDGE_HPP
