#include <cstdint>
#include <string>
#include <fstream>
#include <vector>
#include "gtest/gtest.h"
#include "cpu.hpp"
#include "bus.hpp"
#include "cartridge.hpp"
#include <filesystem> // Added for std::filesystem::exists
#include <cstdlib> // For getenv

// Utility to robustly find a ROM file
std::string find_rom_path(const std::string& rom_name) {
    // 1. Check environment variable
    if (const char* env_p = std::getenv("PYSNES_ROM_DIR")) {
        std::filesystem::path env_path = std::filesystem::path(env_p) / rom_name;
        if (std::filesystem::exists(env_path)) return env_path.string();
    }
    // 2. Check common locations
    std::vector<std::filesystem::path> candidates = {
        std::filesystem::path("tests/roms") / rom_name,
        std::filesystem::path("roms") / rom_name,
        std::filesystem::path("../tests/roms") / rom_name,
        std::filesystem::path("../roms") / rom_name,
        std::filesystem::path(rom_name),
        std::filesystem::path("..") / rom_name
    };
    for (const auto& path : candidates) {
        if (std::filesystem::exists(path)) return path.string();
    }
    // 3. Not found
    return rom_name; // Fallback, will fail later if not found
}

class ROMTest : public ::testing::Test {
protected:
    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
    std::shared_ptr<Cartridge> cart;
    
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        
        cpu->connect_bus(bus);
        bus->connect_cpu(cpu);
    }
    
    // Helper to load ROM file
    bool load_rom(const std::string& filename) {
        printf("DEBUG: Attempting to load ROM: %s\n", filename.c_str());
        if (!std::filesystem::exists(filename)) {
            printf("DEBUG: ROM file does not exist: %s\n", filename.c_str());
            return false;
        }
        
        printf("DEBUG: ROM file exists, creating cartridge...\n");
        // Create cartridge with ROM path
        cart = std::make_shared<Cartridge>(filename);
        bus->connect_cartridge(cart);
        
        bool loaded = cart->is_loaded();
        printf("DEBUG: Cartridge loaded: %s\n", loaded ? "true" : "false");
        return loaded;
    }
    
    // Helper to run CPU for specified cycles
    void run_cpu_cycles(uint32_t cycles) {
        for (uint32_t i = 0; i < cycles; i++) {
            cpu->step();
        }
    }
    
    // Helper to check memory at specific address
    uint8_t read_memory(uint32_t addr) {
        return bus->read(addr);
    }
    
    // Helper to write memory at specific address
    void write_memory(uint32_t addr, uint8_t data) {
        bus->write(addr, data);
    }
};

// Test basic ROM loading
TEST_F(ROMTest, LoadBasicCPUTest) {
    EXPECT_TRUE(load_rom(find_rom_path("cputest-basic.sfc")));
    
    // Reset CPU to known state
    cpu->reset();
    
    // Verify initial state
    EXPECT_EQ(cpu->pc, 0x8000);
    EXPECT_EQ(cpu->p, 0x34);
    EXPECT_EQ(cpu->stkp, 0x01FD);
}

// Test full ROM loading
TEST_F(ROMTest, LoadFullCPUTest) {
    EXPECT_TRUE(load_rom(find_rom_path("cputest-full.sfc")));
    
    // Reset CPU to known state
    cpu->reset();
    
    // Verify initial state
    EXPECT_EQ(cpu->pc, 0x8000);
    EXPECT_EQ(cpu->p, 0x34);
    EXPECT_EQ(cpu->stkp, 0x01FD);
}

// Test basic CPU execution
TEST_F(ROMTest, BasicExecution) {
    EXPECT_TRUE(load_rom(find_rom_path("cputest-basic.sfc")));
    cpu->reset();
    
    // Run a few instructions
    for (int i = 0; i < 10; i++) {
        uint32_t old_pc = cpu->pc;
        uint8_t opcode = read_memory(cpu->pc);
        cpu->step();
        
        // PC should change (either advance or jump)
        EXPECT_NE(cpu->pc, old_pc) << "PC didn't change at cycle " << i << " (opcode: 0x" << std::hex << (int)opcode << ")";
        
        // Log the instruction execution
        printf("Cycle %d: PC 0x%06X -> 0x%06X (opcode: 0x%02X)\n", i, old_pc, cpu->pc, opcode);
    }
}

// Test specific test patterns from ROM
TEST_F(ROMTest, TestPatterns) {
    EXPECT_TRUE(load_rom(find_rom_path("cputest-basic.sfc")));
    cpu->reset();
    
    // Look for test result patterns in memory
    // This will depend on how the test ROM reports results
    
    // Run for a reasonable number of cycles
    run_cpu_cycles(1000);
    
    // Check if test results are written to specific memory locations
    // (You'll need to analyze the test ROM to know where results are stored)
}

// Test error detection
TEST_F(ROMTest, ErrorDetection) {
    EXPECT_TRUE(load_rom(find_rom_path("cputest-basic.sfc")));
    cpu->reset();
    
    // Run until we detect an error condition or completion
    bool error_detected = false;
    bool test_completed = false;
    
    for (int i = 0; i < 10000 && !error_detected && !test_completed; i++) {
        cpu->step();
        
        // Check for error indicators in memory
        // This depends on the specific test ROM implementation
        uint8_t status = read_memory(0x7E0000); // Example status location
        if (status == 0xFF) { // Example error code
            error_detected = true;
        } else if (status == 0x00) { // Example success code
            test_completed = true;
        }
    }
    
    // Log the final state
    printf("Final PC: 0x%06X, Status: 0x%02X\n", cpu->pc, read_memory(0x7E0000));
}

// Test specific instruction sequences
TEST_F(ROMTest, InstructionSequences) {
    EXPECT_TRUE(load_rom(find_rom_path("cputest-basic.sfc")));
    cpu->reset();
    
    // Run and monitor specific instruction patterns
    std::vector<uint8_t> executed_opcodes;
    
    for (int i = 0; i < 100; i++) {
        uint8_t opcode = read_memory(cpu->pc);
        executed_opcodes.push_back(opcode);
        cpu->step();
    }
    
    // Analyze the executed opcodes
    printf("Executed opcodes: ");
    for (size_t i = 0; i < std::min(executed_opcodes.size(), size_t(10)); i++) {
        printf("0x%02X ", executed_opcodes[i]);
    }
    printf("\n");
}

// Test memory access patterns
TEST_F(ROMTest, MemoryAccessPatterns) {
    EXPECT_TRUE(load_rom(find_rom_path("cputest-basic.sfc")));
    cpu->reset();
    
    // Monitor memory access patterns
    std::vector<uint32_t> accessed_addresses;
    
    // This would require modifying the Bus class to track accesses
    // For now, just run and observe
    
    run_cpu_cycles(500);
    
    // Check if certain memory regions are accessed
    uint8_t wram_access = read_memory(0x7E0000);
    uint8_t rom_access = read_memory(0x8000);
    
    printf("WRAM access: 0x%02X, ROM access: 0x%02X\n", wram_access, rom_access);
}

// Test CPU state consistency
TEST_F(ROMTest, CPUStateConsistency) {
    EXPECT_TRUE(load_rom(find_rom_path("cputest-basic.sfc")));
    cpu->reset();
    
    // Run and periodically check CPU state consistency
    for (int i = 0; i < 1000; i++) {
        uint16_t old_a = cpu->a;
        uint16_t old_x = cpu->x;
        uint16_t old_y = cpu->y;
        uint16_t old_stkp = cpu->stkp;
        uint16_t old_p = cpu->p;
        
        cpu->step();
        
        // Basic sanity checks
        EXPECT_LE(cpu->stkp, 0x01FF); // Stack pointer should be in valid range
        EXPECT_GE(cpu->stkp, 0x0100); // Stack pointer should be in valid range
        
        // Check if registers changed unexpectedly
        if (i % 100 == 0) {
            printf("Cycle %d: A=0x%04X, X=0x%04X, Y=0x%04X, SP=0x%04X, P=0x%04X\n",
                   i, cpu->a, cpu->x, cpu->y, cpu->stkp, cpu->p);
        }
    }
}

// Test ROM-specific functionality
TEST_F(ROMTest, ROMSpecificTests) {
    EXPECT_TRUE(load_rom(find_rom_path("cputest-full.sfc")));
    cpu->reset();
    
    // Run the full test suite
    // This will take longer and test more comprehensive functionality
    
    for (int i = 0; i < 50000; i++) {
        cpu->step();
        
        // Check for test completion or failure indicators
        uint8_t test_status = read_memory(0x7E0000);
        if (test_status == 0x00) { // Success
            printf("Test completed successfully at cycle %d\n", i);
            break;
        } else if (test_status == 0xFF) { // Failure
            printf("Test failed at cycle %d\n", i);
            break;
        }
        
        if (i % 10000 == 0) {
            printf("Progress: %d cycles, PC=0x%06X\n", i, cpu->pc);
        }
    }
} 