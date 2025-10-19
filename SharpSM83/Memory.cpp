#include "Memory.h"
#include "SM83.h"
#include <stdio.h>
#include <iostream>

#include "input.h"
#include <stdio.h>
#include <fstream>


Memory::Memory() : input(nullptr)
{
	memset(rom, 0, sizeof(rom));
	memset(vram, 0, sizeof(vram));
	memset(ram, 0, sizeof(ram));
	memset(wram, 0, sizeof(wram));
	memset(oam, 0, sizeof(oam));
	memset(io, 0, sizeof(io));
	memset(hram, 0, sizeof(hram));
	IE = 0;
}

Memory::~Memory()
{
	if (mbc) delete mbc;
	if (fullROM) delete[] fullROM;
}

uint8_t Memory::read(uint16_t addr)
{

	if (addr < 0x8000)
	{
		return mbc->readROM(addr);
	}

	else if (addr < 0xA000)
	{
		return vram[addr - 0x8000];
	}

	else if (addr < 0xC000)
	{
		return mbc->readRAM(addr);
	}

	else if (addr < 0xE000)
	{
		return wram[addr - 0xC000];
	}

	else if (addr < 0xFE00)
	{
		return wram[addr - 0xE000];
	}
	else if (addr < 0xFEA0)
	{
		return oam[addr - 0xFE00];
	}

	else if (addr < 0xFF00)
	{
		return 0xFF;
	}

	else if (addr < 0xFF80)
	{
		if (addr == 0xFF00 && input)
		{
			uint8_t joyp = io[0x00];
			uint8_t result = 0x0F;
			bool selectBtn = (joyp & 0x20) == 0;
			bool selectDpad = (joyp & 0x10) == 0;
			if (selectBtn) result &= input->buttons;
			if (selectDpad) result &= input->dpad;
			return 0xC0 | (joyp & 0x30) | (result & 0x0F);
		}
		return io[addr - 0xFF00];
	}

	else if (addr < 0xFFFF)
	{
		return hram[addr - 0xFF80];
	}

	else
	{
		return IE;
	}
}

void Memory::write(uint16_t addr, uint8_t val)
{

	if (addr < 0x8000)
	{
		mbc->writeROM(addr, val);
	}
	else if (addr < 0xA000)
	{
		vram[addr - 0x8000] = val;
	}

	else if (addr < 0xC000)
	{
		mbc->writeRAM(addr, val);
	}

	else if (addr < 0xE000)
	{
		wram[addr - 0xC000] = val;
	}

	else if (addr < 0xFE00)
	{
		wram[addr - 0xE000] = val;
	}

	else if (addr < 0xFEA0)
	{
		oam[addr - 0xFE00] = val;
	}

	else if (addr < 0xFF00)
	{
		return;
	}
	else if (addr < 0xFF80)
	{
		if (addr == 0xFF04)
		{
			io[0x04] = 0;
		}
		else if (addr == 0xFF44)
		{
			return;
		}
		else if (addr == 0xFF46)
		{
			uint16_t source = val << 8;
			for (int i = 0; i < 0xA0; i++)
			{
				oam[i] = read(source + i);
			}
		}
		else
		{
			io[addr - 0xFF00] = val;
		}
	}

	else if (addr < 0xFFFF)
	{
		hram[addr - 0xFF80] = val;
	}
	else
	{
		IE = val;
	}
}

bool Memory::loadROM(const char* filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file) return false;

	romSize = (uint32_t)file.tellg();
	file.seekg(0, std::ios::beg);

	fullROM = new uint8_t[romSize];
	file.read((char*)fullROM, romSize);
	file.close();

	uint8_t mbcType = fullROM[0x0147];
	uint8_t ramSizeCode = fullROM[0x0149];

	uint32_t ramSize = 0;
	switch (ramSizeCode)
	{
	case 0x00: ramSize = 0; break;
	case 0x01: ramSize = 2 * 1024; break;
	case 0x02: ramSize = 8 * 1024; break;
	case 0x03: ramSize = 32 * 1024; break;
	case 0x04: ramSize = 128 * 1024; break;
	case 0x05: ramSize = 64 * 1024; break;
	}
	switch (mbcType)
	{
	case 0x00: // No MBC
	mbc = new NoMBC(fullROM);
	std::cout << "No MBC" << std::endl;
	break;
	case 0x01: // MBC1
	case 0x02: // MBC1+RAM
	case 0x03: // MBC1+RAM+BATTERY
	mbc = new MBC1(fullROM, romSize, ramSize);
	std::cout << "MBC1" << std::endl;
	break;
	case 0x0F: // MBC3+TIMER+BATTERY
	case 0x10: // MBC3+TIMER+RAM+BATTERY
	case 0x11: // MBC3
	case 0x12: // MBC3+RAM
	case 0x13: // MBC3+RAM+BATTERY
	mbc = new MBC3(fullROM, romSize, ramSize);
	std::cout << "MBC3" << std::endl;
	break;
	case 0x19: // MBC5
	case 0x1A: // MBC5+RAM
	case 0x1B: // MBC5+RAM+BATTERY
	case 0x1C: // MBC5+RUMBLE
	case 0x1D: // MBC5+RUMBLE+RAM
	case 0x1E: // MBC5+RUMBLE+RAM+BATTERY
	mbc = new MBC5(fullROM, romSize, ramSize);
	std::cout << "MBC5" << std::endl;
	break;
	default:
	std::cout << "UNSOPPORTED MBC: " << std::hex << (int)mbcType << std::dec << std::endl;
	mbc = new NoMBC(fullROM);
	break;
	}

	std::cout << "ROM loaded: " << romSize << " bytes, RAM: " << ramSize << " bytes" << std::endl;
	return true;
}