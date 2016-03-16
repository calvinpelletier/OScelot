// idt.c

#include "lib.h"
#include "x86_desc.h"
#include "int_wrapper.h"


// FUNCTION DECLARATIONS
void idt_init(void);


// GLOBAL FUNCTIONS
void idt_init(void) {
    idt_desc_t first;
    // int i;
    // for (i = 0; i < NUM_VEC; i++ ) {
        // first.present = 0;
        first.dpl = 0;
        first.reserved0 = 0;
        first.size = 1;
        first.reserved1 = 1;
        first.reserved2 = 1;
        first.reserved3 = 0;
        first.reserved4 = 0;
        first.seg_selector = KERNEL_CS;
        // idt[i] = first;
    // }

    first.present = 1;

    idt_desc_t divZero = first;
    SET_IDT_ENTRY(divZero, divideByZero_wrapper);
    idt[0] = divZero;

    idt_desc_t dbug = first;
    SET_IDT_ENTRY(dbug, debug_wrapper);
    idt[1] = dbug;

    idt_desc_t nmi = first;
    SET_IDT_ENTRY(nmi, nonMaskableInterrupts_wrapper);
    idt[2] = nmi;

    idt_desc_t bpoint = first;
    SET_IDT_ENTRY(bpoint, breakpoint_wrapper);
    idt[3] = bpoint;

    idt_desc_t oflow = first;
    SET_IDT_ENTRY(oflow, overflow_wrapper);
    idt[4] = oflow;

    idt_desc_t bound = first;
    SET_IDT_ENTRY(bound, bounds_wrapper);
    idt[5] = bound;

    idt_desc_t opCode = first;
    SET_IDT_ENTRY(opCode, invalidOpCode_wrapper);
    idt[6] = opCode;

    idt_desc_t coprocessorNA = first;
    SET_IDT_ENTRY(coprocessorNA, coprocessorNotAvailable_wrapper);
    idt[7] = coprocessorNA;

    idt_desc_t dblFault = first;
    SET_IDT_ENTRY(dblFault, doubleFault_wrapper);
    idt[8] = dblFault;

    idt_desc_t coprocessorSO = first;
    SET_IDT_ENTRY(coprocessorSO, coprocessorSegmentOverrun_wrapper);
    idt[9] = coprocessorSO;

    idt_desc_t invalidTSS = first;
    SET_IDT_ENTRY(invalidTSS, invalidTaskStateSegment_wrapper);
    idt[10] = invalidTSS;

    idt_desc_t segNotPresent = first;
    SET_IDT_ENTRY(segNotPresent, segmentNotPresent_wrapper);
    idt[11] = segNotPresent;

    idt_desc_t stackF = first;
    SET_IDT_ENTRY(stackF, stackFault_wrapper);
    idt[12] = stackF;

    idt_desc_t generalProtectFault = first;
    SET_IDT_ENTRY(generalProtectFault, generalProtectionFault_wrapper);
    idt[13] = generalProtectFault;

    idt_desc_t pageF = first;
    SET_IDT_ENTRY(pageF, pageFault_wrapper);
    idt[14] = pageF;

    idt_desc_t reserve = first;
    SET_IDT_ENTRY(reserve, reserved_wrapper);
    idt[15] = reserve;

    idt_desc_t mathF = first;
    SET_IDT_ENTRY(mathF, mathFault_wrapper);
    idt[16] = mathF;

    idt_desc_t alignCheck = first;
    SET_IDT_ENTRY(alignCheck, alignmentCheck_wrapper);
    idt[17] = alignCheck;

    idt_desc_t machCheck = first;
    SET_IDT_ENTRY(machCheck, machineCheck_wrapper);
    idt[18] = machCheck;

    idt_desc_t simdFPE = first;
    SET_IDT_ENTRY(simdFPE, simdFloatingPointException_wrapper);
    idt[19] = simdFPE;

    idt_desc_t rtc = first;
    SET_IDT_ENTRY(rtc, rtcHandler_wrapper);
    idt[40] = rtc;

    idt_desc_t kb = first;
    SET_IDT_ENTRY(kb, keyboardHandler_wrapper);
    idt[33] = kb;

    // first.reserved3 = 1;
    // first.dpl = 3;
    //
    // idt_desc_t sys = first;
    // SET_IDT_ENTRY(sys, dispatch);
    // idt[127] = sys;
}
