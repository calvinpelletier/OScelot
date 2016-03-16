#ifndef INT_WRAPPER_H
#define INT_WRAPPER_H

extern void divideByZero_wrapper();
extern void debug_wrapper();
extern void nonMaskableInterrupts_wrapper();
extern void breakpoint_wrapper();
extern void overflow_wrapper();
extern void bounds_wrapper();
extern void invalidOpCode_wrapper();
extern void coprocessorNotAvailable_wrapper();
extern void doubleFault_wrapper();
extern void coprocessorSegmentOverrun_wrapper();
extern void invalidTaskStateSegment_wrapper();
extern void segmentNotPresent_wrapper();
extern void stackFault_wrapper();
extern void generalProtectionFault_wrapper();
extern void pageFault_wrapper();
extern void reserved_wrapper();
extern void mathFault_wrapper();
extern void alignmentCheck_wrapper();
extern void machineCheck_wrapper();
extern void simdFloatingPointException_wrapper();
extern void rtcHandler_wrapper();
extern void keyboardHandler_wrapper();

#endif
