#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <vector> //added this - Laura
#include "Litter.h"
#include "ScoreDisplay.hpp"

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
    SDL_Texture* ocean = loadTexture(renderer, "Assets/ocean.png");
    SDL_Texture* submarine = loadTexture(renderer, "Assets/submarine.png");
    if (!ocean || !submarine) {
        std::cerr << "Missing textures! Place ocean.png and submarine.png in /assets\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

// -------------------- LAURA EDITS --------------------

SDL_Texture* canTex = loadTexture(renderer, "Assets/can.png");
SDL_Texture* bottleTex = loadTexture(renderer, "Assets/bottle.png");
SDL_Texture* bagTex = loadTexture(renderer, "Assets/bag.png");
SDL_Texture* cupTex = loadTexture(renderer, "Assets/cup.png");
SDL_Texture* colaTex = loadTexture(renderer, "Assets/cola.png");
SDL_Texture* smallcanTex = loadTexture(renderer, "Assets/smallcan.png");
SDL_Texture* beerTex = loadTexture(renderer, "Assets/beer.png");

std::vector<Litter> litterItems = {
    Litter(canTex, 200, 300, 1.5f),
    Litter(bottleTex, 500, 400, 2.0f),
    Litter(bagTex, 650, 250, 1.8f),
    Litter(cupTex, 350, 200, 1.3f),
    Litter(colaTex, 700, 500, 2.2f),
    Litter(smallcanTex, 100, 450, 1.6f),
    Litter(beerTex, 400, 350, 1.9f)
};



// -------------------- LAURA EDITS --------------------


    // --- Game state ---
    SDL_Rect sub = { 200, 275, 100, 60 }; // submarine position
    float cameraX = 0.0f;                  // how far the background has moved
    const float scrollSpeed = 2.0f;        // speed of ocean movement
    const int bgWidth = 800;
    const int bgHeight = 600;

    // Create score display in top-right corner
    ScoreDisplay scoreDisplay(renderer, 650, 10, 140, 50);  // x, y, width, height
    scoreDisplay.setScore(0);  // Initialize score to 0

    bool running = true;
    SDL_Event event;

    srand(static_cast<unsigned int>(time(nullptr)));


    // --- Main loop ---
    while (running) {
        // --- Event handling ---
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }


        // Keyboard input ---
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_UP])    sub.y -= 5;
        if (keys[SDL_SCANCODE_DOWN])  sub.y += 5;
        if (keys[SDL_SCANCODE_LEFT])  sub.x -= 2;
        if (keys[SDL_SCANCODE_RIGHT]) sub.x += 2;

        // Aggiorna il punteggio se viene premuta la barra spaziatrice
        static bool spaceWasPressed = false;
        if (keys[SDL_SCANCODE_SPACE]) {
            if (!spaceWasPressed) {
                scoreDisplay.setScore(scoreDisplay.getScore() + 10);
                spaceWasPressed = true;
            }
        } else {
            spaceWasPressed = false;
        }

        // Clamp submarine to screen
        if (sub.x < 50)  sub.x = 50;
        if (sub.x > 650) sub.x = 650;
        if (sub.y < 0)   sub.y = 0;
        if (sub.y > 540) sub.y = 540;


//-------------------- LAURA EDITS --------------------

// Check for collisions between submarine and litter

// --- Update litter ---
for (auto& litter : litterItems) {
    litter.update(); // now handles movement & looping

    // Check collision with submarine
    if (litter.checkCollision(sub)) {
        litter.collect(); // disappears, respawns later
    }
}

// --- Render litter ---
for (auto& litter : litterItems) {
    litter.render(renderer);
}


// -------------------- LAURA EDITS --------------------

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

// -------------------- LAURA EDITS --------------------

// Draw litter on top of the ocean background
for (const auto& litter : litterItems) {
    if (!litter.active) continue;  // skip hidden ones

    SDL_Rect dest = {
        static_cast<int>(litter.x),
        static_cast<int>(litter.y),
        90, 90
    };
    SDL_RenderCopy(renderer, litter.texture, nullptr, &dest);
}

// -------------------- LAURA EDITS --------------------

        // draw submarine
        SDL_RenderCopyEx(renderer, submarine, nullptr, &sub, 0, nullptr, SDL_FLIP_HORIZONTAL);

        // Draw score display
        scoreDisplay.render();

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    // --- Cleanup ---

// -------------------- LAURA EDITS --------------------

// Free litter textures
SDL_DestroyTexture(canTex);
SDL_DestroyTexture(bottleTex);
SDL_DestroyTexture(bagTex);
SDL_DestroyTexture(cupTex);
SDL_DestroyTexture(colaTex);
SDL_DestroyTexture(smallcanTex);
SDL_DestroyTexture(beerTex);


// -------------------- LAURA EDITS --------------------

    SDL_DestroyTexture(ocean);
    SDL_DestroyTexture(submarine);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
