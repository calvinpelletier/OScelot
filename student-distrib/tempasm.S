// tempasm.S

#define ASM 1
#include "x86_desc.h"


.text
.globl kernel_to_user, _kernel_to_user
.globl haltasm, _haltasm

haltasm:
_haltasm:
pushl %ebp
movl %esp, %ebp
movl 16(%ebp), %eax
movl 12(%ebp), %esp
movl 8(%ebp), %ebp
jmp end_execute

kernel_to_user:
_kernel_to_user:
cli 					// turn off interrupts
movl 4(%esp),%ecx 		// get first argument: aka the EIP into user code
movw $USER_DS, %ax 		// load user data segment selectors into data segments
movw %ax, %ds
movw %ax, %gs
movw %ax, %fs
movw %ax, %es
           
pushl $USER_DS			// push user data segment selector
pushl $0x083ffffc 		// push user mode stack (this is 132 MB - 4 (last location in the user page acccessible according to GDB))
pushfl 					// push flags
popl %eax
orl $0x200, %eax		// modify flags to reenable IF upon IRET
pushl %eax
pushl $USER_CS			// push user code segment selctor
pushl %ecx 				// push user EIP
iret
end_execute:
leave
ret 
