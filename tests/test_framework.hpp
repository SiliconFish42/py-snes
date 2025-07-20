#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "gtest/gtest.h"
#include "cpu.hpp"
#include "bus.hpp"
#include "cartridge.hpp"

// Test ROM types
enum class TestROMType {
    CPU_65816,
    SPC_700,
    PPU,
    APU,
    UNKNOWN
};

// Test result status
enum class TestStatus {
    PASSED,
    FAILED,
    TIMEOUT,
    CRASHED,
    NOT_STARTED
};

// Test execution context
struct TestContext {
    std::string rom_path;
    TestROMType type;
    uint32_t max_cycles;
    uint32_t timeout_ms;
    bool verbose_output;
    std::function<void(const std::string&)> log_callback;
    
    TestContext() : type(TestROMType::UNKNOWN), max_cycles(1000000), 
                   timeout_ms(30000), verbose_output(false) {}
};

// Test result structure
struct TestResult {
    TestStatus status;
    uint32_t cycles_executed;
    uint32_t execution_time_ms;
    std::string error_message;
    std::vector<std::string> log_messages;
    
    TestResult() : status(TestStatus::NOT_STARTED), cycles_executed(0), 
                  execution_time_ms(0) {}
};

// Base test ROM runner
class TestROMRunner {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    std::shared_ptr<Cartridge> cart;
    TestContext context;
    TestResult result;
    
    // Memory monitoring
    std::vector<uint32_t> memory_accesses;
    std::vector<uint8_t> memory_snapshots;
    
    // Execution monitoring
    uint32_t cycle_count;
    std::chrono::steady_clock::time_point start_time;
    
    // Test completion detection
    virtual bool detect_test_completion() = 0;
    virtual bool detect_test_failure() = 0;
    virtual void log_execution_state() = 0;
    
public:
    TestROMRunner() : cycle_count(0) {}
    virtual ~TestROMRunner() = default;
    
    // Setup and teardown
    virtual bool setup(const TestContext& ctx);
    virtual void teardown();
    
    // Main execution
    virtual TestResult run_test();
    
    // Utility methods
    virtual void log_message(const std::string& message);
    virtual void take_memory_snapshot();
    virtual bool check_timeout();
    
    // Getters
    const TestResult& get_result() const { return result; }
    const TestContext& get_context() const { return context; }
};

// CPU 65816 test runner
class CPU65816TestRunner : public TestROMRunner {
private:
    // CPU-specific test completion detection
    bool detect_test_completion() override;
    bool detect_test_failure() override;
    void log_execution_state() override;
    
    // CPU-specific monitoring
    uint32_t last_pc;
    uint32_t consecutive_brk_count;
    std::vector<uint32_t> pc_history;
    
public:
    CPU65816TestRunner() : last_pc(0), consecutive_brk_count(0) {}
    
    // CPU-specific test methods
    bool check_register_consistency();
    bool check_memory_access_patterns();
    void analyze_instruction_flow();
};

// SPC-700 test runner (placeholder for future implementation)
class SPC700TestRunner : public TestROMRunner {
private:
    bool detect_test_completion() override;
    bool detect_test_failure() override;
    void log_execution_state() override;
    
public:
    SPC700TestRunner() = default;
    
    // SPC-700 specific methods would go here
    // For now, this is a placeholder
};

// Test framework manager
class TestFramework {
private:
    std::vector<std::unique_ptr<TestROMRunner>> runners;
    std::vector<TestResult> results;
    
    // Test ROM type detection
    TestROMType detect_rom_type(const std::string& rom_path);
    
    // Test ROM factory
    std::unique_ptr<TestROMRunner> create_runner(TestROMType type);
    
public:
    TestFramework() = default;
    
    // Main test execution
    std::vector<TestResult> run_all_tests(const std::vector<std::string>& rom_paths, 
                                         const TestContext& base_context = TestContext());
    
    TestResult run_single_test(const std::string& rom_path, 
                              const TestContext& context = TestContext());
    
    // Test management
    void add_custom_runner(std::unique_ptr<TestROMRunner> runner);
    void clear_runners();
    
    // Results and reporting
    void print_results(const std::vector<TestResult>& results);
    void save_results(const std::vector<TestResult>& results, const std::string& filename);
    void generate_report(const std::vector<TestResult>& results, const std::string& filename);
};

// Utility functions
namespace TestUtils {
    std::string test_status_to_string(TestStatus status);
    std::string test_rom_type_to_string(TestROMType type);
    TestROMType string_to_test_rom_type(const std::string& type_str);
    
    // ROM analysis utilities
    bool analyze_rom_header(const std::string& rom_path, std::ostream& out = std::cout);
    std::vector<std::string> find_strings_in_rom(const std::string& rom_path);
    std::vector<uint8_t> extract_rom_data(const std::string& rom_path, 
                                         uint32_t offset, uint32_t length);
    
    // Memory analysis utilities
    std::vector<uint32_t> find_memory_patterns(const std::vector<uint8_t>& data, 
                                              const std::vector<uint8_t>& pattern);
    void dump_memory_region(const std::vector<uint8_t>& data, uint32_t start, 
                           uint32_t length, std::ostream& out = std::cout);
} 