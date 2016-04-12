#include "kernel_handlers.h"
#include "lib.h"

void divideByZero()
{
    printf("0x00\n");
    printf("OScelot won't let you divide by zero\n");
    while(1);
}

void debug()
{
    printf("0x01\n");
    printf("OScelot won't let you debug\n");
    while(1);
}

void nonMaskableInterrupts()
{
    printf("0x02\n");
    printf("OScelot asks about NMIs\n");
    while(1);
}

void breakpoint()
{
    printf("0x03\n");
    printf("OScelot hits a breakpoint\n");
    while(1);
}

void overflow()
{
    printf("0x04\n");
    printf("OScelot's cup has overflowed\n");
    while(1);
}

void bounds()
{
    printf("0x05\n");
    printf("You are intruding on OScelot's bounds\n");
    while(1);
}

void invalidOpCode()
{
    printf("0x06\n");
    printf("OScelot doesn't understand your speech\n");
    while(1);
}

void coprocessorNotAvailable()
{
    printf("0x07\n");
    printf("OScelot's pardner ain't here yet\n");
    while(1);
}

void doubleFault()
{
    printf("0x08\n");
    printf("OScelot hit a wall ... again\n");
    while(1);
}

void coprocessorSegmentOverrun()
{
    printf("0x09\n");
    printf("OScelot's pardner's segment has been overrun\n");
    while(1);
}

void invalidTaskStateSegment()
{
    printf("0x0A\n");
    printf("OScelot's TSS failed. You incompetent programmer\n");
    while(1);
}

void segmentNotPresent()
{
    printf("0x0B\n");
    printf("OScelot doesn't know what segment you are talking about\n");
    while(1);
}

void stackFault()
{
    printf("0x0C\n");
    printf("OScelot suggests you check the stack fault thingamajig\n");
    while(1);
}

void generalProtectionFault()
{
    printf("0x0D\n");
    printf("OScelot can't protect you any more\n");
    while(1);
}

void pageFault()
{
    printf("0x0E\n");
    printf("OScelot's book has a page fault. He can't read his book now. Happy?\n");
    while(1);
}

void reserved()
{
    printf("0x0F\n");
    printf("OScelot won't let you do that\n");
    while(1);
}

void mathFault()
{
    printf("0x10\n");
    printf("OScelot can't do the math\n");
    while(1);
}

void alignmentCheck()
{
    printf("0x11\n");
    printf("OScelot asks you to check your alignment\n");
    while(1);
}

void machineCheck()
{
    printf("0x12\n");
    printf("OScelot needs you to check your machine and cry\n");
    while(1);
}

void simdFloatingPointException()
{
    printf("0x13\n");
    printf("OScelot can't handle floats right now\n");
    while(1);
}
