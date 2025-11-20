#include "GameManager.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include "Litter.h"
#include "Enemies.h"
// ----- EDIT START -----
#include "GameOverScreen.h"
#include "Messages.h" 

//#include "ClueScreen.h"
// ----- EDIT END -----

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
    
// ----- EDIT START -----
    // Initialize upgraded Messages system
    msgManager = new Messages(renderer);

    facts = {
    "Lost fishing line can trap animals and stay in the ocean for up to 600 years.",
    "Sea turtles often mistake plastic bags for jellyfish and can choke or starve.",
    "Ghost nets make up a major part of ocean plastic and trap animals for years.",
    "Illegal dumping harms marine life and destroys fragile ecosystems."
};
// ----- EDIT END -----


    // We'll initialize game objects when entering the gameplay loop inside run()
}

GameManager::~GameManager() {
    delete level;
    delete submarine;
    delete scoreboard;
    delete messages;
    delete msgManager;
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

//--- edit---
    // Start-of-level 1 transition messages
std::vector<std::string> level1Start = {
    "Entering Zone: Coastal Trash Vortex",
    // "This storm is small, but don't underestimate it. Use this mission to get a feel for maneuvering in circular debris flow."
};

// Mid-level milestone messages (triggered every 30 points)
std::vector<std::string> level1Milestones = {
    "Trash currents detected. Move carefully.",
    "Animal signature nearby - avoid contact.",
    "Good work - debris density increasing ahead."
};
int level1MilestoneIndex = 0;

// End-of-level transition (shown when Level 2 begins)
std::string level1End = "Storm dispersing... nice job out there.";

// First time threshold for point-based messages
int nextMessageScore = 30;

//--- edit---

    if (!running) return;

    // ----- EDIT START -----
msgManager->loadMessageList(level1Start);
msgManager->start();

// ----- EDIT END -----


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
    };

    SDL_Event event;
    srand(static_cast<unsigned int>(time(nullptr)));

    // Main loop
    while (running) {
        while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
        running = false;
    }

    // ----- PAUSE MENU (ESC) -----
    if (!gameOver && event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {

        GameOverScreen pause(renderer);

        int factIndex = currentLevel - 1;
        if (factIndex < 0) factIndex = 0;
        if (factIndex >= facts.size()) factIndex = facts.size() - 1;

        // Title is different for pause!
        std::string action = pause.run("Paused", facts);

        if (action == "resume") {
    continue;
}

        if (action == "restart") {
            resetGame();
            continue;
        }
        if (action == "menu") {
            resetGame();
            startGame = false;
            return;   // go back to main menu
        }
        if (action == "exit") {
            running = false;
            break;
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
            // ----- EDIT START -----
if (scoreboard->getLevel() == 1) {
    int currentScore = scoreboard->getScore();

    if (currentScore >= nextMessageScore) {
        if (level1MilestoneIndex < level1Milestones.size()) {
            msgManager->loadMessageList({ level1Milestones[level1MilestoneIndex] });
            msgManager->start();
            level1MilestoneIndex++;
        }
        nextMessageScore += 30;  // Next milestone = +30 points
    }
}
// ----- EDIT END -----
            // Detect level changes and swap background when reaching level 2
            {
                int newLevel = scoreboard->getLevel();
                if (newLevel != currentLevel) {
                    currentLevel = newLevel;
                    if (currentLevel == 2) {
                        // ----- EDIT START -----
                        msgManager->loadMessageList({ level1End });
                        msgManager->start();
// ----- EDIT END -----
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

// ----- EDIT START ----
if (gameOver) {
    GameOverScreen go(renderer);

    std::string result = go.run("Game Over!", facts);

    if (result == "restart") {
        resetGame();
        continue;
    }
    if (result == "menu") {
        resetGame();
        startGame = false;
        return;
    }
    if (result == "exit") {
        running = false;
        break;
    }
}

// Render level story messages
msgManager->update();
msgManager->render();

// ----- EDIT END -----
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