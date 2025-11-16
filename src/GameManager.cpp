#include "GameManager.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include "Litter.h"
#include "Enemies.h"

// Helper to load textures (copied from original main)
static SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path) {
    SDL_Surface* surf = IMG_Load(path);
    if (!surf) {
        std::cerr << "Failed to load image: " << path << " | " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
}

GameManager::GameManager(SDL_Window* window_, SDL_Renderer* renderer_)
    : window(window_), renderer(renderer_), level(nullptr), submarine(nullptr), scoreboard(nullptr), messages(nullptr), menu(nullptr), running(true), startGame(false)
{
    // Create menu 
    menu = new Menu(renderer);

    // We'll initialize game objects when entering the gameplay loop inside run()
}

GameManager::~GameManager() {
    delete level;
    delete submarine;
    delete scoreboard;
    delete messages;
    delete menu;
}

void GameManager::run() {
    // --- MENU LOOP ---
    while (running && !startGame) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            menu->handleEvent(e, running, startGame);
        }

        menu->render();
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    if (!running) return;

    // --- Load shared textures --- (paths kept identical to original)
    SDL_Texture* ocean = loadTexture(renderer, "Assets/ocean.png");
    SDL_Texture* submarineTex = loadTexture(renderer, "Assets/submarine.png");
    if (!ocean || !submarineTex) {
        std::cerr << "Missing textures! Place ocean.png and submarine.png in /assets\n";
        return;
    }

    // Litter textures
    SDL_Texture* canTex = loadTexture(renderer, "Assets/can.png");
    SDL_Texture* bottleTex = loadTexture(renderer, "Assets/bottle.png");
    SDL_Texture* bagTex = loadTexture(renderer, "Assets/bag.png");
    SDL_Texture* cupTex = loadTexture(renderer, "Assets/cup.png");
    SDL_Texture* colaTex = loadTexture(renderer, "Assets/cola.png");
    SDL_Texture* smallcanTex = loadTexture(renderer, "Assets/smallcan.png");
    SDL_Texture* beerTex = loadTexture(renderer, "Assets/beer.png");

    // Enemy textures
    SDL_Texture* swordfishTexture = loadTexture(renderer, "Assets/Swordfish.png");
    SDL_Texture* eelTexture = loadTexture(renderer, "Assets/Eel.png");
    SDL_Texture* octopusTexture = loadTexture(renderer, "Assets/Octopus.png");
    SDL_Texture* anglerTexture = loadTexture(renderer, "Assets/Angler.png");

    std::vector<SDL_Texture*> enemyTextures = { swordfishTexture, eelTexture, octopusTexture, anglerTexture };
    std::vector<float> enemySpeeds = { 4.0f, 3.8f, 3.5f, 4.2f };

    SDL_Texture* heartTex = loadTexture(renderer, "Assets/heart.png");

    // Scoreboard
    scoreboard = new Scoreboard(renderer, 650, 10, 140, 80);
    scoreboard->setScore(0);

    // Submarine
    submarine = new Submarine(submarineTex, 200, 275, 100, 60);

    // Level: pass litter + enemy textures
    level = new Level(renderer,
                      { canTex, bottleTex, bagTex, cupTex, colaTex, smallcanTex, beerTex },
                      enemyTextures, enemySpeeds);

    // Messages (currently minimal)
    messages = new Messages(renderer);

    // Game state
    int lives = 3;
    bool gameOver = false;
    float cameraX = 0.0f;
    const float scrollSpeed = 2.0f;
    const int bgWidth = 800;
    const int bgHeight = 600;

    int currentLevel = scoreboard->getLevel();

    // Reset function
    auto resetGame = [&]() {
        lives = 3;
        gameOver = false;
        submarine->setPosition(200, 275);
        level->reset();
        scoreboard->setScore(0);
        scoreboard->resetLevel();
        cameraX = 0.0f;
        currentLevel = 1;
        SDL_DestroyTexture(ocean);
        ocean = loadTexture(renderer, "Assets/ocean.png");
    };

    SDL_Event event;
    srand(static_cast<unsigned int>(time(nullptr)));

    // Main loop
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN && gameOver) {
                if (event.key.keysym.sym == SDLK_r) {
                    resetGame();
                }
            }
        }

        if (!gameOver) {
            // Keyboard input
            const Uint8* keys = SDL_GetKeyboardState(NULL);
            if (keys[SDL_SCANCODE_UP])    submarine->moveBy(0, -5);
            if (keys[SDL_SCANCODE_DOWN])  submarine->moveBy(0, 5);
            if (keys[SDL_SCANCODE_LEFT])  submarine->moveBy(-5, 0);
            if (keys[SDL_SCANCODE_RIGHT]) submarine->moveBy(5, 0);

            submarine->clamp(50, 650, 0, 540);

            // Update level (handles litter/enemies)
            level->update(*submarine, *scoreboard, lives, gameOver);
            // Detect level changes and swap background when reaching level 2 or 3
            {
                int newLevel = scoreboard->getLevel();
                if (newLevel != currentLevel) {
                    currentLevel = newLevel;
                    if (currentLevel == 2) {
                        SDL_DestroyTexture(ocean);
                        SDL_Texture* newOcean = loadTexture(renderer, "Assets/ocean_background.png");
                        if (newOcean) {
                            ocean = newOcean;
                        } else {
                            SDL_Texture* altOcean = loadTexture(renderer, "/Assets/ocean_background.png");
                            if (altOcean) ocean = altOcean;
                            else std::cerr << "Failed to load ocean_background: Assets/ocean_background.png" << std::endl;
                        }
                    }
                    else if (currentLevel == 3) {
                        SDL_DestroyTexture(ocean);
                        SDL_Texture* newOcean = loadTexture(renderer, "Assets/ocean3.png");
                        if (newOcean) {
                            ocean = newOcean;
                        } else {
                            SDL_Texture* altOcean = loadTexture(renderer, "/Assets/ocean3.png");
                            if (altOcean) ocean = altOcean;
                            else std::cerr << "Failed to load ocean3.png: Assets/ocean3.png" << std::endl;
                        }
                    }
                }
            }
        }

        // Scroll background
        cameraX += scrollSpeed;
        if (cameraX >= bgWidth) cameraX -= bgWidth;

        // Render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Rect srcRect = { 0, 0, bgWidth, bgHeight };
        SDL_Rect dest1 = { static_cast<int>(-cameraX), 0, bgWidth, bgHeight };
        SDL_Rect dest2 = { static_cast<int>(-cameraX) + bgWidth, 0, bgWidth, bgHeight };
        SDL_RenderCopy(renderer, ocean, &srcRect, &dest1);
        SDL_RenderCopy(renderer, ocean, &srcRect, &dest2);

        level->render();
        submarine->render(renderer);
        scoreboard->render();

        // Draw hearts
        int heartSize = 70;
        int16_t spacing = -20;
        int startX = 5;
        int startY = 5;
        for (int i = 0; i < lives; ++i) {
            SDL_Rect heartRect = { startX + i * (heartSize + spacing), startY, heartSize, heartSize };
            if (heartTex) SDL_RenderCopy(renderer, heartTex, nullptr, &heartRect);
        }

        // Game over overlay
        if (gameOver) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 192);
            SDL_Rect overlay = {0, 0, bgWidth, bgHeight};
            SDL_RenderFillRect(renderer, &overlay);

            // Render game over text
            TTF_Font* font = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 36);
            if (font) {
                SDL_Color textColor = {255,255,255,255};
                SDL_Surface* gameOverSurf = TTF_RenderText_Solid(font, "Game Over!", textColor);
                SDL_Surface* restartSurf = TTF_RenderText_Solid(font, "Press R to Restart", textColor);
                if (gameOverSurf && restartSurf) {
                    SDL_Texture* gameOverTex = SDL_CreateTextureFromSurface(renderer, gameOverSurf);
                    SDL_Texture* restartTex = SDL_CreateTextureFromSurface(renderer, restartSurf);
                    if (gameOverTex && restartTex) {
                        SDL_Rect gameOverRect = { (bgWidth - gameOverSurf->w) / 2, (bgHeight - gameOverSurf->h) / 2 - 30, gameOverSurf->w, gameOverSurf->h };
                        SDL_Rect restartRect = { (bgWidth - restartSurf->w) / 2, (bgHeight - restartSurf->h) / 2 + 30, restartSurf->w, restartSurf->h };
                        SDL_RenderCopy(renderer, gameOverTex, NULL, &gameOverRect);
                        SDL_RenderCopy(renderer, restartTex, NULL, &restartRect);
                        SDL_DestroyTexture(gameOverTex);
                        SDL_DestroyTexture(restartTex);
                    }
                    SDL_FreeSurface(gameOverSurf);
                    SDL_FreeSurface(restartSurf);
                }
                TTF_CloseFont(font);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    // Cleanup textures
    SDL_DestroyTexture(canTex);
    SDL_DestroyTexture(bottleTex);
    SDL_DestroyTexture(bagTex);
    SDL_DestroyTexture(cupTex);
    SDL_DestroyTexture(colaTex);
    SDL_DestroyTexture(smallcanTex);
    SDL_DestroyTexture(beerTex);
    SDL_DestroyTexture(swordfishTexture);
    SDL_DestroyTexture(eelTexture);
    SDL_DestroyTexture(octopusTexture);
    SDL_DestroyTexture(anglerTexture);
    SDL_DestroyTexture(heartTex);
    SDL_DestroyTexture(ocean);
    // submarine texture is owned by Submarine and will be destroyed there
}
