#pragma once
#include <cstdint>

// Forward declaration
class CPU;

// 65816 CPU Instructions
class CPUInstructions {
public:
    // Control Instructions
    static void brk(CPU* cpu);
    static void nop(CPU* cpu);
    static void jmp_absolute(CPU* cpu);
    static void jmp_absolute_long(CPU* cpu);
    static void jmp_absolute_indirect(CPU* cpu);
    static void jmp_absolute_indirect_long(CPU* cpu);
    static void jmp_absolute_indirect_x(CPU* cpu);
    static void jsr(CPU* cpu);
    static void jsr_absolute_long(CPU* cpu);
    static void rts(CPU* cpu);
    static void rtl(CPU* cpu);
    static void rti(CPU* cpu);
    static void wai(CPU* cpu);
    static void stp(CPU* cpu);
    
    // Flag Instructions
    static void sei(CPU* cpu);
    static void cli(CPU* cpu);
    static void sec(CPU* cpu);
    static void clc(CPU* cpu);
    static void sed(CPU* cpu);
    static void cld(CPU* cpu);
    static void clv(CPU* cpu);
    static void xce(CPU* cpu);
    
    // Load Instructions
    static void lda_immediate(CPU* cpu);
    static void lda_direct_page(CPU* cpu);
    static void lda_direct_page_x(CPU* cpu);
    static void lda_absolute(CPU* cpu);
    static void lda_absolute_x(CPU* cpu);
    static void lda_absolute_y(CPU* cpu);
    static void lda_dp_indirect_x(CPU* cpu);
    static void lda_dp_indirect_y(CPU* cpu);
    static void lda_dp_indirect(CPU* cpu);
    static void lda_dp_indirect_long(CPU* cpu);
    static void lda_dp_indirect_long_y(CPU* cpu);
    static void lda_absolute_long(CPU* cpu);
    static void lda_absolute_long_x(CPU* cpu);
    static void lda_stack_relative(CPU* cpu);
    static void lda_stack_relative_indirect_y(CPU* cpu);
    
    // Store Instructions
    static void sta_direct_page(CPU* cpu);
    static void sta_direct_page_x(CPU* cpu);
    static void sta_absolute(CPU* cpu);
    static void sta_absolute_x(CPU* cpu);
    static void sta_absolute_y(CPU* cpu);
    static void sta_dp_indirect_x(CPU* cpu);
    static void sta_dp_indirect_y(CPU* cpu);
    static void sta_dp_indirect(CPU* cpu);
    static void sta_dp_indirect_long(CPU* cpu);
    static void sta_dp_indirect_long_y(CPU* cpu);
    static void sta_absolute_long(CPU* cpu);
    static void sta_absolute_long_x(CPU* cpu);
    static void sta_stack_relative(CPU* cpu);
    static void sta_stack_relative_indirect_y(CPU* cpu);
    
    // Transfer Instructions
    static void tax(CPU* cpu);
    static void txa(CPU* cpu);
    static void tay(CPU* cpu);
    static void tya(CPU* cpu);
    static void tsx(CPU* cpu);
    static void txs(CPU* cpu);
    static void txy(CPU* cpu);
    static void tyx(CPU* cpu);
    static void tcd(CPU* cpu);
    static void tdc(CPU* cpu);
    static void tsc(CPU* cpu);
    static void tcs(CPU* cpu);
    static void xba(CPU* cpu);
    
    // Stack Instructions
    static void pha(CPU* cpu);
    static void pla(CPU* cpu);
    static void phx(CPU* cpu);
    static void plx(CPU* cpu);
    static void phy(CPU* cpu);
    static void ply(CPU* cpu);
    static void php(CPU* cpu);
    static void plp(CPU* cpu);
    static void phd(CPU* cpu);
    static void pld(CPU* cpu);
    static void phk(CPU* cpu);
    static void plk(CPU* cpu);
    static void pea(CPU* cpu);
    static void pei(CPU* cpu);
    static void per(CPU* cpu);
    
    // Arithmetic Instructions
    static void adc_immediate(CPU* cpu);
    static void adc_direct_page(CPU* cpu);
    static void adc_direct_page_x(CPU* cpu);
    static void adc_absolute(CPU* cpu);
    static void adc_absolute_x(CPU* cpu);
    static void adc_absolute_y(CPU* cpu);
    static void adc_dp_indirect_x(CPU* cpu);
    static void adc_dp_indirect_y(CPU* cpu);
    static void adc_dp_indirect(CPU* cpu);
    static void adc_dp_indirect_long(CPU* cpu);
    static void adc_dp_indirect_long_y(CPU* cpu);
    
    static void sbc_immediate(CPU* cpu);
    static void sbc_direct_page(CPU* cpu);
    static void sbc_direct_page_x(CPU* cpu);
    static void sbc_absolute(CPU* cpu);
    static void sbc_absolute_x(CPU* cpu);
    static void sbc_absolute_y(CPU* cpu);
    static void sbc_dp_indirect_x(CPU* cpu);
    static void sbc_dp_indirect_y(CPU* cpu);
    static void sbc_dp_indirect(CPU* cpu);
    static void sbc_dp_indirect_long(CPU* cpu);
    static void sbc_dp_indirect_long_y(CPU* cpu);
    
    // Comparison Instructions
    static void cmp_immediate(CPU* cpu);
    static void cmp_direct_page(CPU* cpu);
    static void cmp_direct_page_x(CPU* cpu);
    static void cmp_absolute(CPU* cpu);
    static void cmp_absolute_x(CPU* cpu);
    static void cmp_absolute_y(CPU* cpu);
    static void cmp_dp_indirect_x(CPU* cpu);
    static void cmp_dp_indirect_y(CPU* cpu);
    static void cmp_dp_indirect(CPU* cpu);
    static void cmp_dp_indirect_long(CPU* cpu);
    static void cmp_dp_indirect_long_y(CPU* cpu);
    static void cmp_absolute_long(CPU* cpu);
    static void cmp_absolute_long_x(CPU* cpu);
    static void cmp_stack_relative(CPU* cpu);
    static void cmp_stack_relative_indirect_y(CPU* cpu);
    
    static void cpx_immediate(CPU* cpu);
    static void cpx_direct_page(CPU* cpu);
    static void cpx_absolute(CPU* cpu);
    
    static void cpy_immediate(CPU* cpu);
    static void cpy_direct_page(CPU* cpu);
    static void cpy_absolute(CPU* cpu);
    
    // Logical Instructions
    static void and_immediate(CPU* cpu);
    static void and_direct_page(CPU* cpu);
    static void and_direct_page_x(CPU* cpu);
    static void and_absolute(CPU* cpu);
    static void and_absolute_x(CPU* cpu);
    static void and_absolute_y(CPU* cpu);
    static void and_dp_indirect_x(CPU* cpu);
    static void and_dp_indirect_y(CPU* cpu);
    static void and_dp_indirect(CPU* cpu);
    static void and_dp_indirect_long(CPU* cpu);
    static void and_dp_indirect_long_y(CPU* cpu);
    
    static void ora_immediate(CPU* cpu);
    static void ora_direct_page(CPU* cpu);
    static void ora_direct_page_x(CPU* cpu);
    static void ora_absolute(CPU* cpu);
    static void ora_absolute_x(CPU* cpu);
    static void ora_absolute_y(CPU* cpu);
    static void ora_dp_indirect_x(CPU* cpu);
    static void ora_dp_indirect_y(CPU* cpu);
    static void ora_dp_indirect(CPU* cpu);
    static void ora_dp_indirect_long(CPU* cpu);
    static void ora_dp_indirect_long_y(CPU* cpu);
    
    static void eor_immediate(CPU* cpu);
    static void eor_direct_page(CPU* cpu);
    static void eor_direct_page_x(CPU* cpu);
    static void eor_absolute(CPU* cpu);
    static void eor_absolute_x(CPU* cpu);
    static void eor_absolute_y(CPU* cpu);
    static void eor_dp_indirect_x(CPU* cpu);
    static void eor_dp_indirect_y(CPU* cpu);
    static void eor_dp_indirect(CPU* cpu);
    static void eor_dp_indirect_long(CPU* cpu);
    static void eor_dp_indirect_long_y(CPU* cpu);
    
    // Shift Instructions
    static void asl_accumulator(CPU* cpu);
    static void asl_direct_page(CPU* cpu);
    static void asl_direct_page_x(CPU* cpu);
    static void asl_absolute(CPU* cpu);
    static void asl_absolute_x(CPU* cpu);
    
    static void lsr_accumulator(CPU* cpu);
    static void lsr_direct_page(CPU* cpu);
    static void lsr_direct_page_x(CPU* cpu);
    static void lsr_absolute(CPU* cpu);
    static void lsr_absolute_x(CPU* cpu);
    
    static void rol_accumulator(CPU* cpu);
    static void rol_direct_page(CPU* cpu);
    static void rol_direct_page_x(CPU* cpu);
    static void rol_absolute(CPU* cpu);
    static void rol_absolute_x(CPU* cpu);
    
    static void ror_accumulator(CPU* cpu);
    static void ror_direct_page(CPU* cpu);
    static void ror_direct_page_x(CPU* cpu);
    static void ror_absolute(CPU* cpu);
    static void ror_absolute_x(CPU* cpu);
    
    // Branch Instructions
    static void bcc(CPU* cpu);
    static void bcs(CPU* cpu);
    static void beq(CPU* cpu);
    static void bmi(CPU* cpu);
    static void bne(CPU* cpu);
    static void bpl(CPU* cpu);
    static void bra(CPU* cpu);
    static void bvc(CPU* cpu);
    static void bvs(CPU* cpu);
    static void brl(CPU* cpu);
    
    // Increment/Decrement Instructions
    static void inc_accumulator(CPU* cpu);
    static void inc_direct_page(CPU* cpu);
    static void inc_direct_page_x(CPU* cpu);
    static void inc_absolute(CPU* cpu);
    static void inc_absolute_x(CPU* cpu);
    static void inx(CPU* cpu);
    static void iny(CPU* cpu);
    
    static void dec_accumulator(CPU* cpu);
    static void dec_direct_page(CPU* cpu);
    static void dec_direct_page_x(CPU* cpu);
    static void dec_absolute(CPU* cpu);
    static void dec_absolute_x(CPU* cpu);
    static void dex(CPU* cpu);
    static void dey(CPU* cpu);
    
    // Bit Instructions
    static void bit_immediate(CPU* cpu);
    static void bit_direct_page(CPU* cpu);
    static void bit_absolute(CPU* cpu);
    static void bit_absolute_x(CPU* cpu);
    static void trb_direct_page(CPU* cpu);
    static void trb_absolute(CPU* cpu);
    static void tsb_direct_page(CPU* cpu);
    static void tsb_absolute(CPU* cpu);
    
    // Block Move Instructions
    static void mvp(CPU* cpu);
    static void mvn(CPU* cpu);
}; 