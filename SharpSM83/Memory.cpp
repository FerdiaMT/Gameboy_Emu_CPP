#include "Memory.h"
#include "SM83.h"
#include <stdio.h>
#include <iostream>




#define ROM_LB 0x0000
#define ROM_UB 0x3FFF

#define ROMBANK_LB 0x4000
#define ROMBANK_UB 0x7FFF

#define VRAM_LB 0x8000
#define VRAM_UB 0x9FFF

#define CARTRAM_LB 0xA000
#define CARTRAM_UB 0xBFFF

#define WRAM_LB 0xC000
#define WRAM_UB 0xDFFF

// not going to define echo ram for now

#define OAM_LB  0xFE00
#define OAM_UB  0xFE9F

#define IO_LB   0xFF00
#define IO_UB   0xFF7F

#define HR_LB   0xFF80
#define HR_UB   0xFFFE

uint8_t joypad_select = 0xFF;

/*
MAINTENANCE REQUIRED : REMOVE THE VIEW AND VIEWWORD FUNCTIONS
					   FIGURE OUT WHAT TO DO FOR RESET
*/


bool buttons[6] // false means on, true means off (weird but get used to this)
{
	true,true,true,true,true,true // start , select
};

void Memory::insertKeyboard(bool input[])
{
	std::memcpy(buttons, input, 6);
}



Memory::Memory() 
{
	initFastMap();
};

void Memory::initFastMap()
{
	for (uint16_t addr = 0x0000; addr <= 0x3FFF; ++addr)
		fastMap[addr] = &rom[addr];
	for (uint16_t addr = 0x4000; addr <= 0x7FFF; ++addr)
		fastMap[addr] = &romBank[addr - 0x4000];
	for (uint16_t addr = 0x8000; addr <= 0x9FFF; ++addr)
		fastMap[addr] = &vram[addr - 0x8000];
	for (uint16_t addr = 0xA000; addr <= 0xBFFF; ++addr)
		fastMap[addr] = &cartRam[addr - 0xA000];
	for (uint16_t addr = 0xC000; addr <= 0xDFFF; ++addr)
		fastMap[addr] = &wram[addr - 0xC000];
	for (uint16_t addr = 0xE000; addr <= 0xFDFF; ++addr)
		fastMap[addr] = &wram[addr - 0xE000];
	for (uint16_t addr = 0xFE00; addr <= 0xFE9F; ++addr)
		fastMap[addr] = &oam[addr - 0xFE00];
	for (uint16_t addr = 0xFF00; addr <= 0xFF7F; ++addr)
		fastMap[addr] = &io[addr - 0xFF00];
	for (uint16_t addr = 0xFF80; addr <= 0xFFFE; ++addr)
		fastMap[addr] = &hram[addr - 0xFF80];

	fastMap[0xFFFF] = &IE;
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

uint8_t headerROM[0x30] = {
	0xCE, 0xED, 0x66 ,0x66 ,0xCC ,0x0D ,0x00 ,0x0B, 0x03 ,0x73, 0x00 ,0x83 ,0x00,0x0C,0x00, 0x0D,
	0x00, 0x08 ,0x11 ,0x1F ,0x88 ,0x89, 0x00, 0x0E, 0xDC ,0xCC ,0x6E ,0xE6 ,0xDD, 0xDD, 0xD9 ,0x99,
	0xBB, 0xBB, 0x67 ,0x63, 0x6E, 0x0E ,0xEC ,0xCC ,0xDD ,0xDC, 0x99 ,0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

uint8_t Memory::read(uint16_t address)
{

	if (io[0x50] == 0) // if boot rom is on
	{
		if (address < 256) 
		{
			return bootROM[address];
		}
		if (address >= 0x103 && address <= 0x133)
		{
			return headerROM[address - 0x104];
		}
	}

	if (vramLocked && ((ioFetchLCDC() & 0x80) == 0))
	{
		if (address > VRAM_LB && address < VRAM_UB)
		{
			std::cout << "Bad read" << std::endl;
			return 0xFF;
		}
	}

	//if (address == 0xFF44)
	//{
	//	return 0x90;
	//}


	if (address == 0xFF00) {

		//std::cout << (int)buttons[0] << " " << (int)buttons[1] << " " << (int)buttons[2] << " " << (int)buttons[3] << " " << (int)buttons[4] << " " << (int)buttons[5] << std::endl;

		uint8_t result = joypad_select |0xCF ;

		if (!(joypad_select & 0x10)) 
		{
			if (buttons[0]) result &= ~0x04; 
			if (buttons[1]) result &= ~0x08; 
			if (buttons[2]) result &= ~0x02; 
			if (buttons[3]) result &= ~0x01; 
		}

		if (!(joypad_select & 0x20)) 
		{
			if (buttons[4]) result &= ~0x08; 
			if (buttons[5]) result &= ~0x04;
		}

		return result;
	}

	return *fastMap[address];
}

uint8_t Memory::readPPU(uint16_t address) 
{

	return *fastMap[address];
}

uint16_t Memory::readWord(uint16_t address)
{
	uint16_t low = read(address);
	address++;
	uint16_t high = read(address);
	return (high << 8) | low;
}

void Memory::write(uint16_t address, uint8_t data)
{

	if (address <= ROM_UB)
	{
		rom[address] = data;
	}
	else if (address <= ROMBANK_UB)
	{
		romBank[address - ROMBANK_LB] = data;
	}
	else if (address <= VRAM_UB)
	{
		if (vramLocked)
		{
			std::cout << "badWrite" << std::endl;
			return;
		}
		
		vram[address - VRAM_LB] = data;
	}
	else if (address <= CARTRAM_UB)
	{
		cartRam[address - CARTRAM_LB] = data;
	}
	else if (address <= WRAM_UB)
	{
		wram[address - WRAM_LB] = data;
	}
	else if (address <= OAM_UB)
	{
		if (vramLocked) return;

		oam[address - OAM_LB] = data;

	}
	else if (address <= IO_UB) // i should have no return
	{


		if (address == 0xFF02 && data == 0x81) {
			char c = read(0xFF01);
			std::cout << c; 
		}

		if (address == 0xFF46) //OAM DMA START
		{
			//writing here starts transfer of rom to oam
			//supposed to take 160 machine cycles ? how do i do this
			dmaPending = true;

			//Source:      $XX00 - $XX9F;XX = $00 to $DF
			//Destination : $FE00 - $FE9F
			uint16_t endAddr = 0xFE00;
			for (uint16_t i = (data << 2); i <= ((data << 2) | 0x9F); i++) // from source start to source end
			{
				write(endAddr, read(i));
				endAddr++;
			}

			
		}

		if (address == 0xFF00) {
			joypad_select = data & 0x30; //this is a bit messy, make clean later
			return;
		}


		io[address - IO_LB] = data;

	}
	else if (address <= HR_UB)
	{
		hram[address - HR_LB] = data;
	}
	else if (address == 0xFFFF)
	{
		 IE = data;
	}
	else
	{
		std::cout << "TRIED TO WRITE TO" <<std::hex <<  (int)address << std::endl;
		//something went really wrong if it gets to here
	}

}

void Memory::writePPU(uint16_t address, uint8_t data)
{

	if (address <= ROM_UB)
	{
		rom[address] = data;
	}
	else if (address <= ROMBANK_UB)
	{
		romBank[address - ROMBANK_LB] = data;
	}
	else if (address <= VRAM_UB)
	{
		vram[address - VRAM_LB] = data;
	}
	else if (address <= CARTRAM_UB)
	{
		cartRam[address - CARTRAM_LB] = data;
	}
	else if (address <= WRAM_UB)
	{
		wram[address - WRAM_LB] = data;
	}
	else if (address <= OAM_UB)
	{
		//if (vramLocked) return;

		oam[address - OAM_LB] = data;

	}
	else if (address <= IO_UB) // i should have no return
	{
		if (address == 0xFF46) //OAM DMA START
		{
			//writing here starts transfer of rom to oam
			//supposed to take 160 machine cycles ? how do i do this
			dmaPending = true;

			//Source:      $XX00 - $XX9F;XX = $00 to $DF
			//Destination : $FE00 - $FE9F
			uint16_t endAddr = 0xFE00;
			for (uint16_t i = (data << 2); i <= ((data << 2) | 0x9F); i++) // from source start to source end
			{
				write(endAddr, read(i));
				endAddr++;
			}


		}

		io[address - IO_LB] = data;

	}
	else if (address <= HR_UB)
	{
		hram[address - HR_LB] = data;
	}
	else if (address == 0XFFFF) {
		IE = data;
	}
	else
	{
		std::cout << "TRIED TO WRITE TO" << (int)address << std::endl;
		//something went really wrong if it gets to here
	}

}

void Memory::writeWord(uint16_t address, uint16_t data)
{
	write(address,(data & 0xFF));
	write(address+1, (data >> 8) & 0x00FF);
}



void Memory::reset()
{

}

//THE 0 PAGE HAS DIRECT FETCHES , TO BETTER EMULATE GAMRBOY FETCH SPEEDS

uint8_t Memory::ioFetchJOYP()
{
	return io[0];
}

uint8_t Memory::ioFetchSB()
{
	return io[1];
}

uint8_t Memory::ioFetchSC()
{
	return io[2];
}

uint8_t Memory::ioFetchDIV()
{
	return io[4];
}

uint8_t Memory::ioFetchTIMA()
{
	return io[5];
}

uint8_t Memory::ioFetchTMA()
{
	return io[6];
}

uint8_t Memory::ioFetchTAC()
{
	return io[7];
}

uint8_t Memory::ioFetchIF()
{
	return io[0x0F];
}

uint8_t Memory::ioFetchIE()
{
	return IE;
}


uint8_t Memory::ioFetchLCDC()
{
	return io[0x40];
}

uint8_t Memory::ioFetchSCY()
{
	return io[0x42];
}
uint8_t Memory::ioFetchSCX()
{
	return io[0x43];
}

uint8_t Memory::ioFetchWY()
{
	return io[0x4A];
}

uint8_t Memory::ioFetchWX()
{
	return io[0x4B];
}



uint8_t Memory::ioFetchLY()
{
	return io[0x44];
}

void Memory::ioIncrementLY()
{
	io[0x44]++;
}

void Memory::ioWriteJOYP(uint8_t data)
{
	io[0x0] = data;
}

void Memory::ioWriteSB(uint8_t data)
{
	io[0x1] = data;
}

void Memory::ioWriteSC(uint8_t data)
{
	io[0x2] = data;
}

void Memory::ioWriteDIV(uint8_t data)
{
	io[0x4] = data;
}

void Memory::ioWriteTIMA(uint8_t data)
{
	io[0x5] = data;
}

void Memory::ioWriteTMA(uint8_t data)
{
	io[0x6] = data;
}

void Memory::ioWriteTAC(uint8_t data)
{
	io[0x7] = data;
}

void Memory::ioWriteIF(uint8_t data)
{
	io[0xF] = data;
}

void Memory::ioWriteLY(uint8_t data)
{
	io[0x44] = data;
}

void Memory::ioIncrementDIV()
{
	io[0x4]++;
}


void Memory::ioIncrementTIMA()
{
	io[0x5]++;
}

void Memory::setInterruptVBlank()
{
	io[0x0F] |= 0b00001;
}

void Memory::setInterruptLCD()
{
	io[0x0F] |= 0b00010;
}

void Memory::setInterruptTimer()
{
	io[0x0F] |= 0b00100;
}

void Memory::setInterruptSerial()
{
	io[0x0F] |= 0b01000;
}

void Memory::setInterruptJoypad()
{
	io[0x0F] |= 0b10000;
}





