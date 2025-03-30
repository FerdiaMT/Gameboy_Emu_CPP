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
	void executeTick(int allCycles);
	

private:

	Memory& memory;

	void mode2Tick();
	void mode3Tick();
	void mode0Tick();
	void mode1Tick();

	void fetchTileNo();
	void fetchTileL();
	void fetchTileH();
	void fifoPush();


};
#endif