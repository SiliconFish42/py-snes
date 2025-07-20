#!/usr/bin/env python3
"""
ROM Test Runner for PySNES
This script runs the cputest ROMs and analyzes the results
"""

import os
import subprocess
import sys
import time
from pathlib import Path


def run_command(cmd, timeout=300, cwd=None, env=None):
    """Run a command with timeout"""
    if cwd is None:
        cwd = Path(__file__).parent.parent.parent  # Project root

    try:
        result = subprocess.run(
            cmd, capture_output=True, text=True, timeout=timeout, cwd=cwd, env=env
        )
        return result.returncode, result.stdout, result.stderr
    except subprocess.TimeoutExpired:
        return -1, "", "Command timed out"
    except Exception as e:
        return -1, "", str(e)


def build_tests():
    """Build the test executable"""
    print("Building tests...")

    # Create build directory
    build_dir = Path("build")
    build_dir.mkdir(exist_ok=True)

    # Configure with CMake (run from build directory)
    cmd = ["cmake", ".."]
    returncode, stdout, stderr = run_command(cmd, cwd=build_dir)

    if returncode != 0:
        print(f"CMake configuration failed: {stderr}")
        return False

    # Build
    cmd = ["make", "run_tests"]
    returncode, stdout, stderr = run_command(cmd, cwd=build_dir)

    if returncode != 0:
        print(f"Build failed: {stderr}")
        return False

    print("Build successful!")
    return True


def run_rom_analysis():
    """Run ROM analysis on the test ROMs"""
    print("\n=== Running ROM Analysis ===")

    rom_files = ["tests/roms/cputest-basic.sfc", "tests/roms/cputest-full.sfc"]

    for rom_file in rom_files:
        if not Path(rom_file).exists():
            print(f"Warning: ROM file {rom_file} not found")
            continue

        print(f"\nAnalyzing {rom_file}...")
        cmd = ["python", "tools/rom_analyzer.py", rom_file]
        returncode, stdout, stderr = run_command(cmd)

        if returncode == 0:
            print(stdout)
        else:
            print(f"Analysis failed: {stderr}")


def run_unit_tests():
    """Run the unit tests"""
    print("\n=== Running Unit Tests ===")

    test_executable = Path("build/run_tests")
    if not test_executable.exists():
        print("Test executable not found. Building first...")
        if not build_tests():
            return False

    # Set profiling output directory to build directory
    env = os.environ.copy()
    env["LLVM_PROFILE_FILE"] = str(Path("build/default.profraw"))

    cmd = [str(test_executable)]
    returncode, stdout, stderr = run_command(cmd, timeout=600, env=env)

    print("Test output:")
    print(stdout)

    if stderr:
        print("Test errors:")
        print(stderr)

    return returncode == 0


def run_specific_test(test_name):
    """Run a specific test"""
    print(f"\n=== Running Test: {test_name} ===")

    test_executable = Path("build/run_tests")
    if not test_executable.exists():
        print("Test executable not found. Building first...")
        if not build_tests():
            return False

    # Set profiling output directory to build directory
    env = os.environ.copy()
    env["LLVM_PROFILE_FILE"] = str(Path("build/default.profraw"))

    cmd = [str(test_executable), "--gtest_filter", test_name]
    returncode, stdout, stderr = run_command(cmd, timeout=300, env=env)

    print("Test output:")
    print(stdout)

    if stderr:
        print("Test errors:")
        print(stderr)

    return returncode == 0


def check_rom_files():
    """Check if ROM files exist"""
    print("=== Checking ROM Files ===")

    rom_files = ["tests/roms/cputest-basic.sfc", "tests/roms/cputest-full.sfc"]
    missing_files = []

    for rom_file in rom_files:
        if Path(rom_file).exists():
            size = Path(rom_file).stat().st_size
            print(f"✓ {rom_file} ({size:,} bytes)")
        else:
            print(f"✗ {rom_file} (missing)")
            missing_files.append(rom_file)

    if missing_files:
        print(f"\nWarning: Missing ROM files: {missing_files}")
        print("You may need to obtain the cputest ROMs separately.")
        return False

    return True


def main():
    """Main test runner"""
    print("PySNES ROM Test Runner")
    print("=" * 50)

    # Check ROM files
    if not check_rom_files():
        print("\nSome ROM files are missing. Continuing with available tests...")

    # Build tests
    if not build_tests():
        print("Failed to build tests. Exiting.")
        sys.exit(1)

    # Run ROM analysis
    run_rom_analysis()

    # Run unit tests
    if not run_unit_tests():
        print("Unit tests failed!")
        sys.exit(1)

    # Run specific ROM tests
    print("\n=== Running ROM-Specific Tests ===")

    rom_tests = [
        "ROMTest.LoadBasicCPUTest",
        "ROMTest.LoadFullCPUTest",
        "ROMTest.BasicExecution",
        "ROMTest.TestPatterns",
        "ROMTest.ErrorDetection",
        "ROMTest.InstructionSequences",
        "ROMTest.MemoryAccessPatterns",
        "ROMTest.CPUStateConsistency",
        "ROMTest.ROMSpecificTests",
    ]

    for test in rom_tests:
        print(f"\nRunning {test}...")
        success = run_specific_test(test)
        if not success:
            print(f"Test {test} failed!")
        else:
            print(f"Test {test} passed!")

    print("\n" + "=" * 50)
    print("ROM testing complete!")


if __name__ == "__main__":
    main()
