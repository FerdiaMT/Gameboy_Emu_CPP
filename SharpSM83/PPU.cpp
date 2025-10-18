#include "PPU.h"
#include "SM83.h"
#include "memory.h"
#include <iostream>
#include <queue>
#include <cstring>
#include <bitset>

uint16_t internalDot{};
uint16_t mode3Dots{};


struct SpritePixel {
	uint8_t color;
	bool bgPriority;
	bool transparent;
};

bool mode3Complete = false;
bool spritesSorted = false;
std::queue<uint8_t> backgroundFifo{};
std::queue<SpritePixel> spriteFifo{};
uint8_t screen[160][144];

uint8_t currentLY{};


// resource used -> https://hacktix.github.io/GBEDG/ppu/#oam-scan-mode-2

PPU::PPU(Memory* memory) : memory(memory) //m cycle (using this) = 4 t states
{

}

void PPU::resetPPU()
{

}

uint8_t currentMode{};


void PPU::updateLYC() {
	uint8_t stat = memory.readPPU(0xFF41);
	uint8_t ly = memory.readPPU(0xFF44);
	uint8_t lyc = memory.readPPU(0xFF45);

	bool coincBefore = (stat & 0x04) != 0;
	bool coincNow = (ly == lyc);

	// write coincidence flag
	stat = coincNow ? (stat | 0x04) : (stat & ~0x04);
	memory.writePPU(0xFF41, stat);

	// edge-trigger STAT IF if enabled
	if (!coincBefore && coincNow && (stat & 0x40)) {
		memory.requestInterrupt(0x02);
	}
}

void PPU::updateMode(uint8_t newMode) {
	static uint8_t prevMode = 0xFF;
	uint8_t stat = memory.readPPU(0xFF41);

	if (newMode != prevMode) {
		// set mode bits
		stat = (stat & ~0x03) | newMode;
		memory.writePPU(0xFF41, stat);

		bool req = false;
		if (newMode == 0 && (stat & 0x08)) req = true; // HBlank
		if (newMode == 1 && (stat & 0x10)) req = true; // VBlank
		if (newMode == 2 && (stat & 0x20)) req = true; // OAM

		if (req) {
			memory.requestInterrupt(0x02); // STAT IF
		}
		prevMode = newMode;
	}
}


void initFrame() {
	mode3Dots = 0;
	mode3Complete = false;
}

bool mode2Half = 0;

uint8_t searchX = 0; // up to168
uint8_t searchY = 0; // up to 160
uint16_t searchAddr = 0xFE00;

uint8_t curItemsOnScanline = 0;


std::vector<Sprite> scanlineQueue{};

void PPU::mode2Tick()  // TODO : DOUBLE TALL SPRITE 
{
	if (!mode2Half)
	{

		// for the oam
	}
	else
	{
		uint8_t oamY = memory.read(searchAddr);

		if (currentLY >= oamY-16  && currentLY < oamY-16+8 && curItemsOnScanline <=10)
		{
			
			curItemsOnScanline++;
			Sprite sprite;
			sprite.y = oamY; // this should be fine,   // y
			sprite.x = memory.read(searchAddr + 1); // x 
			sprite.ti = memory.read(searchAddr + 2); // tile index
			sprite.attr = memory.read(searchAddr + 3); // attributes

			//std::cout << "added at ti: x " <<(int)sprite.x << " y : " << (int) sprite.y << " "<< (int)sprite.ti << std::endl;
			scanlineQueue.push_back(sprite);

			//4 bytes  , BYTE  = 8 bit
		}
		

		searchAddr += 0x04; 
	}
	mode2Half = !mode2Half;

}

uint8_t fetcherX{}; 

uint8_t screenX{};
uint8_t screenY{};

uint16_t memAddr{};
uint16_t tileNumber{};

uint8_t bgx{};
uint8_t bgy{};

uint16_t mode3Penalty = 12;


bool renderingWindow = false;
uint16_t windowY{};
uint16_t windowLineCounter = 0;
bool windowTriggered = false;


void PPU::fetchTileNo()
{
	if ((memory.ioFetchLCDC() & 0b1000))
	{
		memAddr = 0x9C00;
	}
	else
	{
		memAddr = 0x9800;
	}

	bgx = (memory.ioFetchSCX() + fetcherX) % 256;
	bgy = (memory.ioFetchSCY() + currentLY) % 256;
	 
	tileNumber = (memory.read(memAddr + (bgx / 8) + (bgy / 8) *32));
}

uint8_t tileDataA{};
uint8_t tileDataB{};

void PPU:: fetchTileL()
{

	uint8_t fineY = (bgy) % 8;


	if ((memory.ioFetchLCDC() & 0x10))
	{
		memAddr = 0x8000;
		tileDataA = memory.read( memAddr + (tileNumber * 0x10) + (fineY *2) );
	}
	else
	{ //8800 mode
		memAddr = 0x9000;
		tileDataA = memory.read(memAddr + ((int8_t)tileNumber*16) + fineY * 2);

	}
}

void PPU::fetchTileH()
{

	uint8_t fineY = (bgy) % 8;

	if ((memory.ioFetchLCDC() & 0x10))
	{
		memAddr = 0x8000;
		tileDataB = memory.read(1+(memAddr + (tileNumber * 0x10) + (fineY * 2)));


	}
	else
	{ //8800 mode
		memAddr = 0x9000;
		tileDataB = memory.read(1+ (memAddr + ((int8_t)tileNumber * 16) + fineY * 2));
	}
}




void PPU::fetchWindowTileNo()
{
	if ((memory.ioFetchLCDC() & 0b01000000))
	{
		if ((memory.ioFetchLCDC() & 0b1000))
		{
			memAddr = 0x9C00;
		}
		else
		{
			memAddr = 0x9800;
		}

		uint8_t winx = memory.ioFetchWX() - 7;
		uint8_t winy = memory.ioFetchWY();

		if (currentLY >= winy)
		{
			windowY = currentLY - winy;
			tileNumber = (memory.read(memAddr + ((fetcherX - (winx)) / 8) + (windowY / 8) * 32));
			renderingWindow = true;
		}
	}
}

void PPU::fetchWindowTileL()
{
	if (!renderingWindow) return;

	uint8_t fineY = windowY % 8;

	if (memory.ioFetchLCDC() & 0x10)
	{
		memAddr = 0x8000;
		tileDataA = memory.read(memAddr + (tileNumber * 0x10) + (fineY * 2));
	}
	else
	{ //8800 mode
		memAddr = 0x9000;
		tileDataA = memory.read(memAddr + ((int8_t)tileNumber * 16) + fineY * 2);
	}
}

void PPU::fetchWindowTileH()
{
	if (!renderingWindow) return;

	uint8_t fineY = windowY % 8;

	if (memory.ioFetchLCDC() & 0x10)
	{
		memAddr = 0x8000;
		tileDataB = memory.read(1 + (memAddr + (tileNumber * 0x10) + (fineY * 2)));
	}
	else
	{ //8800 mode
		memAddr = 0x9000;
		tileDataB = memory.read(1 + (memAddr + ((int8_t)tileNumber * 16) + fineY * 2));
	}
}


bool fifoCanPush = false;

bool tileFetcherFinished = false;
bool pixelOutputMode = false;


void PPU::fetchSpriteTile(const Sprite& sprite)
{
	uint8_t spriteHeight = (memory.ioFetchLCDC() & 0x04) ? 16 : 8;

	int16_t spriteY = currentLY - (sprite.y - 16);
	if (spriteY < 0 || spriteY >= spriteHeight) return;

	uint16_t tileIndex = sprite.ti;
	if (spriteHeight == 16) 
	{
		tileIndex &= 0xFE; 
	}

	if (sprite.attr & 0x40) 
	{
		spriteY = spriteHeight - 1 - spriteY;
	}

	uint16_t addr = 0x8000 + (tileIndex * 16) + spriteY * 2;
	uint8_t dataLo = memory.read(addr);
	uint8_t dataHi = memory.read(addr + 1);

	for (int i = 7; i >= 0; i--)
	{
		int pixelBit = (sprite.attr & 0x20) ? 7 - i : i;
		uint8_t color = ((dataHi >> pixelBit) & 1) << 1 | ((dataLo >> pixelBit) & 1);

		SpritePixel spx;
		spx.color = color;
		spx.bgPriority = sprite.attr & 0x80;
		spx.transparent = (color == 0);

		spriteFifo.push(spx);
	}
}


void PPU::fifoPush() {

	for (int x = 7; x >= 0; x--) 
	{
		backgroundFifo.push(((tileDataB >> x) & 0b1) << 1 | ((tileDataA >> x) & 0b1));
	}
	tileFetcherFinished = true;
}

uint8_t twelveDot{};
uint8_t fetcherState{};



void PPU::tileFetcher()
{
	if (fetcherX < 160) 
	{
		uint8_t winx = memory.ioFetchWX() - 7;
		bool windowActive = (memory.ioFetchLCDC() & 0b01000000) &&(memory.ioFetchWY() <= currentLY) &&(fetcherX >= winx) && (winx <= 166); 

		if (windowActive && !windowTriggered)
		{
			windowTriggered = true;
			windowLineCounter++;
			fetcherX = 0; 
		}

		if (fetcherX % 8 == 0)
		{
			if (windowTriggered)
			{
				fetchWindowTileNo();
				fetchWindowTileL();
				fetchWindowTileH();
			}
			else
			{
				fetchTileNo();
				fetchTileL();
				fetchTileH();
			}
			fifoPush();


			fifoCanPush = true;
		}


		for (const auto& sprite : scanlineQueue)
		{
			//std::cout << "sprite checked"<<std::endl;
			if (sprite.x - 8 == fetcherX)

			{
				fetchSpriteTile(sprite);
			}
		}

		fetcherX++;
	}
	else 
	{
		mode3Complete = true;
		fifoCanPush = false;
		mode3Dots -= 1;
	}
}



uint8_t internalX{};

void PPU::drawPixel()
{
	if (backgroundFifo.empty())
		return;

	uint8_t bgColor = backgroundFifo.front();


	uint8_t BGP = memory.read(0xff47);
	uint8_t finalColor = (BGP >> (bgColor * 2)) & 0x03;

	//finalColor = (internalX / 8 + currentLY / 8) % 4; // THIS IS DEBUG, RMEOVE LATER, DEBUG




	if (!spriteFifo.empty()) {
		SpritePixel spx = spriteFifo.front();
		finalColor = spx.color;
		spriteFifo.pop();
	}

	backgroundFifo.pop();
	screen[internalX][currentLY] = finalColor;
	internalX++;
}



void PPU::mode3Tick() 
{

	if (!spritesSorted)
	{
		std::sort(scanlineQueue.begin(), scanlineQueue.end(), [](const Sprite& a, const Sprite& b) 
		{
			return a.x < b.x;
		});

		spritesSorted = true;
	}

	tileFetcher();
	//spriteFetcher();
	if (!backgroundFifo.empty())
	{
		drawPixel();
	}
}


void resetVals()
{
	twelveDot = 0;
	mode3Penalty = 12;
}

bool enteredModeOne = true;

void PPU::step(int cycles) {
	for (int i = 0; i < cycles; i++) {
		executeTick();
	}
}

void PPU::executeTick() // measured in m cycles
{


	currentLY = memory.ioFetchLY();

	if (memory.writeToLYC() )
	{
		//updateSTAT();
		
		updateLYC();
	}

	if (currentLY >= 144) 
	{
		mode1Tick();
	}
	else
	{
		if (internalDot < 80)
		{
			mode2Tick();
			memory.vramLocked = false;
			currentMode = 0b10;
			updateMode(0b10);
			//updateSTAT();
		}
		else 
		{

			//Mode 3 is 172 to 289 dots long
			if (!mode3Complete)
			{

				memory.vramLocked = true;

				mode3Tick();

				currentMode = 0b11;
				updateMode(0b11);
				//updateSTAT();

				mode3Dots++;
			}
			else
			{

				//this area compensates for both mode 3 and mode 0

				if (mode3Penalty > 0) 
				{
					mode3Penalty--;
				}
				else  //mode 0
				{
					memory.vramLocked = false;
					
					currentMode = 0b0;
					//updateSTAT();
					updateMode(0b0);
				}



			}

		}

		if (internalDot >= 456) { // end of scanline

			//this section is when the ly changes and everything increments

			searchAddr = 0xFE00;
			scanlineQueue.clear();
			curItemsOnScanline = 0;
			spritesSorted = false;
			initFrame();
			internalDot = -1;
			enteredModeOne = true;
			
			fetcherX = 0;
			internalX = 0;

			memory.ioIncrementLY();
			updateLYC();

		}
	}
	//std::cout << "DOT :" << (int)internalDot << " MODE3 dot " << mode3Dots << "   vram:Lock " << (int)memory.vramLocked << "  " << (int)fetcherX << std::endl;
	internalDot++;

}



void PPU::mode1Tick()
{
	if (enteredModeOne) {
		currentMode = 0b1;
		//updateSTAT();
		updateMode(0b1);

		// request VBlank IF
		memory.ioWriteIF(memory.ioFetchIF() | 0x01);

		enteredModeOne = false;
	}

	if (internalDot >= 456) {
		resetVals();
		internalDot = 0;

		uint8_t ly = memory.ioFetchLY() + 1;
		if (ly > 153) ly = 0;
		memory.ioWriteLY(ly);
		updateLYC();         
	}
}

void PPU::updateScreenBuffer(uint8_t(&mainScreen)[160][144])
{
	std::memcpy(mainScreen, screen, sizeof(screen));
}

uint16_t PPU::getInternalDot()
{
	return internalDot;
}