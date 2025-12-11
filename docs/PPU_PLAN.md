# SNES PPU Implementation Plan (TDD-Oriented)

## ✅ Phase 1: Core PPU Memory and Register Infrastructure (COMPLETED)

1. **✅ Write unit tests** for VRAM, CGRAM, and OAM memory access (read/write, edge cases, wraparound)
2. **✅ Write unit tests** for all PPU register read/write paths, including buffering/latching and address wrap
3. **✅ Implement** VRAM, CGRAM, OAM as internal arrays
4. **✅ Implement** all PPU registers ($2100–$213F) as member variables
5. **✅ Implement** register read/write logic with correct side effects and buffering/latching
6. **✅ Integrate** PPU with system bus for CPU access to registers
7. **✅ Refactor/expand tests** as new register logic is added

**Status**: ✅ COMPLETED - All 24 tests passing

- VRAM (64KB), CGRAM (512B), OAM (544B) with proper wraparound
- All $2100-$213F registers implemented with correct read/write behavior
- Register buffering/latching (OAM, VRAM, CGRAM address increment)
- Address increment modes and data access patterns
- Integration with system bus for CPU register access

## ✅ Phase 2: Background and Sprite Attribute Handling (COMPLETED)

1. **✅ Write unit tests** for BG tilemap and attribute table access (BGnSC, BGnNBA, BGMODE, etc.)
2. **✅ Write unit tests** for OAM attribute parsing, sprite table logic, and CGRAM palette access
3. **✅ Implement** BG tilemap and attribute table logic
4. **✅ Implement** OAM attribute parsing and sprite table logic
5. **✅ Implement** CGRAM palette access and color math stubs
6. **✅ Refactor/expand tests** for edge cases and integration

**Status**: ✅ COMPLETED - All 24 tests passing

- BG tilemap base calculation (BGnSC registers)
- BG tile data base calculation (BGnNBA registers)
- OAM attribute parsing (Y, X, tile, attributes, X high bits)
- Sprite scanline evaluation with Y wrapping
- Sprite overflow handling (32 sprite limit per scanline)
- CGRAM palette access (15-bit SNES color format)
- Basic background rendering (Mode 0, 2bpp, scrolling)

## ✅ Phase 3: Rendering Pipeline (Scanline/Frame) (COMPLETED)

1. **✅ Write integration tests** for scanline/frame timing, BG/sprite rendering, and priority logic
2. **✅ Implement** scanline and frame timing logic
3. **✅ Implement** BG rendering (modes 0–7, tile fetch, scrolling, mosaic, etc.)
4. **✅ Implement** sprite rendering (priority, size, overlap, windowing)
5. **✅ Implement** color math, windowing, and main/sub screen logic (stubs/partial)
6. **✅ Refactor/expand tests** for rendering edge cases and priority

**Status**: ✅ COMPLETED - All 24 tests passing

- Scanline/frame timing logic implemented
- Mode 0 background rendering with per-pixel priority logic implemented
- Sprite scanline evaluation and rendering logic implemented
- BG and sprite priority logic tested and passing
- Debug output for BG rendering available
- All PPU tests refactored to use public API only

## 🔄 Phase 4: Video Output and Synchronization (IN PROGRESS)

1. **Write tests** for framebuffer output, video timing, and synchronization (mock or headless if needed)
2. **Implement** framebuffer output (256x224, 512x448, RGB888 or similar)
3. **✅ Expose framebuffer as a Python-accessible NumPy array or buffer for RL/AI use**
4. **(Optional)** Integrate with SDL, PyGame, or similar for real-time visual output (utility only, not core)
5. **Synchronize** PPU and CPU timing (scanline, VBLANK, HBLANK)
6. **Refactor/expand tests** for timing and output correctness

**Status**: 🔄 IN PROGRESS

- Basic framebuffer (256x224) implemented
- PPM export functionality implemented
- VBLANK/HBLANK status register logic implemented and tested
- NMI trigger at VBLANK implemented and tested
- All PPU unit tests, including timing/status, are passing
- **Next step: Implement full video output and synchronization with CPU**
- **Real-time display (SDL, PyGame, etc.) will be an optional, decoupled utility**

## 📋 Phase 5: Advanced Features and Edge Cases (IN PROGRESS)

1. **Write tests** for Mode 7, windowing, mosaic, color math, and hardware quirks
2. **✅ Implement** Windowing logic (masking, logic combinations, inversion)
3. **Implement** Mosaic and special color math
4. **Implement** open bus, register mirroring, and hardware quirks
5. **Add support** for forced blank, brightness, and display enable/disable
6. **Refactor/expand tests** for advanced features and edge cases

**Status**: 🔄 IN PROGRESS

- Basic register mirroring implemented
- ✅ Mode 7 Implementation (Basic + Rotation)
- ✅ Windowing Logic Implementation (Masking, Logic Combinations)
- Need to implement mosaic, color math

## 📋 Phase 6: Comprehensive Test Coverage and CI Integration (PLANNED)

1. **Develop/expand unit and integration tests** for all PPU features
2. **Use known-good ROMs and test patterns** for validation
3. **Add CI integration** for automated PPU test runs
4. **Achieve 100% test coverage** for all implemented PPU logic

**Status**: 📋 PLANNED

- 30+ comprehensive unit tests implemented and passing
- Need to add integration tests with known-good ROMs
- Need to set up CI integration

---

## 🎯 Current Implementation Status

### ✅ Completed Features

- **Memory Management**: VRAM (64KB), CGRAM (512B), OAM (544B) with proper wraparound
- **Register System**: All $2100-$213F registers with correct read/write behavior and side effects
- **Sprite System**: Attribute parsing, scanline evaluation, Y wrapping, overflow handling (32 sprite limit), rendering and priority logic
- **Background System**: Modes 0, 1, and 7 rendering with scrolling, tile fetching. Mode 7 supports Affine transformations (Rotation/Scaling).
- **Advanced Features**: Windowing logic fully implemented (Masking, Logic Combinations). Color Math registers implemented (logic stubbed). Mode 7 registers fully implemented.
- **Timing System**: Scanline/frame timing (262 scanlines, 341 dots per scanline), VBlank/HBlank detection
- **Framebuffer**: 256x224 output with PPM export capability
- **Test Coverage**: 30+ comprehensive unit tests covering all implemented functionality, including Windowing, Mode 1/7, and Priority.

### 🔄 Next Steps (Phase 5)

1. **Implement full video output and synchronization** with CPU
2. **Add support for remaining BG modes** (2-6) with proper tile formats and advanced features
3. **Implement full color math logic** (currently stubs)
4. **Implement Mosaic effects**
5. **Expand test coverage** for advanced rendering and timing edge cases

### 📊 Test Results

- **Total Tests**: 32
- **Passing**: 32 ✅
- **Failing**: 0 ✅
- **Coverage**: Comprehensive coverage of Phases 1-3 functionality + Windowing/Mode 7

---

**References:**

- [Super Famicom Wiki: PPU Registers](https://wiki.superfamicom.org/registers)
- [Full Sprite/Video Example](https://wiki.superfamicom.org/snes-sprites/history/1592699300223)
- [Anomie's SNES Documentation](https://problemkaputt.de/fullsnes.htm)

**Milestone Goal:**

- ✅ Achieved accurate, testable SNES PPU emulation with full register, memory, and basic rendering logic, verified by comprehensive automated tests.
- 🎯 Next: Complete Color Math, Mosaic, and remaining BG modes.

---

## 🎯 Recent Progress

- Implemented VBLANK/HBLANK status register logic ($213C–$213F) in the PPU
- Implemented NMI trigger at the start of VBLANK (scanline 224)
- Added and refined unit tests for PPU timing and status register behavior
- **Implemented Background Mode 1 (3 BGs: 4bpp, 4bpp, 2bpp) rendering logic**
- **Implemented Background Mode 7 (Affine/Rotation/Scaling) rendering logic**
- **Implemented Mode 7 registers ($211A-$2120) and 16-bit latching**
- **Implemented Windowing Logic**: Full support for Window 1/2, Inversion, Logic Combinations (AND/OR/XOR/XNOR), and Masking.
- **Added unit tests `PPUWindowTest.*` (5 tests) covering all windowing features. Verified passing.**
- **Resolved Issue**: Fixed `TM` vs `TMW` register address confusion ($212C vs $212E).
- **Resolved Issue**: Added missing register write handlers for Windowing/Color Math ($2123-$2133). 
- All PPU and regression tests are passing (32 Total)
- Ready to proceed with Color Math and Mosaic implementation.
