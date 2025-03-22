#include "SM83.h"
#include "Memory.h"
#include <stdio.h>
#include <iostream>
#include <cstdint>

int main()
{
	Memory memory;
	SM83 cpu(memory);

	cpu.reset();
	memory.write(0x00, 0x00);
	memory.write(0x01, 0x01);
	memory.write(0x02, 0x02);
	memory.write(0x03, 0x03);
	memory.write(0x04, 0x04);
	memory.write(0x05, 0x05);
	memory.write(0x06, 0x06);
	memory.write(0x07, 0x07);
	memory.write(0x08, 0x08);
	memory.write(0x09, 0x09);
	memory.write(0x0A, 0x0A);
	memory.write(0x0B, 0x0B);
	memory.write(0x0C, 0x0C);
	memory.write(0x0D, 0x0D);
	memory.write(0x0E, 0x0E);
	memory.write(0x0F, 0x0F);
	memory.write(0x00, 0x00);
	memory.write(0x00, 0x00);
	memory.write(0x00, 0x00);
	memory.write(0x00, 0x00);
	memory.write(0x00, 0x00);

	cpu.executeCycle();
	cpu.executeCycle();
	cpu.executeCycle();
	cpu.executeCycle();
	cpu.executeCycle();
	cpu.executeCycle();


	return 0;
}