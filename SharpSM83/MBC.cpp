#include "MBC.h"
#include "MBC.h"
#include <cstring>

NoMBC::NoMBC(uint8_t* romData) : rom(romData) {}

uint8_t NoMBC::readROM(uint16_t addr)
{
	if (addr < 0x8000)
	{
		return rom[addr];
	}
	return 0xFF;
}

uint8_t NoMBC::readRAM(uint16_t addr)
{
	return 0xFF; // No external RAM
}

void NoMBC::writeROM(uint16_t addr, uint8_t val)
{
	// No banking, ignore writes
}

void NoMBC::writeRAM(uint16_t addr, uint8_t val)
{
	// No RAM, ignore writes
}

// ===== MBC1 =====
MBC1::MBC1(uint8_t* romData, uint32_t romSz, uint32_t ramSz)
	: rom(romData), romSize(romSz), ramSize(ramSz),
	romBank(1), ramBank(0), ramEnabled(false), bankingMode(0)
{

	if (ramSize > 0)
	{
		ram = new uint8_t[ramSize];
		memset(ram, 0, ramSize);
	}
	else
	{
		ram = nullptr;
	}
}

MBC1::~MBC1()
{
	if (ram) delete[] ram;
}

uint8_t MBC1::readROM(uint16_t addr)
{
	if (addr < 0x4000)
	{
		// Bank 0
		return rom[addr];
	}
	else if (addr < 0x8000)
	{
		// Switchable bank
		uint32_t offset = (romBank * 0x4000) + (addr - 0x4000);
		if (offset < romSize)
		{
			return rom[offset];
		}
	}
	return 0xFF;
}

uint8_t MBC1::readRAM(uint16_t addr)
{
	if (ramEnabled && ram && addr >= 0xA000 && addr < 0xC000)
	{
		uint32_t offset = (ramBank * 0x2000) + (addr - 0xA000);
		if (offset < ramSize)
		{
			return ram[offset];
		}
	}
	return 0xFF;
}

void MBC1::writeROM(uint16_t addr, uint8_t val)
{
	if (addr < 0x2000)
	{
		// RAM Enable
		ramEnabled = ((val & 0x0F) == 0x0A);
	}
	else if (addr < 0x4000)
	{
		// ROM Bank Number (lower 5 bits)
		romBank = (romBank & 0x60) | (val & 0x1F);
		if ((romBank & 0x1F) == 0)
		{
			romBank++;
		}
	}
	else if (addr < 0x6000)
	{
		// RAM Bank or upper ROM bank bits
		if (bankingMode == 0)
		{
			// ROM banking mode
			romBank = (romBank & 0x1F) | ((val & 0x03) << 5);
		}
		else
		{
			// RAM banking mode
			ramBank = val & 0x03;
		}
	}
	else if (addr < 0x8000)
	{
		// Banking mode select
		bankingMode = val & 0x01;
	}
}

void MBC1::writeRAM(uint16_t addr, uint8_t val)
{
	if (ramEnabled && ram && addr >= 0xA000 && addr < 0xC000)
	{
		uint32_t offset = (ramBank * 0x2000) + (addr - 0xA000);
		if (offset < ramSize)
		{
			ram[offset] = val;
		}
	}
}

// ===== MBC3 =====
MBC3::MBC3(uint8_t* romData, uint32_t romSz, uint32_t ramSz)
	: rom(romData), romSize(romSz), ramSize(ramSz),
	romBank(1), ramBank(0), ramEnabled(false)
{

	if (ramSize > 0)
	{
		ram = new uint8_t[ramSize];
		memset(ram, 0, ramSize);
	}
	else
	{
		ram = nullptr;
	}
}

MBC3::~MBC3()
{
	if (ram) delete[] ram;
}

uint8_t MBC3::readROM(uint16_t addr)
{
	if (addr < 0x4000)
	{
		return rom[addr];
	}
	else if (addr < 0x8000)
	{
		uint32_t offset = (romBank * 0x4000) + (addr - 0x4000);
		if (offset < romSize)
		{
			return rom[offset];
		}
	}
	return 0xFF;
}

uint8_t MBC3::readRAM(uint16_t addr)
{
	if (ramEnabled && ram && addr >= 0xA000 && addr < 0xC000)
	{
		uint32_t offset = (ramBank * 0x2000) + (addr - 0xA000);
		if (offset < ramSize)
		{
			return ram[offset];
		}
	}
	return 0xFF;
}

void MBC3::writeROM(uint16_t addr, uint8_t val)
{
	if (addr < 0x2000)
	{
		ramEnabled = ((val & 0x0F) == 0x0A);
	}
	else if (addr < 0x4000)
	{
		romBank = val & 0x7F;
		if (romBank == 0) romBank = 1;
	}
	else if (addr < 0x6000)
	{
		ramBank = val & 0x03;
	}
	// 0x6000-0x7FFF: RTC latch (not implemented)
}

void MBC3::writeRAM(uint16_t addr, uint8_t val)
{
	if (ramEnabled && ram && addr >= 0xA000 && addr < 0xC000)
	{
		uint32_t offset = (ramBank * 0x2000) + (addr - 0xA000);
		if (offset < ramSize)
		{
			ram[offset] = val;
		}
	}
}

// ===== MBC5 =====
MBC5::MBC5(uint8_t* romData, uint32_t romSz, uint32_t ramSz)
	: rom(romData), romSize(romSz), ramSize(ramSz),
	romBank(1), ramBank(0), ramEnabled(false)
{

	if (ramSize > 0)
	{
		ram = new uint8_t[ramSize];
		memset(ram, 0, ramSize);
	}
	else
	{
		ram = nullptr;
	}
}

MBC5::~MBC5()
{
	if (ram) delete[] ram;
}

uint8_t MBC5::readROM(uint16_t addr)
{
	if (addr < 0x4000)
	{
		return rom[addr];
	}
	else if (addr < 0x8000)
	{
		uint32_t offset = (romBank * 0x4000) + (addr - 0x4000);
		if (offset < romSize)
		{
			return rom[offset];
		}
	}
	return 0xFF;
}

uint8_t MBC5::readRAM(uint16_t addr)
{
	if (ramEnabled && ram && addr >= 0xA000 && addr < 0xC000)
	{
		uint32_t offset = (ramBank * 0x2000) + (addr - 0xA000);
		if (offset < ramSize)
		{
			return ram[offset];
		}
	}
	return 0xFF;
}

void MBC5::writeROM(uint16_t addr, uint8_t val)
{
	if (addr < 0x2000)
	{
		ramEnabled = ((val & 0x0F) == 0x0A);
	}
	else if (addr < 0x3000)
	{
		// Lower 8 bits of ROM bank
		romBank = (romBank & 0x100) | val;
	}
	else if (addr < 0x4000)
	{
		// 9th bit of ROM bank
		romBank = (romBank & 0xFF) | ((val & 0x01) << 8);
	}
	else if (addr < 0x6000)
	{
		ramBank = val & 0x0F;
	}
}

void MBC5::writeRAM(uint16_t addr, uint8_t val)
{
	if (ramEnabled && ram && addr >= 0xA000 && addr < 0xC000)
	{
		uint32_t offset = (ramBank * 0x2000) + (addr - 0xA000);
		if (offset < ramSize)
		{
			ram[offset] = val;
		}
	}
}