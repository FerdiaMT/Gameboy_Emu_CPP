#ifndef INPUT_H
#define INPUT_H

#include <cstdint>
#include <SDL3/SDL.h>

class Memory;

class Input
{
public:
	Memory* memory;
	uint8_t buttons;
	uint8_t dpad;

	Input(Memory* memory);
	void update();
	void keyDown(SDL_Scancode key);
	void keyUp(SDL_Scancode key);
};

#endif // INPUT_H

