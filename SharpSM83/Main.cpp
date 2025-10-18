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
SDL_Texture* texture;


SDL_Window* window2;
SDL_Renderer* renderer2;

SDL_Window* window3;
SDL_Renderer* renderer3;

const int WIDTH = 160;
const int HEIGHT = 144;
const int SCALE = 5;


uint8_t pixelState[160][144]{}; // for now this can be 0,1,2,3,4
bool keyboardState[6]{true,true,true,true,true,true};

/*
Todo:

    OAM transfer 
    STAT reg
    replace view with read in cpu
*/
uint8_t bootRO[256] = {
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

uint8_t headerRO[0x30] = {
    0xCE, 0xED, 0x66 ,0x66 ,0xCC ,0x0D ,0x00 ,0x0B, 0x03 ,0x73, 0x00 ,0x83 ,0x00,0x0C,0x00, 0x0D,
    0x00, 0x08 ,0x11 ,0x1F ,0x88 ,0x89, 0x00, 0x0E, 0xDC ,0xCC ,0x6E ,0xE6 ,0xDD, 0xDD, 0xD9 ,0x99,
    0xBB, 0xBB, 0x67 ,0x63, 0x6E, 0x0E ,0xEC ,0xCC ,0xDD ,0xDC, 0x99 ,0x9F, 0xBB, 0xB9, 0x33, 0x3E
};



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

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_STREAMING, 160, 144);

    if (!texture)
    {
        std::cerr << "texture couldnt be created " << SDL_GetError();
        SDL_Quit;
        return;
    }

}

void setupSDL2()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not initialize" << SDL_GetError();
        return;
    }

    window2 = SDL_CreateWindow("ABC", 8*16 * SCALE, 8*24 * SCALE, SDL_WINDOW_RESIZABLE);
    if (!window2)
    {
        std::cerr << "Window couldnt be created " << SDL_GetError();
        return;
    }

    renderer2 = SDL_CreateRenderer(window2, NULL);
    if (!renderer2)
    {
        std::cerr << "Renderer couldnt be created " << SDL_GetError();
        SDL_Quit;
        return;
    }
}

void setupSDL3()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not initialize" << SDL_GetError();
        return;
    }

    window3 = SDL_CreateWindow("DEF", (8 * 32 * SCALE), (8 * 32 * SCALE), SDL_WINDOW_RESIZABLE);
    if (!window3)
    {
        std::cerr << "Window couldnt be created " << SDL_GetError();
        return;
    }

    renderer3 = SDL_CreateRenderer(window3, NULL);
    if (!renderer3)
    {
        std::cerr << "Renderer couldnt be created " << SDL_GetError();
        SDL_Quit;
        return;
    }
}

void drawTextureWindow(Memory& memory) // going to use this to draw out the textures
{

    uint8_t c = 0;
    int level = 0x8000;

    for (int th = 0; th < 24; th++) {
        for (int tw = 0; tw < 16; tw++) {

            for (int y = 0; y < 8; y++) {

                uint8_t row1 = memory.readPPU(level + (y * 2) + (tw * 16) + (th * 16 * 8 * 2));
                uint8_t row2 = memory.readPPU(level + (y * 2) + 1 + (tw * 16) + (th * 16 * 8*2));

                for (int x = 0; x < 8; x++) {

                    uint8_t mainBit = (row2 >> (7 - x) & 0b1) << 1 | (row1 >> (7 - x) & 0b1);
                    if (mainBit == 0b11)
                    {
                        c = 0;
                    }
                    else
                    {
                        c = 255 - (mainBit * 100);
                    }
                    SDL_SetRenderDrawColor(renderer2, c, c, c, 255);
                    SDL_FRect pixel = { (SCALE * tw*8) + (x * SCALE),  (SCALE * th * 8) + (y * SCALE), SCALE, SCALE};//x,y,w,h
                    SDL_RenderFillRect(renderer2, &pixel);
                }
            }
        }
    }

    SDL_RenderPresent(renderer2);
}

static SDL_Texture* pixelTexture = nullptr;
static uint32_t* pixelBuffer = nullptr;
static bool pixelRendererInitialized = false;

bool initPixelRenderer(SDL_Renderer* renderer) {

    if (pixelRendererInitialized) {
        if (pixelTexture) SDL_DestroyTexture(pixelTexture);
        if (pixelBuffer) delete[] pixelBuffer;
    }

    pixelTexture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH,
        HEIGHT
    );
    SDL_SetTextureScaleMode(pixelTexture, SDL_SCALEMODE_NEAREST);
    pixelBuffer = new uint32_t[WIDTH * HEIGHT];
    //for (int i = 0; i < WIDTH * HEIGHT; i++) {
    //    pixelBuffer[i] = 0xFFFFFFFF;
    //}
    pixelRendererInitialized = true;
    return true;
}

static const uint32_t colors[4] = {
    0xFFFFFFFF,
    0x9B9B9BFF,
    0x373737FF,
    0x000000FF
};


void drawAllPixels(Memory& memory) {


    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            pixelBuffer[y * WIDTH + x] = colors[pixelState[x][y]];
        }
    }
    SDL_UpdateTexture(pixelTexture, nullptr, pixelBuffer, WIDTH * sizeof(uint32_t));
    SDL_FRect destRect = {
        0.0f, 0.0f,
        static_cast<float>(WIDTH * SCALE),
        static_cast<float>(HEIGHT * SCALE)
    };

    SDL_RenderTexture(renderer, pixelTexture, nullptr, &destRect);
    SDL_RenderPresent(renderer);
}

static void drawMapA(Memory& memory)
{
    uint8_t c = 0;
    uint16_t memAdress = 0x9800; // this can also be 9c00 // goes up to 9bFF // 9800

    uint16_t memAdress2 = 0x8000;
    for (int ty = 0; ty < 32; ty++) {
        for (int tx = 0; tx < 32; tx++) {
            uint16_t tileNumber = (memory.readPPU((memAdress + tx + (ty * 32))) * 0x10);
            for (int y = 0; y < 8; y++) {

                uint8_t row1 = memory.readPPU(memAdress2 + tileNumber + (y * 2));
                uint8_t row2 = memory.readPPU(memAdress2 + tileNumber + (y * 2)+1);

                for (int x = 0; x < 8; x++) {

                    uint8_t mainBit = (row2 >> (7 - x) & 0b1) << 1 | (row1 >> (7 - x) & 0b1);
                    if (mainBit == 0b11)
                    {
                        c = 0;
                    }
                    else
                    {
                        c = 255 - (mainBit * 100);
                    }

                    SDL_SetRenderDrawColor(renderer3, c, c, c, 255);
                    SDL_FRect pixel2 = { (SCALE * tx * 8) + (x * SCALE),  (SCALE * ty * 8) + (y * SCALE), SCALE, SCALE };//x,y,w,h
                    SDL_RenderFillRect(renderer3, &pixel2);
                }
              
            }
           
        }
     
    }
    SDL_RenderPresent(renderer3);
}

static void drawMapB(Memory& memory)
{
    uint8_t c = 0;
    uint16_t mapAddr = 0x9800;
    uint16_t tileDataBase = 0x9000; 

    for (int ty = 0; ty < 32; ty++) {
        for (int tx = 0; tx < 32; tx++) {

            int8_t tileNumber = (int8_t)memory.readPPU(mapAddr + tx + (ty * 32));

            uint16_t tileAddr = tileDataBase + ( (int16_t)tileNumber * 16);

            for (int y = 0; y < 8; y++) {

                uint8_t row1 = memory.readPPU(tileAddr + (y * 2));
                uint8_t row2 = memory.readPPU(tileAddr + (y * 2) + 1);

                for (int x = 0; x < 8; x++) {
                    uint8_t mainBit = ((row2 >> (7 - x)) & 0b1) << 1 | ((row1 >> (7 - x)) & 0b1);

                    if (mainBit == 0b11)
                    {
                        c = 0;
                    }
                    else
                    {
                        c = 255 - (mainBit * 100);
                    }

                    SDL_SetRenderDrawColor(renderer3, c, c, c, 255);
                    SDL_FRect pixel2 = { (SCALE * tx * 8) + (x * SCALE),  (SCALE * ty * 8) + (y * SCALE), SCALE, SCALE };
                    SDL_RenderFillRect(renderer3, &pixel2);
                }
            }
        }
    }

    SDL_RenderPresent(renderer3);
}




double cyclesPerFrame = 70224;
double cpuCyclesPerFrame = 4194304.0 / 59.7;


int main()
{
    setupSDL();
    //setupSDL2(); //<- textureviewer
     //setupSDL3(); // <- map viewer

    if (!initPixelRenderer(renderer)) {
        printf("Failed to initialize pixel renderer!\n");
        return 1;
    }

    Gameboy gb;

    std::ifstream file("02.gb", std::ios::binary | std::ios::ate);

    if (file.is_open())
    {
        std::streampos size = file.tellg();
        char* buffer = new char[size];
        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        for (int i = 0; i < size; ++i){ memory.write(i, buffer[i]);}
        delete[] buffer;
        std::cout << "done loading memory" << std::endl;
    }

    int allCycles{};
    auto lastCycleTime = std::chrono::high_resolution_clock::now();
    bool renderFrame = false;
    uint16_t cpuDotsFromExec{};

   

    SDL_Event event;



    while(running)
    {

        while (SDL_PollEvent(&event))
        {

            if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
                bool isPressed = (event.type == SDL_EVENT_KEY_DOWN) ? false : true;

                switch (event.key.key) {

                case SDLK_UP:
                    keyboardState[0] = isPressed;
                    break;
                case SDLK_DOWN:
                    keyboardState[1] = isPressed;
                    break;
                case SDLK_LEFT:
                    keyboardState[2] = isPressed;
                    break;
                case SDLK_RIGHT:
                    keyboardState[3] = isPressed;
                    break;
                case SDLK_Z: // start
                    keyboardState[4] = isPressed;
                    break;
                case SDLK_X: // select
                    keyboardState[5] = isPressed;
                    break;
                }
            }
        }

        for(int i = 0; i < 70224; i+= gb.cpu.cycles)
        {
            gb.step();
        }

        SDL_UpdateTexture(texture, nullptr, gb.ppu.framebuffer, 160 * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        SDL_Delay(16); // this delays it enough to act as 60fps

    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

	return 0;
}