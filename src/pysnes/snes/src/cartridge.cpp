#include "cartridge.hpp"
#include <fstream>

// This is a very basic cartridge implementation that only supports NROM (mapper 0)
// A full implementation would require a factory for different mapper types.

Cartridge::Cartridge(const std::string& rom_path) {
    std::ifstream ifs(rom_path, std::ios::binary | std::ios::ate);
    if (ifs) {
        std::streamsize size = ifs.tellg();
        ifs.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        if (ifs.read(buffer.data(), size)) {
            // iNES header format
            if (buffer[0] == 'N' && buffer[1] == 'E' && buffer[2] == 'S' && buffer[3] == 0x1A) {
                prg_banks = buffer[4];
                chr_banks = buffer[5];
                mapper_id = (buffer[7] & 0xF0) | (buffer[6] >> 4);

                // For now, we only support NROM which can have 1 or 2 PRG banks
                // and 1 CHR bank.
                if (mapper_id == 0) {
                    uint32_t prg_size = prg_banks * 16384;
                    uint32_t chr_size = chr_banks * 8192;

                    prg_memory.resize(prg_size);
                    std::copy(buffer.begin() + 16, buffer.begin() + 16 + prg_size, prg_memory.begin());

                    chr_memory.resize(chr_size);
                    std::copy(buffer.begin() + 16 + prg_size, buffer.begin() + 16 + prg_size + chr_size, chr_memory.begin());

                    loaded = true;
                }
            }
        }
    }
}

Cartridge::~Cartridge() = default;

bool Cartridge::is_loaded() const {
    return loaded;
}

uint8_t Cartridge::read(uint16_t addr) {
    if (addr >= 0x8000 && addr <= 0xFFFF) {
        // For NROM, the address is mirrored if there's only one PRG bank
        uint16_t mapped_addr = addr & (prg_banks > 1 ? 0x7FFF : 0x3FFF);
        return prg_memory[mapped_addr];
    }
    return 0;
}

void Cartridge::write(uint16_t addr, uint8_t data) {
    // NROM has no writable memory on the cartridge from the CPU side
}

bool Cartridge::ppu_read(uint16_t addr, uint8_t& data) {
    if (addr >= 0x0000 && addr <= 0x1FFF) {
        if (chr_banks > 0) {
            data = chr_memory[addr];
            return true;
        }
    }
    return false;
}

bool Cartridge::ppu_write(uint16_t addr, uint8_t data) {
    // NROM CHR memory is ROM, so no writing is allowed.
    // Some mappers have CHR RAM, which would be handled here.
    return false;
}
