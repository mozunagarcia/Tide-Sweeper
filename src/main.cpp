#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Tide Sweeper",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // submarine position and size
    SDL_Rect sub = { 375, 275, 50, 30 };

    bool running = true;
    SDL_Event event;

    while (running) {
        // --- Handle events ---
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        // --- Keyboard state ---
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_UP]) sub.y -= 5;
        if (keys[SDL_SCANCODE_DOWN]) sub.y += 5;
        if (keys[SDL_SCANCODE_LEFT]) sub.x -= 5;
        if (keys[SDL_SCANCODE_RIGHT]) sub.x += 5;

        // --- Rendering ---
        SDL_SetRenderDrawColor(renderer, 0, 100, 255, 255);  // ocean blue background
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);  // yellow submarine
        SDL_RenderFillRect(renderer, &sub);

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
