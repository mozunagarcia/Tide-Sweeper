# TideSweeper - Setup Instructions

## Prerequisites

- CMake 3.16 or higher
- MinGW-w64 (for Windows) or appropriate C++ compiler
- Git

## Installing SDL2 Libraries

### Windows (MinGW)

1. Download the following SDL2 development libraries (MinGW):
   - [SDL2 2.32.6](https://github.com/libsdl-org/SDL/releases/tag/release-2.32.6) - Download `SDL2-devel-2.32.6-mingw.tar.gz`
   - [SDL2_image 2.8.2](https://github.com/libsdl-org/SDL_image/releases/tag/release-2.8.2) - Download `SDL2_image-devel-2.8.2-mingw.tar.gz`
   - [SDL2_ttf 2.24.0](https://github.com/libsdl-org/SDL_ttf/releases/tag/release-2.24.0) - Download `SDL2_ttf-devel-2.24.0-mingw.tar.gz`
   - [SDL2_mixer 2.8.1](https://github.com/libsdl-org/SDL_mixer/releases/tag/release-2.8.1) - Download `SDL2_mixer-devel-2.8.1-mingw.tar.gz`

2. Extract each archive and place them in a `lib/` folder in the project root:
   ```
   TideSweeper/
     lib/
       SDL2-2.32.6/
       SDL2_image-2.8.2/
       SDL2_ttf-2.24.0/
       SDL2_mixer-2.8.1/
   ```

3. Make sure each library folder contains the `x86_64-w64-mingw32/` subdirectory with `include/` and `lib/` folders.

### macOS (Homebrew)

```bash
brew install sdl2 sdl2_image sdl2_ttf sdl2_mixer
```

The CMakeLists.txt is already configured to find Homebrew installations.

## Building the Project

1. Clone the repository:
   ```bash
   git clone https://github.com/mozunagarcia/Tide-Sweeper.git
   cd Tide-Sweeper
   ```

2. Create the `lib/` folder and install SDL2 libraries (Windows only - see above)

3. Configure and build:
   ```bash
   cmake -B build
   cmake --build build --config Debug
   ```

4. Run the game:
   ```bash
   ./build/TideSweeper.exe  # Windows
   ./build/TideSweeper      # macOS/Linux
   ```

## Troubleshooting

- **CMake can't find SDL2**: Make sure your `lib/` folder structure matches exactly as shown above
- **Missing DLLs at runtime**: Copy the `.dll` files from `lib/SDL2-*/x86_64-w64-mingw32/bin/` to the build directory
- **Build errors**: Ensure you have MinGW-w64 installed and in your PATH

## Project Structure

```
TideSweeper/
  Assets/         # Game assets (backgrounds, fonts, music)
  include/        # Header files
  src/            # Source files
  lib/            # SDL2 libraries (not tracked in git)
  build/          # Build output (not tracked in git)
  CMakeLists.txt  # Build configuration
```
