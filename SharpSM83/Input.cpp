#include "input.h"
#include "memory.h"
#include <iostream>

Input::Input(Memory* memory) : memory(memory), buttons(0x0F), dpad(0x0F)
{
}

void Input::update()
{
	uint8_t joyp = memory->read(0xFF00);
	uint8_t result = 0x0F;

	bool selectButtons = (joyp & 0x20) == 0;
	bool selectDpad = (joyp & 0x10) == 0;

	if (selectButtons) result &= buttons;
	if (selectDpad) result &= dpad;

	uint8_t final = 0xC0 | (joyp & 0x30) | (result & 0x0F);
	memory->io[0x00] = final;
}


void Input::keyDown(SDL_Scancode key)
{
	bool wasPressed = false;

	switch (key)
	{
	case SDL_SCANCODE_Z: buttons &= ~0x01; wasPressed = true; break;
	case SDL_SCANCODE_X: buttons &= ~0x02; wasPressed = true; break;
	case SDL_SCANCODE_C: buttons &= ~0x08; wasPressed = true; break;
	case SDL_SCANCODE_V: buttons &= ~0x04; wasPressed = true; break;
	case SDL_SCANCODE_RIGHT: dpad &= ~0x01; wasPressed = true; break;
	case SDL_SCANCODE_LEFT: dpad &= ~0x02; wasPressed = true; break;
	case SDL_SCANCODE_UP: dpad &= ~0x04; wasPressed = true; break;
	case SDL_SCANCODE_DOWN: dpad &= ~0x08; wasPressed = true; break;
	}

	if (wasPressed)
	{
		std::cout << "KEY DOWN - buttons: 0x" << std::hex << (int)buttons
			<< " dpad: 0x" << (int)dpad << std::dec << std::endl;
		update();
		uint8_t IF = memory->read(0xFF0F);
		memory->write(0xFF0F, IF | 0x10);
	}
}
void Input::keyUp(SDL_Scancode key)
{
	switch (key)
	{
	case SDL_SCANCODE_Z: buttons |= 0x01; break;
	case SDL_SCANCODE_X: buttons |= 0x02; break;
	case SDL_SCANCODE_C: buttons |= 0x08; break;
	case SDL_SCANCODE_V: buttons |= 0x04; break;
	case SDL_SCANCODE_RIGHT: dpad |= 0x01; break;
	case SDL_SCANCODE_LEFT: dpad |= 0x02; break;
	case SDL_SCANCODE_UP: dpad |= 0x04; break;
	case SDL_SCANCODE_DOWN: dpad |= 0x08; break;
	}

	update();
}
