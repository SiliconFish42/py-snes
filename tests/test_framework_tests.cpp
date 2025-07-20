#include "gtest/gtest.h"
#include "test_framework.hpp"
#include <filesystem>

class TestFrameworkTest : public ::testing::Test {
protected:
    TestFramework framework;

    void SetUp() override {
        // Framework is ready to use
    }
};

// Test basic framework functionality
TEST_F(TestFrameworkTest, FrameworkInitialization) {
    EXPECT_TRUE(true); // Framework should initialize without errors
}

// Test ROM type detection
TEST_F(TestFrameworkTest, ROMTypeDetection) {
    EXPECT_EQ(TestUtils::string_to_test_rom_type("CPU_65816"), TestROMType::CPU_65816);
    EXPECT_EQ(TestUtils::string_to_test_rom_type("SPC_700"), TestROMType::SPC_700);
    EXPECT_EQ(TestUtils::string_to_test_rom_type("PPU"), TestROMType::PPU);
    EXPECT_EQ(TestUtils::string_to_test_rom_type("APU"), TestROMType::APU);
    EXPECT_EQ(TestUtils::string_to_test_rom_type("UNKNOWN"), TestROMType::UNKNOWN);
}

// Test status string conversion
TEST_F(TestFrameworkTest, StatusStringConversion) {
    EXPECT_EQ(TestUtils::test_status_to_string(TestStatus::PASSED), "PASSED");
    EXPECT_EQ(TestUtils::test_status_to_string(TestStatus::FAILED), "FAILED");
    EXPECT_EQ(TestUtils::test_status_to_string(TestStatus::TIMEOUT), "TIMEOUT");
    EXPECT_EQ(TestUtils::test_status_to_string(TestStatus::CRASHED), "CRASHED");
    EXPECT_EQ(TestUtils::test_status_to_string(TestStatus::NOT_STARTED), "NOT_STARTED");
}

// Test ROM type string conversion
TEST_F(TestFrameworkTest, ROMTypeStringConversion) {
    EXPECT_EQ(TestUtils::test_rom_type_to_string(TestROMType::CPU_65816), "CPU_65816");
    EXPECT_EQ(TestUtils::test_rom_type_to_string(TestROMType::SPC_700), "SPC_700");
    EXPECT_EQ(TestUtils::test_rom_type_to_string(TestROMType::PPU), "PPU");
    EXPECT_EQ(TestUtils::test_rom_type_to_string(TestROMType::APU), "APU");
    EXPECT_EQ(TestUtils::test_rom_type_to_string(TestROMType::UNKNOWN), "UNKNOWN");
}

// Test CPU test runner creation
TEST_F(TestFrameworkTest, CPU65816TestRunnerCreation) {
    auto runner = std::make_unique<CPU65816TestRunner>();
    EXPECT_NE(runner, nullptr);
}

// Test SPC-700 test runner creation
TEST_F(TestFrameworkTest, SPC700TestRunnerCreation) {
    auto runner = std::make_unique<SPC700TestRunner>();
    EXPECT_NE(runner, nullptr);
}

// Test ROM analysis utilities
TEST_F(TestFrameworkTest, ROMAnalysisUtilities) {
    // Test with a non-existent file
    std::ostringstream oss;
    bool result = TestUtils::analyze_rom_header("non_existent_file.sfc", oss);
    EXPECT_FALSE(result);
    EXPECT_FALSE(oss.str().empty());
}

// Test string finding in ROM (with empty data)
TEST_F(TestFrameworkTest, StringFindingInROM) {
    // Create a temporary file with some test data
    std::string temp_file = "temp_test_rom.bin";
    std::ofstream file(temp_file, std::ios::binary);
    if (file.is_open()) {
        // Write some test data with strings
        std::string test_data = "SUCCESS\0FAILED\0TEST\0";
        file.write(test_data.c_str(), test_data.length());
        file.close();

        auto strings = TestUtils::find_strings_in_rom(temp_file);
        EXPECT_FALSE(strings.empty());

        // Clean up
        std::filesystem::remove(temp_file);
    }
}

// Test ROM data extraction
TEST_F(TestFrameworkTest, ROMDataExtraction) {
    // Create a temporary file with test data
    std::string temp_file = "temp_test_rom.bin";
    std::ofstream file(temp_file, std::ios::binary);
    if (file.is_open()) {
        std::vector<uint8_t> test_data = {0x01, 0x02, 0x03, 0x04, 0x05};
        file.write(reinterpret_cast<const char*>(test_data.data()), test_data.size());
        file.close();

        auto extracted = TestUtils::extract_rom_data(temp_file, 0, 3);
        EXPECT_EQ(extracted.size(), 3);
        EXPECT_EQ(extracted[0], 0x01);
        EXPECT_EQ(extracted[1], 0x02);
        EXPECT_EQ(extracted[2], 0x03);

        // Clean up
        std::filesystem::remove(temp_file);
    }
}

// Test context creation and modification
TEST_F(TestFrameworkTest, TestContextCreation) {
    TestContext context;
    EXPECT_EQ(context.type, TestROMType::UNKNOWN);
    EXPECT_EQ(context.max_cycles, 1000000);
    EXPECT_EQ(context.timeout_ms, 30000);
    EXPECT_FALSE(context.verbose_output);

    // Modify context
    context.type = TestROMType::CPU_65816;
    context.max_cycles = 500000;
    context.timeout_ms = 15000;
    context.verbose_output = true;

    EXPECT_EQ(context.type, TestROMType::CPU_65816);
    EXPECT_EQ(context.max_cycles, 500000);
    EXPECT_EQ(context.timeout_ms, 15000);
    EXPECT_TRUE(context.verbose_output);
}

// Test result creation and modification
TEST_F(TestFrameworkTest, TestResultCreation) {
    TestResult result;
    EXPECT_EQ(result.status, TestStatus::NOT_STARTED);
    EXPECT_EQ(result.cycles_executed, 0);
    EXPECT_EQ(result.execution_time_ms, 0);
    EXPECT_TRUE(result.error_message.empty());
    EXPECT_TRUE(result.log_messages.empty());

    // Modify result
    result.status = TestStatus::PASSED;
    result.cycles_executed = 1000;
    result.execution_time_ms = 50;
    result.error_message = "Test completed successfully";
    result.log_messages.push_back("Test started");
    result.log_messages.push_back("Test completed");

    EXPECT_EQ(result.status, TestStatus::PASSED);
    EXPECT_EQ(result.cycles_executed, 1000);
    EXPECT_EQ(result.execution_time_ms, 50);
    EXPECT_EQ(result.error_message, "Test completed successfully");
    EXPECT_EQ(result.log_messages.size(), 2);
}

// Test framework with actual ROM files (if available)
class TestFrameworkWithROMsTest : public ::testing::Test {
protected:
    TestFramework framework;
    std::vector<std::string> available_roms;

    void SetUp() override {
        // Check for available ROM files
        std::vector<std::string> possible_roms = {
            "cputest-basic.sfc",
            "cputest-full.sfc",
            "../cputest-basic.sfc",
            "../cputest-full.sfc"
        };

        for (const auto& rom : possible_roms) {
            if (std::filesystem::exists(rom)) {
                available_roms.push_back(rom);
            }
        }
    }
};

// Test ROM analysis with actual files
TEST_F(TestFrameworkWithROMsTest, ROMAnalysisWithRealFiles) {
    if (available_roms.empty()) {
        GTEST_SKIP() << "No test ROMs available for testing";
        return;
    }

    std::ostringstream oss;
    bool result = TestUtils::analyze_rom_header(available_roms[0], oss);
    EXPECT_TRUE(result);

    std::string analysis = oss.str();
    EXPECT_FALSE(analysis.empty());
    EXPECT_NE(analysis.find("ROM Analysis:"), std::string::npos);
}

// Test string finding with real ROMs
TEST_F(TestFrameworkWithROMsTest, StringFindingWithRealROMs) {
    if (available_roms.empty()) {
        GTEST_SKIP() << "No test ROMs available for testing";
        return;
    }

    auto strings = TestUtils::find_strings_in_rom(available_roms[0]);
    // Real ROMs should contain some strings
    EXPECT_FALSE(strings.empty());

    // Print found strings for debugging
    std::cout << "Found " << strings.size() << " strings in " << available_roms[0] << ":\n";
    for (const auto& str : strings) {
        if (str.length() > 4) { // Only show meaningful strings
            std::cout << "  \"" << str << "\"\n";
        }
    }
}

// Test framework execution with real ROMs (basic test)
TEST_F(TestFrameworkWithROMsTest, FrameworkExecutionWithRealROMs) {
    if (available_roms.empty()) {
        GTEST_SKIP() << "No test ROMs available for testing";
        return;
    }

    // Create a test context
    TestContext context;
    context.rom_path = available_roms[0];
    context.type = TestROMType::CPU_65816;
    context.max_cycles = 10000; // Limit cycles for quick test
    context.timeout_ms = 5000;  // 5 second timeout
    context.verbose_output = false;

    // Run the test
    TestResult result = framework.run_single_test(available_roms[0], context);

    // Basic validation
    EXPECT_NE(result.status, TestStatus::NOT_STARTED);
    EXPECT_GT(result.cycles_executed, 0);
    EXPECT_GT(result.execution_time_ms, 0);

    std::cout << "Test result: " << TestUtils::test_status_to_string(result.status)
              << " (" << result.cycles_executed << " cycles, "
              << result.execution_time_ms << "ms)\n";

    if (!result.error_message.empty()) {
        std::cout << "Error: " << result.error_message << "\n";
    }
}

// Test multiple ROM execution
TEST_F(TestFrameworkWithROMsTest, MultipleROMExecution) {
    if (available_roms.size() < 2) {
        GTEST_SKIP() << "Need at least 2 test ROMs for this test";
        return;
    }

    // Create a test context
    TestContext context;
    context.max_cycles = 5000; // Limit cycles for quick test
    context.timeout_ms = 3000;  // 3 second timeout
    context.verbose_output = false;

    // Run all tests
    std::vector<TestResult> results = framework.run_all_tests(available_roms, context);

    // Validate results
    EXPECT_EQ(results.size(), available_roms.size());

    for (size_t i = 0; i < results.size(); i++) {
        const auto& result = results[i];
        EXPECT_NE(result.status, TestStatus::NOT_STARTED);
        EXPECT_GT(result.cycles_executed, 0);

        std::cout << "ROM " << (i + 1) << " (" << available_roms[i] << "): "
                  << TestUtils::test_status_to_string(result.status)
                  << " (" << result.cycles_executed << " cycles)\n";
    }

    // Print summary
    framework.print_results(results);
}
