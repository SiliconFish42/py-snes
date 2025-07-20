# PySNES: A Super Nintendo Emulator for AI Training

**PySNES** is a work-in-progress Super Nintendo Entertainment System (SNES) emulator designed to be used as a Python library for reinforcement learning and AI training. The project provides a simple interface to the emulator core, allowing an AI agent to interact with SNES games programmatically.

This project is heavily inspired by the architecture and goals of the excellent [nes-py](https://github.com/Kautenja/nes-py) project, which provides a similar environment for the original Nintendo Entertainment System.

## Current State

The project is currently in the early stages of development.

* ✅ **Modern Build System**: The project is built using `scikit-build-core` and `pybind11`, allowing for a clean and robust compilation of the C++ core into a Python extension.
* ✅ **Core Architecture**: A basic C++ architecture is in place, including classes for the CPU, PPU, and Bus. The PIMPL design pattern is used to provide a clean public interface.
* ✅ **Successful Compilation**: The C++ core successfully compiles and can be imported as a Python module (`pysnes_cpp`).
* ✅ **Comprehensive Testing**: A robust test framework is in place with unit tests, ROM tests, and automated test reporting.
* ⚙️ **In-Progress Emulation**: The core emulation logic is still being implemented. The CPU is structured with a `switch`-based dispatcher, but most of the opcode and addressing mode functions are currently stubs.
* 🐍 **Basic Python Interface**: A simple Gymnasium environment (`SnesEnv`) has been created to wrap the C++ core, and a basic command-line interface is available for running ROMs.

## Build Instructions

### Prerequisites

* A C++14 compatible compiler (like Clang or GCC)
* Python 3.11+
* [CMake](https://cmake.org/install/)
* [uv](https://github.com/astral-sh/uv) (required; Makefile will check for it)

### Installation & Build

The recommended way to build and install PySNES is using the Makefile, which manages all dependencies and build steps:

```bash
make build
```

This will:
- Ensure `uv` is installed (the Makefile will print an error if not)
- Build the Python package and C++ extension in editable mode using `uv pip install -e .`

Alternatively, you can run the build directly:

```bash
uv pip install -e .
```

## Usage

Once installed, you can run a ROM file using the command-line interface. Currently, the emulator will only execute placeholder logic, but it demonstrates that the core is running.

```bash
pysnes /path/to/your/rom.sfc --random
```

## Testing

To run all tests (Python and C++):

```bash
make test
```

This will:
- Build the Python package and C++ extension (if not already built)
- Build the C++ test suite
- Run the comprehensive test framework and C++/GTest tests

To clean build and test outputs:

```bash
make clean
```

See [TEST_FRAMEWORK_GUIDE.md](TEST_FRAMEWORK_GUIDE.md) and [ROM_TESTING.md](ROM_TESTING.md) for detailed testing documentation.

## Future Goals

* Implement the full 65816 CPU instruction set.
* Develop the PPU to correctly render graphics.
* Add support for audio processing (APU).
* Expand the Gymnasium environment with proper reward functions for different games.
