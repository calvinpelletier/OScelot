#ifndef KERNEL_HANDLER_H
#define KERNEL_HANDLER_H

void divideByZero();
void debug();
void nonMaskableInterrupts();
void breakpoint();
void overflow();
void bounds();
void invalidOpCode();
void coprocessorNotAvailable();
void doubleFault();
void coprocessorSegmentOverrun();
void invalidTaskStateSegment();
void segmentNotPresent();
void stackFault();
void generalProtectionFault();
void pageFault();
void reserved();
void mathFault();
void alignmentCheck();
void machineCheck();
void simdFloatingPointException();

#endif
