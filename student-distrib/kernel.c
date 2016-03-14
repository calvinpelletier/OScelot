/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "kernel_handlers.h"
#include "rtc.h"
#include "keyboard.h"

/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))


/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void
entry (unsigned long magic, unsigned long addr)
{
	multiboot_info_t *mbi;

	/* Clear the screen. */
	clear();

	/* Am I booted by a Multiboot-compliant boot loader? */
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		printf ("Invalid magic number: 0x%#x\n", (unsigned) magic);
		return;
	}

	/* Set MBI to the address of the Multiboot information structure. */
	mbi = (multiboot_info_t *) addr;

	/* Print out the flags. */
	printf ("flags = 0x%#x\n", (unsigned) mbi->flags);

	/* Are mem_* valid? */
	if (CHECK_FLAG (mbi->flags, 0))
		printf ("mem_lower = %uKB, mem_upper = %uKB\n",
				(unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);

	/* Is boot_device valid? */
	if (CHECK_FLAG (mbi->flags, 1))
		printf ("boot_device = 0x%#x\n", (unsigned) mbi->boot_device);

	/* Is the command line passed? */
	if (CHECK_FLAG (mbi->flags, 2))
		printf ("cmdline = %s\n", (char *) mbi->cmdline);

	if (CHECK_FLAG (mbi->flags, 3)) {
		int mod_count = 0;
		int i;
		module_t* mod = (module_t*)mbi->mods_addr;
		while(mod_count < mbi->mods_count) {
			printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
			printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
			printf("First few bytes of module:\n");
			for(i = 0; i<16; i++) {
				printf("0x%x ", *((char*)(mod->mod_start+i)));
			}
			printf("\n");
			mod_count++;
			mod++;
		}
	}
	/* Bits 4 and 5 are mutually exclusive! */
	if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
	{
		printf ("Both bits 4 and 5 are set.\n");
		return;
	}

	/* Is the section header table of ELF valid? */
	if (CHECK_FLAG (mbi->flags, 5))
	{
		elf_section_header_table_t *elf_sec = &(mbi->elf_sec);

		printf ("elf_sec: num = %u, size = 0x%#x,"
				" addr = 0x%#x, shndx = 0x%#x\n",
				(unsigned) elf_sec->num, (unsigned) elf_sec->size,
				(unsigned) elf_sec->addr, (unsigned) elf_sec->shndx);
	}

	/* Are mmap_* valid? */
	if (CHECK_FLAG (mbi->flags, 6))
	{
		memory_map_t *mmap;

		printf ("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
				(unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
		for (mmap = (memory_map_t *) mbi->mmap_addr;
				(unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
				mmap = (memory_map_t *) ((unsigned long) mmap
					+ mmap->size + sizeof (mmap->size)))
			printf (" size = 0x%x,     base_addr = 0x%#x%#x\n"
					"     type = 0x%x,  length    = 0x%#x%#x\n",
					(unsigned) mmap->size,
					(unsigned) mmap->base_addr_high,
					(unsigned) mmap->base_addr_low,
					(unsigned) mmap->type,
					(unsigned) mmap->length_high,
					(unsigned) mmap->length_low);
	}

	/* Construct an LDT entry in the GDT */
	{
		seg_desc_t the_ldt_desc;
		the_ldt_desc.granularity    = 0;
		the_ldt_desc.opsize         = 1;
		the_ldt_desc.reserved       = 0;
		the_ldt_desc.avail          = 0;
		the_ldt_desc.present        = 1;
		the_ldt_desc.dpl            = 0x0;
		the_ldt_desc.sys            = 0;
		the_ldt_desc.type           = 0x2;

		SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
		ldt_desc_ptr = the_ldt_desc;
		lldt(KERNEL_LDT);
	}

	/* Construct a TSS entry in the GDT */
	{
		seg_desc_t the_tss_desc;
		the_tss_desc.granularity    = 0;
		the_tss_desc.opsize         = 0;
		the_tss_desc.reserved       = 0;
		the_tss_desc.avail          = 0;
		the_tss_desc.seg_lim_19_16  = TSS_SIZE & 0x000F0000;
		the_tss_desc.present        = 1;
		the_tss_desc.dpl            = 0x0;
		the_tss_desc.sys            = 0;
		the_tss_desc.type           = 0x9;
		the_tss_desc.seg_lim_15_00  = TSS_SIZE & 0x0000FFFF;

		SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

		tss_desc_ptr = the_tss_desc;

		tss.ldt_segment_selector = KERNEL_LDT;
		tss.ss0 = KERNEL_DS;
		tss.esp0 = 0x800000;
		ltr(KERNEL_TSS);
	}

	/* Set the IDT appropriately */
	{
		idt_desc_t first;
		int i;
		for (i = 0; i < NUM_VEC; i++ ) {
			first.present = 0;
			first.dpl = 0;
			first.reserved0 = 0;
			first.size = 1;
			first.reserved1 = 1;
			first.reserved2 = 1;
			first.reserved3 = 0;
			first.reserved4 = 0;
			first.seg_selector = KERNEL_CS;
			idt[i] = first;
		}

		first.present = 1;

		idt_desc_t divZero = first;
		SET_IDT_ENTRY(divZero, &divideByZero);
		idt[0] = divZero;

		idt_desc_t dbug = first;
		SET_IDT_ENTRY(dbug, &debug);
		idt[1] = dbug;

		idt_desc_t nmi = first;
		SET_IDT_ENTRY(nmi, &nonMaskableInterrupts);
		idt[2] = nmi;

		idt_desc_t bpoint = first;
		SET_IDT_ENTRY(bpoint, &breakpoint);
		idt[3] = bpoint;

		idt_desc_t oflow = first;
		SET_IDT_ENTRY(oflow, &overflow);
		idt[4] = oflow;

		idt_desc_t bound = first;
		SET_IDT_ENTRY(bound, &bounds);
		idt[5] = bound;

		idt_desc_t opCode = first;
		SET_IDT_ENTRY(opCode, &invalidOpCode);
		idt[6] = opCode;

		idt_desc_t coprocessorNA = first;
		SET_IDT_ENTRY(coprocessorNA, &coprocessorNotAvailable);
		idt[7] = coprocessorNA;

		idt_desc_t dblFault = first;
		SET_IDT_ENTRY(dblFault, &doubleFault);
		idt[8] = dblFault;

		idt_desc_t coprocessorSO = first;
		SET_IDT_ENTRY(coprocessorSO, &coprocessorSegmentOverrun);
		idt[9] = coprocessorSO;

		idt_desc_t invalidTSS = first;
		SET_IDT_ENTRY(invalidTSS, &invalidTaskStateSegment);
		idt[10] = invalidTSS;

		idt_desc_t segNotPresent = first;
		SET_IDT_ENTRY(segNotPresent, &segmentNotPresent);
		idt[11] = segNotPresent;

		idt_desc_t stackF = first;
		SET_IDT_ENTRY(stackF, &stackFault);
		idt[12] = stackF;

		idt_desc_t generalProtectFault = first;
		SET_IDT_ENTRY(generalProtectFault, &generalProtectionFault);
		idt[13] = generalProtectFault;

		idt_desc_t pageF = first;
		SET_IDT_ENTRY(pageF, &pageFault);
		idt[14] = pageF;

		idt_desc_t reserve = first;
		SET_IDT_ENTRY(reserve, &reserved);
		idt[15] = reserve;

		idt_desc_t mathF = first;
		SET_IDT_ENTRY(mathF, &mathFault);
		idt[16] = mathF;

		idt_desc_t alignCheck = first;
		SET_IDT_ENTRY(alignCheck, &alignCheck);
		idt[17] = alignCheck;

		idt_desc_t machCheck = first;
		SET_IDT_ENTRY(machCheck, &machineCheck);
		idt[18] = machCheck;

		idt_desc_t simdFPE = first;
		SET_IDT_ENTRY(simdFPE, &simdFloatingPointException);
		idt[19] = simdFPE;

		idt_desc_t rtc = first;
		SET_IDT_ENTRY(rtc, &test_interrupts);
		idt[32] = rtc;

		idt_desc_t kb = first;
		SET_IDT_ENTRY(kb, &keyboard_handler);
		idt[33] = kb;

		first.reserved3 = 1;
		first.dpl = 3;

		idt_desc_t sys = first;
		SET_IDT_ENTRY(sys, &dispatch);
		idt[127] = sys;
	}

	/* Init the PIC */
	i8259_init();

	/* Initialize devices, memory, filesystem, enable device interrupts on the
	 * PIC, any other initialization stuff... */
	 // initialize RTC
	 rtc_init();
	 enable_irq(RTC_IRQ_NUM);
	 // initialize keyboard
	 if (keyboard_init()) {
		 printf("ERROR: keyboard failed initialization.");
	 }
	 enable_irq(KEYBOARD_IRQ_NUM);

	/* Enable interrupts */
	/* Do not enable the following until after you have set up your
	 * IDT correctly otherwise QEMU will triple fault and simple close
	 * without showing you any output */
	printf("Enabling Interrupts\n");
	sti();

	/* Execute the first program (`shell') ... */

	/* Spin (nicely, so we don't chew up cycles) */
	asm volatile(".1: hlt; jmp .1;");
}
