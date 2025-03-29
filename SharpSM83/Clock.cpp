#include "Clock.h"
#include "Memory.h"
#include <cstdint>

#include <iostream>
#include <chrono>
#include <thread>


/*
The timer uses the following memory registers:

DIV (0xFF04): the divider register increments at a fixed rate of 16,384 Hz. 
This means that we have to increment DIV’s value every 256 clock cycles (4,194,304 / 16,384 = 256).

TIMA (0xFF05): the timer register increments at a configurable frequency. 
In this case, we have to update it every 16, 64, 256 or 1024 clock cycles depending on the frequency
set in the TAC register (see how the previous clock cycle value was calculated). 
When TIMA overflows an interrupt is triggered and it’s value is reset to TMA’s value. 
TIMA should only be counting if the timer is enabled in the TAC register.

TMA (0xFF06): this value is used when TIMA overflows.

TAC (0xFF07): Timer Control, it has the following structure:
*/

// TAC :
// Bits 1-0 - Input Clock Select
//            00: 4096   Hz 
//            01: 262144 Hz
//            10: 65536  Hz
//            11: 16384  Hz
// Bit  2   - Timer Enable
// 
// Note: The "Timer Enable" bit only affects TIMA, 
// DIV is ALWAYS counting.

//4,194,304 Hz
//4194304 per clock
// 
//238.41857910156 nanosecond wait period per clock cycle

//953.674316406 nanosecond wait period per machine cycle 


double globalCycles{};
double globalMCycles{};

Clock::Clock(Memory& memory) // 1 machine cycle = 4 clock cycles (t)
{
	// basically we want to advance clock cycles, and then see what we should do
}

void Clock::increment(int& cycleDelay) 
{
	using namespace std::chrono_literals;

	//const auto start = std::chrono::high_resolution_clock::now();
	//wait 4,194,304
	std::this_thread::sleep_for(953ns);
	cycleDelay++;
}

void Clock::update()
{
	globalCycles++;
}

double Clock::getGlobalCycles()
{
	return globalCycles;
}

void Clock::updateMC()
{
	globalMCycles++;
}

double Clock::getGlobalMCycles()
{
	return globalMCycles;
}
