#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>
#include <memory>
#include "GameManager.h"

namespace {
    // RAII helper to ensure SDL subsystems are properly cleaned up
    struct SDLInitializer {
        bool success = true;

        SDLInitializer() {
            if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
                std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
                success = false;
                return;
            }

            if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
                std::cerr << "SDL_image init failed: " << IMG_GetError() << std::endl;
                SDL_Quit();
                success = false;
                return;
            }

            if (TTF_Init() < 0) {
                std::cerr << "SDL_ttf init failed: " << TTF_GetError() << std::endl;
                IMG_Quit();
                SDL_Quit();
                success = false;
                return;
            }
        }

        ~SDLInitializer() {
            Mix_CloseAudio();
            TTF_Quit();
            IMG_Quit();
            SDL_Quit();
        }
    };

    // Helper to create window and handle errors
    SDL_Window* createWindow() {
        SDL_Window* window = SDL_CreateWindow(
            "Tide Sweeper",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            800, 600, SDL_WINDOW_SHOWN); //can add full sceen but will need to change some of the rendering logic to handle different resolutions

        if (!window) {
            std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        }
        return window;
    }

    // Helper to create renderer and handle errors
    SDL_Renderer* createRenderer(SDL_Window* window) {
        SDL_Renderer* renderer = SDL_CreateRenderer(
            window, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

        if (!renderer) {
            std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        }
        return renderer;
    }
}

int main(int argc, char* argv[]) {
    // Initialize SDL and all subsystems
    SDLInitializer sdl;
    if (!sdl.success) {
        return 1;
    }

    // Create window and renderer
    SDL_Window* window = createWindow();
    if (!window) {
        return 1;
    }

    SDL_Renderer* renderer = createRenderer(window);
    if (!renderer) {
        SDL_DestroyWindow(window);
        return 1;
    }

    // Run the game
    {
        GameManager game(window, renderer);
        game.run();
    }

    // Cleanup (SDLInitializer handles subsystem cleanup)
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return 0;
}
