#!/usr/bin/env python3
"""
ROM Analyzer for cputest ROMs
This tool helps understand the structure and test patterns in the cputest ROMs
"""

import sys
import os
import struct
from typing import List, Dict, Any

class ROMAnalyzer:
    def __init__(self, rom_path: str):
        self.rom_path = rom_path
        self.rom_data = None
        self.load_rom()
    
    def load_rom(self):
        """Load ROM file into memory"""
        try:
            with open(self.rom_path, 'rb') as f:
                self.rom_data = f.read()
            print(f"Loaded ROM: {self.rom_path} ({len(self.rom_data)} bytes)")
        except FileNotFoundError:
            print(f"Error: ROM file not found: {self.rom_path}")
            sys.exit(1)
    
    def analyze_header(self):
        """Analyze SNES ROM header"""
        if len(self.rom_data) < 0x8000:
            print("Error: ROM too small for SNES header")
            return
        
        # SNES header is at 0x7FC0-0x7FFF
        header = self.rom_data[0x7FC0:0x8000]
        
        print("\n=== SNES ROM Header Analysis ===")
        print(f"Game Title: {header[0x00:0x15].decode('ascii', errors='ignore').strip()}")
        print(f"ROM Type: {header[0x15]:02X}")
        print(f"ROM Size: {header[0x16]:02X}")
        print(f"RAM Size: {header[0x17]:02X}")
        print(f"Country: {header[0x18]:02X}")
        print(f"License: {header[0x19]:02X}")
        print(f"Version: {header[0x1A]:02X}")
        print(f"Checksum: {struct.unpack('<H', header[0x1C:0x1E])[0]:04X}")
        print(f"Checksum Complement: {struct.unpack('<H', header[0x1E:0x20])[0]:04X}")
        
        # Reset vector
        reset_vector = struct.unpack('<H', header[0x3C:0x3E])[0]
        print(f"Reset Vector: 0x{reset_vector:04X}")
    
    def find_strings(self, min_length: int = 4):
        """Find ASCII strings in ROM"""
        print(f"\n=== ASCII Strings (min length {min_length}) ===")
        strings = []
        current_string = ""
        
        for i, byte in enumerate(self.rom_data):
            if 32 <= byte <= 126:  # Printable ASCII
                current_string += chr(byte)
            else:
                if len(current_string) >= min_length:
                    strings.append((i - len(current_string), current_string))
                current_string = ""
        
        # Don't forget the last string
        if len(current_string) >= min_length:
            strings.append((len(self.rom_data) - len(current_string), current_string))
        
        # Print unique strings
        unique_strings = set()
        for addr, string in strings:
            if string not in unique_strings:
                unique_strings.add(string)
                print(f"0x{addr:06X}: {repr(string)}")
    
    def analyze_code_patterns(self):
        """Analyze code patterns and common opcodes"""
        print("\n=== Code Pattern Analysis ===")
        
        # Look for common 65816 opcodes
        opcode_counts = {}
        for i in range(0, len(self.rom_data) - 1, 2):
            opcode = self.rom_data[i]
            opcode_counts[opcode] = opcode_counts.get(opcode, 0) + 1
        
        # Sort by frequency
        sorted_opcodes = sorted(opcode_counts.items(), key=lambda x: x[1], reverse=True)
        
        print("Most common opcodes:")
        for opcode, count in sorted_opcodes[:20]:
            opcode_name = self.get_opcode_name(opcode)
            print(f"  0x{opcode:02X} ({opcode_name}): {count} occurrences")
    
    def get_opcode_name(self, opcode: int) -> str:
        """Get opcode name for common 65816 instructions"""
        opcode_names = {
            0xA9: "LDA #imm",
            0xA5: "LDA dp",
            0xAD: "LDA abs",
            0x85: "STA dp",
            0x8D: "STA abs",
            0xAA: "TAX",
            0x8A: "TXA",
            0xA8: "TAY",
            0x98: "TYA",
            0x48: "PHA",
            0x68: "PLA",
            0x28: "PLP",
            0x08: "PHP",
            0x4C: "JMP abs",
            0x6C: "JMP (abs)",
            0x20: "JSR abs",
            0x60: "RTS",
            0x40: "RTI",
            0x00: "BRK",
            0xEA: "NOP",
            0x78: "SEI",
            0x58: "CLI",
            0xF8: "SED",
            0xD8: "CLD",
            0x38: "SEC",
            0x18: "CLC",
            0xB8: "CLV",
            0xBA: "TSX",
            0x9A: "TXS",
            0xE8: "INX",
            0xC8: "INY",
            0xCA: "DEX",
            0x88: "DEY",
        }
        return opcode_names.get(opcode, "Unknown")
    
    def find_test_vectors(self):
        """Look for potential test vectors or data patterns"""
        print("\n=== Test Vector Analysis ===")
        
        # Look for patterns that might indicate test data
        test_patterns = [
            b'\x00\x00\x00\x00',  # All zeros
            b'\xFF\xFF\xFF\xFF',  # All ones
            b'\xAA\xAA\xAA\xAA',  # Alternating bits
            b'\x55\x55\x55\x55',  # Alternating bits (inverted)
        ]
        
        for pattern in test_patterns:
            count = self.rom_data.count(pattern)
            if count > 0:
                print(f"Pattern {pattern.hex()}: {count} occurrences")
        
        # Look for sequential data
        sequential_count = 0
        for i in range(len(self.rom_data) - 4):
            start_byte = self.rom_data[i]
            # Check if we can create a valid 4-byte sequence starting from this byte
            if start_byte + 3 < 256:  # Ensure the range doesn't exceed 256
                expected_sequence = bytes(range(start_byte, start_byte + 4))
                if (self.rom_data[i:i+4] == expected_sequence):
                    sequential_count += 1
        
        print(f"Sequential 4-byte patterns: {sequential_count}")
    
    def analyze_memory_map(self):
        """Analyze memory access patterns"""
        print("\n=== Memory Map Analysis ===")
        
        # Look for code sections
        code_sections = []
        current_section_start = None
        current_section_type = None
        
        for i in range(0, len(self.rom_data), 0x1000):  # 4KB blocks
            block = self.rom_data[i:i+0x1000]
            if not block:
                break
            
            # Analyze block characteristics
            zero_count = block.count(0)
            ff_count = block.count(0xFF)
            printable_count = sum(1 for b in block if 32 <= b <= 126)
            
            if zero_count > len(block) * 0.9:
                section_type = "Zero-filled"
            elif ff_count > len(block) * 0.9:
                section_type = "FF-filled"
            elif printable_count > len(block) * 0.7:
                section_type = "Text/Data"
            else:
                section_type = "Code"
            
            if section_type != current_section_type:
                if current_section_start is not None:
                    code_sections.append((current_section_start, i, current_section_type))
                current_section_start = i
                current_section_type = section_type
        
        # Add the last section
        if current_section_start is not None:
            code_sections.append((current_section_start, len(self.rom_data), current_section_type))
        
        print("Memory sections:")
        for start, end, section_type in code_sections:
            print(f"  0x{start:06X}-0x{end:06X}: {section_type}")
    
    def find_entry_points(self):
        """Find potential entry points and jump tables"""
        print("\n=== Entry Point Analysis ===")
        
        # Look for common entry point patterns
        entry_points = []
        
        # Reset vector (should be in header)
        if len(self.rom_data) >= 0x8000:
            reset_vector = struct.unpack('<H', self.rom_data[0x7FFC:0x7FFE])[0]
            entry_points.append(("Reset Vector", reset_vector))
        
        # Look for JMP instructions
        for i in range(len(self.rom_data) - 3):
            if self.rom_data[i] == 0x4C:  # JMP abs
                target = struct.unpack('<H', self.rom_data[i+1:i+3])[0]
                entry_points.append((f"JMP at 0x{i:06X}", target))
        
        # Look for JSR instructions
        for i in range(len(self.rom_data) - 3):
            if self.rom_data[i] == 0x20:  # JSR abs
                target = struct.unpack('<H', self.rom_data[i+1:i+3])[0]
                entry_points.append((f"JSR at 0x{i:06X}", target))
        
        print("Potential entry points:")
        for desc, addr in entry_points[:20]:  # Limit to first 20
            print(f"  {desc}: 0x{addr:04X}")
    
    def run_full_analysis(self):
        """Run complete ROM analysis"""
        print(f"Analyzing ROM: {self.rom_path}")
        print("=" * 50)
        
        self.analyze_header()
        self.analyze_code_patterns()
        self.find_strings()
        self.find_test_vectors()
        self.analyze_memory_map()
        self.find_entry_points()
        
        print("\n" + "=" * 50)
        print("Analysis complete!")

def main():
    if len(sys.argv) != 2:
        print("Usage: python rom_analyzer.py <rom_file>")
        sys.exit(1)
    
    rom_path = sys.argv[1]
    analyzer = ROMAnalyzer(rom_path)
    analyzer.run_full_analysis()

if __name__ == "__main__":
    main() 