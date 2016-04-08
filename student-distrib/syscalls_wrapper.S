.globl halt, execute, read, write, open, close
.align 4

halt:
_halt:
pushl   %eax
pushl   %ebx
pushl   %ecx
pushl   %edx
pushl   %esi
pushl   %edi
pushl   %ebp
call	halt_helper
popl    %ebp
popl    %edi
popl    %esi
popl    %edx
popl    %ecx
popl    %ebx
popl    %eax
iret

execute:
_execute:
pushl   %eax
pushl   %ebx
pushl   %ecx
pushl   %edx
pushl   %esi
pushl   %edi
pushl   %ebp
call	execute_helper
popl    %ebp
popl    %edi
popl    %esi
popl    %edx
popl    %ecx
popl    %ebx
popl    %eax
iret

read:
_read:
pushl   %eax
pushl   %ebx
pushl   %ecx
pushl   %edx
pushl   %esi
pushl   %edi
pushl   %ebp
call	read_helper
popl    %ebp
popl    %edi
popl    %esi
popl    %edx
popl    %ecx
popl    %ebx
popl    %eax
iret

write:
_write:
pushl   %eax
pushl   %ebx
pushl   %ecx
pushl   %edx
pushl   %esi
pushl   %edi
pushl   %ebp
call	write_helper
popl    %ebp
popl    %edi
popl    %esi
popl    %edx
popl    %ecx
popl    %ebx
popl    %eax
iret

open:
_open:
pushl   %eax
pushl   %ebx
pushl   %ecx
pushl   %edx
pushl   %esi
pushl   %edi
pushl   %ebp
call	open_helper
popl    %ebp
popl    %edi
popl    %esi
popl    %edx
popl    %ecx
popl    %ebx
popl    %eax
iret

close:
_close:
pushl   %eax
pushl   %ebx
pushl   %ecx
pushl   %edx
pushl   %esi
pushl   %edi
pushl   %ebp
call	close_helper
popl    %ebp
popl    %edi
popl    %esi
popl    %edx
popl    %ecx
popl    %ebx
popl    %eax
iret
