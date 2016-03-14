# boot.S - start point for the kernel after GRUB gives us control
# vim:ts=4 noexpandtab

#define ASM     1

#include "multiboot.h"
#include "x86_desc.h"

.text

	# Multiboot header (required for GRUB to boot us)
	.long MULTIBOOT_HEADER_MAGIC
	.long MULTIBOOT_HEADER_FLAGS
	.long -(MULTIBOOT_HEADER_MAGIC+MULTIBOOT_HEADER_FLAGS)

gdtr:
	# A 6-byte operand needed to load the GDTR with the base address (upper 4 bytes) and limit (size -1 in bytes) of the GDT
	.word 0 /* Stores size of GDT */
	.long 0 /* Stores base of GDT */

idtr:
	.word 0 /* stores size of IDT */
	.long 0 /* stores base of GDT */

# Entrypoint to the kernel
.globl  start, _start

.align 4
start:
_start:
	# Make sure interrupts are off
	cli
	jmp     continue

continue:
	# Load the GDT
	# Get base and size of gdt from x86_desc.S, store them in gdtr operand, and lgdt with these values
	pushl	%eax
	leal	gdt, %eax
	movl 	%eax, gdtr+2
	movw	gdt_size, %ax
	movw 	%ax, gdtr
	lgdt 	gdtr

	# Load the IDT in a similar fashion as the GDT above
	leal	ldt, %eax
	movl	%eax, idtr+2
	movw	idt_size, %ax
	movw	%ax, idtr
	lidt	idtr

	popl 	%eax

	# flush GDT - initialize CS to KERNEL_CS and all other segments to KERNEL_DS:
	# Load CS with the new descriptor value
	ljmp    $KERNEL_CS, $keep_going

keep_going:
	# Set up ESP so we can have an initial stack
	movl    $0x800000, %esp

	# Set up the rest of the segment selector registers
	movw    $KERNEL_DS, %cx
	movw    %cx, %ss
	movw    %cx, %ds
	movw    %cx, %es
	movw    %cx, %fs
	movw    %cx, %gs

	# Push the parameters that entry() expects (see kernel.c):
	# eax = multiboot magic
	# ebx = address of multiboot info struct
	pushl   %ebx
	pushl   %eax

	# Jump to the C entrypoint to the kernel.
	call    entry

	# We'll never get back here, but we put in a hlt anyway.
halt:
	hlt
	jmp     halt
