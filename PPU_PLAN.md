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

## 🔄 Phase 3: Rendering Pipeline (Scanline/Frame) (IN PROGRESS)

1. **Write integration tests** for scanline/frame timing, BG/sprite rendering, and priority logic
2. **Implement** scanline and frame timing logic
3. **Implement** BG rendering (modes 0–7, tile fetch, scrolling, mosaic, etc.)
4. **Implement** sprite rendering (priority, size, overlap, windowing)
5. **Implement** color math, windowing, and main/sub screen logic
6. **Refactor/expand tests** for rendering edge cases and priority

**Status**: 🔄 READY TO START
- Basic scanline/frame timing implemented
- Mode 0 background rendering stub implemented
- Sprite scanline evaluation implemented
- Need to implement full rendering pipeline with priority logic

## 📋 Phase 4: Video Output and Synchronization (PLANNED)

1. **Write tests** for framebuffer output, video timing, and synchronization (mock or headless if needed)
2. **Implement** framebuffer output (256x224, 512x448, RGB888 or similar)
3. **Integrate** with SDL or similar for visual output (optional for headless test)
4. **Synchronize** PPU and CPU timing (scanline, VBLANK, HBLANK)
5. **Refactor/expand tests** for timing and output correctness

**Status**: 📋 PLANNED
- Basic framebuffer (256x224) implemented
- PPM export functionality implemented
- Need to implement full video output and CPU synchronization

## 📋 Phase 5: Advanced Features and Edge Cases (PLANNED)

1. **Write tests** for Mode 7, windowing, mosaic, color math, and hardware quirks
2. **Implement** Mode 7, windowing, mosaic, and special color math
3. **Implement** open bus, register mirroring, and hardware quirks
4. **Add support** for forced blank, brightness, and display enable/disable
5. **Refactor/expand tests** for advanced features and edge cases

**Status**: 📋 PLANNED
- Basic register mirroring implemented
- Need to implement Mode 7, windowing, mosaic, color math

## 📋 Phase 6: Comprehensive Test Coverage and CI Integration (PLANNED)

1. **Develop/expand unit and integration tests** for all PPU features
2. **Use known-good ROMs and test patterns** for validation
3. **Add CI integration** for automated PPU test runs
4. **Achieve 100% test coverage** for all implemented PPU logic

**Status**: 📋 PLANNED
- 24 comprehensive unit tests implemented and passing
- Need to add integration tests with known-good ROMs
- Need to set up CI integration

---

## 🎯 Current Implementation Status

### ✅ Completed Features
- **Memory Management**: VRAM (64KB), CGRAM (512B), OAM (544B) with proper wraparound
- **Register System**: All $2100-$213F registers with correct read/write behavior and side effects
- **Sprite System**: Attribute parsing, scanline evaluation, Y wrapping, overflow handling (32 sprite limit)
- **Background System**: Mode 0 rendering with scrolling, tile fetching, and tilemap/tiledata base calculation
- **Timing System**: Scanline/frame timing (262 scanlines, 341 dots per scanline), VBlank/HBlank detection
- **Framebuffer**: 256x224 output with PPM export capability
- **Test Coverage**: 24 comprehensive unit tests covering all implemented functionality

### 🔄 Next Steps (Phase 3)
1. **Implement full rendering pipeline** with BG/sprite priority logic
2. **Add support for all BG modes** (0-7) with proper tile formats
3. **Implement sprite rendering** with priority, size, and overlap handling
4. **Add color math and windowing** support
5. **Expand test coverage** for rendering edge cases

### 📊 Test Results
- **Total Tests**: 24
- **Passing**: 24 ✅
- **Failing**: 0 ✅
- **Coverage**: Comprehensive coverage of Phases 1-2 functionality

---

**References:**

- [Super Famicom Wiki: PPU Registers](https://wiki.superfamicom.org/registers)
- [Full Sprite/Video Example](https://wiki.superfamicom.org/snes-sprites/history/1592699300223)
- [Anomie's SNES Documentation](https://problemkaputt.de/fullsnes.htm)

**Milestone Goal:**

- ✅ Achieved accurate, testable SNES PPU emulation with full register, memory, and basic rendering logic, verified by comprehensive automated tests.
- 🎯 Next: Complete rendering pipeline with priority logic and advanced features.
