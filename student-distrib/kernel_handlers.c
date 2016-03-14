#include "kernel_handlers.h"
#include "lib.h"

void divideByZero()
{
	;
    printf("\n0x00\n");
    printf("OScelot won't let you divide by zero");
	while(1);
}

void debug()
{
	;
    printf("\n0x01\n");
    printf("OScelot won't let you debug");
	while(1);
}

void nonMaskableInterrupts()
{
	;
    printf("\n0x02\n");
    printf("OScelot asks about NMIs");
	while(1);
}

void breakpoint()
{
	;
    printf("\n0x03\n");
    printf("OScelot hits a breakpoint");
	while(1);
}

void overflow()
{
	;
    printf("\n0x04\n");
    printf("OScelot's cup has overflowed");
	while(1);
}

void bounds()
{
	;
    printf("\n0x05\n");
    printf("You are intruding on OScelot's bounds");
	while(1);
}

void invalidOpCode()
{
	;
    printf("\n0x06\n");
    printf("OScelot doesn't understand your speech");
	while(1);
}

void coprocessorNotAvailable()
{
	;
    printf("\n0x07\n");
    printf("OScelot's pardner ain't here yet");
	while(1);
}

void doubleFault()
{
	;
    printf("\n0x08\n");
    printf("OScelot hit a wall ... again");
	while(1);
}

void coprocessorSegmentOverrun()
{
	;
    printf("\n0x09\n");
    printf("OScelot's pardner's segment has been overrun");
	while(1);
}

void invalidTaskStateSegment()
{
	;
    printf("\n0x0A\n");
    printf("OScelot's TSS failed. You incompetent programmer");
	while(1);
}

void segmentNotPresent()
{
	;
    printf("\n0x0B\n");
    printf("OScelot doesn't know what segment you are talking about");
	while(1);
}

void stackFault()
{
	;
    printf("\n0x0C\n");
    printf("OScelot suggests you check the stack fault thingamajig");
	while(1);
}

void generalProtectionFault()
{
	;
    printf("\n0x0D\n");
    printf("OScelot can't protect you any more");
	while(1);
}

void pageFault()
{
	;
    printf("\n0x0E\n");
    printf("OScelot's book has a page fault. He can't read his book now. Happy?");
	while(1);
}

void reserved()
{
	;
    printf("\n0x0F\n");
    printf("OScelot won't let you do that");
	while(1);
}

void mathFault()
{
	;
    printf("\n0x10\n");
    printf("OScelot can't do the math");
	while(1);
}

void alignmentCheck()
{
	;
    printf("\n0x11\n");
    printf("OScelot asks you to check your alignment");
	while(1);
}

void machineCheck()
{
	;
    printf("\n0x12\n");
    printf("OScelot needs you to check your machine and cry");
	while(1);
}

void simdFloatingPointException()
{
	;
    printf("\n0x13\n");
    printf("OScelot can't handle floats right now");
	while(1);
}
