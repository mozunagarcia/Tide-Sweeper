# Tide Sweeper 

A simple side-scrolling submarine game built with SDL2 and CMake.

##  Dependencies

| Platform | How to install |
|-----------|----------------|
| **Windows** | Install MSYS2 and run:<br>`pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_image` |
| **macOS** | Install Homebrew and run:<br>`brew install sdl2 sdl2_image cmake` |
| **Linux** | Run:<br>`sudo apt install build-essential cmake libsdl2-dev libsdl2-image-dev` |

##  Building

```bash
git clone https://github.com/mozunagarcia/Tide-Sweeper.git
cd Tide-Sweeper
mkdir build && cd build
cmake ..
cmake --build . --config Release
