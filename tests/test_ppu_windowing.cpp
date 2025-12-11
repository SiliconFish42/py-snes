#include "ppu.hpp"
#include <gtest/gtest.h>
#include <vector>

class PPUWindowTest : public ::testing::Test {
protected:
  PPU ppu;

  void SetUp() override { ppu.reset(); }
};

// Test check_window_hit directly
TEST_F(PPUWindowTest, CheckWindowHit_Basics) {
  // Setup Window 1: Left=10, Right=20
  ppu.write_register(0x2126, 10);
  ppu.write_register(0x2127, 20);

  // Enable Window 1 for BG1 ($2123)
  // Logic: BG1 is layer 0. W12SEL lower byte.
  // Bits: 0=Inv1, 1=En1, 2=Inv2, 3=En2
  // Enable W1 for BG1: Bit 1 = 1 -> 0x02
  ppu.write_register(0x2123, 0x02);

  // Check positions
  EXPECT_TRUE(ppu.check_window_hit(10, 0, 0));  // Edge
  EXPECT_TRUE(ppu.check_window_hit(15, 0, 0));  // Inside
  EXPECT_TRUE(ppu.check_window_hit(20, 0, 0));  // Edge
  EXPECT_FALSE(ppu.check_window_hit(9, 0, 0));  // Outside Left
  EXPECT_FALSE(ppu.check_window_hit(21, 0, 0)); // Outside Right

  // Test Inversion
  // Bit 0 = 1 -> Invert W1. Total = 0x03
  ppu.write_register(0x2123, 0x03);
  EXPECT_FALSE(ppu.check_window_hit(15, 0, 0)); // Inside -> False
  EXPECT_TRUE(ppu.check_window_hit(9, 0, 0));   // Outside -> True
}

TEST_F(PPUWindowTest, CheckWindowHit_Window2) {
  // Setup Window 2: Left=50, Right=60
  ppu.write_register(0x2128, 50);
  ppu.write_register(0x2129, 60);

  // Enable Window 2 for BG1 ($2123)
  // Bits: 3=En2. Val = 0x08
  ppu.write_register(0x2123, 0x08);

  EXPECT_TRUE(ppu.check_window_hit(55, 0, 1));  // Inside W2
  EXPECT_FALSE(ppu.check_window_hit(49, 0, 1)); // Outside W2
}

TEST_F(PPUWindowTest, CombineWindowLogic) {
  bool h1 = true;
  bool h2 = false;
  // logic_param_offset is not used in combine_window_logic currently (always
  // uses register) But we need to set WBG/OBJLOG to test modes.

  // BG1 Logic: $212A bits 0-1
  // 0=OR, 1=AND, 2=XOR, 3=XNOR

  // OR (00)
  ppu.write_register(0x212A, 0x00);
  EXPECT_TRUE(ppu.combine_window_logic(true, false, 0, 0));
  EXPECT_TRUE(ppu.combine_window_logic(false, true, 0, 0));
  EXPECT_FALSE(ppu.combine_window_logic(false, false, 0, 0));

  // AND (01)
  ppu.write_register(0x212A, 0x01);
  EXPECT_FALSE(ppu.combine_window_logic(true, false, 0, 0));
  EXPECT_TRUE(ppu.combine_window_logic(true, true, 0, 0));

  // XOR (10)
  ppu.write_register(0x212A, 0x02);
  EXPECT_TRUE(ppu.combine_window_logic(true, false, 0, 0));
  EXPECT_FALSE(ppu.combine_window_logic(true, true, 0, 0));

  // XNOR (11)
  ppu.write_register(0x212A, 0x03);
  EXPECT_FALSE(ppu.combine_window_logic(true, false, 0, 0));
  EXPECT_TRUE(ppu.combine_window_logic(true, true, 0, 0));
}

TEST_F(PPUWindowTest, ApplyWindowMasking_TMW) {
  // Simplified Integration Test through public API (sort of,
  // apply_window_masking is public) Setup Fake Scanline Buffer
  PPU::PixelInfo line[256];
  for (int i = 0; i < 256; ++i) {
    line[i].bg_layer = 0; // BG1
    line[i].transparent = false;
    line[i].color = 0xFFFF;
  }

  // Setup Window 1: 0-10. ($2126=0, $2127=10)
  ppu.write_register(0x2126, 0);
  ppu.write_register(0x2127, 10);

  // Enable W1 for BG1 ($2123 = 0x02)
  ppu.write_register(0x2123, 0x02);

  // Enable Window Masking for Main Screen BG1 (TMW $212E = 0x01)
  // NOTE: Used to be $212C (TM), fixed to $212E (TMW)
  ppu.write_register(0x212E, 0x01);

  // Apply Masking
  ppu.apply_window_masking(0, line);

  // Expect 0-10 to be VISIBLE (Not Transparent)
  // Expect 11-255 to be MASKED (Transparent)
  // "Bits set in TMW ... disable display OUTSIDE the window"
  // So INSIDE = Visible.

  EXPECT_FALSE(line[5].transparent);
  EXPECT_TRUE(line[15].transparent);
}

TEST_F(PPUWindowTest, ApplyWindowMasking_DisabledWindowsBoth) {
  // Regression check: TMW enabled, but W1/W2 disabled in SEL.
  // Result should be fully masked (Hidden).
  PPU::PixelInfo line[256];
  for (int i = 0; i < 256; ++i) {
    line[i].bg_layer = 0;
    line[i].transparent = false;
  }

  ppu.write_register(0x212E, 0x01); // TMW on BG1
  ppu.write_register(0x2123, 0x00); // W1/W2 Disabled for BG1

  ppu.apply_window_masking(0, line);

  EXPECT_TRUE(line[5].transparent);
  EXPECT_TRUE(line[100].transparent);
}
