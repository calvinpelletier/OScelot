#define ASM 1
#include "x86_desc.h"

.globl syscall_wrapper
.align 4

syscall_wrapper:
_syscall_wrapper:
    cmpl    $0, %eax
    jle     fail
    cmpl    $10, %eax
    jg      fail

    pushl   %es
    pushl   %ds
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp

    pushl   %edx
    pushl   %ecx
    pushl   %ebx

    decl    %eax
    call	*jmptbl(,%eax,4)
    addl    $12, %esp

    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %ds
    popl    %es
    iret

fail:
    movl    $-1, %eax
    iret

jmptbl:
    .long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
