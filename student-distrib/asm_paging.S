// asm_paging.S

.text
.globl loadPageDir, _loadPageDir
.globl enablePaging, _enablePaging
.globl enable4MB, _enable4MB

loadPageDir:
_loadPageDir:
pushl %ebp
movl %esp, %ebp
movl 8(%esp), %eax
movl %eax, %cr3
leave
ret

enablePaging:
_enablePaging:
pushl %ebp
movl %esp, %ebp
movl %cr0, %eax
orl $0x80000000, %eax
movl %eax, %cr0
leave
ret

enable4MB:
_enable4MB:
pushl %ebp
movl %esp, %ebp
movl %cr4, %eax
orl  $0x00000010, %eax
movl %eax, %cr4
leave
ret
