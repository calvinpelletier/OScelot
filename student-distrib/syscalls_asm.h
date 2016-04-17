// syscalls_asm.h

#ifndef SYSCALLS_ASM_H
#define SYSCALLS_ASM_H

extern void syscall_wrapper();
extern void kernel_to_user(uint32_t entry);
extern void haltasm(int32_t ebp, int32_t esp, uint32_t ret);

#endif
