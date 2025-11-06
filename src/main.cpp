#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector> //added this - Laura
#include "Litter.h"
#include "ScoreDisplay.hpp"
#include "Enemies.h"

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

    // lives (move later)
    int lives = 3;

    // Initialize SDL, SDL_image, and SDL_ttf
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image init failed: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    if (TTF_Init() < 0) {
        std::cerr << "SDL_ttf init failed: " << TTF_GetError() << std::endl;
        IMG_Quit();
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
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
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

// -------------------- Mari EDITS --------------------

SDL_Texture* swordfishTexture = loadTexture(renderer, "Assets/Swordfish.png");
SDL_Texture* eelTexture = loadTexture(renderer, "Assets/Eel.png");
SDL_Texture* octopusTexture = loadTexture(renderer, "Assets/Octopus.png");
SDL_Texture* anglerTexture = loadTexture(renderer, "Assets/Angler.png");

// Create a vector of enemy textures for random selection
std::vector<SDL_Texture*> enemyTextures = {
    swordfishTexture, eelTexture, octopusTexture, anglerTexture
};
std::vector<float> enemySpeeds = { 4.0f, 3.8f, 3.5f, 4.2f };    // controls speed of enemies

// Empty enemies vector
std::vector<Enemies> enemyItems;

// Spawn control variables
int spawnTimer = 0;
const int spawnInterval = 120;  // Spawn check every 2 seconds (120 frames at 60 FPS)
const int maxActiveEnemies = 2; // Maximum number of active enemies at once

// Heart Texture 
SDL_Texture* heartTex = loadTexture(renderer, "Assets/heart.png");

// Load font for game over screen
TTF_Font* font = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 36); // 36 is the font size
if (!font) {
    std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
}

// -------------------- LAURA EDITS --------------------


    // --- Game state ---
    bool gameOver = false;
    SDL_Rect sub = { 200, 275, 100, 60 }; // submarine position
    float cameraX = 0.0f;                  // how far the background has moved
    const float scrollSpeed = 2.0f;        // speed of ocean movement
    const int bgWidth = 800;
    const int bgHeight = 600;

    // Create score display in top-right corner
    ScoreDisplay scoreDisplay(renderer, 650, 10, 140, 50);  // x, y, width, height
    scoreDisplay.setScore(0);  // Initialize score to 0

// --- MARI EDITS ---
    // Function to reset game state
    auto resetGame = [&]() {
        lives = 3;
        gameOver = false;
        sub = { 200, 275, 100, 60 };
        enemyItems.clear();
        scoreDisplay.setScore(0);
    };

    bool running = true;
    SDL_Event event;

    srand(static_cast<unsigned int>(time(nullptr)));

    // Debug: log heart texture load status once
    bool heartStatusLogged = false;


    // --- Main loop ---
    while (running) {
        // --- Event handling ---
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN && gameOver) {
                if (event.key.keysym.sym == SDLK_r) {
                    // Reset game when R is pressed during game over
                    resetGame();
                }
            }
        }

        // Skip game updates if game over
        if (!gameOver) {
            // Keyboard input ---
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_UP])    sub.y -= 5;
        if (keys[SDL_SCANCODE_DOWN])  sub.y += 5;
        if (keys[SDL_SCANCODE_LEFT])  sub.x -= 5;
        if (keys[SDL_SCANCODE_RIGHT]) sub.x += 5;

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

//-------------------- MARI EDITS --------------------

// Spawn timer logic
spawnTimer++;
if (spawnTimer >= spawnInterval) {
    spawnTimer = 0;
    
    // Count active enemies
    int activeCount = 0;
    for (const auto& enemy : enemyItems) {
        if (enemy.active) activeCount++;
    }
    
    // Try to spawn a new enemy if we're under the limit
    if (activeCount < maxActiveEnemies) {
        // Pick a random enemy type
        int randomIndex = rand() % enemyTextures.size();
        
        // Spawn off the right side of the screen
        float startX = 850;
        float startY = rand() % 500 + 50; // Random Y position
        
        // Create new enemy
        enemyItems.emplace_back(
            enemyTextures[randomIndex],
            startX,
            startY,
            enemySpeeds[randomIndex]
        );
    }
}

// Update and cleanup enemies
for (auto it = enemyItems.begin(); it != enemyItems.end();) {
    // Remove enemies that have gone off screen to the left
    if (it->x < -100) {
        it = enemyItems.erase(it);
    } else {
        // Update and check collisions
        it->update();
        it->render(renderer);
        
        if (it->checkCollision(sub)) {
            lives--;  // Decrease player lives
            it = enemyItems.erase(it);  // Remove enemy on collision
            
            // Check for game over
            if (lives <= 0) {
                gameOver = true;
            }
        } else {
            ++it;
        }
    }
}

} // Close the if(!gameOver) block


// -------------------- LAURA EDITS --------------------

        // --- Scroll background to the RIGHT visually ---
        // Increase offset so the texture slides right (new tiles appear from left)
        cameraX += scrollSpeed;
        if (cameraX >= bgWidth)
            cameraX -= bgWidth;

        // --- Render ---
        // SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black background
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

// ------------ MARI EDITS ------------
// Draw enemies on top of the ocean background
for (const auto & enemy : enemyItems) {
    if (!enemy.active) continue;  // skip hidden ones

    SDL_Rect dest = {
        static_cast<int>(enemy.x),
        static_cast<int>(enemy.y),
        90, 90
    };
    SDL_RenderCopy(renderer, enemy.texture, nullptr, &dest);
}

// -------------------- LAURA EDITS --------------------

        // draw submarine
        SDL_RenderCopyEx(renderer, submarine, nullptr, &sub, 0, nullptr, SDL_FLIP_HORIZONTAL);

    // Draw score display
    scoreDisplay.render();

// -------------------- MARI EDITS ---------------------
    // Draw hearts
    int heartSize = 70;     // much smaller heart size
    int16_t spacing = -20;  // adjusted spacing for smaller hearts
    int startX = 5;         // tiny offset from the corner
    int startY = 5;         // tiny offset from the corner

        for (int i = 0; i < lives; ++i) {
            SDL_Rect heartRect = { startX + i * (heartSize + spacing), startY, heartSize, heartSize };
            if (heartTex) {
                SDL_RenderCopy(renderer, heartTex, nullptr, &heartRect);
            }
        }

// ----------------- MARI EDITS -----------------
        // Render game over screen if game is over
        if (gameOver) {
            // Semi-transparent dark overlay
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 192);
            SDL_Rect overlay = {0, 0, bgWidth, bgHeight};
            SDL_RenderFillRect(renderer, &overlay);

            // Render game over text
            if (font) {
                SDL_Color textColor = {255, 255, 255, 255}; // White text
                SDL_Surface* gameOverSurf = TTF_RenderText_Solid(font, "Game Over!", textColor);
                SDL_Surface* restartSurf = TTF_RenderText_Solid(font, "Press R to Restart", textColor);
                
                if (gameOverSurf && restartSurf) {
                    SDL_Texture* gameOverTex = SDL_CreateTextureFromSurface(renderer, gameOverSurf);
                    SDL_Texture* restartTex = SDL_CreateTextureFromSurface(renderer, restartSurf);
                    
                    if (gameOverTex && restartTex) {
                        // Position game over text in center of screen
                        SDL_Rect gameOverRect = {
                            (bgWidth - gameOverSurf->w) / 2,
                            (bgHeight - gameOverSurf->h) / 2 - 30,
                            gameOverSurf->w,
                            gameOverSurf->h
                        };
                        
                        // Position restart text below game over text
                        SDL_Rect restartRect = {
                            (bgWidth - restartSurf->w) / 2,
                            (bgHeight - restartSurf->h) / 2 + 30,
                            restartSurf->w,
                            restartSurf->h
                        };
                        
                        SDL_RenderCopy(renderer, gameOverTex, NULL, &gameOverRect);
                        SDL_RenderCopy(renderer, restartTex, NULL, &restartRect);
                        
                        SDL_DestroyTexture(gameOverTex);
                        SDL_DestroyTexture(restartTex);
                    }
                    
                    SDL_FreeSurface(gameOverSurf);
                    SDL_FreeSurface(restartSurf);
                }
            }
        }

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

// -------------------- MARI EDITS --------------------
SDL_DestroyTexture(swordfishTexture);
SDL_DestroyTexture(eelTexture);
SDL_DestroyTexture(octopusTexture);
SDL_DestroyTexture(anglerTexture);
SDL_DestroyTexture(heartTex);

// -------------------- LAURA EDITS --------------------

    if (font) {
        TTF_CloseFont(font);
    }
    SDL_DestroyTexture(ocean);
    SDL_DestroyTexture(submarine);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
