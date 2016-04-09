.globl syscall_wrapper
.align 4

syscall_wrapper:
_syscall_wrapper:
    pushl   %es
    pushl   %ds
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp

    pushl   %edx
    pushl   %ecx
    pushl   %ebx
    call	*jmptbl(,%eax,4)
    addl    $12, %esp

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


jmptbl:
    .long 0x0, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
