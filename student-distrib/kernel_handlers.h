#ifndef KERNEL_HANDLER_H
#define KERNEL_HANDLER_H

extern void divideByZero();
extern void debug();
extern void nonMaskableInterrupts();
extern void breakpoint();
extern void overflow();
extern void bounds();
extern void invalidOpCode();
extern void coprocessorNotAvailable();
extern void doubleFault();
extern void coprocessorSegmentOverrun();
extern void invalidTaskStateSegment();
extern void segmentNotPresent();
extern void stackFault();
extern void generalProtectionFault();
extern void pageFault();
extern void reserved();
extern void mathFault();
extern void alignmentCheck();
extern void machineCheck();
extern void simdFloatingPointException();
extern void rtcTest();

#endif
