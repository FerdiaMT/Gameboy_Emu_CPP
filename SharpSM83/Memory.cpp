#include "Memory.h"
#include "SM83.h"
#include <stdio.h>
#include <iostream>

#include "input.h"
#include <stdio.h>
#include <fstream>


Memory::Memory() : input(nullptr) {
	memset(rom, 0, sizeof(rom));
	memset(vram, 0, sizeof(vram));
	memset(ram, 0, sizeof(ram));
	memset(wram, 0, sizeof(wram));
	memset(oam, 0, sizeof(oam));
	memset(io, 0, sizeof(io));
	memset(hram, 0, sizeof(hram));
	IE = 0;
}

uint8_t Memory::read(uint16_t addr) {
	if (addr < 0x8000) return rom[addr];
	else if (addr < 0xA000) return vram[addr - 0x8000];
	else if (addr < 0xC000) return ram[addr - 0xA000];
	else if (addr < 0xE000) return wram[addr - 0xC000];
	else if (addr < 0xFE00) return wram[addr - 0xE000];
	else if (addr < 0xFEA0) return oam[addr - 0xFE00];
	else if (addr < 0xFF00) return 0xFF;
	else if (addr < 0xFF80) {

		//SPECIAL CASE FOR INPUT HANDLING
		if (addr == 0xFF00 && input) {
			uint8_t joyp = io[0x00];
			uint8_t result = 0x0F;

			bool selectBtn = (joyp & 0x20) == 0;
			bool selectDpad = (joyp & 0x10) == 0;

			if (selectBtn) {
				result &= input->buttons;
			}

			if (selectDpad) {
				result &= input->dpad;
			}

			return 0xC0 | (joyp & 0x30) | (result & 0x0F);
		}
		return io[addr - 0xFF00];

	}else if (addr < 0xFFFF) {
		return hram[addr - 0xFF80];
	}
	else return IE;
}

void Memory::write(uint16_t addr, uint8_t val) {
	if (addr < 0x8000) return; 
	else if (addr < 0xA000) vram[addr - 0x8000] = val;
	else if (addr < 0xC000) ram[addr - 0xA000] = val;
	else if (addr < 0xE000) wram[addr - 0xC000] = val;
	else if (addr < 0xFE00) wram[addr - 0xE000] = val; 
	else if (addr < 0xFEA0) oam[addr - 0xFE00] = val;
	else if (addr < 0xFF00) return; 
	else if (addr < 0xFF80) {
		if (addr == 0xFF04) io[0x04] = 0; 
		else if (addr == 0xFF44) return; 
		else if (addr == 0xFF46) {

			uint16_t source = val << 8;
			for (int i = 0; i < 0xA0; i++) {
				oam[i] = read(source + i);
			}
		}
		else io[addr - 0xFF00] = val;
	}
	else if (addr < 0xFFFF) hram[addr - 0xFF80] = val;
	else IE = val;
}

bool Memory::loadROM(const char* filename) {
	std::ifstream file(filename, std::ios::binary);
	if (!file) return false;
	file.read((char*)rom, 0x8000);
	std::cout << "ROAM LOADED " << std::endl;
	return true;
}