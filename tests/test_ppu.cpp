#include <gtest/gtest.h>
#include "../src/pysnes/snes/include/ppu.hpp"
#include <fstream>
#include <cstdio>

class PPUTest : public ::testing::Test {
protected:
    PPU ppu;
};

TEST_F(PPUTest, VRAMReadWrite) {
    ppu.write_vram(0, 0x12);
    EXPECT_EQ(ppu.read_vram(0), 0x12);
    ppu.write_vram(65535, 0x34);
    EXPECT_EQ(ppu.read_vram(65535), 0x34);
    ppu.write_vram(65536, 0x56);
    EXPECT_EQ(ppu.read_vram(0), 0x56);
}

TEST_F(PPUTest, VRAMWriteReadBasic) {
    ppu.write_vram(1234, 0xAB);
    EXPECT_EQ(ppu.read_vram(1234), 0xAB);
    ppu.write_vram(0, 0xCD);
    EXPECT_EQ(ppu.read_vram(0), 0xCD);
    ppu.write_vram(65535, 0xEF);
    EXPECT_EQ(ppu.read_vram(65535), 0xEF);
}

TEST_F(PPUTest, CGRAMReadWrite) {
    ppu.write_cgram(0, 0xAB);
    EXPECT_EQ(ppu.read_cgram(0), 0xAB);
    ppu.write_cgram(511, 0xCD);
    EXPECT_EQ(ppu.read_cgram(511), 0xCD);
    ppu.write_cgram(512, 0xEF);
    EXPECT_EQ(ppu.read_cgram(0), 0xEF);
}

TEST_F(PPUTest, OAMReadWrite) {
    ppu.write_oam(0, 0x55);
    EXPECT_EQ(ppu.read_oam(0), 0x55);
    ppu.write_oam(543, 0xAA);
    EXPECT_EQ(ppu.read_oam(543), 0xAA);
    ppu.write_oam(544, 0x77);
    EXPECT_EQ(ppu.read_oam(0), 0x77);
}

TEST_F(PPUTest, RegisterAccess) {
    // $2100: INIDISP (Display control)
    ppu.write_register(0x2100, 0x8F);
    EXPECT_EQ(ppu.read_register(0x2100), 0);

    // $2101: OBSEL (Object size/data area)
    ppu.write_register(0x2101, 0xA5);
    EXPECT_EQ(ppu.read_register(0x2101), 0);

    // $2102/$2103: OAM address (low/high)
    ppu.write_register(0x2102, 0x34); // low byte
    ppu.write_register(0x2103, 0x01); // high bit
    // OAM address should now be 0x134
    ppu.write_register(0x2104, 0x12); // write low byte
    ppu.write_register(0x2104, 0x34); // write high byte, should increment address
    // Read back OAM at 0x134 and 0x135
    EXPECT_EQ(ppu.read_oam(0x134), 0x12);
    EXPECT_EQ(ppu.read_oam(0x135), 0x34);

    // $2105: BG mode/char size
    ppu.write_register(0x2105, 0x77);
    EXPECT_EQ(ppu.read_register(0x2105), 0);

    // $212C: Main screen designation (OBJ enable)
    ppu.write_register(0x212C, 0x10);
    EXPECT_EQ(ppu.read_register(0x212C), 0);

    // Open bus: read from unimplemented register
    EXPECT_EQ(ppu.read_register(0x213F), 0);
}

TEST_F(PPUTest, BGRegistersAccess) {
    // $2105: BGMODE
    ppu.write_register(0x2105, 0x03);
    EXPECT_EQ(ppu.read_register(0x2105), 0);
    // $2107–$210A: BGnSC tilemap base
    ppu.write_register(0x2107, 0x12);
    ppu.write_register(0x2108, 0x34);
    ppu.write_register(0x2109, 0x56);
    ppu.write_register(0x210A, 0x78);
    // No direct getter, but should not crash and should store state
    EXPECT_EQ(ppu.read_register(0x2107), 0);
    EXPECT_EQ(ppu.read_register(0x210A), 0);
    // $210B–$210C: BGnNBA tile data base
    ppu.write_register(0x210B, 0x9A);
    ppu.write_register(0x210C, 0xBC);
    EXPECT_EQ(ppu.read_register(0x210B), 0);
    EXPECT_EQ(ppu.read_register(0x210C), 0);
}

TEST_F(PPUTest, OAMAttributeParsingBasic) {
    // Write to OAM and check attribute bytes
    // Sprite 0 attribute bytes at OAM[0x02] and OAM[0x03]
    ppu.write_oam(0x02, 0xAA); // X low
    ppu.write_oam(0x03, 0x55); // Attribute
    EXPECT_EQ(ppu.read_oam(0x02), 0xAA);
    EXPECT_EQ(ppu.read_oam(0x03), 0x55);
    // Sprite 1 attribute bytes at OAM[0x06] and OAM[0x07]
    ppu.write_oam(0x06, 0xBB);
    ppu.write_oam(0x07, 0x66);
    EXPECT_EQ(ppu.read_oam(0x06), 0xBB);
    EXPECT_EQ(ppu.read_oam(0x07), 0x66);
}

TEST_F(PPUTest, CGRAMPaletteAccess) {
    // Write and read palette entries
    ppu.write_cgram(0, 0x11);
    ppu.write_cgram(1, 0x22);
    ppu.write_cgram(2, 0x33);
    EXPECT_EQ(ppu.read_cgram(0), 0x11);
    EXPECT_EQ(ppu.read_cgram(1), 0x22);
    EXPECT_EQ(ppu.read_cgram(2), 0x33);
    // Edge case: wraparound
    ppu.write_cgram(512, 0x44);
    EXPECT_EQ(ppu.read_cgram(0), 0x44);
}

TEST_F(PPUTest, BGTilemapBaseCalculation) {
    // BGnSC: bits 0-5 = base address (2KB units)
    ppu.write_register(0x2107, 0x01); // BG1SC = 0x01 -> 0x800
    EXPECT_EQ(ppu.get_bg_tilemap_base(0), 0x800);
    ppu.write_register(0x2108, 0x3F); // BG2SC = 0x3F -> 0x1F800
    EXPECT_EQ(ppu.get_bg_tilemap_base(1), 0x1F800);
}

TEST_F(PPUTest, BGTiledataBaseCalculation) {
    // BGnNBA: bits 0-2 = base address (4KB units)
    ppu.write_register(0x210B, 0x21); // BG1NBA = 0x1, BG2NBA = 0x2
    EXPECT_EQ(ppu.get_bg_tiledata_base(0), 0x1000); // BG1NBA = 1
    EXPECT_EQ(ppu.get_bg_tiledata_base(1), 0x2000); // BG2NBA = 2
    ppu.write_register(0x210C, 0x43); // BG3NBA = 0x3, BG4NBA = 0x4
    EXPECT_EQ(ppu.get_bg_tiledata_base(2), 0x3000); // BG3NBA = 3
    EXPECT_EQ(ppu.get_bg_tiledata_base(3), 0x4000); // BG4NBA = 4
}

TEST_F(PPUTest, OAMSpriteAttributeParsing) {
    // Sprite 0: OAM[0..3], X high/size in OAM[0x200]
    ppu.write_oam(0, 0x10); // Y
    ppu.write_oam(1, 0x22); // Tile
    ppu.write_oam(2, 0x33); // Attr
    ppu.write_oam(3, 0x44); // X low
    ppu.write_oam(0x200, 0x01); // X high bit for sprite 0
    auto attr = ppu.parse_sprite_attr(0);
    EXPECT_EQ(attr.y, 0x10);
    EXPECT_EQ(attr.tile, 0x22);
    EXPECT_EQ(attr.attr, 0x33);
    EXPECT_EQ(attr.x_low, 0x44);
    EXPECT_EQ(attr.x_high, 0x01);
    // Sprite 4: OAM[16..19], X high/size in OAM[0x201]
    ppu.write_oam(16, 0x55);
    ppu.write_oam(17, 0x66);
    ppu.write_oam(18, 0x77);
    ppu.write_oam(19, 0x88);
    ppu.write_oam(0x201, 0x01); // X high bit for sprite 4 (bit 0)
    auto attr4 = ppu.parse_sprite_attr(4);
    EXPECT_EQ(attr4.y, 0x55);
    EXPECT_EQ(attr4.tile, 0x66);
    EXPECT_EQ(attr4.attr, 0x77);
    EXPECT_EQ(attr4.x_low, 0x88);
    EXPECT_EQ(attr4.x_high, 0x01);
}

TEST_F(PPUTest, CGRAMColorFetch) {
    // Write 15-bit color (0x1234) to CGRAM[2,3] (palette index 1)
    ppu.write_cgram(2, 0x34);
    ppu.write_cgram(3, 0x12);
    EXPECT_EQ(ppu.get_cgram_color(1), 0x1234);
    // Edge: out of range
    EXPECT_EQ(ppu.get_cgram_color(-1), 0);
    EXPECT_EQ(ppu.get_cgram_color(256), 0);
}

// Helper for test: fill framebuffer with a pattern
static void fill_framebuffer_for_test(PPU& ppu, uint16_t value) {
    for (int y = 0; y < PPU::kScreenHeight; ++y) {
        uint16_t* row = const_cast<uint16_t*>(ppu.get_framebuffer_row(y));
        for (int x = 0; x < PPU::kScreenWidth; ++x) {
            row[x] = value;
        }
    }
}

TEST_F(PPUTest, ScanlineFrameTiming) {
    ppu.reset();
    int vblank_start = PPU::kScreenHeight;
    int vblank_end = PPU::kTotalScanlines;
    int total_dots = PPU::kTotalScanlines * PPU::kDotsPerScanline;
    int vblank_count = 0;
    for (int i = 0; i < total_dots; ++i) {
        ppu.step_dot();
        if (ppu.get_scanline() >= vblank_start && ppu.get_scanline() < vblank_end) {
            EXPECT_TRUE(ppu.get_vblank());
            vblank_count++;
        } else {
            EXPECT_FALSE(ppu.get_vblank());
        }
        if (ppu.get_dot() >= PPU::kDotsPerScanline - 40) {
            EXPECT_TRUE(ppu.get_hblank());
        } else {
            EXPECT_FALSE(ppu.get_hblank());
        }
    }
    EXPECT_EQ(ppu.get_frame(), 1);
    EXPECT_EQ(vblank_count, (vblank_end - vblank_start) * PPU::kDotsPerScanline);
}

TEST_F(PPUTest, RenderScanlineStubWritesFramebuffer) {
    // Step one scanline and check framebuffer row is filled with expected value
    ppu.reset();
    for (int i = 0; i < 10; ++i) ppu.step_scanline();
    ppu.render_scanline_stub();
    const uint16_t* row = ppu.get_framebuffer_row(10);
    for (int x = 0; x < PPU::kScreenWidth; ++x) {
        EXPECT_EQ(row[x], (10 & 0x1F) << 10);
    }
}

TEST_F(PPUTest, RenderSpriteStubWritesFramebuffer) {
    // Call render_sprite_stub and check for white square at (120,100)
    ppu.reset();
    ppu.render_sprite_stub();
    for (int dy = 0; dy < 16; ++dy) {
        for (int dx = 0; dx < 16; ++dx) {
            int y = 100 + dy, x = 120 + dx;
            if (y < PPU::kScreenHeight && x < PPU::kScreenWidth) {
                EXPECT_EQ(ppu.get_framebuffer_row(y)[x], 0x7FFF);
            }
        }
    }
}

TEST_F(PPUTest, BGMode0SimpleTileFetch) {
    // Setup: Mode 0, BG1 tilemap at 0x0000, tiledata at 0x0000
    ppu.write_register(0x2105, 0x00); // Mode 0
    ppu.write_register(0x2107, 0x00); // BG1SC: tilemap base 0x0000
    ppu.write_register(0x210B, 0x01); // BG1NBA: tiledata base 0x1000
    // Fill VRAM with a simple 8x8 tile: all pixels = 1 (2bpp, Mode 0)
    // SNES 2bpp tile: 16 bytes per tile, each bitplane is 8 bytes
    // For tile 0, set all bits in bitplane 0, clear bitplane 1
    for (int i = 0; i < 8; ++i) ppu.write_vram(0x1000 + i, 0xFF); // bitplane 0
    for (int i = 8; i < 16; ++i) ppu.write_vram(0x1000 + i, 0x00); // bitplane 1
    // Set BG1 tilemap to use tile 0 for first 32 tiles of scanline 0
    for (int i = 0; i < 32; ++i) {
        ppu.write_vram(0x0000 + i * 2, 0x00); // tile index low
        ppu.write_vram(0x0000 + i * 2 + 1, 0x00); // tile index high, palette 0
    }
    // Set scroll to 0 using register interface
    ppu.write_register(0x210D, 0x00); // BG1HOFS low
    ppu.write_register(0x210D, 0x00); // BG1HOFS high
    ppu.write_register(0x2111, 0x00); // BG1VOFS low
    ppu.write_register(0x2111, 0x00); // BG1VOFS high
    // Debug: check VRAM contents for tile 0 data
    for (int i = 0; i < 16; ++i) {
        EXPECT_EQ(ppu.read_vram(0x1000 + i), (i < 8) ? 0xFF : 0x00);
    }
    // Debug: check VRAM contents for tilemap
    for (int i = 0; i < 32; ++i) {
        EXPECT_EQ(ppu.read_vram(0x0000 + i * 2), 0x00);
        EXPECT_EQ(ppu.read_vram(0x0000 + i * 2 + 1), 0x00);
    }
    // Implement a stub: render_bg_scanline_stub(scanline)
    ppu.render_bg_scanline_stub(0);
    // Check that framebuffer row 0 is all 1s
    const uint16_t* row = ppu.get_framebuffer_row(0);
    for (int x = 0; x < PPU::kScreenWidth; ++x) {
        EXPECT_EQ(row[x], 1);
    }
}

TEST_F(PPUTest, BGMode0ScrollingAndWraparound) {
    // Setup: Mode 0, BG1 tilemap at 0x0000, tiledata at 0x1000
    ppu.write_register(0x2105, 0x00); // Mode 0
    ppu.write_register(0x2107, 0x00); // BG1SC: tilemap base 0x0000
    ppu.write_register(0x210B, 0x01); // BG1NBA: tiledata base 0x1000
    // Fill VRAM with two tiles: tile 0 = all 1s, tile 1 = all 2s
    for (int i = 0; i < 8; ++i) ppu.write_vram(0x1000 + i, 0xFF); // tile 0, bitplane 0
    for (int i = 8; i < 16; ++i) ppu.write_vram(0x1000 + i, 0x00); // tile 0, bitplane 1
    for (int i = 0; i < 8; ++i) ppu.write_vram(0x1010 + i, 0x00); // tile 1, bitplane 0
    for (int i = 8; i < 16; ++i) ppu.write_vram(0x1010 + i, 0xFF); // tile 1, bitplane 1
    // Set BG1 tilemap: first 16 tiles = tile 1, next 16 = tile 0
    for (int i = 0; i < 16; ++i) {
        ppu.write_vram(0x0000 + i * 2, 0x01); // tile 1
        ppu.write_vram(0x0000 + i * 2 + 1, 0x00);
    }
    for (int i = 16; i < 32; ++i) {
        ppu.write_vram(0x0000 + i * 2, 0x00); // tile 0
        ppu.write_vram(0x0000 + i * 2 + 1, 0x00);
    }
    // Set scroll: horizontal = 8 (one tile), vertical = 0
    ppu.write_register(0x210D, 0x08); // BG1HOFS low
    ppu.write_register(0x210D, 0x00); // BG1HOFS high
    ppu.write_register(0x2111, 0x00); // BG1VOFS low
    ppu.write_register(0x2111, 0x00); // BG1VOFS high
    // Render scanline 0
    ppu.render_bg_scanline_stub(0);
    // The first 120 pixels should be from tile 1 (color 2), next 128 from tile 0 (color 1), last 8 wrap to tile 1 (color 2)
    const uint16_t* row = ppu.get_framebuffer_row(0);
    for (int x = 0; x < 120; ++x) {
        EXPECT_EQ(row[x], 2) << "x=" << x;
    }
    for (int x = 120; x < 248; ++x) {
        EXPECT_EQ(row[x], 1) << "x=" << x;
    }
    for (int x = 248; x < 256; ++x) {
        EXPECT_EQ(row[x], 2) << "x=" << x;
    }
}

// --- Sprite Rendering: Scanline Evaluation and Overflow ---

TEST_F(PPUTest, SpriteScanlineEvaluation_Basic) {
    // Place 3 sprites on scanline 10
    for (int i = 0; i < 3; ++i) {
        int base = 4 * i;
        ppu.write_oam(base + 0, 10); // Y
        ppu.write_oam(base + 1, 0x20 + i); // Tile
        ppu.write_oam(base + 2, 0x00); // Attr
        ppu.write_oam(base + 3, 0x30 + i * 8); // X
    }
    // Place 1 sprite off scanline 10
    ppu.write_oam(12, 50); // Y (not on scanline 10)
    // Use the new scanline evaluation method
    auto indices = ppu.get_sprites_on_scanline(10);
    EXPECT_EQ(indices.size(), 3);
    EXPECT_EQ(indices[0], 0);
    EXPECT_EQ(indices[1], 1);
    EXPECT_EQ(indices[2], 2);
    auto indices_off = ppu.get_sprites_on_scanline(50);
    EXPECT_EQ(indices_off.size(), 1);
    EXPECT_EQ(indices_off[0], 3);
}

TEST_F(PPUTest, SpriteScanlineEvaluation_Overflow) {
    // Place 40 sprites on scanline 20 (SNES limit is 32 per scanline)
    for (int i = 0; i < 40; ++i) {
        int base = 4 * i;
        ppu.write_oam(base + 0, 20); // Y
        ppu.write_oam(base + 1, 0x10 + i); // Tile
        ppu.write_oam(base + 2, 0x00); // Attr
        ppu.write_oam(base + 3, 0x20 + i * 2); // X
    }
    auto indices = ppu.get_sprites_on_scanline(20);
    EXPECT_EQ(indices.size(), 32); // SNES limit
    for (int i = 0; i < 32; ++i) {
        EXPECT_EQ(indices[i], i);
    }
}

TEST_F(PPUTest, SpritePriorityAndOrder) {
    // Place two overlapping sprites on scanline 30
    // Sprite 0: lower priority (attr = 0x00)
    ppu.write_oam(0, 30); // Y
    ppu.write_oam(1, 0x01); // Tile
    ppu.write_oam(2, 0x00); // Attr (priority 0)
    ppu.write_oam(3, 0x40); // X
    // Sprite 1: higher priority (attr = 0x20)
    ppu.write_oam(4, 30); // Y
    ppu.write_oam(5, 0x02); // Tile
    ppu.write_oam(6, 0x20); // Attr (priority 1)
    ppu.write_oam(7, 0x40); // X (same X)
    // TODO: When rendering is implemented, check that sprite 1 appears above sprite 0
    auto attr0 = ppu.parse_sprite_attr(0);
    auto attr1 = ppu.parse_sprite_attr(1);
    EXPECT_EQ(attr0.y, 30);
    EXPECT_EQ(attr1.y, 30);
    EXPECT_EQ(attr0.attr & 0x20, 0x00); // priority 0
    EXPECT_EQ(attr1.attr & 0x20, 0x20); // priority 1
}

TEST_F(PPUTest, SpriteScanlineEvaluation_Size8x8And16x16) {
    // Set OBSEL to 8x8 (0), then 16x16 (1)
    ppu.write_register(0x2101, 0x00); // 8x8
    // Sprite at Y=50, should appear on scanline 50 (8x8)
    ppu.write_oam(0, 50); // Y
    auto indices_8x8 = ppu.get_sprites_on_scanline(50);
    EXPECT_EQ(indices_8x8.size(), 1);
    // Now set OBSEL to 16x16 (bit 0 = 1)
    ppu.write_register(0x2101, 0x01); // 16x16
    // Sprite at Y=50, should appear on scanlines 50-65 (16 lines)
    int count = 0;
    for (int s = 50; s < 66; ++s) {
        auto indices_16x16 = ppu.get_sprites_on_scanline(s);
        if (!indices_16x16.empty() && indices_16x16[0] == 0) count++;
    }
    EXPECT_EQ(count, 16);
}

TEST_F(PPUTest, SpriteScanlineEvaluation_OverflowLimit) {
    // Place 40 sprites on scanline 100
    for (int i = 0; i < 40; ++i) {
        int base = 4 * i;
        ppu.write_oam(base + 0, 100); // Y
    }
    // Only 32 should be selected (SNES limit)
    auto indices = ppu.get_sprites_on_scanline(100);
    EXPECT_EQ(indices.size(), 32);
    for (int i = 0; i < 32; ++i) {
        EXPECT_EQ(indices[i], i);
    }
    // Optionally: check for overflow flag (if implemented)
    // EXPECT_TRUE(ppu.get_sprite_overflow_flag());
}

TEST_F(PPUTest, SpriteScanlineEvaluation_YWrapping) {
    // Clear OAM to ensure only the intended sprite is present
    for (int i = 0; i < 544; ++i) ppu.write_oam(i, 0xFF);
    // Place a sprite at Y=223 (bottom of screen)
    ppu.write_oam(0, 223); // Y
    ppu.write_register(0x2101, 0x00); // 8x8 size
    
    // Sprite should appear on scanline 223 and 0..6 (Y wrapping)
    // Check scanline 223 (should have sprite)
    auto indices_223 = ppu.get_sprites_on_scanline(223);
    EXPECT_EQ(indices_223.size(), 1);
    EXPECT_EQ(indices_223[0], 0);
    
    // Check scanlines 0-6 (should have sprite due to wrapping)
    for (int s = 0; s < 7; ++s) {
        auto indices = ppu.get_sprites_on_scanline(s);
        EXPECT_EQ(indices.size(), 1);
        EXPECT_EQ(indices[0], 0);
    }
}

TEST_F(PPUTest, StatusRegisterVBlankHBlankBits) {
    ppu.reset();
    // Step to the start of the last visible scanline
    int scanline_start = (PPU::kScreenHeight - 1) * PPU::kDotsPerScanline;
    for (int i = 0; i < scanline_start; ++i) {
        ppu.step_dot();
    }
    // For the last visible scanline, check status bits for each dot BEFORE stepping
    for (int dot = 0; dot < PPU::kDotsPerScanline; ++dot) {
        uint8_t status = ppu.read_register(0x213C);
        if (dot >= PPU::kDotsPerScanline - 40) {
            EXPECT_EQ(status & 0x40, 0x40) << "dot=" << dot;
        } else {
            EXPECT_EQ(status & 0x40, 0x00) << "dot=" << dot;
        }
        EXPECT_EQ(status & 0x80, 0x00) << "dot=" << dot; // VBLANK should not be set yet
        ppu.step_dot();
    }
    // At the start of the next scanline (scanline 224, dot 0), VBLANK should be set
    uint8_t status = ppu.read_register(0x213C);
    EXPECT_EQ(status & 0x80, 0x80) << "VBLANK should be set at start of scanline 224";
    // HBLANK should be clear at dot 0
    EXPECT_EQ(status & 0x40, 0x00) << "HBLANK should be clear at start of scanline 224";
    // Step through the first 40 dots of VBLANK scanline and check HBLANK
    for (int dot = 0; dot < PPU::kDotsPerScanline; ++dot) {
        uint8_t status = ppu.read_register(0x213C);
        if (dot >= PPU::kDotsPerScanline - 40) {
            EXPECT_EQ(status & 0x40, 0x40) << "dot=" << dot << " (VBLANK)";
        } else {
            EXPECT_EQ(status & 0x40, 0x00) << "dot=" << dot << " (VBLANK)";
        }
        EXPECT_EQ(status & 0x80, 0x80) << "dot=" << dot << " (VBLANK)";
        ppu.step_dot();
    }
}

TEST_F(PPUTest, ExportFramebufferPPMWritesPPMFile) {
    // Fill framebuffer with a test pattern (gradient)
    fill_framebuffer_for_test(ppu, 0x1234);
    const std::string filename = "test_framebuffer.ppm";
    ppu.export_framebuffer_ppm(filename);
    // Check file exists and header is correct
    std::ifstream ifs(filename, std::ios::binary);
    ASSERT_TRUE(ifs.is_open());
    std::string header;
    std::getline(ifs, header);
    EXPECT_EQ(header, "P6");
    int w = 0, h = 0, maxval = 0;
    ifs >> w >> h >> maxval;
    EXPECT_EQ(w, PPU::kScreenWidth);
    EXPECT_EQ(h, PPU::kScreenHeight);
    EXPECT_EQ(maxval, 255);
    // Skip single whitespace after maxval
    ifs.get();
    // Check file size: header + pixel data
    ifs.seekg(0, std::ios::end);
    std::streamoff file_size = ifs.tellg();
    std::streamoff expected_size = header.size() + 1 + std::to_string(w).size() + 1 + std::to_string(h).size() + 1 + std::to_string(maxval).size() + 1 + w * h * 3;
    EXPECT_GE(file_size, expected_size - 16); // Allow for whitespace differences
    ifs.close();
    std::remove(filename.c_str());
}
