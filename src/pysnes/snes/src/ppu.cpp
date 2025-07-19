#include "ppu.hpp"
#include "bus.hpp" // For DMA access

// This is a simplified PPU implementation. A full implementation is extremely complex.

PPU::PPU() {
    // Initialize palette colors (ARGB format)
    palette_colors = {
        0xFF545454, 0xFF001E74, 0xFF081090, 0xFF300088, 0xFF440064, 0xFF5C0030, 0xFF540400, 0xFF3C1800, 0xFF202A00, 0xFF083A00, 0xFF004000, 0xFF003C00, 0xFF00323C, 0xFF000000, 0xFF000000, 0xFF000000,
        0xFF989698, 0xFF084CC4, 0xFF3032EC, 0xFF5C1EE4, 0xFF8814B0, 0xFFA80074, 0xFFA81020, 0xFF883000, 0xFF504C00, 0xFF086A00, 0xFF007400, 0xFF006C00, 0xFF005A54, 0xFF000000, 0xFF000000, 0xFF000000,
        0xFFECEEEC, 0xFF4C9AEC, 0xFF787CEC, 0xFFB062EC, 0xFFE458E4, 0xFFEC58B4, 0xFFEC6A64, 0xFFD48820, 0xFFA0A200, 0xFF74C400, 0xFF4CD020, 0xFF38CC6C, 0xFF38B4CC, 0xFF3C3C3C, 0xFF000000, 0xFF000000,
        0xFFECEEEC, 0xFFA8CCEC, 0xFFBCBCEC, 0xFFD4B2EC, 0xFFECAEEC, 0xFFECAED4, 0xFFECC4B0, 0xFFE4D490, 0xFFCCD888, 0xFFB4DE88, 0xFFA8E298, 0xFF98DDD8, 0xFF98D2F0, 0xFF9C9C9C, 0xFF000000, 0xFF000000,
    };
}

PPU::~PPU() = default;

void PPU::connect_cartridge(std::shared_ptr<Cartridge> n_cartridge) {
    this->cartridge = n_cartridge;
}

std::array<uint32_t, 256 * 240>& PPU::get_screen() {
    return screen;
}

uint8_t PPU::read_register(uint16_t addr) {
    uint8_t data = 0x00;
    switch (addr) {
        case 0x2002: // Status
            data = (status.reg & 0xE0) | (vram_read_buffer & 0x1F);
            status.vertical_blank = 0;
            // w = 0;
            break;
        case 0x2004: // OAM Data
            data = oam_data[oam_addr];
            break;
        case 0x2007: // Data
            data = vram_read_buffer;
            vram_read_buffer = ppu_read(vram_addr);
            if (vram_addr >= 0x3F00) data = vram_read_buffer;
            vram_addr += (control.vram_increment ? 32 : 1);
            break;
    }
    return data;
}

void PPU::write_register(uint16_t addr, uint8_t data) {
    switch (addr) {
        case 0x2000: // Control
            control.reg = data;
            tram_addr = (tram_addr & 0xF3FF) | ((uint16_t)(data & 0x03) << 10);
            break;
        case 0x2001: // Mask
            mask.reg = data;
            break;
        case 0x2003: // OAM Address
            oam_addr = data;
            break;
        case 0x2004: // OAM Data
            oam_data[oam_addr] = data;
            oam_addr++;
            break;
        case 0x2005: // Scroll
            // First write
            // t.coarse_x = data >> 3
            // fine_x = data & 0x07
            // w = 1
            // Second write
            // t.coarse_y = data >> 3
            // t.fine_y = data & 0x07
            // w = 0
            break;
        case 0x2006: // Address
            // First write
            // t = (t & 0x00FF) | (data & 0x3F) << 8
            // w = 1
            // Second write
            // t = (t & 0xFF00) | data
            // v = t
            // w = 0
            break;
        case 0x2007: // Data
            ppu_write(vram_addr, data);
            vram_addr += (control.vram_increment ? 32 : 1);
            break;
    }
}

void PPU::start_dma(uint8_t page) {
    dma_page = page;
    dma_addr = 0x00;
    dma_transfer = true;
}


void PPU::step() {
    // This is a highly simplified step function.
    // A real PPU step function is a state machine that progresses through
    // scanlines and cycles, fetching data and rendering pixels.

    // For now, we'll just check for V-Blank to trigger NMI
    if (scanline == 241 && cycle == 1) {
        status.vertical_blank = 1;
        if (control.nmi_enable) {
            nmi = true;
        }
    }

    if (scanline == -1 && cycle == 1) {
        status.vertical_blank = 0;
        status.sprite_overflow = 0;
        status.sprite_zero_hit = 0;
    }

    // A very basic rendering placeholder
    if (scanline >= 0 && scanline < 240 && cycle >= 0 && cycle < 256) {
         // In a real emulator, pixel rendering for background and sprites happens here.
         // For now, let's just draw a color from the palette based on scanline.
         uint8_t palette_index = ppu_read(0x3F00 + (scanline / 8) % 16);
         screen[scanline * 256 + cycle] = palette_colors[palette_index];
    }


    cycle++;
    if (cycle >= 341) {
        cycle = 0;
        scanline++;
        if (scanline >= 261) {
            scanline = -1;
            frame_complete = true;
        }
    }
}

void PPU::reset() {
    control.reg = 0;
    mask.reg = 0;
    status.reg = 0;
    vram_addr = 0;
    tram_addr = 0;
    fine_x = 0;
    scanline = -1;
    cycle = 0;
    frame_complete = false;
    nmi = false;
}

void PPU::power_on() {
    reset();
    screen.fill(0);
}


uint8_t PPU::ppu_read(uint16_t addr) {
    addr &= 0x3FFF;
    if (cartridge && cartridge->ppu_read(addr, data)) {
        // Cartridge handles the read
    } else if (addr >= 0x0000 && addr <= 0x1FFF) {
        // Pattern Tables (from cartridge)
        // This would normally be handled by the cartridge mapping
    } else if (addr >= 0x2000 && addr <= 0x3EFF) {
        // Name Tables
        addr &= 0x0FFF;
        // This needs to handle mirroring from the cartridge
        return name_tables[addr];
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        // Palette RAM
        addr &= 0x001F;
        if (addr == 0x0010) addr = 0x0000;
        if (addr == 0x0014) addr = 0x0004;
        if (addr == 0x0018) addr = 0x0008;
        if (addr == 0x001C) addr = 0x000C;
        return palette_ram[addr];
    }
    return 0;
}

void PPU::ppu_write(uint16_t addr, uint8_t data) {
    addr &= 0x3FFF;
     if (cartridge && cartridge->ppu_write(addr, data)) {
        // Cartridge handles the write
    }
    else if (addr >= 0x0000 && addr <= 0x1FFF) {
        // Pattern Tables (usually in ROM on cartridge)
    } else if (addr >= 0x2000 && addr <= 0x3EFF) {
        // Name Tables
        addr &= 0x0FFF;
        // This needs to handle mirroring from the cartridge
        name_tables[addr] = data;
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        // Palette RAM
        addr &= 0x001F;
        if (addr == 0x0010) addr = 0x0000;
        if (addr == 0x0014) addr = 0x0004;
        if (addr == 0x0018) addr = 0x0008;
        if (addr == 0x001C) addr = 0x000C;
        palette_ram[addr] = data;
    }
}
