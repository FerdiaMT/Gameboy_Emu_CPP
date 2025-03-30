#ifndef PPU_H
#define PPU_H

#include <cstdint>
#include "Memory.h"
#include "SM83.h"
class PPU
{
public:
	PPU(Memory& memory);
	void resetPPU();

	void executeFrame();

	

private:

	Memory& memory;

};
#endif