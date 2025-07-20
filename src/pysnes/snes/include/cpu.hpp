#pragma once
#include <cstdint>
#include <memory>

// Forward declarations
class Bus;

// 65816 CPU core for SNES
class CPU {
public:
    // Constructor/Destructor
    CPU();
    ~CPU();

    // Core CPU registers (65816 specific)
    uint16_t a = 0x0000;        // Accumulator (16-bit in native mode)
    uint16_t x = 0x0000;        // X index register (16-bit in native mode)
    uint16_t y = 0x0000;        // Y index register (16-bit in native mode)
    uint16_t stkp = 0x01FD;     // Stack pointer (16-bit in native mode)
    uint32_t pc = 0x8000;       // Program counter (24-bit)
    uint16_t p = 0x34;          // Processor status register
    uint16_t d = 0x0000;        // Direct Page register
    uint8_t pb = 0x00;          // Program Bank register
    uint8_t db = 0x00;          // Data Bank register

    // Bus connection
    void connect_bus(std::shared_ptr<Bus> b);
    std::shared_ptr<Bus> bus;   // Made public for instruction access

    // Core execution
    void step();                // Execute one instruction
    void reset();               // Reset CPU state
    void irq();                 // Interrupt request
    void nmi();                 // Non-maskable interrupt

    // Flag management
    enum FLAGS {
        C = (1 << 0),           // Carry
        Z = (1 << 1),           // Zero
        I = (1 << 2),           // IRQ Disable
        D = (1 << 3),           // Decimal
        X = (1 << 4),           // Index register size (0=16, 1=8)
        M = (1 << 5),           // Accumulator size (0=16, 1=8)
        V = (1 << 6),           // Overflow
        N = (1 << 7),           // Negative
        E = (1 << 8)            // Emulation flag
    };
    
    void set_flag(FLAGS f, bool v);
    bool get_flag(FLAGS f) const;

    // State information
    uint8_t cycles = 0;         // Cycles for current instruction
    uint8_t opcode = 0;         // Current opcode (for debugging)

    // Debug/testing helpers
    uint8_t get_opcode() const { return opcode; }

    // Helper functions (made public for instruction access)
    void setZN(uint16_t value, bool is16);
    void validate_stack_pointer();

private:
    // Internal state
    uint32_t addr_abs = 0;      // Absolute address
    uint32_t addr_rel = 0;      // Relative address
    uint16_t fetched = 0;       // Fetched data
};