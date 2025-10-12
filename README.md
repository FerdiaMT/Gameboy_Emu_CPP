# SM83 Game Boy Emulator

A **C++ Game Boy emulator** built from scratch, accurately replicating the SM83 CPU, PPU, and memory systems of Nintendo’s original hardware.
Uses SDL3 for rendering the graphics.




## Features

- Full **SM83 CPU** instruction set (passes all Blaarg tests)
- **PPU** with scanline-based FIFO rendering (background, window, sprites in progress)
- **Accurate LCDC/STAT/LYC**, OAM search, and timing behavior
- **Boot ROM** supported (starts at 0x0000, no skip)
- Modular architecture: `CPU`, `PPU`, `Memory`, `Clock`

---



## Roadmap

- [x] CPU  
- [x] Background and window rendering  
- [x] Sprite rendering
- [ ] Full cycle accurate interactions between CPU and PPU
- [ ] Cartridge MBCs and save states
- [ ] Built in Debugger
- [ ] APU (sound) 


---<img width="797" height="705" alt="tetris" src="https://github.com/user-attachments/assets/9f52f8fe-e40c-4739-a8c5-1b3e4ea190e3" />
