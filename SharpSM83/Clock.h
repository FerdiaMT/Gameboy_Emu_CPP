#ifndef CLOCK_H
#define CLOCK_H

#include <cstdint>
#include "Memory.h"
#include "SM83.h"
class Clock
{
public:
	Memory* memory;
	Clock(Memory* memory);


	void step(int amt);

	int divCounter;
	int timaCounter;
private:


};
#endif