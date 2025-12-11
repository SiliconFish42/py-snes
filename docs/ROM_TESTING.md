# ROM Testing Guide for PySNES

This guide explains how to test the PySNES CPU emulator against the cputest ROMs.

## Overview

The cputest ROMs are specialized test ROMs designed to verify the accuracy of 65816 CPU emulation. They test various aspects of the CPU including:

- Instruction execution
- Addressing modes
- Flag handling
- Memory access patterns
- Edge cases and corner conditions

## Prerequisites

1. **ROM Files**: You need the cputest ROM files:
   - `tests/roms/cputest-basic.sfc` - Basic CPU tests
   - `tests/roms/cputest-full.sfc` - Comprehensive CPU tests

2. **Build Tools**:
   - CMake 3.15+
   - C++ compiler (GCC, Clang, or MSVC)
   - Python 3.11+

3. **Dependencies**:
   - Google Test (automatically downloaded by CMake)
   - pybind11 (for Python bindings)

## Quick Start

### 1. Build and Run Tests

```bash
# Build the test suite
python tests/scripts/run_rom_tests.py
```

This will:

- Build the test executable
- Analyze the ROM files
- Run unit tests
- Run ROM-specific tests

### 2. Manual Testing

```bash
# Build tests manually
mkdir build
cd build
cmake ..
make run_tests

# Run specific tests (from build directory)
./run_tests --gtest_filter="ROMTest.*"

# Run with XML output to test_output directory
./run_tests --gtest_output=xml:../test_output/test_results.xml

# Run ROM analysis
python ../tools/rom_analyzer.py ../tests/roms/cputest-basic.sfc
```

## Test Structure

### Unit Tests (`tests/test_cpu.cpp`)

- Test individual CPU instructions
- Verify addressing modes
- Check flag handling
- Test transfer instructions
- Test stack operations

### ROM Tests (`tests/test_rom.cpp`)

- Load and execute actual ROM files
- Monitor CPU state during execution
- Detect test completion/failure
- Analyze memory access patterns
- Verify instruction sequences

### ROM Analysis (`tools/rom_analyzer.py`)

- Analyze ROM structure and content
- Find strings and patterns
- Identify entry points
- Map memory sections
- Count opcode frequencies

## Test Output Management

### Output Directory

All test outputs are organized in the `test_output/` directory:

- **Reports**: JSON and text reports from test runs
- **XML Output**: Google Test XML output files
- **Logs**: Detailed execution logs and debugging information

### Cleanup

```bash
# Clean up old test reports (older than 7 days)
python tools/cleanup_tests.py --old

# Clean up all test outputs
python tools/cleanup_tests.py --all
```

## Understanding Test Results

### Success Indicators

- CPU executes instructions without crashing
- Program counter advances normally
- Memory access patterns are reasonable
- Test completion signals are detected

### Failure Indicators

- CPU crashes or hangs
- Invalid memory access
- Unexpected register values
- Test failure signals detected

### Common Issues

1. **Missing ROM Files**

   ```
   Warning: ROM file cputest-basic.sfc not found
   ```

   Solution: Obtain the cputest ROM files and place them in the project root.

2. **Build Failures**

   ```
   CMake Error: Could not find pybind11
   ```

   Solution: Install pybind11: `pip install pybind11`

3. **Test Timeouts**

   ```
   Command timed out
   ```

   Solution: Increase timeout values or debug infinite loops in CPU emulation.

## Interpreting ROM Analysis

### Header Information

```
Game Title: CPUTEST
ROM Type: 00
ROM Size: 08
Reset Vector: 0x8000
```

### Opcode Frequency

```
Most common opcodes:
  0xA9 (LDA #imm): 150 occurrences
  0x85 (STA dp): 120 occurrences
  0xEA (NOP): 100 occurrences
```

### Memory Sections

```
Memory sections:
  0x000000-0x007FC0: Code
  0x007FC0-0x008000: Header
  0x008000-0x010000: ROM Data
```

## Debugging Tips

### 1. Enable Debug Output

Add debug prints to the CPU step function:

```cpp
void CPU::step() {
    opcode = bus->read(pc);
    printf("DEBUG: PC=0x%06X, Opcode=0x%02X\n", pc, opcode);
    // ... rest of step function
}
```

### 2. Monitor Specific Addresses

```cpp
// In test code
uint8_t status = bus->read(0x7E0000);
if (status != 0) {
    printf("Status changed: 0x%02X at PC=0x%06X\n", status, cpu->pc);
}
```

### 3. Track Instruction Sequences

```cpp
std::vector<uint8_t> opcodes;
for (int i = 0; i < 100; i++) {
    opcodes.push_back(bus->read(cpu->pc));
    cpu->step();
}
```

## Advanced Testing

### Custom Test ROMs

You can create custom test ROMs to test specific functionality:

1. Write assembly code for the 65816
2. Assemble to binary
3. Add SNES ROM header
4. Test with the emulator

### Performance Testing

```bash
# Time test execution
time python run_rom_tests.py

# Profile CPU performance
./run_tests --gtest_filter="ROMTest.ROMSpecificTests" --gtest_repeat=10
```

### Memory Leak Detection

```bash
# Use Valgrind (Linux/macOS)
valgrind --leak-check=full ./run_tests

# Use AddressSanitizer
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address" ..
```

## Contributing

When adding new CPU instructions or fixing bugs:

1. Add unit tests for the specific instruction
2. Update ROM tests to verify the fix
3. Run the full test suite
4. Document any changes in behavior

## Resources

- [65816 Instruction Set Reference](https://wiki.superfamicom.org/65816-reference)
- [SNES ROM Header Format](https://wiki.superfamicom.org/snes-rom-header)
- [Google Test Documentation](https://google.github.io/googletest/)

## Troubleshooting

### Common Build Issues

1. **CMake not found**: Install CMake from cmake.org
2. **pybind11 not found**: `pip install pybind11`
3. **Compiler not found**: Install Xcode Command Line Tools (macOS) or Visual Studio (Windows)

### Test Failures

1. **CPU crashes**: Check for unhandled opcodes or invalid memory access
2. **Wrong results**: Verify instruction implementation against 65816 reference
3. **Infinite loops**: Add cycle limits and debug output

### Performance Issues

1. **Slow execution**: Profile with gprof or similar tools
2. **Memory usage**: Check for memory leaks with Valgrind
3. **Build time**: Use parallel builds: `make -j$(nproc)`
