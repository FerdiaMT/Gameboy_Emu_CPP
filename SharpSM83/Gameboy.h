#ifndef GAMEBOY_H
#define GAMEBOY_H

#include "memory.h"
#include "sm83.h"
#include "ppu.h"
#include "clock.h"
#include "input.h"

class Gameboy {
public:
    Memory memory;
    SM83 cpu;
    PPU ppu;
    Clock timer;

    Gameboy();
    bool loadROM(const char* filename);
    void step();
};

#endif // GAMEBOY_H