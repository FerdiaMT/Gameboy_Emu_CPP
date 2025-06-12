#include "PPU.h"
#include "SM83.h"
#include "memory.h"
#include <iostream>
#include <queue>
#include <cstring>

#define LCDC 0xFF40 // <- this is the most important one 
#define STAT 0xFF41 // lcd status
#define	SCY  0xFF42 // screen y 
#define	SCX  0xFF43 // screen x
#define LY   0xFF44 // LCD Y coordinate
#define LYC  0xFF45// LY compare
#define DMA  0xFF46 // OAM dma
#define BGP  0xFF47 // background pallete
#define OBP0 0xFF48// obj p 0
#define OBP1 0xFF49 // obj p 1
#define WY   0xFF4A// window x 
#define WX   0xFF4B // window y 

uint16_t Mem{}; //<- not sure why i added this, may delete later 



PPU::PPU(Memory& memory) : memory(memory) //m cycle (using this) = 4 t states
{

}

void PPU::resetPPU()
{

}

uint8_t screen[160][144];
uint8_t spriteBuffer[40]{}; //10 4 byte sprits
uint8_t spriteBufferPointer{};

uint16_t oamSearch = 0;
int internalTick{};
uint8_t currentLY{};

std::queue<uint8_t> backgroundFifo{};
std::queue<uint8_t>  spriteFifo{};

bool mode3Finished = false;
bool fetchedFirstTile = false;

bool windowTriggeredThisScanline = false;
bool windowEnabled = false; 
int mode3MCycles{};
int mode3TCycles{};

uint8_t currentTileI{};

int pixelsX{};
uint8_t lcdc;
uint8_t windowLine{};
uint8_t lowerByte{};
uint8_t upperByte{};
uint8_t dotDelay{};
uint8_t dotDelayBuffer{};

int curModeDebugTool{};

void PPU::writeIntoSTAT(uint8_t mode)
{
	uint8_t val = memory.readPPU(0xFF41);
	val = (val & 0b11111100) | mode; 
	memory.writePPU(0xFF41, val);

	if (mode == 0b00 && ((val & 0b1000) >> 3 == 1)) //mode 0 trigger
	{
		uint8_t temp = memory.ioFetchIF();
		memory.ioWriteIF(temp |= 0b10);
	}
	else if (mode == 0b01 && ((val & 0b10000) >> 4 == 1)) //mode 1 trigger
	{
		uint8_t temp = memory.ioFetchIF();
		memory.ioWriteIF(temp |= 0b10);
	}
	else if (mode == 0b10 && ((val & 0b100000) >> 5 == 1)) //mode 2 trigger
	{
		uint8_t temp = memory.ioFetchIF();
		memory.ioWriteIF(temp |= 0b10);
	}
	else if (mode == 0b11 && ((val & 0b1000000) >> 6 == 1)) //mode 3 trigger
	{
		uint8_t temp = memory.ioFetchIF();
		memory.ioWriteIF(temp |= 0b10);
	}

	if ((val & 0b100) >> 2 == (val & 0b1000000) >> 6) // LY = LYC
	{
		uint8_t temp = memory.ioFetchIF();
		memory.ioWriteIF(temp |= 0b10);
	}

}

void PPU::mode2Tick() // search for objs on this current line 
{ // each sprite takes up 4 bytes 

	if (spriteBufferPointer > 36) return;

	uint8_t spriteY = memory.readPPU(0xFE00 + oamSearch); //FFE0

	uint8_t currentModeHeight = 8;

	if (memory.ioFetchLCDC() & 0b00000100) //grab if its in tall mode or not 
	{
		currentModeHeight = 16; // make this swap later 
	}
	

	if (currentLY + 16 >= spriteY && currentLY + 16 <= spriteY + currentModeHeight) // this is the byte 0
	{
		// add this entire sprite to the buffer
		uint8_t spriteX = memory.readPPU(0xFE01 + oamSearch);
		if (spriteX > 8 && spriteX < 168) // not sure if this means 0 as in 0 or 8
		{
			spriteBuffer[spriteBufferPointer++] = spriteY;
			spriteBuffer[spriteBufferPointer++] = spriteX;
			spriteBuffer[spriteBufferPointer++] = memory.readPPU(0xFE02 + oamSearch);
			spriteBuffer[spriteBufferPointer++] = memory.readPPU(0xFE03 + oamSearch);

			//std::cout << "Added sprite to the pointer" << std::endl;
				
		}
	}

	oamSearch += 0x04;
	windowTriggeredThisScanline = false;
	windowEnabled = false; // Only true during window fetches
	
}

void PPU::mode3Tick() // send pixels to the lcd
{
	mode3TCycles++;

	uint8_t scx = memory.readPPU(0xFF43);
	uint8_t wx = memory.readPPU(0xFF4B);
	uint8_t wy = memory.readPPU(0xFF4A);
	uint8_t ly = memory.readPPU(0xFF44);
	uint8_t fineX = scx % 8;




	if (pixelsX >= 160)
	{
		mode3Finished = true;
		memory.vramLocked = false;
		return;
	}

	lcdc = memory.ioFetchLCDC();
	bool windowDisplayEnable = (lcdc & 0x20);

	if (windowDisplayEnable && ly >= wy && pixelsX == (wx - 7) && !windowTriggeredThisScanline)
	{
		windowEnabled = true;              
		windowTriggeredThisScanline = true; 
		windowLine++;                       
	}

	//std::cout << "Scanline " << (int)currentLY << ": windowLine = " << (int)windowLine << "\n";


	if (pixelsX % 8 == 0 || backgroundFifo.empty())
	{
		fetchTileNo();  
		fetchTileL();
		fetchTileH();
		fifoPush();
	}

	shiftPixel();
}


void PPU::mode0Tick(){}
void PPU::mode1Tick(){}

void PPU::fetchTileNo()
{
	uint16_t baseAddr{};
	uint8_t wy = memory.readPPU(0xFF4A); 
	uint8_t wx = memory.readPPU(0xFF4B);
	uint8_t scx = memory.readPPU(0xFF43);
	uint8_t scy = memory.readPPU(0xFF42);

	uint8_t tileX, tileY;


	bool windowEnabled = (lcdc & 0x20) && (currentLY >= wy) && (pixelsX >= (wx - 7));


	if (windowEnabled) {

	}

	if (windowEnabled) {
		baseAddr = (lcdc & 0x40) ? 0x9C00 : 0x9800;
		tileX = (pixelsX - (wx - 7)) / 8;
		tileY = windowLine / 8;
	}
	else {
		baseAddr = (lcdc & 0x08) ? 0x9C00 : 0x9800;
		uint16_t bg_x = (pixelsX + scx) % 256;
		uint16_t bg_y = (currentLY + scy) % 256;
		tileX = bg_x / 8;
		tileY = bg_y / 8;
	}

	uint16_t tileIndexAddr = baseAddr + tileY * 32 + tileX;
	currentTileI = memory.readPPU(tileIndexAddr);




}
void PPU::fetchTileL()
{
	//first thing, check if were using standard indexing mode or alternate
	uint16_t tileBase{};
	if (lcdc & 0b00010000) 
	{
		tileBase = 0x8000 + (currentTileI * 16);
	}
	else 
	{
		tileBase = 0x9000 + ((int8_t)currentTileI * 16);
	}
	uint8_t yOffs = ((currentLY + memory.readPPU(0xFF42)) % 8) ;
	lowerByte = memory.readPPU(tileBase +yOffs);
}

void PPU::fetchTileH()
{
	uint16_t tileBase = 0x8000;
	if (!(lcdc & 0b10000)) //0x8000 mode 
	{
		tileBase += (currentTileI * 16);
	}
	else
	{
		tileBase = (0x9000 + ((int8_t)currentTileI * 16)); // cast to signed int 
	}
	uint8_t yOffs = ((currentLY + memory.readPPU(0xFF42)) % 8);
	upperByte = memory.readPPU(tileBase + yOffs +8);
}
void PPU::fifoPush() {
	uint8_t scx = memory.readPPU(0xFF43);
	uint8_t fineX = scx % 8;

	for (int i = 7; i >= 0; i--) {
		uint8_t pixel = ((upperByte >> i) & 1) << 1 | ((lowerByte >> i) & 1);
		backgroundFifo.push(pixel);
	}

	if (!fetchedFirstTile) {
		for (int i = 0; i < fineX; i++) {
			if (!backgroundFifo.empty()) backgroundFifo.pop();
		}
		fetchedFirstTile = true;
	}
}

void PPU::shiftPixel() {
	if (!backgroundFifo.empty()) {
		uint8_t bgPixel = backgroundFifo.front();
		backgroundFifo.pop();

		uint8_t finalPixel = mergeWithSprite(bgPixel, pixelsX);

		screen[pixelsX % 160][currentLY] = finalPixel;
		pixelsX++;
	}
}

uint8_t PPU::mergeWithSprite(uint8_t bgPixel, int pixelsX)
{
	for (int i = 0; i < spriteBufferPointer/4; i++)
	{
		uint8_t spriteY = spriteBuffer[i * 4 + 0];  // sprite Y 
		uint8_t spriteX = spriteBuffer[i * 4 + 1];  // sprite X
		uint8_t tileIndex = spriteBuffer[i * 4 + 2]; // sprite tileI
		uint8_t attributes = spriteBuffer[i * 4 + 3]; // sprite flag

		if (pixelsX < spriteX || pixelsX >= spriteX + 8)
			continue;

		int spritePixelX = pixelsX - spriteX;

		uint8_t spritePixel = getSpritePixel(i, spritePixelX);

		if (spritePixel == 0)
			continue;

		dotDelay += 6;  
		if (spriteX < 8) dotDelay += 5;

		if ((attributes & 0x80) == 0 || bgPixel == 0)
		{
			return spritePixel;  
		}
	}

	return bgPixel; 
}

uint8_t PPU::getSpritePixel(int spriteIndex, int spritePixelX) {
	uint8_t spriteY = spriteBuffer[spriteIndex * 4 + 0];
	uint8_t spriteX = spriteBuffer[spriteIndex * 4 + 1];
	uint8_t tileIndex = spriteBuffer[spriteIndex * 4 + 2];
	uint8_t attributes = spriteBuffer[spriteIndex * 4 + 3];

	int yOffset = currentLY - (spriteY - 16);
	if (attributes & 0x40) 
	{ 
		yOffset = 7 - yOffset;
	}

	uint16_t tileAddr = 0x8000 + tileIndex * 16 + yOffset * 2;
	uint8_t lowByte = memory.readPPU(tileAddr);
	uint8_t highByte = memory.readPPU(tileAddr + 1);

	int bitIndex = (attributes & 0x20) ? spritePixelX : (7 - spritePixelX);
	uint8_t pixel = ((highByte >> bitIndex) & 1) << 1 | ((lowByte >> bitIndex) & 1);

	return pixel;
}


uint16_t PPU::fetchTileData(uint8_t tileIndex)
{
	uint16_t tileAddress = 0x8000 + tileIndex * 16;  
	uint16_t tileData = 0;

	for (int i = 0; i < 8; i++)
	{
		uint8_t lowByte = memory.readPPU(tileAddress + i);     
		uint8_t highByte = memory.readPPU(tileAddress + i + 8);  
		tileData |= (lowByte | (highByte << 8));           
	}

	return tileData;

}

void PPU::updateScreenBuffer(uint8_t (&mainScreen)[160][144])
{
	std::memcpy(mainScreen, screen, sizeof(screen));// not sure if this is going to work 100%

	//for (int y = 0; y < 144; y++) {
	//	for (int x = 0; x < 160; x++) {

	//		mainScreen[x][y] = screen[x][y];
	//	}
	//}
}

void PPU::executeTick(int allCycles) // measured in m cycles
{									 // 4 dots = 1 m cycle
	//std::cout << "ON TICK" << internalTick << "With vramAcess=" << memory.vramLocked << std::endl;
	//std::cout << curModeDebugTool << "   "<< internalTick << "  LY:"<< (int)memory.ioFetchLY() << std::endl;

	/*static uint8_t auto_scroll = 0;
	memory.write(0xFF43, auto_scroll); 
	auto_scroll = (auto_scroll + 1) % 256;  */


	if (memory.ioFetchLY() == memory.readPPU(0xFF45))
	{
		uint8_t temp = memory.readPPU(0xFF41);
		temp |= 0b100;
		memory.writePPU(0xFF41, temp);
	}

	if (memory.ioFetchLY() < 144)
	{

		if (internalTick == 0) {

			currentLY = memory.ioFetchLY();
			writeIntoSTAT(0b10);

			curModeDebugTool = 2;

			// this might should go here ?? V
			backgroundFifo = std::queue<uint8_t>();
			spriteFifo = std::queue<uint8_t>();
			pixelsX = 0;

		}

		if (internalTick < 20)
		{
			mode2Tick();
			mode2Tick();
		}
		
		else if (!mode3Finished)
		{


			if (internalTick == 20)
			{
				dotDelay = (memory.readPPU(0xFF43) % 8);
				//std::cout <<(int) dotDelay << std::endl;
				dotDelay += 12;

				writeIntoSTAT(0b11);

				memory.vramLocked = true;// gonan try this for now
				fetchedFirstTile = false;


				curModeDebugTool = 3;
			}

			for (int i = 0; i < 2; i++)
			{
				if (dotDelay > 0)
				{
					dotDelay-=2;
					mode3TCycles+=2;
				}
				else
				{
					mode3Tick();
					mode3Tick();

				}
			}
		}
		else if (internalTick <= 114)//mode 0
		{
			//we want mode 0 to run for 376- mode 3s duration in t cycles 
			//mode0Tick();
			//MODE0TICk does nothing, so bettwr to just wait for now with nothing in this function
			curModeDebugTool = 0;
		}
		else {


			//reset everything
			internalTick = -1;
			memset(spriteBuffer, 0, 40);
			spriteBufferPointer = 0;

			oamSearch = 0;
			memory.writePPU(0xFF44, memory.ioFetchLY() + 1);
			mode3Finished = false;
			mode3TCycles = 0;
			pixelsX = 0;


			//uint8_t currentTileI{};
			//uint8_t windowLine = 0;
			//uint8_t dotDelay = 0;
		}

		internalTick++;
	}
	else // curently in vBlank mode
	{
		if (memory.ioFetchLY() == 144) 
		{
			curModeDebugTool = 1;
			writeIntoSTAT(0b01);

			//request vBlank interrupt
			uint8_t temp = memory.ioFetchIF();
			memory.ioWriteIF(temp |= 0b1);
		}
		
		if (internalTick < 1140)// wait here for 1140 cycles 
		{
			internalTick++;
			if (memory.ioFetchLY() < 153) {
				memory.writePPU(0xFF44, memory.ioFetchLY() + 1);
			}
			else {
				memory.writePPU(0xFF44, 0);
				internalTick = 0;
			}
		}
		else 
		{
			memory.writePPU(0xFF44, 0);
		}
	}
}
