#ifndef CLOCK_H
#define CLOCK_H

#include <cstdint>
#include "Memory.h"
#include "SM83.h"
class Clock
{
public:
	Clock(Memory& memory);
	void resetClock();
	void handleTimers(int allCycles);

private:
	Memory& memory;

	void fetchTac();

};
#endif