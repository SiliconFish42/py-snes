#ifndef PPU_HPP
#define PPU_HPP

#include <cstdint>
#include <memory>
#include <vector>
#include <array>

#include "cartridge.hpp"

class PPU {
public:
    PPU();
    ~PPU();

    // PPU Registers
    uint8_t read_register(uint16_t addr);
    void write_register(uint16_t addr, uint8_t data);

    void connect_cartridge(std::shared_ptr<Cartridge> cartridge);

    // PPU Core
    void step();
    void reset();
    void power_on();

    // For DMA
    void start_dma(uint8_t page);

    // Screen buffer
    std::vector<uint32_t>& get_screen();
    bool frame_complete = false;
    bool nmi = false;

private:
    std::shared_ptr<Cartridge> cartridge;

    // PPU Memory
    std::array<uint8_t, 2048> name_tables;
    std::array<uint8_t, 32> palette_ram;
    std::array<uint8_t, 256> oam_data;

    // Internal Registers
    uint16_t vram_addr = 0x0000; // Current VRAM address
    uint16_t tram_addr = 0x0000; // Temporary VRAM address
    uint8_t fine_x = 0;

    uint8_t vram_read_buffer = 0;

    // PPU Control Register
    union {
        struct {
            uint8_t nametable_x : 1;
            uint8_t nametable_y : 1;
            uint8_t vram_increment : 1;
            uint8_t sprite_pattern_addr : 1;
            uint8_t bg_pattern_addr : 1;
            uint8_t sprite_size : 1;
            uint8_t master_slave : 1; // Unused
            uint8_t nmi_enable : 1;
        };
        uint8_t reg = 0;
    } control;

    // PPU Mask Register
    union {
        struct {
            uint8_t greyscale : 1;
            uint8_t show_bg_leftmost : 1;
            uint8_t show_sprites_leftmost : 1;
            uint8_t show_bg : 1;
            uint8_t show_sprites : 1;
            uint8_t emphasize_red : 1;
            uint8_t emphasize_green : 1;
            uint8_t emphasize_blue : 1;
        };
        uint8_t reg = 0;
    } mask;

    // PPU Status Register
    union {
        struct {
            uint8_t unused : 5;
            uint8_t sprite_overflow : 1;
            uint8_t sprite_zero_hit : 1;
            uint8_t vertical_blank : 1;
        };
        uint8_t reg = 0;
    } status;

    uint8_t oam_addr = 0;

    // DMA
    bool dma_transfer = false;
    uint8_t dma_page = 0x00;
    uint8_t dma_addr = 0x00;
    uint8_t dma_data = 0x00;


    // Rendering state
    int16_t scanline = -1;
    int16_t cycle = 0;

    // Screen buffer
    std::vector<uint32_t> screen;
    // System palette
    std::array<uint32_t, 64> palette_colors;

    // PPU Bus
    uint8_t ppu_read(uint16_t addr);
    void ppu_write(uint16_t addr, uint8_t data);
};

#endif // PPU_HPP
