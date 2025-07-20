#pragma once
#include <cstdint>
#include <array>
#include <vector>
#include <string>

// SNES PPU (Picture Processing Unit) - Initial Skeleton
// VRAM: 64KB, CGRAM: 512B, OAM: 544B
class PPU {
public:
    PPU();
    ~PPU();

    // Reset PPU state
    void reset();

    // VRAM access
    uint8_t read_vram(uint16_t addr) const;
    void write_vram(uint16_t addr, uint8_t value);

    // CGRAM access
    uint8_t read_cgram(uint16_t addr) const;
    void write_cgram(uint16_t addr, uint8_t value);

    // OAM access
    uint8_t read_oam(uint16_t addr) const;
    void write_oam(uint16_t addr, uint8_t value);

    // Register access stubs (to be implemented)
    uint8_t read_register(uint16_t addr);
    void write_register(uint16_t addr, uint8_t value);

    // Bus interface for CPU register access
    uint8_t cpu_read(uint16_t addr) { return read_register(addr); }
    void cpu_write(uint16_t addr, uint8_t data) { write_register(addr, data); }

private:
    // 64KB Video RAM
    std::array<uint8_t, 64 * 1024> vram_;
    // 512B Color RAM
    std::array<uint8_t, 512> cgram_;
    // 544B Object Attribute Memory (OAM)
    std::array<uint8_t, 544> oam_;

    // OAM address and latch state for $2102/$2103/$2104
    uint16_t oam_addr_ = 0; // 9-bit OAM address
    bool oam_priority_rotation_ = false;
    bool oam_addr_msb_ = false;
    bool oam_latch_low_ = true; // Track low/high byte writes

    // VRAM and CGRAM read buffers (for $2139/$213A and $213B)
    uint16_t vram_read_buffer_ = 0;
    uint8_t cgram_read_buffer_ = 0;

    // --- PPU Register State ($2100–$213F) ---
    uint8_t inidisp_ = 0;      // $2100: Display control
    uint8_t obsel_ = 0;        // $2101: Object size/data area
    uint8_t bgmode_ = 0;       // $2105: BG mode/char size
    uint8_t mosaic_ = 0;       // $2106: Mosaic
    uint8_t bg_sc_[4] = {0};   // $2107–$210A: BGnSC tilemap base
    uint8_t bg_nba_[2] = {0};  // $210B–$210C: BGnNBA tile data base
    uint16_t bg_hofs_[4] = {0}; // $210D–$2110: BGnHOFS
    uint8_t bg_hofs_latch_[4] = {0}; // Latch for low byte
    bool bg_hofs_latch_state_[4] = {true, true, true, true}; // true: next write is low byte
    uint16_t bg_vofs_[4] = {0}; // $2111–$2114: BGnVOFS
    uint8_t vmain_ = 0;        // $2115: VRAM increment mode
    uint16_t vram_addr_ = 0;   // $2116/$2117: VRAM address
    uint8_t cgram_addr_ = 0;   // $2121: CGRAM address
    uint8_t tm_ = 0;           // $212C: Main screen designation
    uint8_t ts_ = 0;           // $212D: Sub screen designation
    // ... add more as needed for $2100–$213F ...
    // TODO: Add windowing, color math, mode 7, and status registers

    // --- BG tilemap/tile data base helpers ---
public:
    uint32_t get_bg_tilemap_base(int bg) const;
    uint32_t get_bg_tiledata_base(int bg) const;
    uint8_t get_bgmode() const { return bgmode_; }

    // --- OAM attribute parsing ---
    struct SpriteAttr {
        uint8_t y;
        uint8_t tile;
        uint8_t attr;
        uint8_t x_low;
        uint8_t x_high;
        uint8_t size;
        // Add more fields as needed
    };
    SpriteAttr parse_sprite_attr(int index) const;

    // --- CGRAM palette access ---
    uint16_t get_cgram_color(int index) const; // Returns 15-bit SNES color

    // --- Scanline/frame timing and rendering state ---
public:
    static constexpr int kScreenWidth = 256;
    static constexpr int kScreenHeight = 224;
    static constexpr int kTotalScanlines = 262; // NTSC
    static constexpr int kDotsPerScanline = 341; // SNES typical

    int scanline_ = 0;
    int dot_ = 0;
    int frame_ = 0;
    bool vblank_ = false;
    bool hblank_ = false;

    // Simple framebuffer: 256x224, 16-bit SNES color
    uint16_t framebuffer_[kScreenHeight][kScreenWidth] = {};

    // Timing and rendering stubs
    void step_dot();
    void step_scanline();
    void step_frame();
    void render_scanline_stub();
    void render_sprite_stub();
    void render_bg_scanline_stub(int scanline); // New: BG scanline rendering stub

    // Expose flags for testing
    bool get_vblank() const { return vblank_; }
    bool get_hblank() const { return hblank_; }
    int get_scanline() const { return scanline_; }
    int get_dot() const { return dot_; }
    int get_frame() const { return frame_; }
    const uint16_t* get_framebuffer_row(int y) const { return framebuffer_[y]; }

    // --- Sprite scanline evaluation (stub) ---
    std::vector<int> get_sprites_on_scanline(int scanline) const;

    void export_framebuffer_ppm(const std::string& filename) const;
};
