#include "test_framework.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

// TestROMRunner implementation
bool TestROMRunner::setup(const TestContext& ctx) {
    context = ctx;
    result = TestResult();

    try {
        // Create components
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();

        // Connect components
        cpu->connect_bus(bus);
        bus->connect_cpu(cpu);

        // Load ROM
        if (!std::filesystem::exists(context.rom_path)) {
            result.status = TestStatus::FAILED;
            result.error_message = "ROM file not found: " + context.rom_path;
            return false;
        }

        cart = std::make_shared<Cartridge>(context.rom_path);
        bus->connect_cartridge(cart);

        if (!cart->is_loaded()) {
            result.status = TestStatus::FAILED;
            result.error_message = "Failed to load ROM: " + context.rom_path;
            return false;
        }

        // Reset CPU
        cpu->reset();

        // Initialize monitoring
        cycle_count = 0;
        memory_accesses.clear();
        memory_snapshots.clear();

        log_message("Test setup completed successfully");
        return true;

    } catch (const std::exception& e) {
        result.status = TestStatus::CRASHED;
        result.error_message = "Setup failed: " + std::string(e.what());
        return false;
    }
}

void TestROMRunner::teardown() {
    if (context.verbose_output) {
        log_message("Test teardown completed");
    }
}

TestResult TestROMRunner::run_test() {
    start_time = std::chrono::steady_clock::now();

    try {
        while (cycle_count < context.max_cycles) {
            // Check for timeout
            if (check_timeout()) {
                result.status = TestStatus::TIMEOUT;
                result.error_message = "Test timed out after " +
                                     std::to_string(context.timeout_ms) + "ms";
                break;
            }

            // Check for test completion
            if (detect_test_completion()) {
                result.status = TestStatus::PASSED;
                break;
            }

            // Check for test failure
            if (detect_test_failure()) {
                result.status = TestStatus::FAILED;
                result.error_message = "Test failure detected";
                break;
            }

            // Execute one cycle
            uint32_t old_pc = cpu->pc;
            cpu->step();
            cycle_count++;

            // Log execution state if verbose
            if (context.verbose_output && (cycle_count % 1000 == 0)) {
                log_execution_state();
            }

            // Check for infinite loops (PC not changing)
            if (cpu->pc == old_pc && cycle_count > 1000) {
                result.status = TestStatus::FAILED;
                result.error_message = "CPU stuck at PC 0x" +
                                     std::to_string(cpu->pc);
                break;
            }
        }

        // If we reached max cycles without completion
        if (cycle_count >= context.max_cycles && result.status == TestStatus::NOT_STARTED) {
            result.status = TestStatus::TIMEOUT;
            result.error_message = "Test exceeded maximum cycles: " +
                                 std::to_string(context.max_cycles);
        }

    } catch (const std::exception& e) {
        result.status = TestStatus::CRASHED;
        result.error_message = "Test crashed: " + std::string(e.what());
    }

    // Calculate execution time
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    result.execution_time_ms = duration.count();
    result.cycles_executed = cycle_count;

    return result;
}

void TestROMRunner::log_message(const std::string& message) {
    std::string timestamped_message = "[" + std::to_string(cycle_count) + " cycles] " + message;
    result.log_messages.push_back(timestamped_message);

    if (context.log_callback) {
        context.log_callback(timestamped_message);
    }

    if (context.verbose_output) {
        std::cout << timestamped_message << std::endl;
    }
}

void TestROMRunner::take_memory_snapshot() {
    // Take a snapshot of key memory regions
    memory_snapshots.clear();

    // Snapshot stack area (0x0100-0x01FF)
    for (uint32_t addr = 0x0100; addr <= 0x01FF; addr++) {
        memory_snapshots.push_back(bus->read(addr));
    }

    // Snapshot zero page (0x0000-0x00FF)
    for (uint32_t addr = 0x0000; addr <= 0x00FF; addr++) {
        memory_snapshots.push_back(bus->read(addr));
    }
}

bool TestROMRunner::check_timeout() {
    auto current_time = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
    return elapsed.count() > context.timeout_ms;
}

// CPU65816TestRunner implementation
bool CPU65816TestRunner::detect_test_completion() {
    // Check for common test completion patterns in CPU test ROMs

    // Pattern 1: Check for "SUCCESS" string in memory
    std::vector<uint8_t> success_pattern = {'S', 'U', 'C', 'C', 'E', 'S', 'S'};
    for (uint32_t addr = 0x0000; addr <= 0xFFFF; addr++) {
        bool found = true;
        for (size_t i = 0; i < success_pattern.size(); i++) {
            if (bus->read((addr + i) & 0xFFFF) != success_pattern[i]) {
                found = false;
                break;
            }
        }
        if (found) {
            log_message("Test completion detected: SUCCESS string found at 0x" +
                       std::to_string(addr));
            return true;
        }
    }

    // Pattern 2: Check for specific memory addresses that indicate completion
    // Many test ROMs write to specific addresses to indicate test status
    uint8_t test_status = bus->read(0x2000); // Common test status address
    if (test_status == 0x01) {
        log_message("Test completion detected: Status byte at 0x2000 = 0x01");
        return true;
    }

    // Pattern 3: Check for infinite BRK loop (common in test ROMs)
    if (consecutive_brk_count > 100) {
        log_message("Test completion detected: BRK loop (likely test finished)");
        return true;
    }

    return false;
}

bool CPU65816TestRunner::detect_test_failure() {
    // Check for common test failure patterns

    // Pattern 1: Check for "FAILED" string in memory
    std::vector<uint8_t> failed_pattern = {'F', 'A', 'I', 'L', 'E', 'D'};
    for (uint32_t addr = 0x0000; addr <= 0xFFFF; addr++) {
        bool found = true;
        for (size_t i = 0; i < failed_pattern.size(); i++) {
            if (bus->read((addr + i) & 0xFFFF) != failed_pattern[i]) {
                found = false;
                break;
            }
        }
        if (found) {
            log_message("Test failure detected: FAILED string found at 0x" +
                       std::to_string(addr));
            return true;
        }
    }

    // Pattern 2: Check for error status byte
    uint8_t error_status = bus->read(0x2001); // Common error status address
    if (error_status == 0xFF) {
        log_message("Test failure detected: Error status byte at 0x2001 = 0xFF");
        return true;
    }

    // Pattern 3: Check for register corruption
    if (!check_register_consistency()) {
        log_message("Test failure detected: Register consistency check failed");
        return true;
    }

    return false;
}

void CPU65816TestRunner::log_execution_state() {
    std::stringstream ss;
    ss << "PC: 0x" << std::hex << std::setw(6) << std::setfill('0') << cpu->pc
       << " A: 0x" << std::setw(4) << cpu->a
       << " X: 0x" << std::setw(4) << cpu->x
       << " Y: 0x" << std::setw(4) << cpu->y
       << " SP: 0x" << std::setw(4) << cpu->stkp
       << " P: 0x" << std::setw(2) << (cpu->p & 0xFF);

    log_message(ss.str());
}

bool CPU65816TestRunner::check_register_consistency() {
    // Check if registers have reasonable values
    if (cpu->stkp > 0x01FF || cpu->stkp < 0x0100) {
        return false; // Stack pointer out of valid range
    }

    // Check if program counter is in valid ROM range
    if (cpu->pc < 0x8000 || cpu->pc > 0xFFFF) {
        return false; // PC out of valid ROM range
    }

    return true;
}

bool CPU65816TestRunner::check_memory_access_patterns() {
    // Analyze memory access patterns for anomalies
    // This is a placeholder for more sophisticated memory analysis
    return true;
}

void CPU65816TestRunner::analyze_instruction_flow() {
    // Analyze the instruction flow for patterns
    pc_history.push_back(cpu->pc);

    // Keep only last 1000 PC values
    if (pc_history.size() > 1000) {
        pc_history.erase(pc_history.begin());
    }

    // Check for BRK instructions
    if (cpu->get_opcode() == 0x00) {
        consecutive_brk_count++;
    } else {
        consecutive_brk_count = 0;
    }
}

// SPC700TestRunner implementation (placeholder)
bool SPC700TestRunner::detect_test_completion() {
    // TODO: Implement SPC-700 specific test completion detection
    return false;
}

bool SPC700TestRunner::detect_test_failure() {
    // TODO: Implement SPC-700 specific test failure detection
    return false;
}

void SPC700TestRunner::log_execution_state() {
    // TODO: Implement SPC-700 specific state logging
    log_message("SPC-700 execution state logging not implemented");
}

// TestFramework implementation
TestROMType TestFramework::detect_rom_type(const std::string& rom_path) {
    // Simple detection based on filename patterns
    std::string filename = std::filesystem::path(rom_path).filename().string();
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

    if (filename.find("cputest") != std::string::npos ||
        filename.find("cpu") != std::string::npos) {
        return TestROMType::CPU_65816;
    }

    if (filename.find("spctest") != std::string::npos ||
        filename.find("spc") != std::string::npos) {
        return TestROMType::SPC_700;
    }

    if (filename.find("pputest") != std::string::npos ||
        filename.find("ppu") != std::string::npos) {
        return TestROMType::PPU;
    }

    if (filename.find("aputest") != std::string::npos ||
        filename.find("apu") != std::string::npos) {
        return TestROMType::APU;
    }

    // Default to CPU tests for unknown ROMs
    return TestROMType::CPU_65816;
}

std::unique_ptr<TestROMRunner> TestFramework::create_runner(TestROMType type) {
    switch (type) {
        case TestROMType::CPU_65816:
            return std::make_unique<CPU65816TestRunner>();
        case TestROMType::SPC_700:
            return std::make_unique<SPC700TestRunner>();
        default:
            return std::make_unique<CPU65816TestRunner>(); // Default fallback
    }
}

std::vector<TestResult> TestFramework::run_all_tests(const std::vector<std::string>& rom_paths,
                                                    const TestContext& base_context) {
    std::vector<TestResult> all_results;

    for (const auto& rom_path : rom_paths) {
        TestContext context = base_context;
        context.rom_path = rom_path;
        context.type = detect_rom_type(rom_path);

        TestResult result = run_single_test(rom_path, context);
        all_results.push_back(result);
    }

    return all_results;
}

TestResult TestFramework::run_single_test(const std::string& rom_path, const TestContext& context) {
    auto runner = create_runner(context.type);

    if (!runner->setup(context)) {
        return runner->get_result();
    }

    TestResult result = runner->run_test();
    runner->teardown();

    return result;
}

void TestFramework::add_custom_runner(std::unique_ptr<TestROMRunner> runner) {
    runners.push_back(std::move(runner));
}

void TestFramework::clear_runners() {
    runners.clear();
}

void TestFramework::print_results(const std::vector<TestResult>& results) {
    std::cout << "\n=== Test Results ===\n";

    for (size_t i = 0; i < results.size(); i++) {
        const auto& result = results[i];
        std::cout << "Test " << (i + 1) << ": "
                  << TestUtils::test_status_to_string(result.status)
                  << " (" << result.cycles_executed << " cycles, "
                  << result.execution_time_ms << "ms)\n";

        if (!result.error_message.empty()) {
            std::cout << "  Error: " << result.error_message << "\n";
        }
    }

    // Summary
    int passed = 0, failed = 0, timeout = 0, crashed = 0;
    for (const auto& result : results) {
        switch (result.status) {
            case TestStatus::PASSED: passed++; break;
            case TestStatus::FAILED: failed++; break;
            case TestStatus::TIMEOUT: timeout++; break;
            case TestStatus::CRASHED: crashed++; break;
            default: break;
        }
    }

    std::cout << "\nSummary: " << passed << " passed, " << failed << " failed, "
              << timeout << " timeout, " << crashed << " crashed\n";
}

// TestUtils implementation
std::string TestUtils::test_status_to_string(TestStatus status) {
    switch (status) {
        case TestStatus::PASSED: return "PASSED";
        case TestStatus::FAILED: return "FAILED";
        case TestStatus::TIMEOUT: return "TIMEOUT";
        case TestStatus::CRASHED: return "CRASHED";
        case TestStatus::NOT_STARTED: return "NOT_STARTED";
        default: return "UNKNOWN";
    }
}

std::string TestUtils::test_rom_type_to_string(TestROMType type) {
    switch (type) {
        case TestROMType::CPU_65816: return "CPU_65816";
        case TestROMType::SPC_700: return "SPC_700";
        case TestROMType::PPU: return "PPU";
        case TestROMType::APU: return "APU";
        case TestROMType::UNKNOWN: return "UNKNOWN";
        default: return "UNKNOWN";
    }
}

TestROMType TestUtils::string_to_test_rom_type(const std::string& type_str) {
    std::string upper = type_str;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    if (upper == "CPU_65816" || upper == "CPU") return TestROMType::CPU_65816;
    if (upper == "SPC_700" || upper == "SPC") return TestROMType::SPC_700;
    if (upper == "PPU") return TestROMType::PPU;
    if (upper == "APU") return TestROMType::APU;

    return TestROMType::UNKNOWN;
}

bool TestUtils::analyze_rom_header(const std::string& rom_path, std::ostream& out) {
    std::ifstream file(rom_path, std::ios::binary);
    if (!file.is_open()) {
        out << "Error: Could not open ROM file: " << rom_path << "\n";
        return false;
    }

    // Read SNES ROM header (first 64 bytes)
    std::vector<uint8_t> header(64);
    file.read(reinterpret_cast<char*>(header.data()), 64);

    if (file.gcount() < 64) {
        out << "Error: ROM file too small for SNES header\n";
        return false;
    }

    // Extract header information
    std::string game_title;
    for (int i = 0; i < 21; i++) {
        char c = header[0x10 + i];
        if (c >= 32 && c <= 126) { // Printable ASCII
            game_title += c;
        }
    }

    // Extract reset vector
    uint16_t reset_vector = (header[0x3C] << 8) | header[0x3C + 1];

    out << "ROM Analysis:\n";
    out << "  Game Title: " << game_title << "\n";
    out << "  Reset Vector: 0x" << std::hex << reset_vector << "\n";
    out << "  ROM Size: " << std::dec << std::filesystem::file_size(rom_path) << " bytes\n";

    return true;
}

std::vector<std::string> TestUtils::find_strings_in_rom(const std::string& rom_path) {
    std::vector<std::string> strings;
    std::ifstream file(rom_path, std::ios::binary);

    if (!file.is_open()) {
        return strings;
    }

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());

    std::string current_string;
    for (size_t i = 0; i < data.size(); i++) {
        char c = data[i];
        if (c >= 32 && c <= 126) { // Printable ASCII
            current_string += c;
        } else if (c == 0) { // Null terminator - end current string
            if (current_string.length() >= 4) { // Minimum string length
                strings.push_back(current_string);
            }
            current_string.clear();
        } else { // Other non-printable characters
            if (current_string.length() >= 4) { // Minimum string length
                strings.push_back(current_string);
            }
            current_string.clear();
        }
    }

    // Don't forget the last string if it's long enough
    if (current_string.length() >= 4) {
        strings.push_back(current_string);
    }

    return strings;
}

std::vector<uint8_t> TestUtils::extract_rom_data(const std::string& rom_path,
                                                uint32_t offset, uint32_t length) {
    std::vector<uint8_t> data;
    std::ifstream file(rom_path, std::ios::binary);

    if (!file.is_open()) {
        return data;
    }

    file.seekg(offset);
    data.resize(length);
    file.read(reinterpret_cast<char*>(data.data()), length);

    return data;
}
