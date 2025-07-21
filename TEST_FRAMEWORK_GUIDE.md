# PySNES Test Framework Guide

## Overview

The PySNES Test Framework validates the accuracy of the PySNES SNES emulator. It supports automated testing using test ROMs, with detailed reporting and flexible configuration.

---

## What Works Now

- **Supported:**
  - **CPU 65816 Test ROMs**: Fully implemented and supported.
- **Planned/Placeholders:**
  - **SPC-700, PPU, APU Test ROMs**: Placeholders exist, but these are not yet functional.

---

## Quick Start

### Build and Run All Tests (Recommended)

Use the Makefile for all build and test tasks. The Makefile will check for 'uv' and print an error if it is missing.

```bash
make build      # Build the Python package and C++ extension
make test       # Run all tests (Python and C++)
make clean      # Remove build and test outputs
```

- The Makefile is the preferred workflow for most users.
- Alternatively, you can build the Python package directly with:
  ```bash
  uv pip install -e .
  ```

### Advanced/Manual C++ Test Suite Build

If you need to build the C++ test suite directly (not via Makefile):

```bash
mkdir build
cd build
cmake ..
make run_tests
```

### Run All Python Tests Only

```bash
python tests/scripts/run_comprehensive_tests.py
```

- Use `--build-dir <dir>` to specify a custom build directory.
- Use `--verbose` for detailed output.

### Run Specific Test Categories (C++/GTest)

```bash
cd build
cmake ..
make run_tests

# Run all framework unit tests
./run_tests --gtest_filter=TestFrameworkTest.*

# Run ROM tests
./run_tests --gtest_filter=TestFrameworkWithROMsTest.*

# Run CPU instruction tests
./run_tests --gtest_filter=LDATest.*:STATest.*:TransferTest.*:StackTest.*

# Output XML results
./run_tests --gtest_output=xml:../test_output/test_results.xml
```

### Analyze ROMs

```bash
python tools/rom_analyzer.py tests/roms/cputest-basic.sfc
python tools/rom_analyzer.py tests/roms/cputest-basic.sfc --strings
```

### Clean Up Test Outputs

```bash
python tools/cleanup_tests.py --old
python tools/cleanup_tests.py --all
python tools/cleanup_tests.py --old --days 3
```

---

## Configuration

All configuration is in `tests/config/test_config.json`.

### Example Structure

```json
{
  "test_framework": { ... },
  "test_roms": {
    "cpu_65816": {
      "description": "65816 CPU instruction tests",
      "file_patterns": ["cputest-*.sfc", ...],
      "test_parameters": {
        "max_cycles": 1000000,
        "timeout_ms": 30000,
        "verbose_output": false,
        "detection_patterns": {
          "success": ["SUCCESS", "PASSED", "OK"],
          "failure": ["FAILED", "ERROR", "FAIL"],
          "completion_addresses": [8192, 8193, 8194]
        }
      }
    },
    ...
  },
  "test_suites": {
    "quick": { ... },
    "standard": { ... },
    "comprehensive": { ... },
    "continuous_integration": { ... }
  },
  "reporting": { ... },
  "memory_monitoring": { ... },
  "debugging": { ... }
}
```

#### Key Sections

| Section              | Purpose                                    |
|----------------------|--------------------------------------------|
| `test_roms`          | Defines ROM types, file patterns, detection|
| `test_suites`        | Predefined test suite parameters           |
| `reporting`          | Output formats, log retention, etc.        |
| `memory_monitoring`  | Regions and intervals for snapshots        |
| `debugging`          | Tracing, logs, break on failure            |

#### Adding a New Test ROM Type

```json
{
  "test_roms": {
    "my_custom_test": {
      "description": "Describe your test",
      "file_patterns": ["tests/roms/mytest-*.sfc"],
      "test_parameters": {
        "max_cycles": 500000,
        "timeout_ms": 10000,
        "verbose_output": false,
        "detection_patterns": {
          "success": ["MY_SUCCESS"],
          "failure": ["MY_FAILED"],
          "completion_addresses": [12288]
        }
      }
    }
  }
}
```

---

## Test Output

- All outputs are stored in `test_output/`.
- Supported formats: **JSON**, **TXT**, **HTML** (configurable in `test_config.json`).
- Retention and cleanup are also configurable.

---

## Test Completion & Failure Patterns

The framework detects test completion/failure using:

- **Strings in memory:**
  - "SUCCESS" (completion)
  - "FAILED" (failure)
- **Status bytes at addresses:**
  - `0x2000` = 0x01 (success)
  - `0x2001`

## New Test Types and Coverage Stubs (2024)

### Unit Test Coverage Gaps (Stubs Added)
- CPU: Illegal opcodes, decimal mode, interrupts (NMI/IRQ/RESET), WAI/STP, XCE, emulation mode transitions
- PPU: Mode 7, mosaic, windowing, raster effects, mid-frame register changes, timing accuracy
- ROM: LoROM/HiROM mapping, header corruption, SRAM
- APU: Placeholder for future unit/ROM-based tests
- Python: Input handling, state save/load
- Framework: Reporting, error handling, config edge cases

### ROM-Based Test Additions
- `illegal_opcodes`: Tests for illegal/undocumented opcodes (ROM: `illegal-opcode-*.sfc`)
- `decimal_mode`: Tests for decimal mode ADC/SBC (ROM: `decimal-mode-*.sfc`)
- `interrupts`: NMI/IRQ/RESET interrupt tests (ROM: `interrupt-*.sfc`)
- `lorom`/`hirom`: LoROM/HiROM mapping (ROM: `lorom-*.sfc`, `hirom-*.sfc`)
- `header_corruption`: Corrupted header handling (ROM: `header-corrupt-*.sfc`)
- `sram`: SRAM save/load (ROM: `sram-*.sfc`)
- `apu_advanced`: Advanced PPU/APU features (ROM: `ppu-adv-*.sfc`, `mode7-*.sfc`, `raster-*.sfc`)

### Adding New ROMs/Tests
1. Place new ROM files in `tests/roms/`.
2. Add a new entry in `tests/config/test_config.json` under `test_roms` with a unique key, description, file pattern, and detection patterns.
3. Ensure the test runner scripts (`run_rom_tests.py`, `run_comprehensive_tests.py`) can discover and execute the new ROM/test.
4. For unit tests, add new `TEST_F` or `TEST_P` cases in the appropriate C++ or Python test file. Use `GTEST_SKIP()` or `pytest.skip()` for stubs.

### Interpreting Test Results
- Skipped tests indicate coverage stubs for future implementation.
- New ROM-based tests will appear in reports with their configured detection patterns.
- CI/CD will fail if any required test fails (including new coverage areas once implemented).

### Updating the Framework
- Update this guide and `test_config.json` whenever new test types or ROMs are added.
- Ensure all new tests are integrated into CI/CD and reporting.
