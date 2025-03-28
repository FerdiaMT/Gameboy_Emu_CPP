#ifndef PPU_H
#define PPU_H

#include <cstdint>
#include "Memory.h"
#include "SM83.h"
class Clock
{
public:
	Clock(Memory& memory);
	void resetClock();
private:


};
#endif