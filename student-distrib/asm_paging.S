// paging.S

.text
.globl loadPageDir
.globl enablePaging

loadPageDir:
pushl %ebp
movl %esp, %ebp
movl 8(%esp), %eax
movl %eax, %cr3
movl %ebp, %esp
popl %ebp
ret

enablePaging:
pushl %ebp
movl %esp, %ebp
movl %cr0, %eax
orl $0x80000000, %eax
movl %eax, %cr0
movl %cr4, %eax
orl $0x00000010, %eax
movl %eax, %cr4
movl %ebp, %esp
popl %ebp
ret
