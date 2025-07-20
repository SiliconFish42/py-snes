#!/usr/bin/env python3
"""
Comprehensive Test Runner for PySNES
This script uses the new test framework to run tests against various ROMs
"""

import subprocess
import sys
import os
import time
import json
import argparse
from pathlib import Path
from typing import List, Dict, Any

class ComprehensiveTestRunner:
    def __init__(self, build_dir: str = "build"):
        self.build_dir = Path(build_dir)
        self.test_executable = self.build_dir / "run_tests"
        self.results = []
        
    def run_command(self, cmd: List[str], timeout: int = 300, cwd: Path = None, env: dict = None) -> Dict[str, Any]:
        """Run a command and return results"""
        if cwd is None:
            cwd = self.build_dir.parent
        
        try:
            result = subprocess.run(
                cmd, 
                capture_output=True, 
                text=True, 
                timeout=timeout,
                cwd=cwd,
                env=env
            )
            return {
                "returncode": result.returncode,
                "stdout": result.stdout,
                "stderr": result.stderr,
                "success": result.returncode == 0
            }
        except subprocess.TimeoutExpired:
            return {
                "returncode": -1,
                "stdout": "",
                "stderr": "Command timed out",
                "success": False
            }
        except Exception as e:
            return {
                "returncode": -1,
                "stdout": "",
                "stderr": str(e),
                "success": False
            }
    
    def build_tests(self) -> bool:
        """Build the test executable"""
        print("Building comprehensive test framework...")
        
        # Create build directory
        self.build_dir.mkdir(exist_ok=True)
        
        # Configure with CMake (run from build directory)
        config_result = self.run_command(["cmake", str(self.build_dir.parent)], cwd=self.build_dir)
        if not config_result["success"]:
            print(f"CMake configuration failed: {config_result['stderr']}")
            return False
        
        # Build tests
        build_result = self.run_command(["make", "run_tests"], cwd=self.build_dir)
        if not build_result["success"]:
            print(f"Build failed: {build_result['stderr']}")
            return False
        
        print("✅ Test framework built successfully")
        return True
    
    def find_test_roms(self) -> List[str]:
        """Find available test ROMs"""
        rom_patterns = [
            "tests/roms/cputest-*.sfc",
            "tests/roms/spctest-*.sfc", 
            "tests/roms/pputest-*.sfc",
            "tests/roms/aputest-*.sfc",
            "tests/roms/test-*.sfc",
            "tests/roms/*.sfc"
        ]
        
        roms = []
        for pattern in rom_patterns:
            roms.extend(Path(".").glob(pattern))
        
        # Remove duplicates and sort
        roms = sorted(list(set(roms)))
        
        print(f"Found {len(roms)} test ROMs:")
        for rom in roms:
            print(f"  - {rom}")
        
        return [str(rom) for rom in roms]
    
    def run_framework_tests(self) -> Dict[str, Any]:
        """Run the test framework unit tests"""
        print("\n🧪 Running test framework unit tests...")
        
        # Set profiling output directory to build directory
        env = os.environ.copy()
        env['LLVM_PROFILE_FILE'] = str(self.build_dir / "default.profraw")
        
        result = self.run_command([
            str(self.test_executable),
            "--gtest_filter=TestFrameworkTest.*"
        ], env=env)
        
        if result["success"]:
            print("✅ Test framework unit tests passed")
        else:
            print(f"❌ Test framework unit tests failed: {result['stderr']}")
        
        return result
    
    def run_rom_tests(self, roms: List[str]) -> Dict[str, Any]:
        """Run tests with actual ROM files"""
        print(f"\n🎮 Running ROM tests with {len(roms)} ROMs...")
        
        # Set profiling output directory to build directory
        env = os.environ.copy()
        env['LLVM_PROFILE_FILE'] = str(self.build_dir / "default.profraw")
        
        result = self.run_command([
            str(self.test_executable),
            "--gtest_filter=TestFrameworkWithROMsTest.*"
        ], env=env)
        
        if result["success"]:
            print("✅ ROM tests completed")
        else:
            print(f"⚠️  ROM tests had issues: {result['stderr']}")
        
        return result
    
    def run_cpu_tests(self) -> Dict[str, Any]:
        """Run CPU instruction tests"""
        print("\n🔧 Running CPU instruction tests...")
        
        # Set profiling output directory to build directory
        env = os.environ.copy()
        env['LLVM_PROFILE_FILE'] = str(self.build_dir / "default.profraw")
        
        result = self.run_command([
            str(self.test_executable),
            "--gtest_filter=ROMTest.*"
        ], env=env)
        
        if result["success"]:
            print("✅ CPU instruction tests passed")
        else:
            print(f"❌ CPU instruction tests failed: {result['stderr']}")
        
        return result
    
    def run_rom_analysis(self, roms: List[str]) -> Dict[str, Any]:
        """Run ROM analysis tools"""
        print("\n📊 Running ROM analysis...")
        
        for rom in roms:
            print(f"  Analyzing {rom}...")
            result = self.run_command([
                "python", "tools/rom_analyzer.py", rom
            ])
            
            if result["success"]:
                print(f"    ✅ Analysis completed")
            else:
                print(f"    ⚠️  Analysis failed: {result['stderr']}")
    
    def generate_report(self, results: List[Dict[str, Any]]) -> None:
        """Generate a comprehensive test report"""
        print("\n📋 Generating test report...")
        
        report = {
            "timestamp": time.strftime("%Y-%m-%d %H:%M:%S"),
            "test_results": results,
            "summary": {
                "total_tests": len(results),
                "passed": sum(1 for r in results if r.get("success", False)),
                "failed": sum(1 for r in results if not r.get("success", False))
            }
        }
        
        # Ensure output directory exists
        output_dir = Path("test_output")
        output_dir.mkdir(exist_ok=True)
        
        # Save JSON report
        report_file = output_dir / "test_report.json"
        with open(report_file, 'w') as f:
            json.dump(report, f, indent=2)
        
        # Save text report
        text_report_file = output_dir / "test_report.txt"
        with open(text_report_file, 'w') as f:
            f.write("PySNES Comprehensive Test Report\n")
            f.write("=" * 40 + "\n\n")
            f.write(f"Generated: {report['timestamp']}\n\n")
            
            f.write("Summary:\n")
            f.write(f"  Total tests: {report['summary']['total_tests']}\n")
            f.write(f"  Passed: {report['summary']['passed']}\n")
            f.write(f"  Failed: {report['summary']['failed']}\n\n")
            
            f.write("Detailed Results:\n")
            f.write("-" * 20 + "\n")
            for i, result in enumerate(results, 1):
                f.write(f"Test {i}: {'✅ PASSED' if result.get('success') else '❌ FAILED'}\n")
                if result.get('stderr'):
                    f.write(f"  Error: {result['stderr']}\n")
                f.write("\n")
        
        print(f"✅ Reports saved to {report_file} and {text_report_file}")
    
    def run_all_cpp_tests(self) -> Dict[str, Any]:
        """Run all C++/GTest tests (no filter)"""
        print("\n🧩 Running ALL C++/GTest tests (no filter)...")
        env = os.environ.copy()
        env['LLVM_PROFILE_FILE'] = str(self.build_dir / "default.profraw")
        result = self.run_command([
            str(self.test_executable)
        ], env=env)
        if result["success"]:
            print("✅ All C++/GTest tests passed")
        else:
            print(f"❌ Some C++/GTest tests failed: {result['stderr']}")
        return result
    
    def run_comprehensive_tests(self) -> bool:
        """Run all comprehensive tests"""
        print("🚀 Starting PySNES Comprehensive Test Suite")
        print("=" * 50)
        
        # Build tests
        if not self.build_tests():
            return False
        
        # Find test ROMs
        roms = self.find_test_roms()
        
        # Run different test categories
        test_results = []
        
        # Only run all C++/GTest tests (no filter)
        all_cpp_result = self.run_all_cpp_tests()
        test_results.append({
            "name": "All C++/GTest Tests (No Filter)",
            "success": all_cpp_result["success"],
            "stdout": all_cpp_result["stdout"],
            "stderr": all_cpp_result["stderr"]
        })
        
        # ROM tests (if ROMs available)
        if roms:
            rom_result = self.run_rom_tests(roms)
            test_results.append({
                "name": "ROM Tests",
                "success": rom_result["success"],
                "stdout": rom_result["stdout"],
                "stderr": rom_result["stderr"]
            })
            
            # 4. ROM analysis
            self.run_rom_analysis(roms)
        
        # Generate report
        self.generate_report(test_results)
        
        # Print summary
        passed = sum(1 for r in test_results if r["success"])
        total = len(test_results)
        
        print("\n" + "=" * 50)
        print(f"🎯 Test Suite Complete: {passed}/{total} test categories passed")
        
        if passed == total:
            print("🎉 All tests passed! PySNES is ready for development.")
            return True
        else:
            print("⚠️  Some tests failed. Check the report for details.")
            return False

def main():
    parser = argparse.ArgumentParser(description="PySNES Comprehensive Test Runner")
    parser.add_argument("--build-dir", default="build", help="Build directory")
    parser.add_argument("--timeout", type=int, default=300, help="Test timeout in seconds")
    parser.add_argument("--verbose", "-v", action="store_true", help="Verbose output")
    
    args = parser.parse_args()
    
    runner = ComprehensiveTestRunner(args.build_dir)
    
    try:
        success = runner.run_comprehensive_tests()
        sys.exit(0 if success else 1)
    except KeyboardInterrupt:
        print("\n⚠️  Test run interrupted by user")
        sys.exit(1)
    except Exception as e:
        print(f"\n❌ Test run failed with error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main() 