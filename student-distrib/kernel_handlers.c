#include "kernel_handlers.h"
#include "lib.h"

void divideByZero()
{
	clear();
    printf("\n0x00\n");
    printf("OScelot won't let you divide by zero");
	while(1);
}

void debug()
{
	clear();
    printf("\n0x01\n");
    printf("OScelot won't let you debug");
	while(1);
}

void nonMaskableInterrupts()
{
	clear();
    printf("\n0x02\n");
    printf("OScelot asks about NMIs");
	while(1);
}

void breakpoint()
{
	clear();
    printf("\n0x03\n");
    printf("OScelot hits a breakpoint");
	while(1);
}

void overflow()
{
	clear();
    printf("\n0x04\n");
    printf("OScelot's cup has overflowed");
	while(1);
}

void bounds()
{
	clear();
    printf("\n0x05\n");
    printf("You are intruding on OScelot's bounds");
	while(1);
}

void invalidOpCode()
{
	clear();
    printf("\n0x06\n");
    printf("OScelot doesn't understand your speech");
	while(1);
}

void coprocessorNotAvailable()
{
	clear();
    printf("\n0x07\n");
    printf("OScelot's pardner ain't here yet");
	while(1);
}

void doubleFault()
{
	clear();
    printf("\n0x08\n");
    printf("OScelot hit a wall ... again");
	while(1);
}

void coprocessorSegmentOverrun()
{
	clear();
    printf("\n0x09\n");
    printf("OScelot's pardner's segment has been overrun");
	while(1);
}

void invalidTaskStateSegment()
{
	clear();
    printf("\n0x0A\n");
    printf("OScelot's TSS failed. You incompetent programmer");
	while(1);
}

void segmentNotPresent()
{
	clear();
    printf("\n0x0B\n");
    printf("OScelot doesn't know what segment you are talking about");
	while(1);
}

void stackFault()
{
	clear();
    printf("\n0x0C\n");
    printf("OScelot suggests you check the stack fault thingamajig");
	while(1);
}

void generalProtectionFault()
{
	clear();
    printf("\n0x0D\n");
    printf("OScelot can't protect you any more");
	while(1);
}

void pageFault()
{
	clear();
    printf("\n0x0E\n");
    printf("OScelot's book has a page fault. He can't read his book now. Happy?");
	while(1);
}

void reserved()
{
	clear();
    printf("\n0x0F\n");
    printf("OScelot won't let you do that");
	while(1);
}

void mathFault()
{
	clear();
    printf("\n0x10\n");
    printf("OScelot can't do the math");
	while(1);
}

void alignmentCheck()
{
	clear();
    printf("\n0x11\n");
    printf("OScelot asks you to check your alignment");
	while(1);
}

void machineCheck()
{
	clear();
    printf("\n0x12\n");
    printf("OScelot needs you to check your machine and cry");
	while(1);
}

void simdFloatingPointException()
{
	clear();
    printf("\n0x13\n");
    printf("OScelot can't handle floats right now");
	while(1);
}

void rtcTest()
{
    clear();
    printf("Hello World\n");
    while(1);
}
