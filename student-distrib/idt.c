// idt.c

#include "lib.h"
#include "x86_desc.h"
#include "int_wrapper.h"
#include "syscalls_asm.h"

// FUNCTION DECLARATIONS
void idt_init();


// GLOBAL FUNCTIONS
void idt_init() {
    idt_desc_t the_idt_desc;

    the_idt_desc.dpl = 0;
    the_idt_desc.size = 1;
    the_idt_desc.reserved0 = 0;
    the_idt_desc.reserved1 = 1;
    the_idt_desc.reserved2 = 1;
    the_idt_desc.reserved3 = 0;
    the_idt_desc.reserved4 = 0;
    the_idt_desc.seg_selector = KERNEL_CS;

    the_idt_desc.present = 1;

    idt_desc_t divZero = the_idt_desc;
    SET_IDT_ENTRY(divZero, divideByZero_wrapper);
    idt[0] = divZero;

    idt_desc_t dbug = the_idt_desc;
    SET_IDT_ENTRY(dbug, debug_wrapper);
    idt[1] = dbug;

    idt_desc_t nmi = the_idt_desc;
    SET_IDT_ENTRY(nmi, nonMaskableInterrupts_wrapper);
    idt[2] = nmi;

    idt_desc_t bpoint = the_idt_desc;
    SET_IDT_ENTRY(bpoint, breakpoint_wrapper);
    idt[3] = bpoint;

    idt_desc_t oflow = the_idt_desc;
    SET_IDT_ENTRY(oflow, overflow_wrapper);
    idt[4] = oflow;

    idt_desc_t bound = the_idt_desc;
    SET_IDT_ENTRY(bound, bounds_wrapper);
    idt[5] = bound;

    idt_desc_t opCode = the_idt_desc;
    SET_IDT_ENTRY(opCode, invalidOpCode_wrapper);
    idt[6] = opCode;

    idt_desc_t coprocessorNA = the_idt_desc;
    SET_IDT_ENTRY(coprocessorNA, coprocessorNotAvailable_wrapper);
    idt[7] = coprocessorNA;

    idt_desc_t dblFault = the_idt_desc;
    SET_IDT_ENTRY(dblFault, doubleFault_wrapper);
    idt[8] = dblFault;

    idt_desc_t coprocessorSO = the_idt_desc;
    SET_IDT_ENTRY(coprocessorSO, coprocessorSegmentOverrun_wrapper);
    idt[9] = coprocessorSO;

    idt_desc_t invalidTSS = the_idt_desc;
    SET_IDT_ENTRY(invalidTSS, invalidTaskStateSegment_wrapper);
    idt[10] = invalidTSS;

    idt_desc_t segNotPresent = the_idt_desc;
    SET_IDT_ENTRY(segNotPresent, segmentNotPresent_wrapper);
    idt[11] = segNotPresent;

    idt_desc_t stackF = the_idt_desc;
    SET_IDT_ENTRY(stackF, stackFault_wrapper);
    idt[12] = stackF;

    idt_desc_t generalProtectFault = the_idt_desc;
    SET_IDT_ENTRY(generalProtectFault, generalProtectionFault_wrapper);
    idt[13] = generalProtectFault;

    idt_desc_t pageF = the_idt_desc;
    SET_IDT_ENTRY(pageF, pageFault_wrapper);
    idt[14] = pageF;

    idt_desc_t reserve = the_idt_desc;
    SET_IDT_ENTRY(reserve, reserved_wrapper);
    idt[15] = reserve;

    idt_desc_t mathF = the_idt_desc;
    SET_IDT_ENTRY(mathF, mathFault_wrapper);
    idt[16] = mathF;

    idt_desc_t alignCheck = the_idt_desc;
    SET_IDT_ENTRY(alignCheck, alignmentCheck_wrapper);
    idt[17] = alignCheck;

    idt_desc_t machCheck = the_idt_desc;
    SET_IDT_ENTRY(machCheck, machineCheck_wrapper);
    idt[18] = machCheck;

    idt_desc_t simdFPE = the_idt_desc;
    SET_IDT_ENTRY(simdFPE, simdFloatingPointException_wrapper);
    idt[19] = simdFPE;

    idt_desc_t rtc = the_idt_desc;
    SET_IDT_ENTRY(rtc, rtcHandler_wrapper);
    idt[40] = rtc;

    idt_desc_t kb = the_idt_desc;
    SET_IDT_ENTRY(kb, keyboardHandler_wrapper);
    idt[33] = kb;

    the_idt_desc.reserved3 = 1;
    the_idt_desc.dpl = 3;

    idt_desc_t sys = the_idt_desc;
    SET_IDT_ENTRY(sys, syscall_wrapper);
    idt[128] = sys;
}
