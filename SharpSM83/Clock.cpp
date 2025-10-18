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

Clock::Clock(Memory & memory) : memory(memory) // 1 machine cycle = 4 clock cycles (t)
{
    // basically we want to advance clock cycles, and then see what we should do
}

void Clock::resetClock()
{
}

int modulo{};

uint8_t Tac{};

void Clock::fetchTac()
{
    Tac = memory.ioFetchTAC();
}

int tacModulo()
{
    if (Tac & 0b00)
    {
        return 256;
    }
    if (Tac & 0b01)
    {
        return 4;
    }
    if (Tac & 0b10)
    {
        return 16;
    }
    return 64;
}

int clockCycle{};


void Clock::resetClockCycle()
{
    //clockCycle = 0;
}


int timaReloadDelay = -1;
bool timaWillReload = false;


void Clock::step(int amt) {
    for (int i = 0; i < amt; i++) {
        executeTick();
    }
}

void Clock::executeTick()
{
    uint16_t prevDivCounter = divCounter;
    divCounter++;
    memory.ioWriteDIV(static_cast<uint8_t>(divCounter >> 8));

    fetchTac();

    if (Tac & 0b100)
    {
        int bit = 0;
        switch (Tac & 0b11)
        {
        case 0: bit = 9; break;
        case 1: bit = 3; break;
        case 2: bit = 5; break;
        case 3: bit = 7; break;
        }

        bool prevBit = ((prevDivCounter >> bit) & 1) != 0;
        bool currBit = ((divCounter >> bit) & 1) != 0;

        if (prevBit && !currBit)
        {
            if (!timaWillReload)
            {
                uint8_t tima = memory.ioFetchTIMA();
                if (tima == 0xFF)
                {
                    memory.ioWriteTIMA(0x00);
                    timaReloadDelay = 4;
                    timaWillReload = true;
                }
                else
                {
                    memory.ioIncrementTIMA();
                }
            }
        }
    }


    if (timaReloadDelay >= 0)
    {
        timaReloadDelay--;
        if (timaReloadDelay == 0)
        {
            memory.ioWriteTIMA(memory.ioFetchTMA());
            memory.requestInterrupt(0x04);
            timaWillReload = false;
            timaReloadDelay = -1;
        }
    }

}