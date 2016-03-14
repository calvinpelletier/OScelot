#include "kernel_handlers.h"
#include "lib.h"

void divideByZero()
{
	clear();
    printf("0x00");
    printf("OScelot won't let you divide by zero");
	while(1);
}

void debug()
{
	clear();
    printf("0x01");
    printf("OScelot won't let you debug");
	while(1);
}

void nonMaskableInterrupts()
{
	clear();
    printf("0x02");
    printf("OScelot asks about NMIs");
	while(1);
}

void breakpoint()
{
	clear();
    printf("0x03");
    printf("OScelot hits a breakpoint");
	while(1);
}

void overflow()
{
	clear();
    printf("0x04");
    printf("OScelot's cup has overflowed");
	while(1);
}

void bounds()
{
	clear();
    printf("0x05");
    printf("You are intruding on OScelot's bounds");
	while(1);
}

void invalidOpCode()
{
	clear();
    printf("0x06");
    printf("OScelot doesn't understand your speech");
	while(1);
}

void coprocessorNotAvailable()
{
	clear();
    printf("0x07");
    printf("OScelot's pardner ain't here yet");
	while(1);
}

void doubleFault()
{
	clear();
    printf("0x08");
    printf("OScelot hit a wall ... again");
	while(1);
}

void coprocessorSegmentOverrun()
{
	clear();
    printf("0x09");
    printf("OScelot's pardner's segment has been overrun");
	while(1);
}

void invalidTaskStateSegment()
{
	clear();
    printf("0x0A");
    printf("OScelot's TSS failed. You incompetent programmer");
	while(1);
}

void segmentNotPresent()
{
	clear();
    printf("0x0B");
    printf("OScelot doesn't know what segment you are talking about");
	while(1);
}

void stackFault()
{
	clear();
    printf("0x0C");
    printf("OScelot suggests you check the stack fault thingamajig");
	while(1);
}

void generalProtectionFault()
{
	clear();
    printf("0x0D");
    printf("OScelot can't protect you any more");
	while(1);
}

void pageFault()
{
	clear();
    printf("0x0E");
    printf("OScelot's book has a page fault. He can't read his book now. Happy?");
	while(1);
}

void reserved()
{
	clear();
    printf("0x0F");
    printf("OScelot won't let you do that");
	while(1);
}

void mathFault()
{
	clear();
    printf("0x10");
    printf("OScelot can't do the math");
	while(1);
}

void alignmentCheck()
{
	clear();
    printf("0x11");
    printf("OScelot asks you to check your alignment");
	while(1);
}

void machineCheck()
{
	clear();
    printf("0x12");
    printf("OScelot needs you to check your machine and cry");
	while(1);
}

void simdFloatingPointException()
{
	clear();
    printf("0x13");
    printf("OScelot can't handle floats right now");
	while(1);
}
