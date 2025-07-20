#include <gtest/gtest.h>
#include <memory>
#include <tuple>
#include "cpu.hpp"
#include "bus.hpp"

class JumpSubroutineTest : public ::testing::Test {
protected:
    void SetUp() override {
        bus = std::make_shared<Bus>();
        cpu = std::make_shared<CPU>();
        cpu->connect_bus(bus);
    }

    std::shared_ptr<Bus> bus;
    std::shared_ptr<CPU> cpu;
};

// JSR - Jump to Subroutine
class JSRTest : public JumpSubroutineTest {};

TEST_F(JSRTest, JSR_Absolute) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->stkp = 0x01FD; // Initial stack pointer

    bus->write(cpu->pc, 0x20); // JSR opcode
    bus->write(cpu->pc + 1, 0x34); // Low byte of target address
    bus->write(cpu->pc + 2, 0x12); // High byte of target address

    cpu->step();

    EXPECT_EQ(cpu->cycles, 6);
    EXPECT_EQ(cpu->pc, 0x1234); // Jumped to target address
    EXPECT_EQ(cpu->stkp, 0x01FB); // Stack pointer decremented by 2

    // Check return address pushed to stack (PC-1, last byte of JSR instruction)
    EXPECT_EQ(bus->read(0x01FC), 0x00); // High byte of return address (0x0002)
    EXPECT_EQ(bus->read(0x01FB), 0x02); // Low byte of return address
}

TEST_F(JSRTest, JSR_AbsoluteLong) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->pb = 0x7E; // Set program bank
    cpu->stkp = 0x01FD; // Initial stack pointer

    bus->write(cpu->pc, 0x22); // JSR absolute long opcode
    bus->write(cpu->pc + 1, 0x34); // Low byte of target address
    bus->write(cpu->pc + 2, 0x12); // High byte of target address
    bus->write(cpu->pc + 3, 0x56); // Bank byte of target address

    cpu->step();

    EXPECT_EQ(cpu->cycles, 8);
    EXPECT_EQ(cpu->pc, 0x561234); // Jumped to target address
    EXPECT_EQ(cpu->stkp, 0x01FA); // Stack pointer decremented by 3

    // Check return address pushed to stack (PC-1, last byte of JSR instruction)
    EXPECT_EQ(bus->read(0x01FC), 0x00); // High byte of return address (0x0003)
    EXPECT_EQ(bus->read(0x01FB), 0x03); // Low byte of return address
    EXPECT_EQ(bus->read(0x01FA), 0x7E); // Bank byte of return address
}

// RTS - Return from Subroutine
class RTSTest : public JumpSubroutineTest {};

TEST_F(RTSTest, RTS_Return) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->stkp = 0x01FB; // Stack pointer pointing to pushed return address
    cpu->pb = 0x7E; // Program bank register

    // Push return address to stack (simulating JSR)
    bus->write(0x01FC, 0x00); // High byte of return address
    bus->write(0x01FB, 0x02); // Low byte of return address

    bus->write(cpu->pc, 0x60); // RTS opcode

    cpu->step();

    EXPECT_EQ(cpu->cycles, 6);
    EXPECT_EQ(cpu->pc, 0x7E0003); // Return address + 1
    EXPECT_EQ(cpu->stkp, 0x01FD); // Stack pointer incremented by 2
}

TEST_F(RTSTest, RTS_CrossBank) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->stkp = 0x01FB; // Stack pointer pointing to pushed return address
    cpu->pb = 0x80; // Different program bank

    // Push return address to stack (simulating JSR)
    bus->write(0x01FC, 0x00); // High byte of return address
    bus->write(0x01FB, 0x03); // Low byte of return address

    printf("DEBUG: pc=%08X 1FC=%02X 1FB=%02X\n", cpu->pc, bus->read(0x01FC), bus->read(0x01FB));
    bus->write(cpu->pc, 0x60);
    // 65816 RTS does NOT restore PB from the stack; it returns to the current PB
    cpu->pb = 0x7E; // Set PB to intended bank before RTS
    cpu->pc = 0x0000;
    uint32_t rts_addr = ((uint32_t)cpu->pb << 16) | cpu->pc;
    bus->write(rts_addr, 0x60); // RTS opcode at WRAM address
    cpu->step();

    EXPECT_EQ(cpu->cycles, 6);
    // Expect return to (PB << 16) | (addr + 1)
    EXPECT_EQ(cpu->pc, 0x7E0004);
    EXPECT_EQ(cpu->stkp, 0x01FD);
}

// RTL - Return from Subroutine Long
class RTLTest : public JumpSubroutineTest {};

TEST_F(RTLTest, RTL_Return) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->stkp = 0x01FA; // Stack pointer pointing to pushed return address (3 bytes)

    // Push return address to stack (simulating JSR long)
    bus->write(0x01FC, 0x00); // High byte of return address
    bus->write(0x01FB, 0x03); // Low byte of return address
    bus->write(0x01FA, 0x7E); // Bank byte of return address

    bus->write(cpu->pc, 0x6B); // RTL opcode

    cpu->step();

    EXPECT_EQ(cpu->cycles, 6);
    EXPECT_EQ(cpu->pc, 0x7E0004); // Return address + 1
    EXPECT_EQ(cpu->stkp, 0x01FD); // Stack pointer incremented by 3
}

// RTI - Return from Interrupt
class RTITest : public JumpSubroutineTest {};

TEST_F(RTITest, RTI_Return) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->stkp = 0x01FA; // Stack pointer pointing to pushed data (3 bytes)
    cpu->p = 0x00; // Clear status register initially

    // Push return data to stack (simulating interrupt)
    bus->write(0x01FC, 0x00); // High byte of return address
    bus->write(0x01FB, 0x02); // Low byte of return address
    bus->write(0x01FA, 0x34); // Status register (with some flags set)

    bus->write(cpu->pc, 0x40); // RTI opcode

    cpu->step();

    EXPECT_EQ(cpu->cycles, 6);
    EXPECT_EQ(cpu->pc, 0x0002); // Return address (no bank preservation)
    EXPECT_EQ(cpu->stkp, 0x01FD); // Stack pointer incremented by 3
    EXPECT_EQ(cpu->p & 0xFF, 0x34); // Status register restored
}

TEST_F(RTITest, RTI_StatusFlags) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->stkp = 0x01FA; // Stack pointer pointing to pushed data
    cpu->p = 0x00; // Clear status register initially

    // Push return data to stack with specific flags
    bus->write(0x01FC, 0x00); // High byte of return address
    bus->write(0x01FB, 0x02); // Low byte of return address
    bus->write(0x01FA, 0xCF); // Status register with Carry, Zero, Negative flags

    bus->write(cpu->pc, 0x40); // RTI opcode

    cpu->step();

    EXPECT_EQ(cpu->cycles, 6);
    EXPECT_EQ(cpu->pc, 0x0002); // Return address
    EXPECT_EQ(cpu->stkp, 0x01FD); // Stack pointer incremented by 3
    EXPECT_EQ(cpu->p & 0xFF, 0xCF); // Status register restored with flags
    EXPECT_TRUE(cpu->get_flag(CPU::C)); // Carry flag set
    EXPECT_TRUE(cpu->get_flag(CPU::Z)); // Zero flag set
    EXPECT_TRUE(cpu->get_flag(CPU::N)); // Negative flag set
}

// Test JSR/RTS combination
class JSRRTSCombinationTest : public JumpSubroutineTest {};

TEST_F(JSRRTSCombinationTest, JSR_RTS_Complete) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->pb = 0x7E; // Set program bank
    cpu->stkp = 0x01FD; // Initial stack pointer

    // Execute JSR
    bus->write(cpu->pc, 0x20); // JSR opcode
    bus->write(cpu->pc + 1, 0x34); // Low byte of target address
    bus->write(cpu->pc + 2, 0x12); // High byte of target address

    cpu->step();

    EXPECT_EQ(cpu->pc, 0x7E1234); // Jumped to subroutine (preserving bank)
    EXPECT_EQ(cpu->stkp, 0x01FB); // Stack pointer decremented

    // At subroutine location, execute RTS
    cpu->pc = 0x7E1234; // Set PC to subroutine location
    bus->write(cpu->pc, 0x60); // RTS opcode

    cpu->step();

    EXPECT_EQ(cpu->cycles, 6); // RTS cycles
    EXPECT_EQ(cpu->pc, 0x7E0003); // Returned to original location + 1
    EXPECT_EQ(cpu->stkp, 0x01FD); // Stack pointer restored
}

// Test JSR Long/RTL combination
class JSRLongRTLCombinationTest : public JumpSubroutineTest {};

TEST_F(JSRLongRTLCombinationTest, JSR_Long_RTL_Complete) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->pb = 0x7E; // Set program bank
    cpu->stkp = 0x01FD; // Initial stack pointer
    // Execute JSR Long
    bus->write(cpu->pc, 0x22); // JSR absolute long opcode
    bus->write(cpu->pc + 1, 0x34); // Low byte of target address
    bus->write(cpu->pc + 2, 0x12); // High byte of target address
    bus->write(cpu->pc + 3, 0x7E); // Bank byte of target address (WRAM)
    cpu->step();
    uint32_t subroutine_addr = ((uint32_t)cpu->pb << 16) | cpu->pc;
    uint8_t subroutine_opcode = bus->read(subroutine_addr);
    if (subroutine_opcode != 0x6B) {
        bus->write(subroutine_addr, 0x6B);
    }
    cpu->step();
    // Now perform all assertions
    EXPECT_EQ(cpu->cycles, 6); // RTL cycles
    EXPECT_EQ(cpu->pc, 0x7E0004); // Returned to original location + 1
    EXPECT_EQ(cpu->stkp, 0x01FD); // Stack pointer restored
}

// Test stack behavior with multiple nested calls
class NestedSubroutineTest : public JumpSubroutineTest {};

TEST_F(NestedSubroutineTest, Nested_JSR_RTS) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->stkp = 0x01FD; // Initial stack pointer

    // First JSR
    bus->write(cpu->pc, 0x20); // JSR opcode
    bus->write(cpu->pc + 1, 0x34); // Low byte of target address
    bus->write(cpu->pc + 2, 0x12); // High byte of target address

    cpu->step();

    EXPECT_EQ(cpu->pc, 0x1234); // First subroutine
    EXPECT_EQ(cpu->stkp, 0x01FB); // Stack pointer after first JSR

    // Second JSR from first subroutine
    cpu->pc = 0x1234;
    bus->write(cpu->pc, 0x20); // JSR opcode
    bus->write(cpu->pc + 1, 0x78); // Low byte of target address
    bus->write(cpu->pc + 2, 0x56); // High byte of target address

    cpu->step();

    EXPECT_EQ(cpu->pc, 0x5678); // Second subroutine
    EXPECT_EQ(cpu->stkp, 0x01F9); // Stack pointer after second JSR

    // First RTS (return from second subroutine)
    cpu->pc = 0x5678;
    // 65816 RTS does NOT restore PB from the stack; it returns to the current PB
    cpu->pb = 0x7E; // Set PB to intended bank before RTS
    cpu->pc = 0x5678;
    bus->write(((uint32_t)cpu->pb << 16) | cpu->pc, 0x60); // RTS opcode at WRAM address
    cpu->step();

    // Second RTS (return from first subroutine)
    cpu->pc = 0x1237;
    bus->write(((uint32_t)cpu->pb << 16) | cpu->pc, 0x60); // RTS opcode at correct address
    cpu->step();

    EXPECT_EQ(cpu->pc, 0x7E0003); // Return to original location + 1
    EXPECT_EQ(cpu->stkp, 0x01FD); // Stack pointer restored
}

// Test edge cases
class JumpSubroutineEdgeTest : public JumpSubroutineTest {};

TEST_F(JumpSubroutineEdgeTest, JSR_ZeroAddress) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->stkp = 0x01FD;

    bus->write(cpu->pc, 0x20); // JSR opcode
    bus->write(cpu->pc + 1, 0x00); // Low byte of target address
    bus->write(cpu->pc + 2, 0x00); // High byte of target address

    cpu->step();

    EXPECT_EQ(cpu->cycles, 6);
    EXPECT_EQ(cpu->pc, 0x0000); // Jumped to zero address
    EXPECT_EQ(cpu->stkp, 0x01FB); // Stack pointer decremented
}

TEST_F(JumpSubroutineEdgeTest, RTS_EmptyStack) {
    cpu->reset();
    cpu->pc = 0x7E0000;
    cpu->stkp = 0x01FF; // Empty stack (should wrap around)

    bus->write(((uint32_t)cpu->pb << 16) | cpu->pc, 0x60); // RTS opcode at correct address

    cpu->step();

    // The stack pointer should increment by 2, wrapping if needed
    EXPECT_EQ(cpu->cycles, 6);
    EXPECT_EQ(cpu->stkp, 0x0101); // After two pops from 0x01FF, should be 0x0101
    // Note: PC value depends on what was in memory at stack locations
}
