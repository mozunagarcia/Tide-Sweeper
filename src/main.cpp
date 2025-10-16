#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>

// ---------------------------------------------
// Helper: load an image as an SDL texture
// ---------------------------------------------
SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path) {
    SDL_Surface* surf = IMG_Load(path);
    if (!surf) {
        std::cerr << "Failed to load image: " << path
                  << " | " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
}

int main(int argc, char* argv[]) {
    // --- Initialize SDL and SDL_image ---
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image init failed: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // --- Create window + renderer ---
    SDL_Window* window = SDL_CreateWindow(
        "Tide Sweeper",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // --- Load textures ---
    SDL_Texture* ocean = loadTexture(renderer, "assets/ocean.png");
    SDL_Texture* submarine = loadTexture(renderer, "assets/submarine.png");
    if (!ocean || !submarine) {
        std::cerr << "Missing textures! Place ocean.png and submarine.png in /assets\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // --- Game state ---
    SDL_Rect sub = { 200, 275, 100, 60 }; // submarine position
    float cameraX = 0.0f;                  // how far the background has moved
    const float scrollSpeed = 2.0f;        // speed of ocean movement
    const int bgWidth = 800;
    const int bgHeight = 600;

    bool running = true;
    SDL_Event event;

    // --- Main loop ---
    while (running) {
        // --- Event handling ---
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        // --- Keyboard input ---
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_UP])    sub.y -= 5;
        if (keys[SDL_SCANCODE_DOWN])  sub.y += 5;
        if (keys[SDL_SCANCODE_LEFT])  sub.x -= 2;
        if (keys[SDL_SCANCODE_RIGHT]) sub.x += 2;

        // Clamp submarine to screen
        if (sub.x < 50)  sub.x = 50;
        if (sub.x > 650) sub.x = 650;
        if (sub.y < 0)   sub.y = 0;
        if (sub.y > 540) sub.y = 540;

        // --- Scroll background to the RIGHT visually ---
        // Increase offset so the texture slides right (new tiles appear from left)
        cameraX += scrollSpeed;
        if (cameraX >= bgWidth)
            cameraX -= bgWidth;

        // --- Render ---
        SDL_RenderClear(renderer);

        SDL_Rect srcRect = { 0, 0, bgWidth, bgHeight };
        SDL_Rect dest1 = { static_cast<int>(-cameraX), 0, bgWidth, bgHeight };
        SDL_Rect dest2 = { static_cast<int>(-cameraX) + bgWidth, 0, bgWidth, bgHeight };

        // draw two copies for seamless rightward scrolling
        SDL_RenderCopy(renderer, ocean, &srcRect, &dest1);
        SDL_RenderCopy(renderer, ocean, &srcRect, &dest2);

        // draw submarine
        SDL_RenderCopyEx(renderer, submarine, nullptr, &sub, 0, nullptr, SDL_FLIP_HORIZONTAL);

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    // --- Cleanup ---
    SDL_DestroyTexture(ocean);
    SDL_DestroyTexture(submarine);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
