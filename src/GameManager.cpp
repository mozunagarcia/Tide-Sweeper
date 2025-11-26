#include "GameManager.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>
#include "Litter.h"
#include "Enemies.h"
// ----- EDIT START -----
#include "GameOverScreen.h"
#include "Messages.h" 
#include "StoryManager.h"

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
    : window(window_), renderer(renderer_), level(nullptr), submarine(nullptr), scoreboard(nullptr), messages(nullptr), menu(nullptr), running(true), startGame(false), backgroundMusic(nullptr)
{
    // Create menu 
    menu = new Menu(renderer);
    
// ----- EDIT START -----
    // Initialize upgraded Messages system
    msgManager = new Messages(renderer);
    storyManager = new StoryManager(msgManager);


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
    delete msgManager;
    delete menu;
    
    // Stop and free music
    if (backgroundMusic) {
        Mix_HaltMusic();
        Mix_FreeMusic(backgroundMusic);
        backgroundMusic = nullptr;
    }
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

    // Stop menu music and load game music
    Mix_HaltMusic();
    
    // Load game background music
    backgroundMusic = Mix_LoadMUS("Assets/music/beach-house-tune-144457.mp3");
    if (!backgroundMusic) {
        // Try .wav if .mp3 doesn't exist
        backgroundMusic = Mix_LoadMUS("Assets/music/beach-house-tune-144457.wav");
        if (!backgroundMusic) {
            std::cerr << "Failed to load game music! Mix_Error: " << Mix_GetError() << std::endl;
        }
    }
    
    // Play game music on loop (-1 means infinite loop)
    if (backgroundMusic) {
        Mix_PlayMusic(backgroundMusic, -1);
        Mix_VolumeMusic(MIX_MAX_VOLUME / 2); // Set to 50% volume
    }


    // --- Load shared textures --- (paths kept identical to original)
    SDL_Texture* ocean = loadTexture(renderer, "Assets/ocean.png");
    SDL_Texture* submarineTex = loadTexture(renderer, "Assets/submarine.png");
    if (!ocean || !submarineTex) {
        std::cerr << "Missing textures! Place ocean.png and submarine.png in /assets\n";
        return;
    }
    //Radio textures
    SDL_Texture* radioTex = loadTexture(renderer, "Assets/Radio.png");

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
    SDL_Texture* sharkTexture = loadTexture(renderer, "Assets/Shark.png");

    std::vector<SDL_Texture*> enemyTextures = { swordfishTexture, eelTexture, octopusTexture, anglerTexture, sharkTexture };
    std::vector<float> enemySpeeds = { 4.0f, 3.8f, 3.5f, 4.2f, 4.5f };
    std::vector<int> enemyWidths = { 70, 70, 60, 60, 60 };    // Swordfish, Eel, Octopus, Angler, Shark widths
    std::vector<int> enemyHeights = { 50, 30, 60, 55, 40 };   // Swordfish, Eel, Octopus, Angler, Shark heights

    SDL_Texture* heartTex = loadTexture(renderer, "Assets/heart.png");
    SDL_Texture* oilTex = loadTexture(renderer, "Assets/oil.png");

    // Scoreboard
    scoreboard = new Scoreboard(renderer, 650, 10, 140, 80);
    scoreboard->setScore(0);

    // Submarine
    submarine = new Submarine(submarineTex, 200, 275, 100, 60);

    // Level: Start with Level1 (no animals)
    level = new Level1(renderer,
                       { canTex, bottleTex, bagTex, cupTex, colaTex, smallcanTex, beerTex },
                       enemyTextures, enemySpeeds, enemyWidths, enemyHeights);
    
    // Set oil texture for level 3 blackout effect
    level->setOilTexture(oilTex);

    //----edit
    storyManager->onLevelChange(1);
    //----edit
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
        //edit
        storyManager->reset(); 
        //edit
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
            
            // Calm ability with SPACE (prevents enemy from attacking when chasing is activated)
            if (keys[SDL_SCANCODE_SPACE]) {
                SDL_Rect subRect = submarine->getRect();
                float subX = subRect.x + subRect.w / 2.0f;
                float subY = subRect.y + subRect.h / 2.0f;
                level->calmEnemies(subX, subY, 150.0f);  // 150 pixel radius
            }

            submarine->clamp(50, 650, 0, 540);

            // Update submarine blink effect
            submarine->updateBlink();

            // Update submarine blink effect
            submarine->updateBlink();

            // Update level (handles litter/enemies and level 3 blackout)
            level->update(*submarine, *scoreboard, lives, gameOver);

            // --- edit start ---
            storyManager->update(scoreboard->getScore(), scoreboard->getLevel());
            // --- edit end ---

            
            // Detect level changes and swap background + create new level instance
            {
                int newLevel = scoreboard->getLevel();
                if (newLevel != currentLevel) {
                   
                    // --- edit start ---
                    storyManager->onLevelEnd(currentLevel);
                    // --- edit end ---

                    currentLevel = newLevel;
                    
                    // Save litter state before deleting old level
                    std::vector<Litter> savedLitter = level->getLitterItems();
                    std::vector<Enemies> savedEnemies = level->getEnemyItems();

                    // Delete old level and create new one based on level number
                    delete level;
                    level = nullptr;

                    // --- edit start ---
                    storyManager->onLevelChange(currentLevel);
                    // --- edit end ---
                    
                    if (currentLevel == 1) {
                        level = new Level1(renderer,
                                          { canTex, bottleTex, bagTex, cupTex, colaTex, smallcanTex, beerTex },
                                          enemyTextures, enemySpeeds, enemyWidths, enemyHeights);
                        level->setLitterItems(savedLitter);
                        level->setEnemyItems(savedEnemies);
                    }
                    else if (currentLevel == 2) {                 
                        level = new Level2(renderer,
                                          { canTex, bottleTex, bagTex, cupTex, colaTex, smallcanTex, beerTex },
                                          enemyTextures, enemySpeeds, enemyWidths, enemyHeights);
                        level->setLitterItems(savedLitter);
                        level->setEnemyItems(savedEnemies);
                        
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
                        level = new Level3(renderer,
                                          { canTex, bottleTex, bagTex, cupTex, colaTex, smallcanTex, beerTex },
                                          enemyTextures, enemySpeeds, enemyWidths, enemyHeights);
                        level->setLitterItems(savedLitter);
                        level->setEnemyItems(savedEnemies);
                        level->setOilTexture(oilTex);
                        
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
                    else if (currentLevel >= 4) {
                        level = new Level4(renderer,
                                          { canTex, bottleTex, bagTex, cupTex, colaTex, smallcanTex, beerTex },
                                          enemyTextures, enemySpeeds, enemyWidths, enemyHeights);
                        level->setLitterItems(savedLitter);
                        level->setEnemyItems(savedEnemies);
                        level->setOilTexture(oilTex);
                        // Keep level 3 background for level 4
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
        
        // Level 3+: Render blackout effects (oil spots and blackout overlay)
        level->renderBlackoutEffects(*submarine);
        
        scoreboard->render();

        // Draw hearts
        int heartSizeX = 40;
        int heartSizeY = 35;
        int16_t spacing = 5;
        int startX = 5;
        int startY = 5;
        for (int i = 0; i < lives; ++i) {
            SDL_Rect heartRect = { startX + i * (heartSizeX + spacing), startY, heartSizeX, heartSizeY };
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
storyManager->renderLevelChange(renderer);

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
    SDL_DestroyTexture(sharkTexture);
    SDL_DestroyTexture(heartTex);
    SDL_DestroyTexture(oilTex);
    SDL_DestroyTexture(ocean);
    // submarine texture is owned by Submarine and will be destroyed there
}