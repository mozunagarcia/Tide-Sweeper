#include "GameManager.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>
#include "Litter.h"
#include "Enemies.h"
#include "GameOverScreen.h"
#include "Messages.h" 
#include "StoryManager.h"

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
    : window(window_),
      renderer(renderer_),
      level(nullptr),
      submarine(nullptr),
      scoreboard(nullptr),
      messages(nullptr),
      menu(nullptr),
      running(true),
      startGame(false),
      backgroundMusic(nullptr),
      levelCompleteSound(nullptr),   
      animalCollisionSound(nullptr), 
      showingLevel4Intro(false),
      level4IntroTimer(0),
      level4IntroBlinkCounter(0)
{
    // Create menu 
    menu = new Menu(renderer);

    // Initialize upgraded Messages system
    msgManager = new Messages(renderer);
    storyManager = new StoryManager(msgManager);
    storyManager->reset();

    facts = {
        "Lost fishing line can trap animals and stay in the ocean for up to 600 years.",
        "Sea turtles often mistake plastic bags for jellyfish and can choke or starve.",
        "Ghost nets make up a major part of ocean plastic and trap animals for years.",
        "Illegal dumping harms marine life and destroys fragile ecosystems.",
        "Over 8 million tons of plastic enter the ocean every year.",
        "Microplastics have been found in Arctic snow and deep-sea trenches.",
        "Plastic never fully disappears; it breaks into tiny pieces that last for centuries.",
        "Coral reefs can get sick from chemicals in sunscreen or plastic waste.",
        "Around 700 marine species are harmed by plastic pollution.",
        "The Great Pacific Garbage Patch is larger than Texas.",
        "Some plastics absorb toxic chemicals and become more dangerous to animals.",
        "Cigarette filters are the most common litter found on beaches.",
        "One liter of oil can pollute up to one million liters of seawater.",
        "The ocean floor contains millions of tons of trash, including lost cargo.",
        "Recycling one plastic bottle saves enough energy to power a light bulb for hours."
    };
}

GameManager::~GameManager() {

    if (level) {
        delete level;
        level = nullptr;
    }

    if (submarine) {
        delete submarine;
        submarine = nullptr;
    }

    if (scoreboard) {
        delete scoreboard;
        scoreboard = nullptr;
    }

    if (storyManager) {
        delete storyManager;
        storyManager = nullptr;
    }

    if (msgManager) {
        delete msgManager;
        msgManager = nullptr;
    }

    if (menu) {
        delete menu;
        menu = nullptr;
    }

    if (backgroundMusic) {
        Mix_HaltMusic();
        Mix_FreeMusic(backgroundMusic);
        backgroundMusic = nullptr;
    }
    if (timerSound) {
        Mix_FreeChunk(timerSound);
        timerSound = nullptr;
    }

    if (levelCompleteSound) {
        Mix_FreeChunk(levelCompleteSound);
        levelCompleteSound = nullptr;
    }

    if (animalCollisionSound) {
        Mix_FreeChunk(animalCollisionSound);
        animalCollisionSound = nullptr;
    }
    
    if (victorySound) {
        Mix_FreeChunk(victorySound);
        victorySound = nullptr;
    }
}



void GameManager::run() {
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
    
    // Load level complete sound effect
    levelCompleteSound = Mix_LoadWAV("Assets/sound_effects/level_complete1.wav");
    if (!levelCompleteSound) {
        std::cerr << "Failed to load level complete sound! Mix_Error: " << Mix_GetError() << std::endl;
    } else {
        Mix_VolumeChunk(levelCompleteSound, MIX_MAX_VOLUME / 8);  // Set volume
    }
    
    // Load animal collision sound effect
    animalCollisionSound = Mix_LoadWAV("Assets/sound_effects/animal_collision9.wav");
    if (!animalCollisionSound) {
        std::cerr << "Failed to load animal collision sound! Mix_Error: " << Mix_GetError() << std::endl;
    } else {
        Mix_VolumeChunk(animalCollisionSound, MIX_MAX_VOLUME / 4);  // Set volume
    }
    
    // Load victory sound effect
    victorySound = Mix_LoadWAV("Assets/sound_effects/victory.wav");
    if (!victorySound) {
        std::cerr << "Failed to load victory sound! Mix_Error: " << Mix_GetError() << std::endl;
    } else {
        Mix_VolumeChunk(victorySound, MIX_MAX_VOLUME);  // Set volume
    }
    
    // Load game background music
    backgroundMusic = Mix_LoadMUS("Assets/music/beach-house-tune-144457.mp3");
    if (!backgroundMusic) {
        backgroundMusic = Mix_LoadMUS("Assets/music/beach-house-tune-144457.wav");
        if (!backgroundMusic) {
            std::cerr << "Failed to load game music! Mix_Error: " << Mix_GetError() << std::endl;
        }
    }
    
    // Load 10-second timer sound for Level 4
    timerSound = Mix_LoadWAV("Assets/sound_effects/timer_10s.mp3");
    if (!timerSound) {
        std::cerr << "Failed to load timer sound! Mix_Error: " << Mix_GetError() << std::endl;
    } else {
        Mix_VolumeChunk(timerSound, MIX_MAX_VOLUME / 4);  // Set volume
    }
    
    // Play game music on loop
    if (backgroundMusic) {
        Mix_PlayMusic(backgroundMusic, -1);
        Mix_VolumeMusic(MIX_MAX_VOLUME / 2); // Set volume
    }


    // Load shared textures
    SDL_Texture* ocean = loadTexture(renderer, "Assets/backgrounds/Level1.png");
    SDL_Texture* submarineTex = loadTexture(renderer, "Assets/submarine.png");
    if (!ocean || !submarineTex) {
        std::cerr << "Missing textures! Place Level1.png and submarine.png in /assets\n";
        return;
    }

    // Pause + Game Over Backgrounds
    SDL_Texture* pauseBG = loadTexture(renderer, "Assets/backgrounds/gameover_bg.png");
    SDL_Texture* gameOverBG = loadTexture(renderer, "Assets/backgrounds/gameover_bg.png");

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
    std::vector<float> enemySpeeds = { 6.0f, 6.0f, 6.0f, 6.0f, 4.0f };
    std::vector<int> enemyWidths = { 70, 70, 60, 60, 60 };    // Swordfish, Eel, Octopus, Angler, Shark widths
    std::vector<int> enemyHeights = { 50, 30, 60, 55, 40 };   // Swordfish, Eel, Octopus, Angler, Shark heights

    SDL_Texture* heartTex = loadTexture(renderer, "Assets/heart.png");
    SDL_Texture* oilTex = loadTexture(renderer, "Assets/oil.png");

    // Scoreboard
    scoreboard = new Scoreboard(renderer, 650, 10, 140, 80);
    scoreboard->setScore(0);

    // Submarine
    int texW, texH;
    SDL_QueryTexture(submarineTex, nullptr, nullptr, &texW, &texH);

    // Scale tuned for your scene
    float scale = 0.11f;

    int subW = (int)(texW * scale);
    int subH = (int)(texH * scale);

    submarine = new Submarine(submarineTex, 200, 275, subW, subH);



    // Level: Start with Level1 (no animals)
    level = new Level1(renderer,
                       { canTex, bottleTex, bagTex, cupTex, colaTex, smallcanTex, beerTex },
                       enemyTextures, enemySpeeds, enemyWidths, enemyHeights);
    
    storyManager->setLevelPointer(level);

    // Set oil texture for level 3 blackout effect
    level->setOilTexture(oilTex);
    level->setAnimalCollisionSound(animalCollisionSound);

    storyManager->onLevelChange(1);

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
        storyManager->onLevelChange(1);
        msgManager->reset();
        msgManager->update();

        lives = 3;
        gameOver = false;
        submarine->setPosition(200, 275);
        submarine->reset();
        scoreboard->setScore(0);
        scoreboard->resetLevel();
        cameraX = 0.0f;
        currentLevel = 1;
        
        // Destroy old level and recreate as Level1
        delete level;
        level = new Level1(renderer,
                          { canTex, bottleTex, bagTex, cupTex, colaTex, smallcanTex, beerTex },
                          enemyTextures, enemySpeeds, enemyWidths, enemyHeights);
       
       storyManager->setLevelPointer(level);
       level->setOilTexture(oilTex);
       level->setAnimalCollisionSound(animalCollisionSound);
        
        // Reset ocean background to level 1
        SDL_DestroyTexture(ocean);
        ocean = loadTexture(renderer, "Assets/backgrounds/Level1.png");
        
        // Recreate Level 1 to properly reset all state
        delete level;
        level = new Level1(renderer,
                           { canTex, bottleTex, bagTex, cupTex, colaTex, smallcanTex, beerTex },
                           enemyTextures, enemySpeeds, enemyWidths, enemyHeights);
        
        storyManager->setLevelPointer(level);

        level->setOilTexture(oilTex);
        level->setAnimalCollisionSound(animalCollisionSound);

        // Reset music to start from the beginning
        if (backgroundMusic) {
            Mix_RewindMusic();
            Mix_PlayMusic(backgroundMusic, -1);
            Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
        }
    };

    SDL_Event event;
    srand(static_cast<unsigned int>(time(nullptr)));

    // Frame timing for accurate updates
    Uint32 lastTime = SDL_GetTicks();

    // Main loop
    while (running) {
        while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
        running = false;
    }

    //AUTO V ICTORY SCENE (for demo purposes)

    // Press V to trigger the victory screen instantly 
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_v) {

        // Stop all gameplay audio
        Mix_HaltMusic();
        if (victorySound) {
            Mix_PlayChannel(-1, victorySound, 0);
        }

        VictoryScreen vs(renderer);

        // Show victory screen immediately with your real facts & score
        std::string result = vs.run(scoreboard->getScore());


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

    // PAUSE MENU (ESC)
    if (!gameOver && event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {

        GameOverScreen pause(renderer, pauseBG);

        int factIndex = currentLevel - 1;
        if (factIndex < 0) factIndex = 0;
        if (factIndex >= facts.size()) factIndex = facts.size() - 1;

        std::string action = pause.run("Paused", facts);

        if (action == "resume") { 
            continue;   // resume
    }

        if (action == "restart") {
            resetGame();
            continue;   // restart
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
            
            // Check if submarine is in blackout to slow movement (only for Level3)
            SDL_Rect subRect = submarine->getRect();
            int subCenterX = subRect.x + subRect.w / 2;
            int subCenterY = subRect.y + subRect.h / 2;
            bool inBlackout = false;
            Level3* level3 = dynamic_cast<Level3*>(level);
            if (level3) {
                inBlackout = level3->isPositionInBlackout(subCenterX, subCenterY);
            }
            int moveSpeed = inBlackout ? 2 : 5; // Slow movement in blackout
            
            if (keys[SDL_SCANCODE_UP])    submarine->moveBy(0, -moveSpeed);
            if (keys[SDL_SCANCODE_DOWN])  submarine->moveBy(0, moveSpeed);
            if (keys[SDL_SCANCODE_LEFT])  submarine->moveBy(-moveSpeed, 0);
            if (keys[SDL_SCANCODE_RIGHT]) submarine->moveBy(moveSpeed, 0);
            
            // Calm ability with SPACE 
            if (keys[SDL_SCANCODE_SPACE]) {
                float subX = subRect.x + subRect.w / 2.0f;
                float subY = subRect.y + subRect.h / 2.0f;
                level->calmEnemies(subX, subY, 150.0f);  // 150 pixel radius
            }

            submarine->clamp(50, 650, 0, 540);

            // Update submarine blink effect
            submarine->updateBlink();

            // Level 4 intro sequence - pause gameplay
            if (showingLevel4Intro) {
                level4IntroTimer++;
                level4IntroBlinkCounter++;
                
                // End intro after 3 seconds 
                if (level4IntroTimer >= 180) {
                    showingLevel4Intro = false;
                    timerMusicPlayed = false;  // Reset timer music flag for Level 4
                }
            } else {
                // Normal gameplay - update level
                level->update(*submarine, *scoreboard, lives, gameOver);
                
                // Check if we're in Level 4 and need to play timer music
                if (currentLevel == 4 && !timerMusicPlayed) {
                    Level4* level4 = dynamic_cast<Level4*>(level);
                    if (level4 && level4->getStormTimer() <= 660) {  
                        if (timerSound) {
                            Mix_PlayChannel(-1, timerSound, 0);  // Play timer sound on available channel
                        }
                        timerMusicPlayed = true;
                    }
                }
            }

            int timeRemaining = 0;

            // If level 4, get the timer
            if (currentLevel == 4)
            {
                Level4* lvl4 = dynamic_cast<Level4*>(level);
                if (lvl4) {
                    timeRemaining = lvl4->getStormTimer() / 60; // convert frames → seconds
                }
            }

            storyManager->update(scoreboard->getScore(), scoreboard->getLevel(), timeRemaining);

            // FIRST ANIMAL DETECTION (Level 2) 
            if (currentLevel == 2 && !storyManager->animalMessagePlayed)
            {
                // If enemies exist, an animal has spawned
                if (!level->getEnemyItems().empty())
                {
                    storyManager->onFirstAnimal();
                }
            }

            // FIRST OIL SLICK DETECTION (Level 3) 
            if (currentLevel == 3 && !storyManager->oilMessagePlayed)
            {
                 Level3* level3 = dynamic_cast<Level3*>(level);
            if (level3)
            {
                // Oil slick begins the moment the warning phase activates
                if (level3->isOilWarning())
                {
                    storyManager->onOilDetected();
                }
            }
            }
            
            // Detect level changes and swap background + create new level instance
            {
                int newLevel = scoreboard->getLevel();
                if (newLevel != currentLevel) {
                   
                    storyManager->onLevelEnd(currentLevel);

                    // Play level complete sound
                    if (levelCompleteSound) {
                        Mix_PlayChannel(-1, levelCompleteSound, 0);
                    }
                    currentLevel = newLevel;
                    
                    // Save litter state before deleting old level
                    std::vector<Litter> savedLitter = level->getLitterItems();
                    std::vector<Enemies> savedEnemies = level->getEnemyItems();

                    // Delete old level and create new one based on level number
                    delete level;
                    level = nullptr;

                    storyManager->onLevelChange(currentLevel);
                    
                    if (currentLevel == 1) {
                        level = new Level1(renderer,
                                          { canTex, bottleTex, bagTex, cupTex, colaTex, smallcanTex, beerTex },
                                          enemyTextures, enemySpeeds, enemyWidths, enemyHeights);
                       storyManager->setLevelPointer(level);

                       level->setLitterItems(savedLitter);
                        level->setEnemyItems(savedEnemies);
                        level->setAnimalCollisionSound(animalCollisionSound);
                    }
                    else if (currentLevel == 2) {                 
                        level = new Level2(renderer,
                                          { canTex, bottleTex, bagTex, cupTex, colaTex, smallcanTex, beerTex },
                                          enemyTextures, enemySpeeds, enemyWidths, enemyHeights);
                        storyManager->setLevelPointer(level);

                        level->setLitterItems(savedLitter);
                        level->setEnemyItems(savedEnemies);
                        level->setAnimalCollisionSound(animalCollisionSound);
                        
                        SDL_DestroyTexture(ocean);
                        SDL_Texture* newOcean = loadTexture(renderer, "Assets/backgrounds/Level2.png");
                        if (newOcean) {
                            ocean = newOcean;
                        } else {
                            SDL_Texture* altOcean = loadTexture(renderer, "Assets/backgrounds/Level2.png");
                            if (altOcean) ocean = altOcean;
                            else std::cerr << "Failed to load Level 2background: Assets/Level2.png" << std::endl;
                        }
                    }
                    else if (currentLevel == 3) {
                        level = new Level3(renderer,
                                          { canTex, bottleTex, bagTex, cupTex, colaTex, smallcanTex, beerTex },
                                          enemyTextures, enemySpeeds, enemyWidths, enemyHeights);
                       storyManager->setLevelPointer(level);

                        level->setLitterItems(savedLitter);
                        level->setEnemyItems(savedEnemies);
                        level->setOilTexture(oilTex);
                        level->setAnimalCollisionSound(animalCollisionSound);
                        
                        SDL_DestroyTexture(ocean);
                        SDL_Texture* newOcean = loadTexture(renderer, "Assets/backgrounds/Level3.png");
                         if (newOcean) {
                            ocean = newOcean;
                        } else {
                            SDL_Texture* altOcean = loadTexture(renderer, "Assets/backgrounds/Level3.png");
                            if (altOcean) ocean = altOcean;
                            else std::cerr << "Failed to load ocean3.png: Assets/background/Level3.png" << std::endl;
                        }
                    }
                    else if (currentLevel >= 4) {
                        // Load final level background FIRST
                        SDL_DestroyTexture(ocean);
                        SDL_Texture* trashCluster = loadTexture(renderer, "Assets/backgrounds/Level4.png");
                        if (trashCluster) {
                            ocean = trashCluster;
                        } else {
                            std::cerr << "Failed to load Level4.png" << std::endl;
                        }
                        
                        // Start Level 4 intro sequence
                        showingLevel4Intro = true;
                        level4IntroTimer = 0;
                        level4IntroBlinkCounter = 0;
                        
                        level = new Level4(renderer,
                                          { canTex, bottleTex, bagTex, cupTex, colaTex, smallcanTex, beerTex },
                                          enemyTextures, enemySpeeds, enemyWidths, enemyHeights);
                        storyManager->setLevelPointer(level);

                        level->setOilTexture(oilTex);
                        level->setAnimalCollisionSound(animalCollisionSound);
                    }
                }
            }
        }

        // Scroll background (faster in Level 4)
        float effectiveScrollSpeed = scrollSpeed;
        if (currentLevel == 4) {
            Level4* level4 = dynamic_cast<Level4*>(level);
            if (level4) {
                effectiveScrollSpeed += level4->getScrollOffset() * 0.1f;  // Additional scroll
            }
        }
        cameraX += effectiveScrollSpeed;
        if (cameraX >= bgWidth) cameraX -= bgWidth;

        // Render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Rect srcRect = { 0, 0, bgWidth, bgHeight };
        SDL_Rect dest1 = { static_cast<int>(-cameraX), 0, bgWidth, bgHeight };
        SDL_Rect dest2 = { static_cast<int>(-cameraX) + bgWidth, 0, bgWidth, bgHeight };
        SDL_RenderCopy(renderer, ocean, &srcRect, &dest1);
        SDL_RenderCopy(renderer, ocean, &srcRect, &dest2);

        // Level 4 intro overlay
        if (showingLevel4Intro) {
            // Render scrolling background animation
            cameraX += scrollSpeed * 2.0f;  // Faster scroll during intro
            
            // "Ready, Set, Go" 
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            
            // Each color shows for 0.5 seconds (30 frames)
            int phase = level4IntroBlinkCounter / 30;  // 0, 1, 2, 3, 4, 5
            
            switch (phase) {
                case 0:
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 100);    // Red (Ready)
                    break;
                case 1:
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);      // Black
                    break;
                case 2:
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 100);  // Yellow (Set)
                    break;
                case 3:
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);      // Black
                    break;
                case 4:
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 100);    // Green (Go)
                    break;
                default:
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);      // Black
                    break;
            }
            
            SDL_Rect overlayRect = {0, 0, 800, 600};
            SDL_RenderFillRect(renderer, &overlayRect);
            
            // Render intro text
            TTF_Font* introFont = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 32);
            if (introFont) {
                SDL_Color textColor = {255, 255, 255, 255};
                
                // Line 1: "Final Level"
                SDL_Surface* line1Surf = TTF_RenderText_Blended(introFont, "Final Level", textColor);
                if (line1Surf) {
                    SDL_Texture* line1Tex = SDL_CreateTextureFromSurface(renderer, line1Surf);
                    if (line1Tex) {
                        SDL_Rect line1Rect = {400 - line1Surf->w / 2, 200, line1Surf->w, line1Surf->h};
                        SDL_RenderCopy(renderer, line1Tex, nullptr, &line1Rect);
                        SDL_DestroyTexture(line1Tex);
                    }
                    SDL_FreeSurface(line1Surf);
                }
                
                // Line 2: "Collect as much as you can"
                SDL_Surface* line2Surf = TTF_RenderText_Blended(introFont, "Collect as much as you can", textColor);
                if (line2Surf) {
                    SDL_Texture* line2Tex = SDL_CreateTextureFromSurface(renderer, line2Surf);
                    if (line2Tex) {
                        SDL_Rect line2Rect = {400 - line2Surf->w / 2, 260, line2Surf->w, line2Surf->h};
                        SDL_RenderCopy(renderer, line2Tex, nullptr, &line2Rect);
                        SDL_DestroyTexture(line2Tex);
                    }
                    SDL_FreeSurface(line2Surf);
                }
                
                // Line 3: "before the timer runs out!"
                SDL_Surface* line3Surf = TTF_RenderText_Blended(introFont, "before the timer runs out!", textColor);
                if (line3Surf) {
                    SDL_Texture* line3Tex = SDL_CreateTextureFromSurface(renderer, line3Surf);
                    if (line3Tex) {
                        SDL_Rect line3Rect = {400 - line3Surf->w / 2, 320, line3Surf->w, line3Surf->h};
                        SDL_RenderCopy(renderer, line3Tex, nullptr, &line3Rect);
                        SDL_DestroyTexture(line3Tex);
                    }
                    SDL_FreeSurface(line3Surf);
                }
                
                TTF_CloseFont(introFont);
            }
        } else {
            // Normal gameplay rendering
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
        }

// Check if Level 4 timer has completed (treat as game over for now)
if (currentLevel == 4 && lives > 0) {
    Level4* level4 = dynamic_cast<Level4*>(level);
    if (level4 && level4->getStormTimer() <= 0) {
        gameOver = true;  // Trigger game over when timer completes
    }
}

// Check for victory condition (Level 4 timer completed with lives > 0)
bool victory = false;
if (currentLevel == 4 && lives > 0) {
    Level4* level4 = dynamic_cast<Level4*>(level);
    if (level4 && level4->getStormTimer() <= 0) {
        victory = true;
    }
}

if (victory) {
    // Stop background music and play victory sound
    Mix_HaltMusic();
    if (victorySound) {
        Mix_PlayChannel(-1, victorySound, 0);
    }
    
    VictoryScreen vs(renderer);
    std::string result = vs.run(scoreboard->getScore());

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
} else if (gameOver) {
    GameOverScreen go(renderer, gameOverBG);

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

        SDL_RenderPresent(renderer);
        
        // Frame timing - maintain exactly 60 FPS
        Uint32 frameEnd = SDL_GetTicks();
        Uint32 frameTime = frameEnd - lastTime;
        
        if (frameTime < 17) {  // Target 17ms per frame for ~60 FPS
            SDL_Delay(17 - frameTime);
        }
        
        lastTime = SDL_GetTicks();
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