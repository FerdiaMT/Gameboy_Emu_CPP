#include "SM83.h"
#include "Memory.h"
#include "PPU.h"
#include "Clock.h"
#include <fstream>

#include <stdio.h>
#include <iostream>
#include <cstdint>
#include <SDL3/sdl.h>
#include <string> 
#include <thread>
#include <chrono>



bool running = true;

SDL_Window* window;
SDL_Renderer* renderer;

const int WIDTH = 160;
const int HEIGHT = 144;
const int SCALE = 5;


uint8_t pixelState[160][144]{}; // for now this can be 0,1,2,3,4
bool keyboardState[16]{};

/*
Todo:

    OAM transfer 
    STAT reg
    replace view with read in cpu
*/

void setupSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not initialize" << SDL_GetError();
        return;
    }

    window = SDL_CreateWindow("DMG", WIDTH * SCALE, HEIGHT * SCALE, SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        std::cerr << "Window couldnt be created " << SDL_GetError();
        return;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer)
    {
        std::cerr << "Renderer couldnt be created " << SDL_GetError();
        SDL_Quit;
        return;
    }
}

void setPixelState(Memory& memory) //do this later
{

}

void drawAllPixels(Memory& memory)
{
    for (int x = 0; x < WIDTH; x++)
    {
        for (int y = 0; y < HEIGHT; y++)
        {
            int c{};
            switch (pixelState[x][y]) // each of these can be either 0,1,2 or 3 (11 - white, 10 - 33%, 01 - 66%, 00 = 100%)
            {
            case(3): {
                c = 255;
            }break;
            case(2): {
                c = 170;
            }break;
            case(1): {
                c = 85;
            }break;
            case(0): {
                c = 0; // black
            }break;

            }
            
            SDL_SetRenderDrawColor(renderer, c, c,c, 255);
            SDL_FRect pixel = { x * SCALE, y * SCALE, SCALE, SCALE };//x,y,w,h
            SDL_RenderFillRect(renderer, &pixel);
        }
    }

    SDL_RenderPresent(renderer);
}


uint8_t bootROM[256] = {
    0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,
0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50
};

uint8_t cartROM[0x30] = {
    0xE, 0xED, 0x66 ,0x66 ,0xCC ,0x0D ,0x00 ,0x0B, 0x03 ,0x73, 0x00 ,0x83 ,0x00,0x0C,0x00, 0x0D,
    0x00, 0x08 ,0x11 ,0x1F ,0x88 ,0x89, 0x00, 0x0E, 0xDC ,0xCC ,0x6E ,0xE6 ,0xDD, 0xDD, 0xD9 ,0x99,
    0xBB, 0xBB, 0x67 ,0x63, 0x6E, 0x0E ,0xEC ,0xCC ,0xDD ,0xDC, 0x99 ,0x9F, 0xBB, 0xB9, 0x33, 0x3E
};


double cyclesPerFrame = 70224;
double cpuCyclesPerFrame = 4194304.0 / 59.7;


/*
*
* Things to make :
*
* Cartridge
* PPU
* timer
* screen mabye ?
* input
* interrupts as seperate class, mabye
*/

int main()
{
	Memory memory;

	SM83 cpu(memory);

    PPU ppu(memory);

    Clock clock(memory);

	cpu.reset();
    memory.reset();
    ppu.resetPPU();
    clock.resetClock();
   
    //DEBUGS TO GET WORKING: -, 2, - , - , - , - , - , -, - , - , -
    std::ifstream file("cpu_instrs.gb", std::ios::binary | std::ios::ate);

    bool skipBootROM = true; 

    if (file.is_open())
    {

        std::streampos size = file.tellg();
        char* buffer = new char[size];

        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        for (int i = 0; i < size; ++i)
        {
           memory.write(i, buffer[i]);
        }
        delete[] buffer;
        std::cout << "done loading memory" << std::endl;
    }

    for (uint16_t i = 0; i < 256; i++) {
        memory.write(i, bootROM[i]);
    }

    for (uint16_t i = 0x104; i < 0x133; i++)
    {
        memory.write(i, cartROM[i - 0x104]);
    }

    if (skipBootROM) {

        cpu.debugRegs();

        memory.write(0xFF05, 0x00); 
        memory.write(0xFF06, 0x00); 
        memory.write(0xFF07, 0x00); 
        memory.write(0xFF10, 0x80);
        memory.write(0xFF11, 0xBF); 
        memory.write(0xFF12, 0xF3); 
        memory.write(0xFF14, 0xBF); 
        memory.write(0xFF16, 0x3F); 
        memory.write(0xFF17, 0x00); 
        memory.write(0xFF19, 0xBF);  
        memory.write(0xFF1A, 0x7F); 
        memory.write(0xFF1B, 0xFF); 
        memory.write(0xFF1C, 0x9F);
        memory.write(0xFF1E, 0xBF); 
        memory.write(0xFF20, 0xFF);
        memory.write(0xFF21, 0x00);
        memory.write(0xFF22, 0x00);
        memory.write(0xFF23, 0xBF); 
        memory.write(0xFF24, 0x77);
        memory.write(0xFF25, 0xF3);
        memory.write(0xFF26, 0xF1);
        memory.write(0xFF40, 0x91);
        memory.write(0xFF42, 0x00);
        memory.write(0xFF43, 0x00);
        memory.write(0xFF45, 0x00);
        memory.write(0xFF47, 0xFC);
        memory.write(0xFF48, 0xFF);
        memory.write(0xFF49, 0xFF); 
        memory.write(0xFF4A, 0x00); 
        memory.write(0xFF4B, 0x00); 
        memory.write(0xFF50, 0x01); 
    }

    setupSDL();
    SDL_Event event;

    //going to do this without the whole clock class for now

    int allCycles{};

    auto lastCycleTime = std::chrono::high_resolution_clock::now();

    bool renderFrame = false;

    while(running)
    {

        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();
        
        if (dt >= 16.73) // once every 60 seconds (16.73 milliseconds)
        {
            int waitCycle = allCycles - 17573;
            if (waitCycle > 0)
            {
                using namespace std::chrono_literals;
                //for each cycle too fast, wait 952ns
                auto wait = (waitCycle) * 952ns;
                std::this_thread::sleep_for(wait);
            }
            ppu.updateScreenBuffer(pixelState);
            drawAllPixels(memory);
           
            lastCycleTime = currentTime;
            allCycles = 0;
            cpu.cycles = 0; // i know this is bad practice, but this is just demoing my idea
        }

        allCycles++;

        cpu.executeCycle(allCycles);

        clock.handleTimers(allCycles);

        ppu.executeTick(allCycles);

        while (SDL_PollEvent(&event))
        {
            memset(keyboardState, 0, sizeof(keyboardState));

            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                
            }
        }

       // SDL_Delay(1000);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

	return 0;
}