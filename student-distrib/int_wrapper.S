.globl divideByZero_wrapper
.globl debug_wrapper
.globl nonMaskableInterrupts_wrapper
.globl breakpoint_wrapper
.globl overflow_wrapper
.globl bounds_wrapper
.globl invalidOpCode_wrapper
.globl coprocessorNotAvailable_wrapper
.globl doubleFault_wrapper
.globl coprocessorSegmentOverrun_wrapper
.globl invalidTaskStateSegment_wrapper
.globl segmentNotPresent_wrapper
.globl stackFault_wrapper
.globl generalProtectionFault_wrapper
.globl pageFault_wrapper
.globl reserved_wrapper
.globl mathFault_wrapper
.globl alignmentCheck_wrapper
.globl machineCheck_wrapper
.globl simdFloatingPointException_wrapper
.globl rtcHandler_wrapper
.globl keyboardHandler_wrapper
.align 4

divideByZero_wrapper:
_divideByZero_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	divideByZero
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret

debug_wrapper:
_debug_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	debug
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret

nonMaskableInterrupts_wrapper:
_nonMaskableInterrupts_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	nonMaskableInterrupts
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret

breakpoint_wrapper:
_breakpoint_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	breakpoint
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret

overflow_wrapper:
_overflow_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	overflow
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret

bounds_wrapper:
_bounds_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	bounds
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret

invalidOpCode_wrapper:
_invalidOpCode_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	invalidOpCode
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret

coprocessorNotAvailable_wrapper:
_coprocessorNotAvailable_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	coprocessorNotAvailable
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret

doubleFault_wrapper:
_doubleFault_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	doubleFault
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret

coprocessorSegmentOverrun_wrapper:
_coprocessorSegmentOverrun_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	coprocessorSegmentOverrun
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret

invalidTaskStateSegment_wrapper:
_invalidTaskStateSegment_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	invalidTaskStateSegment
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret

segmentNotPresent_wrapper:
_segmentNotPresent_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	segmentNotPresent
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret

stackFault_wrapper:
_stackFault_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	stackFault
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret

generalProtectionFault_wrapper:
_generalProtectionFault_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	generalProtectionFault
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret

pageFault_wrapper:
_pageFault_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	pageFault
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret

reserved_wrapper:
_reserved_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	reserved
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret

mathFault_wrapper:
_mathFault_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	mathFault
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret

alignmentCheck_wrapper:
_alignmentCheck_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	alignmentCheck
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret

machineCheck_wrapper:
_machineCheck_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	machineCheck
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret

simdFloatingPointException_wrapper:
_simdFloatingPointException_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	simdFloatingPointException
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret

rtcHandler_wrapper:
_rtcHandler_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	rtcHandler
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret

keyboardHandler_wrapper:
_keyboardHandler_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    call	keyboardHandler
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    popl    %ds
    popl    %es
    iret
