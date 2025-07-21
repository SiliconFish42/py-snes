#pragma once
#include <cstdint>
#include <array>
#include <vector>
#include <string>

// SNES PPU (Picture Processing Unit) - Initial Skeleton
// VRAM: 64KB, CGRAM: 512B, OAM: 544B
class Bus; // Forward declaration
class PPU {
public:
    // --- Data Structures ---
    struct PixelInfo {
        uint8_t color = 0;
        uint8_t priority = 0;
        bool transparent = true;
        int bg_layer = -1;
        int sprite_index = -1;
    };
    struct SpriteAttr {
        uint8_t y;
        uint8_t tile;
        uint8_t attr;
        uint8_t x_low;
        uint8_t x_high;
        uint8_t size;
    };

    // --- Constants ---
    static constexpr int kScreenWidth = 256;
    static constexpr int kScreenHeight = 224;
    static constexpr int kTotalScanlines = 262; // NTSC
    static constexpr int kDotsPerScanline = 341; // SNES typical

    // --- Constructors/Destructors ---
    PPU();
    ~PPU();

    // --- Core API ---
    void reset();
    // VRAM
    uint8_t read_vram(uint16_t addr) const;
    void write_vram(uint16_t addr, uint8_t value);
    // CGRAM
    uint8_t read_cgram(uint16_t addr) const;
    void write_cgram(uint16_t addr, uint8_t value);
    // OAM
    uint8_t read_oam(uint16_t addr) const;
    void write_oam(uint16_t addr, uint8_t value);
    // Register access
    uint8_t read_register(uint16_t addr);
    void write_register(uint16_t addr, uint8_t value);
    // Bus interface
    uint8_t cpu_read(uint16_t addr) { return read_register(addr); }
    void cpu_write(uint16_t addr, uint8_t data) { write_register(addr, data); }

    // --- Rendering and Timing API ---
    void step_dot();
    void step_scanline();
    void step_frame();
    void render_full_scanline(int scanline);
    void render_background_layer(int bg, int scanline);
    void render_sprite_layer(int scanline);
    void apply_priority_logic(int scanline);
    void apply_color_math(int scanline);
    void apply_mosaic_effect(int scanline);
    void apply_window_masking(int scanline);
    void render_mode0_background(int bg, int scanline, PixelInfo* out);
    void render_mode7_background(int scanline);
    void render_scanline_stub();
    void render_sprite_stub();
    void render_bg_scanline_stub(int scanline);

    // --- Memory/Tile/Palette Helpers ---
    uint32_t get_bg_tilemap_base(int bg) const;
    uint32_t get_bg_tiledata_base(int bg) const;
    uint8_t get_bgmode() const { return bgmode_; }
    SpriteAttr parse_sprite_attr(int index) const;
    uint16_t get_cgram_color(int index) const;
    std::vector<int> get_sprites_on_scanline(int scanline) const;
    PixelInfo get_pixel_info(int x, int scanline) const;
    uint16_t blend_colors(uint16_t color1, uint16_t color2, bool additive) const;
    bool is_window_enabled(int x, int y, int window) const;

    // --- Framebuffer and Output ---
    void export_framebuffer_ppm(const std::string& filename) const;
    const uint16_t* get_framebuffer_row(int y) const { return framebuffer_[y]; }

    // --- State Getters (for tests/inspection) ---
    bool get_vblank() const { return vblank_; }
    bool get_hblank() const { return hblank_; }
    int get_scanline() const { return scanline_; }
    int get_dot() const { return dot_; }
    int get_frame() const { return frame_; }

    void set_bus(Bus* bus) { bus_ = bus; }

private:
    // --- PPU Memory ---
    std::array<uint8_t, 64 * 1024> vram_;
    std::array<uint8_t, 512> cgram_;
    std::array<uint8_t, 544> oam_;

    // --- Framebuffer ---
    uint16_t framebuffer_[kScreenHeight][kScreenWidth] = {};

    // --- Timing State ---
    int scanline_ = 0;
    int dot_ = 0;
    int frame_ = 0;
    bool vblank_ = false;
    bool hblank_ = false;

    // --- Register State and Latches ---
    uint16_t oam_addr_ = 0;
    bool oam_priority_rotation_ = false;
    bool oam_addr_msb_ = false;
    bool oam_latch_low_ = true;
    uint16_t vram_read_buffer_ = 0;
    uint8_t cgram_read_buffer_ = 0;
    uint8_t inidisp_ = 0;
    uint8_t obsel_ = 0;
    uint8_t bgmode_ = 0;
    uint8_t mosaic_ = 0;
    uint8_t bg_sc_[4] = {0};
    uint8_t bg_nba_[2] = {0};
    uint16_t bg_hofs_[4] = {0};
    uint8_t bg_hofs_latch_[4] = {0};
    bool bg_hofs_latch_state_[4] = {true, true, true, true};
    uint16_t bg_vofs_[4] = {0};
    uint8_t vmain_ = 0;
    uint16_t vram_addr_ = 0;
    uint8_t cgram_addr_ = 0;
    uint8_t tm_ = 0;
    uint8_t ts_ = 0;
    // TODO: Add windowing, color math, mode 7, and status registers
    Bus* bus_ = nullptr;
};
