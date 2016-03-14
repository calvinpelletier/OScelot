#include "idt.h"
#include "x86_desc.h"
#include "kernel_handlers.h"

static void init_idt ()
{
    idt_desc_t first;

    first.present = 1;
    first.dpl = 0;
    first.reserved0 = 0;
    first.size = 1;
    first.reserved1 = 1;
    first.reserved2 = 1;
    first.reserved3 = 0;
    first.reserved4 = 0;
    first.seg_selector = KERNEL_CS;

    idt_desc_t divZero = first;
    SET_IDT_ENTRY(divZero, divideByZero);
    idt[0] = divZero;

    idt_desc_t dbug = first;
    SET_IDT_ENTRY(dbug, debug);
    idt[1] = dbug;

    idt_desc_t nmi = first;
    SET_IDT_ENTRY(nmi, nonMaskableInterrupts);
    idt[2] = nmi;

    idt_desc_t bpoint = first;
    SET_IDT_ENTRY(bpoint, breakpoint);
    idt[3] = bpoint;

    idt_desc_t oflow = first;
    SET_IDT_ENTRY(oflow, overflow);
    idt[4] = oflow;

    idt_desc_t bound = first;
    SET_IDT_ENTRY(bound, bounds);
    idt[5] = bound;

    idt_desc_t opCode = first;
    SET_IDT_ENTRY(opCode, invalidOpCode);
    idt[6] = opCode;

    idt_desc_t coprocessorNA = first;
    SET_IDT_ENTRY(coprocessorNA, coprocessorNotAvailable);
    idt[7] = coprocessorNA;

    idt_desc_t dblFault = first;
    SET_IDT_ENTRY(dblFault, doubleFault);
    idt[8] = dblFault;

    idt_desc_t coprocessorSO = first;
    SET_IDT_ENTRY(coprocessorSO, coprocessorSegmentOverrun);
    idt[9] = coprocessorSO;

    idt_desc_t invalidTSS = first;
    SET_IDT_ENTRY(invalidTSS, invalidTaskStateSegment);
    idt[10] = invalidTSS;

    idt_desc_t segNotPresent = first;
    SET_IDT_ENTRY(segNotPresent, segmentNotPresent);
    idt[11] = segNotPresent;

    idt_desc_t stackF = first;
    SET_IDT_ENTRY(stackF, stackFault);
    idt[12] = stackF;

    idt_desc_t generalProtectFault = first;
    SET_IDT_ENTRY(generalProtectFault, generalProtectionFault);
    idt[13] = generalProtectFault;

    idt_desc_t pageF = first;
    SET_IDT_ENTRY(pageF, pageFault);
    idt[14] = pageF;

    idt_desc_t reserve = first;
    SET_IDT_ENTRY(reserve, reserved);
    idt[15] = reserve;

    idt_desc_t mathF = first;
    SET_IDT_ENTRY(mathF, mathFault);
    idt[16] = mathF;

    idt_desc_t alignCheck = first;
    SET_IDT_ENTRY(alignCheck, alignmentCheck);
    idt[17] = alignCheck;

    idt_desc_t machCheck = first;
    SET_IDT_ENTRY(machCheck, machineCheck);
    idt[18] = machCheck;

    idt_desc_t simdFPE = first;
    SET_IDT_ENTRY(simdFPE, simdFloatingPointException);
    idt[19] = simdFPE;

    idt_desc_t rtc = first;
    SET_IDT_ENTRY(rtc, rtc_handler);
    //idt[32] = rtc;
    // NOTE: try idt[112]
    idt[112] = rtc;

    idt_desc_t kb = first;
    SET_IDT_ENTRY(kb, keyboard_handler);
    idt[33] = kb;

    first.reserved3 = 1; // for the IRQ stuff


    // idt_desc_t irq0 = first;
    // SET_IDT_ENTRY(irq0, generic_irq_handler);
    // idt[32] = irq0;
    //
    // idt_desc_t irq1 = first;
    // SET_IDT_ENTRY(irq1, generic_irq_handler);
    // idt[33] = irq1;
    //
    // idt_desc_t irq2 = first;
    // SET_IDT_ENTRY(irq2, generic_irq_handler);
    // idt[34] = irq2;
    //
    // idt_desc_t irq3 = first;
    // SET_IDT_ENTRY(irq3, generic_irq_handler);
    // idt[35] = irq3;
    //
    // idt_desc_t irq4 = first;
    // SET_IDT_ENTRY(irq4, generic_irq_handler);
    // idt[36] = irq4;
    //
    // idt_desc_t irq5 = first;
    // SET_IDT_ENTRY(irq5, generic_irq_handler);
    // idt[37] = irq5;
    //
    // idt_desc_t irq6 = first;
    // SET_IDT_ENTRY(irq6, generic_irq_handler);
    // idt[38] = irq6;
    //
    // idt_desc_t irq7 = first;
    // SET_IDT_ENTRY(irq7, generic_irq_handler);
    // idt[39] = irq7;
    //
    // idt_desc_t irq8 = first;
    // SET_IDT_ENTRY(irq8, generic_irq_handler);
    // idt[40] = irq8;
    //
    // idt_desc_t irq9 = first;
    // SET_IDT_ENTRY(irq9, generic_irq_handler);
    // idt[41] = irq9;
    //
    // idt_desc_t irq10 = first;
    // SET_IDT_ENTRY(irq10, generic_irq_handler);
    // idt[42] = irq10;
    //
    // idt_desc_t irq11 = first;
    // SET_IDT_ENTRY(irq11, generic_irq_handler);
    // idt[43] = irq11;
    //
    // idt_desc_t irq12 = first;
    // SET_IDT_ENTRY(irq12, generic_irq_handler);
    // idt[44] = irq12;
    //
    // idt_desc_t irq13 = first;
    // SET_IDT_ENTRY(irq13, generic_irq_handler);
    // idt[45] = irq13;
    //
    // idt_desc_t irq14 = first;
    // SET_IDT_ENTRY(irq14, generic_irq_handler);
    // idt[46] = irq14;
    //
    // idt_desc_t irq15 = first;
    // SET_IDT_ENTRY(irq15, generic_irq_handler);
    // idt[47] = irq15;


    // first.reserved3 = 1;
    // first.dpl = 3;
    //
    // idt_desc_t sys = first;
    // SET_IDT_ENTRY(sys, dispatch);
    // idt[127] = sys;
}
