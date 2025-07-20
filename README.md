# PySNES: A Super Nintendo Emulator for AI Training

**PySNES** is a Super Nintendo Entertainment System (SNES) emulator designed for use as a Python library, with a focus on reinforcement learning and AI training. The project provides a clean, modern interface to the emulator core, allowing AI agents to interact with SNES games programmatically.

This project is inspired by the architecture and goals of the excellent [nes-py](https://github.com/Kautenja/nes-py) project, which provides a similar environment for the NES.

## Features

- **65816 CPU Core:** Fully implemented and tested, including all addressing modes and instructions.
- **PPU (Graphics):** In-progress. Basic scanline/frame timing, VRAM/CGRAM/OAM, and framebuffer implemented. Advanced features (windowing, color math, mode 7) are planned.
- **Bus, Cartridge, Controller:** Robust bus connects CPU, PPU, cartridge, and controllers. LoROM mapping and basic controller input supported.
- **Python API:** Exposes the C++ core via `pybind11` as `pysnes_cpp`.
- **Gymnasium Environment:** `SnesEnv` provides a Gymnasium-compatible RL environment.
- **CLI:** Command-line interface for running ROMs in random or human mode.
- **Comprehensive Testing:** Extensive C++ and ROM-based test framework, with automated reporting and CI support.

## Current State

- ✅ **CPU Core:** Complete and tested.
- ⚙️ **PPU:** Basic structure and scanline logic implemented; advanced features in progress.
- ✅ **Bus, Cartridge, Controller:** Fully functional for core emulation.
- ✅ **Python & Gymnasium API:** Usable for RL/AI research.
- ✅ **Testing:** Robust framework for unit, integration, and ROM-based tests.

## Build Instructions

### Prerequisites

* A C++14 compatible compiler (Clang or GCC recommended)
* Python 3.11+
* [CMake](https://cmake.org/install/)
* [uv](https://github.com/astral-sh/uv) (required; Makefile will check for it)

### Installation & Build

The recommended way to build and install PySNES is using the Makefile:

```bash
make build
```

This will:
- Ensure `uv` is installed
- Build the Python package and C++ extension in editable mode using `uv pip install -e .`

Alternatively, you can run the build directly:

```bash
uv pip install -e .
```

## Usage

### Command-Line Interface

Run a ROM file using the CLI:

```bash
pysnes /path/to/your/rom.sfc --random
```

- `--random`: Runs the emulator with random actions (demonstration mode)
- `--human`: (Stub) Intended for interactive play (not yet implemented)

### Python API & Gymnasium Environment

You can use PySNES as a Gymnasium environment for RL/AI:

```python
import pysnes
from pysnes.env import SnesEnv

env = SnesEnv('/path/to/your/rom.sfc')
obs = env.reset()
done = False
while not done:
    action = env.action_space.sample()  # Replace with your agent's action
    obs, reward, done, info = env.step(action)
    env.render()
env.close()
```

- `action_space`: 8-bit discrete (SNES controller buttons)
- `observation_space`: 256x240 RGBA framebuffer (numpy array)
- `reward` and `done` logic are placeholders for user customization

## Testing

To run all tests (C++ and Python):

```bash
make test
```

This will:
- Build the Python package and C++ extension (if not already built)
- Build the C++ test suite
- Run the comprehensive test framework and C++/GTest tests

### Test Framework Highlights
- **Unit Tests:** GTest-based, covering CPU, stack, transfer, control, and ROM loading.
- **ROM Tests:** Automated execution and validation of test ROMs (CPU, PPU, APU, SPC-700 planned).
- **Configurable Suites:** JSON-driven configuration for quick, standard, comprehensive, and CI test runs.
- **Automated Reporting:** Generates JSON, TXT, and HTML reports in `test_output/`.

To clean build and test outputs:

```bash
make clean
```

See [TEST_FRAMEWORK_GUIDE.md](TEST_FRAMEWORK_GUIDE.md) and [ROM_TESTING.md](ROM_TESTING.md) for detailed documentation.

## Contributing

Contributions are welcome! To get started:
- Fork the repository and create a feature branch
- Build and test your changes locally (`make build && make test`)
- Add or update tests as appropriate
- Submit a pull request with a clear description of your changes

See [DEV_PLAN.md](DEV_PLAN.md) and [TEST_PLAN.md](TEST_PLAN.md) for more information on project direction and testing strategy.

## Future Goals

* Complete PPU implementation (graphics rendering, windowing, color math, mode 7)
* Add support for audio processing (APU)
* Expand Gymnasium environment with proper reward functions and episode logic
* Improve CLI for interactive play and debugging
* Enhance test coverage for PPU, APU, and full-system integration
