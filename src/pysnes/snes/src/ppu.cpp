#include "ppu.hpp"
#include <cstring>
#include <string>
#include <vector>
#include <fstream>
#include "bus.hpp"
#include "cpu.hpp"

PPU::PPU() {
    reset();
}

PPU::~PPU() {
    // No dynamic memory to free
}

void PPU::reset() {
    // Clear all PPU memory regions
    vram_.fill(0);
    cgram_.fill(0);
    oam_.fill(0);
    // Reset VRAM and CGRAM read buffers
    vram_read_buffer_ = 0;
    cgram_read_buffer_ = 0;
    // Reset PPU register state
    inidisp_ = 0;
    obsel_ = 0;
    bgmode_ = 0;
    mosaic_ = 0;
    for (int i = 0; i < 4; ++i) { 
        bg_sc_[i] = 0; 
        bg_hofs_[i] = 0; 
        bg_vofs_[i] = 0; 
        bg_hofs_latch_[i] = 0;
        bg_hofs_latch_state_[i] = true;
    }
    for (int i = 0; i < 2; ++i) { bg_nba_[i] = 0; }
    vmain_ = 0;
    vram_addr_ = 0;
    cgram_addr_ = 0;
    tm_ = 0;
    ts_ = 0;
    oam_addr_ = 0;
    oam_priority_rotation_ = false;
    oam_addr_msb_ = false;
    oam_latch_low_ = true;
    // TODO: Reset windowing, color math, mode 7, and status registers
    // TODO: Reset internal PPU state and registers
}

// VRAM access
uint8_t PPU::read_vram(uint16_t addr) const {
    return vram_[addr % vram_.size()];
}

void PPU::write_vram(uint16_t addr, uint8_t value) {
    vram_[addr % vram_.size()] = value;
}

// CGRAM access
uint8_t PPU::read_cgram(uint16_t addr) const {
    return cgram_[addr % cgram_.size()];
}

void PPU::write_cgram(uint16_t addr, uint8_t value) {
    cgram_[addr % cgram_.size()] = value;
}

// OAM access
uint8_t PPU::read_oam(uint16_t addr) const {
    return oam_[addr % oam_.size()];
}

void PPU::write_oam(uint16_t addr, uint8_t value) {
    oam_[addr % oam_.size()] = value;
}

// Register access stubs
uint8_t PPU::read_register(uint16_t addr) {
    switch (addr) {
        // $2100–$213F: PPU registers
        // $2100: INIDISP (Display Control)
        case 0x2100:
            // Write-only register - return 0 (open bus)
            return 0;
        // $2101: OBSEL (Object Size and Data Area Designation)
        case 0x2101:
            // Write-only register - return 0 (open bus)
            return 0;
        // $2102/$2103: OAM Address (write-only)
        case 0x2102:
        case 0x2103:
            return 0;
        // $2104: OAM Data Write (write-only)
        case 0x2104:
            return 0;
        // $2105: BG mode/char size
        case 0x2105:
            // Write-only register - return 0 (open bus)
            return 0;
        // $2106: Mosaic
        case 0x2106:
            // Write-only register - return 0 (open bus)
            return 0;
        // $2107–$210A: BGnSC tilemap base (write-only)
        case 0x2107: case 0x2108: case 0x2109: case 0x210A:
            return 0;
        // $210B–$210C: BGnNBA tile data base (write-only)
        case 0x210B: case 0x210C:
            return 0;
        // $210D–$2114: BG scroll registers (write-only)
        case 0x210D: case 0x210E: case 0x210F: case 0x2110:
        case 0x2111: case 0x2112: case 0x2113: case 0x2114:
            return 0;
        // $2115: VMAIN (VRAM Address Increment Mode)
        case 0x2115:
            // Write-only register - return 0 (open bus)
            return 0;
        // $2116/$2117: VRAM Address (write-only)
        case 0x2116: case 0x2117:
            return 0;
        // $2118/$2119: VRAM Data Write (write-only)
        case 0x2118: case 0x2119:
            return 0;
        // $211A–$2120: Misc (write-only)
        case 0x211A: case 0x211B: case 0x211C: case 0x211D:
        case 0x211E: case 0x211F: case 0x2120:
            return 0;
        // $2121: CGRAM Address (write-only)
        case 0x2121:
            return 0;
        // $2122: CGRAM Data Write (write-only)
        case 0x2122:
            return 0;
        // $2123–$2136: Windowing, color math, etc. (mostly write-only)
        case 0x2123: case 0x2124: case 0x2125: case 0x2126:
        case 0x2127: case 0x2128: case 0x2129: case 0x212A:
        case 0x212B: case 0x212C: case 0x212D: case 0x212E:
        case 0x212F: case 0x2130: case 0x2131: case 0x2132:
        case 0x2133: case 0x2134: case 0x2135: case 0x2136:
            return 0;
        // $2137: Latch H/V counter
        case 0x2137:
            // TODO: Implement H/V counter latch
            return 0;
        // $2138: OAM Data Read
        case 0x2138:
            if (oam_latch_low_) {
                return oam_[oam_addr_ % oam_.size()];
            } else {
                return oam_[(oam_addr_ % oam_.size()) + 1];
            }
        case 0x2139: { // VRAM Data Read (low byte)
            uint8_t result = vram_read_buffer_ & 0xFF;
            // Load buffer with word at current VRAM address
            vram_read_buffer_ = read_vram(vram_addr_) | (read_vram(vram_addr_ + 1) << 8);
            // Increment VRAM address based on VMAIN
            if (vmain_ & 0x80) {
                // Fixed increment
                vram_addr_ += (1 << ((vmain_ >> 4) & 0x07));
            } else {
                // Increment by 1
                vram_addr_ += 1;
            }
            return result;
        }
        case 0x213A: { // VRAM Data Read (high byte)
            uint8_t result = (vram_read_buffer_ >> 8) & 0xFF;
            // Load buffer with word at current VRAM address
            vram_read_buffer_ = read_vram(vram_addr_) | (read_vram(vram_addr_ + 1) << 8);
            // Increment VRAM address based on VMAIN
            if (vmain_ & 0x80) {
                // Fixed increment
                vram_addr_ += (1 << ((vmain_ >> 4) & 0x07));
            } else {
                // Increment by 1
                vram_addr_ += 1;
            }
            return result;
        }
        case 0x213B: { // CGRAM Data Read
            uint8_t result = cgram_read_buffer_;
            // Load buffer with byte at current CGRAM address
            cgram_read_buffer_ = read_cgram(cgram_addr_);
            // Increment CGRAM address
            cgram_addr_ = (cgram_addr_ + 1) & 0x1FF; // 9-bit address
            return result;
        }
        // $213C–$213F: Status registers
        case 0x213C: case 0x213D: case 0x213E: case 0x213F: {
            // Bit 7: VBLANK, Bit 6: HBLANK (simplified)
            uint8_t status = 0;
            if (vblank_) status |= 0x80;
            if (hblank_) status |= 0x40;
            return status;
        }
        default:
            // Unmapped or open bus
            return 0;
    }
}

void PPU::write_register(uint16_t addr, uint8_t value) {
    switch (addr) {
        case 0x2100: // INIDISP (Display control)
            inidisp_ = value;
            break;
        case 0x2101: // OBSEL (Object size/data area)
            obsel_ = value;
            break;
        case 0x2102: // OAM Address low byte
            oam_addr_ = (oam_addr_ & 0x100) | (value & 0xFF);
            oam_priority_rotation_ = (value & 0x80) != 0;
            oam_addr_msb_ = (value & 0x01) != 0;
            oam_latch_low_ = true; // Reset latch on address set
            break;
        case 0x2103: // OAM Address high bit
            oam_addr_ = (oam_addr_ & 0xFF) | ((value & 0x01) << 8);
            oam_latch_low_ = true; // Reset latch on address set
            break;
        case 0x2104: // OAM Data Write
            if (oam_latch_low_) {
                // Write low byte
                oam_[oam_addr_ % oam_.size()] = value;
            } else {
                // Write high byte (SNES OAM is 16-bit word addressed)
                oam_[(oam_addr_ % oam_.size()) + 1] = value;
                // Increment OAM address after high byte
                oam_addr_ = (oam_addr_ + 2) & 0x1FF;
            }
            oam_latch_low_ = !oam_latch_low_;
            break;
        case 0x2105: // BG mode/char size
            bgmode_ = value;
            break;
        case 0x2106: // Mosaic
            mosaic_ = value;
            break;
        case 0x2107: case 0x2108: case 0x2109: case 0x210A: // BGnSC tilemap base
            bg_sc_[addr - 0x2107] = value;
            break;
        case 0x210B: // BG1NBA/BG2NBA
            bg_nba_[0] = value;
            break;
        case 0x210C: // BG3NBA/BG4NBA
            bg_nba_[1] = value;
            break;
        case 0x210D: { // BG1HOFS (horizontal scroll)
            int bg = 0;
            if (bg_hofs_latch_state_[bg]) {
                // First write: low byte
                bg_hofs_latch_[bg] = value;
            } else {
                // Second write: high bit (bit 0), latch value
                bg_hofs_[bg] = (bg_hofs_latch_[bg] | ((value & 0x01) << 8));
            }
            bg_hofs_latch_state_[bg] = !bg_hofs_latch_state_[bg];
            break;
        }
        case 0x210E: { // BG2HOFS (horizontal scroll)
            int bg = 1;
            if (bg_hofs_latch_state_[bg]) {
                bg_hofs_latch_[bg] = value;
            } else {
                bg_hofs_[bg] = (bg_hofs_latch_[bg] | ((value & 0x01) << 8));
            }
            bg_hofs_latch_state_[bg] = !bg_hofs_latch_state_[bg];
            break;
        }
        case 0x210F: { // BG3HOFS (horizontal scroll)
            int bg = 2;
            if (bg_hofs_latch_state_[bg]) {
                bg_hofs_latch_[bg] = value;
            } else {
                bg_hofs_[bg] = (bg_hofs_latch_[bg] | ((value & 0x01) << 8));
            }
            bg_hofs_latch_state_[bg] = !bg_hofs_latch_state_[bg];
            break;
        }
        case 0x2110: { // BG4HOFS (horizontal scroll)
            int bg = 3;
            if (bg_hofs_latch_state_[bg]) {
                bg_hofs_latch_[bg] = value;
            } else {
                bg_hofs_[bg] = (bg_hofs_latch_[bg] | ((value & 0x01) << 8));
            }
            bg_hofs_latch_state_[bg] = !bg_hofs_latch_state_[bg];
            break;
        }
        case 0x2111: // BG1VOFS (vertical scroll)
            bg_vofs_[0] = value;
            break;
        case 0x2112: // BG2VOFS (vertical scroll)
            bg_vofs_[1] = value;
            break;
        case 0x2113: // BG3VOFS (vertical scroll)
            bg_vofs_[2] = value;
            break;
        case 0x2114: // BG4VOFS (vertical scroll)
            bg_vofs_[3] = value;
            break;
        case 0x2115: // VMAIN (VRAM Address Increment Mode)
            vmain_ = value;
            break;
        case 0x2116: // VMADDL (VRAM Address low byte)
            vram_addr_ = (vram_addr_ & 0xFF00) | value;
            break;
        case 0x2117: // VMADDH (VRAM Address high byte)
            vram_addr_ = (vram_addr_ & 0x00FF) | (value << 8);
            break;
        case 0x2118: // VMDATAL (VRAM Data Write low byte)
            write_vram(vram_addr_, value);
            // Increment VRAM address based on VMAIN
            if (vmain_ & 0x80) {
                // Fixed increment
                vram_addr_ += (1 << ((vmain_ >> 4) & 0x07));
            } else {
                // Increment by 1
                vram_addr_ += 1;
            }
            break;
        case 0x2119: // VMDATAH (VRAM Data Write high byte)
            write_vram(vram_addr_, value);
            // Increment VRAM address based on VMAIN
            if (vmain_ & 0x80) {
                // Fixed increment
                vram_addr_ += (1 << ((vmain_ >> 4) & 0x07));
            } else {
                // Increment by 1
                vram_addr_ += 1;
            }
            break;
        case 0x2121: // CGADD (CGRAM Address)
            cgram_addr_ = value;
            break;
        case 0x2122: // CGDATA (CGRAM Data Write)
            write_cgram(cgram_addr_, value);
            cgram_addr_ = (cgram_addr_ + 1) & 0x1FF; // 9-bit address
            break;
        case 0x212C: // Main screen designation (OBJ enable)
            tm_ = value;
            break;
        // TODO: Add more register logic as needed for $2100–$213F
        default:
            // Unimplemented registers: do nothing (open bus on read)
            break;
    }
}

// --- BG tilemap/tile data base helpers ---
uint32_t PPU::get_bg_tilemap_base(int bg) const {
    // BGnSC: bits 0-5 = tilemap base address (in VRAM, 2KB units)
    //         bit 6 = size (0=32x32, 1=64x32/32x64/64x64)
    //         bit 7 = unused
    if (bg < 0 || bg > 3) return 0;
    return (bg_sc_[bg] & 0x3F) * 0x800; // 2KB units
}

uint32_t PPU::get_bg_tiledata_base(int bg) const {
    // BGnNBA: bits 0-2 = tile data base address (in VRAM, 4KB units)
    //          bits 3-7 = unused
    if (bg < 0 || bg > 3) return 0;
    int nba_index = (bg < 2) ? 0 : 1;
    uint8_t nba = bg_nba_[nba_index];
    int shift = (bg % 2) * 4;
    return ((nba >> shift) & 0x07) * 0x1000; // 4KB units
}

// --- OAM attribute parsing ---
PPU::SpriteAttr PPU::parse_sprite_attr(int index) const {
    // Each sprite is 4 bytes in OAM (0-127 sprites)
    // OAM[4*index+0]: Y
    // OAM[4*index+1]: Tile
    // OAM[4*index+2]: Attr
    // OAM[4*index+3]: X low
    // X high bit is in OAM[0x200 + (index >> 2)]
    SpriteAttr attr{};
    if (index < 0 || index >= 128) return attr;
    int base = 4 * index;
    attr.y = oam_[base];
    attr.tile = oam_[base + 1];
    attr.attr = oam_[base + 2];
    attr.x_low = oam_[base + 3];
    int x_high_addr = 0x200 + (index >> 2);
    int shift = 2 * (index & 0x3);
    int x_high_bit = (oam_[x_high_addr] >> shift) & 0x01;
    attr.x_high = x_high_bit;
    attr.size = (oam_[x_high_addr] >> (shift + 1)) & 0x01;
    return attr;
}

// --- CGRAM palette access ---
uint16_t PPU::get_cgram_color(int index) const {
    // Each color is 2 bytes (little endian), 15-bit SNES BGR
    if (index < 0 || index >= 256) return 0;
    int addr = index * 2;
    uint16_t color = cgram_[addr] | (cgram_[addr + 1] << 8);
    return color & 0x7FFF; // 15-bit color
}

// --- Scanline/frame timing and rendering stubs ---
void PPU::step_dot() {
    dot_++;
    // HBlank: last 40 dots of each scanline (typical SNES)
    hblank_ = (dot_ >= (kDotsPerScanline - 40));
    if (dot_ == 0 && scanline_ < kScreenHeight) {
        render_scanline_stub();
    }
    if (dot_ >= kDotsPerScanline) {
        dot_ = 0;
        step_scanline();
    }
}

void PPU::step_scanline() {
    scanline_++;
    hblank_ = false;
    // VBlank: scanlines 224-261 (NTSC)
    vblank_ = (scanline_ >= kScreenHeight && scanline_ < kTotalScanlines);
    if (scanline_ == kScreenHeight) {
        // Start of VBlank
        render_sprite_stub();
        // Trigger NMI on CPU at start of VBLANK
        if (bus_ && bus_->get_cpu()) {
            bus_->get_cpu()->nmi();
        }
    }
    if (scanline_ >= kTotalScanlines) {
        scanline_ = 0;
        vblank_ = false;
        step_frame();
    }
}

void PPU::step_frame() {
    frame_++;
    scanline_ = 0;
    dot_ = 0;
    vblank_ = false;
    hblank_ = false;
    // Optionally clear framebuffer or do nothing
}

void PPU::render_scanline_stub() {
    // Simple stub that fills the current scanline with a pattern
    if (scanline_ >= 0 && scanline_ < kScreenHeight) {
        for (int x = 0; x < kScreenWidth; ++x) {
            framebuffer_[scanline_][x] = (scanline_ & 0x1F) << 10;
        }
    }
}

void PPU::render_bg_scanline_stub(int scanline) {
    // Mode 0: BG1, 2bpp, 32x32 tilemap, 8x8 tiles
    if (scanline < 0 || scanline >= kScreenHeight) return;
    int bg = 0; // BG1
    int tilemap_base = get_bg_tilemap_base(bg); // 0x0000
    int tiledata_base = get_bg_tiledata_base(bg); // 0x1000
    int hscroll = bg_hofs_[bg] & 0x1FF;
    int vscroll = bg_vofs_[bg] & 0x1FF;
    int tile_y = ((scanline + vscroll) / 8) % 32;
    for (int x = 0; x < kScreenWidth; ++x) {
        int tile_x = ((x + hscroll) >> 3) % 32;
        int map_addr = tilemap_base + 2 * (tile_y * 32 + tile_x);
        uint8_t tile_lo = vram_[map_addr % vram_.size()];
        uint8_t tile_hi = vram_[(map_addr + 1) % vram_.size()];
        uint16_t tile_index = tile_lo | ((tile_hi & 0x03) << 8); // 10 bits
        int palette = (tile_hi >> 2) & 0x07; // 3 bits (not used here)
        int tile_addr = tiledata_base + tile_index * 16;
        int y_in_tile = (scanline + vscroll) % 8;
        // 2bpp: fetch bitplane 0 and 1
        uint8_t bp0 = vram_[(tile_addr + y_in_tile) % vram_.size()];
        uint8_t bp1 = vram_[(tile_addr + y_in_tile + 8) % vram_.size()];
        int x_in_tile = 7 - ((x + hscroll) & 7);
        int color = ((bp1 >> x_in_tile) & 1) << 1 | ((bp0 >> x_in_tile) & 1);
        framebuffer_[scanline][x] = color;
    }
}

void PPU::render_sprite_stub() {
    // Overlay a simple sprite pattern at a fixed location
    int y = 100, x = 120;
    for (int dy = 0; dy < 16; ++dy) {
        for (int dx = 0; dx < 16; ++dx) {
            if (y + dy < kScreenHeight && x + dx < kScreenWidth) {
                framebuffer_[y + dy][x + dx] = 0x7FFF; // white square
            }
        }
    }
}

std::vector<int> PPU::get_sprites_on_scanline(int scanline) const {
    std::vector<int> indices;
    int sprite_size = (obsel_ & 0x01) ? 16 : 8;
    for (int i = 0; i < 128; ++i) {
        auto attr = parse_sprite_attr(i);
        int y = attr.y;
        
        // Skip sprites with Y=0xFF (invisible sprites)
        if (y == 0xFF) continue;
        
        // Handle Y wrapping: if Y >= 224, sprite wraps to top of screen
        int sprite_top = (y < 224) ? y : (y - 256);
        
        // Check if sprite appears on this scanline
        for (int dy = 0; dy < sprite_size; ++dy) {
            int sprite_scanline = sprite_top + dy;
            // Handle wrapping: if sprite_scanline < 0, it wraps to bottom
            if (sprite_scanline < 0) {
                sprite_scanline += 224;
            }
            // Also handle wrapping for sprites that extend past the bottom
            if (sprite_scanline >= 224) {
                sprite_scanline -= 224;
            }
            if (sprite_scanline == scanline) {
                indices.push_back(i);
                break;
            }
        }
        if (indices.size() == 32) break; // SNES limit
    }
    return indices;
}

void PPU::export_framebuffer_ppm(const std::string& filename) const {
    std::ofstream ofs(filename, std::ios::binary);
    ofs << "P6\n" << kScreenWidth << " " << kScreenHeight << "\n255\n";
    for (int y = 0; y < kScreenHeight; ++y) {
        for (int x = 0; x < kScreenWidth; ++x) {
            uint16_t color = framebuffer_[y][x] & 0x7FFF; // 15-bit BGR
            uint8_t r = (color & 0x1F) << 3;      // 5 bits red
            uint8_t g = ((color >> 5) & 0x1F) << 3; // 5 bits green
            uint8_t b = ((color >> 10) & 0x1F) << 3; // 5 bits blue
            ofs.put(r); ofs.put(g); ofs.put(b);
        }
    }
    ofs.close();
}

// Commented out methods not declared in PPU.hpp and not needed for Phase 1
/*
void PPU::step_scanline() {
    // Advance dot/cycle counter (not used in this stub)
    dot = 0;

    // Advance scanline
    scanline++;

    // If we've reached the end of the frame, reset scanline and optionally clear framebuffer
    if (scanline >= 262) {
        scanline = 0;
        // For now, clear framebuffer for new frame (stub)
        std::fill(framebuffer.begin(), framebuffer.end(), 0);
        // TODO: trigger VBlank, set flags, generate NMI/IRQ if enabled
    }

    // TODO: fetch background and sprite data for this scanline
    // fetch_background();
}

void PPU::render_frame() {
    // Stub: clear framebuffer
    std::fill(framebuffer.begin(), framebuffer.end(), 0);
}

void PPU::fetch_background() {
    // Stub
}

void PPU::fetch_sprites() {
    // Stub
}
*/

void PPU::render_full_scanline(int scanline) {
    if (scanline < 0 || scanline >= kScreenHeight) return;

    // Temporary per-pixel candidate for BG color/priority
    PixelInfo bg_candidate[kScreenWidth] = {};

    int bgmode = bgmode_ & 0x07;
    if (bgmode == 0) {
        // Render BGs in order: BG0 (lowest) to BG3 (highest)
        for (int bg = 0; bg < 4; ++bg) {
            if (tm_ & (1 << bg)) {
                // For each BG, fill in candidates for non-transparent pixels
                PixelInfo this_bg[kScreenWidth] = {};
                render_mode0_background(bg, scanline, this_bg);
                for (int x = 0; x < kScreenWidth; ++x) {
                    // If this BG pixel is non-transparent, overwrite previous
                    if (!this_bg[x].transparent) {
                        bg_candidate[x] = this_bg[x];
                    }
                }
            }
        }
        // Write resolved color to framebuffer
        for (int x = 0; x < kScreenWidth; ++x) {
            framebuffer_[scanline][x] = bg_candidate[x].color;
        }
    } else if (bgmode == 7) {
        render_mode7_background(scanline);
    }
    // TODO: Add other BG modes
}

// Modified render_mode0_background: fills PixelInfo array instead of framebuffer
void PPU::render_mode0_background(int bg, int scanline, PixelInfo* out) {
    int tilemap_base = get_bg_tilemap_base(bg);
    int tiledata_base = get_bg_tiledata_base(bg);
    int hscroll = bg_hofs_[bg] & 0x1FF;
    int vscroll = bg_vofs_[bg] & 0x1FF;
    int tile_y = ((scanline + vscroll) / 8) % 32;
    for (int x = 0; x < kScreenWidth; ++x) {
        int tile_x = ((x + hscroll) >> 3) % 32;
        int map_addr = tilemap_base + 2 * (tile_y * 32 + tile_x);
        uint8_t tile_lo = vram_[map_addr % vram_.size()];
        uint8_t tile_hi = vram_[(map_addr + 1) % vram_.size()];
        uint16_t tile_index = tile_lo | ((tile_hi & 0x03) << 8);
        int palette = (tile_hi >> 2) & 0x07;
        int tile_addr = tiledata_base + tile_index * 16;
        int y_in_tile = (scanline + vscroll) % 8;
        uint8_t bp0 = vram_[(tile_addr + y_in_tile) % vram_.size()];
        uint8_t bp1 = vram_[(tile_addr + y_in_tile + 8) % vram_.size()];
        int x_in_tile = 7 - ((x + hscroll) & 7);
        int color_index = ((bp1 >> x_in_tile) & 1) << 1 | ((bp0 >> x_in_tile) & 1);
        if (color_index > 0) {
            int cgram_index = palette * 4 + color_index;
            out[x].color = get_cgram_color(cgram_index);
            out[x].priority = bg;
            out[x].transparent = false;
            out[x].bg_layer = bg;
        } else {
            out[x].color = 0;
            out[x].priority = bg;
            out[x].transparent = true;
            out[x].bg_layer = bg;
        }
    }
}

void PPU::render_mode7_background(int scanline) {
    // Stub: Mode 7 rendering not implemented yet
    (void)scanline;
}

std::vector<uint8_t> PPU::get_framebuffer_rgb() const {
    std::vector<uint8_t> rgb;
    rgb.reserve(kScreenHeight * kScreenWidth * 3);
    for (int y = 0; y < kScreenHeight; ++y) {
        for (int x = 0; x < kScreenWidth; ++x) {
            uint16_t color = framebuffer_[y][x] & 0x7FFF; // 15-bit BGR
            uint8_t r = (color & 0x1F) << 3;      // 5 bits red
            uint8_t g = ((color >> 5) & 0x1F) << 3; // 5 bits green
            uint8_t b = ((color >> 10) & 0x1F) << 3; // 5 bits blue
            rgb.push_back(r);
            rgb.push_back(g);
            rgb.push_back(b);
        }
    }
    return rgb;
}
