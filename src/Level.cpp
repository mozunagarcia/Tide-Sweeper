#include "Level.h"
#include <SDL.h>
#include <cstdlib>
#include <ctime>
#include <iostream>

Level::Level(SDL_Renderer* renderer_, const std::vector<SDL_Texture*>& litterTextures,
             const std::vector<SDL_Texture*>& enemyTextures_, const std::vector<float>& enemySpeeds_)
    : renderer(renderer_), enemyTextures(enemyTextures_), enemySpeeds(enemySpeeds_), spawnTimer(0),
      oilTexture(nullptr), blackoutNext(0), warningFrameCounter(0), isBlackout(false), isWarning(false), blackoutCounter(0)
{
    // Create litter from provided textures using the original initial positions/speeds
    // If there are fewer textures than expected, fall back to a simple spawn pattern
    if (litterTextures.size() >= 7) {
        litterItems.emplace_back(Litter(litterTextures[0], 200, 300, 1.5f));
        litterItems.emplace_back(Litter(litterTextures[1], 500, 400, 2.0f));
        litterItems.emplace_back(Litter(litterTextures[2], 650, 250, 1.8f));
        litterItems.emplace_back(Litter(litterTextures[3], 350, 200, 1.3f));
        litterItems.emplace_back(Litter(litterTextures[4], 700, 500, 2.2f));
        litterItems.emplace_back(Litter(litterTextures[5], 100, 450, 1.6f));
        litterItems.emplace_back(Litter(litterTextures[6], 400, 350, 1.9f));
    } else {
        for (auto tex : litterTextures) {
            litterItems.emplace_back(Litter(tex, 850, rand() % 500 + 50, 1.5f));
        }
    }
}

Level::~Level() {
    // Textures are owned elsewhere; Litter/Enemies don't free them here
}

void Level::reset() {
    // Reset litter positions and make them active
    for (auto& l : litterItems) {
        l.active = true;
        l.x = 850; l.y = rand() % 500 + 50;
    }
    enemyItems.clear();
    spawnTimer = 0;
    
    // Reset blackout variables
    blackoutNext = 0;
    isBlackout = false;
    isWarning = false;
    blackoutCounter = 0;
    warningFrameCounter = 0;
    oilSpots.clear();
}

void Level::update(Submarine& submarine, Scoreboard& scoreboard, int& lives, bool& gameOver, int currentLevel) {
    // Update litter
    for (auto& litter : litterItems) {
        bool missed = litter.update();
        if (missed) {
            scoreboard.setScore(scoreboard.getScore() - 10);
        }
        if (litter.active && litter.checkCollision(submarine.getRect())) {
            litter.collect();
            scoreboard.setScore(scoreboard.getScore() + 10);
        }
    }

    // Spawn enemies periodically
    spawnTimer++;
    if (spawnTimer >= spawnInterval) {
        spawnTimer = 0;
        int activeCount = 0;
        for (const auto& enemy : enemyItems) if (enemy.active) activeCount++;
        if (activeCount < maxActiveEnemies && !enemyTextures.empty()) {
            int randomIndex = rand() % enemyTextures.size();
            float startX = 850;
            float startY = rand() % 500 + 50;
            enemyItems.emplace_back(enemyTextures[randomIndex], startX, startY, enemySpeeds[randomIndex]);
        }
    }

    // Update enemies
    for (auto it = enemyItems.begin(); it != enemyItems.end();) {
        if (it->x < -100) {
            it = enemyItems.erase(it);
        } else {
            it->update();
            if (it->checkCollision(submarine.getRect())) {
                lives--;
                it = enemyItems.erase(it);
                if (lives <= 0) gameOver = true;
            } else ++it;
        }
    }
    
    // Update blackout mechanic for level 3
    updateBlackoutMechanic(currentLevel);
}

void Level::render() {
    for (auto& litter : litterItems) litter.render(renderer);
    for (auto& e : enemyItems) e.render(renderer);
}

void Level::setOilTexture(SDL_Texture* oilTex) {
    oilTexture = oilTex;
}

void Level::updateBlackoutMechanic(int currentLevel) {
    if (currentLevel != 3) return;
    
    blackoutNext++;
    if (isBlackout) {
        blackoutCounter++;
        if (blackoutCounter >= blackoutDuration) {
            // End blackout
            isBlackout = false;
            isWarning = false;
            blackoutCounter = 0;
            blackoutNext = 0;
            oilSpots.clear();
        }
    } else if (isWarning) {
        // In warning phase, update oil spots appearance
        warningFrameCounter++;
        
        // Update alpha for fading in oil spots
        for (auto& spot : oilSpots) {
            if (warningFrameCounter >= spot.spawnFrame) {
                // Fade in over 15 frames
                if (spot.alpha < 1.0f) {
                    spot.alpha += 0.066f; // Reaches 1.0 in ~15 frames
                    if (spot.alpha > 1.0f) spot.alpha = 1.0f;
                }
            }
        }
        
        if (blackoutNext >= blackoutInterval + blackoutWarning) {
            // Start full blackout
            isBlackout = true;
            blackoutCounter = 0;
        }
    } else {
        if (blackoutNext >= blackoutInterval) {
            // Start warning phase with oil spots
            isWarning = true;
            warningFrameCounter = 0;
            oilSpots.clear();
            // Generate 3 huge oil spots positioned to cover the entire screen
            // Screen is 800x600, spots will overlap significantly
            OilSpot spot1, spot2, spot3;
            
            // Top-left quadrant spot
            spot1.x = 50;
            spot1.y = 50;
            spot1.size = 600; // Very large
            spot1.spawnFrame = 0;
            spot1.alpha = 0.0f;
            oilSpots.push_back(spot1);
            
            // Top-right to bottom area spot
            spot2.x = 350;
            spot2.y = 100;
            spot2.size = 550;
            spot2.spawnFrame = 20;
            spot2.alpha = 0.0f;
            oilSpots.push_back(spot2);
            
            // Center-bottom spot
            spot3.x = 150;
            spot3.y = 200;
            spot3.size = 500;
            spot3.spawnFrame = 40;
            spot3.alpha = 0.0f;
            oilSpots.push_back(spot3);
        }
    }
}

void Level::renderBlackoutEffects(int currentLevel, Submarine& submarine) {
    if (currentLevel != 3) return;
    
    // Show oil spots during warning phase with fade-in effect
    if (isWarning && !isBlackout && oilTexture) {
        for (const auto& spot : oilSpots) {
            if (spot.alpha > 0.0f) {
                SDL_SetTextureAlphaMod(oilTexture, static_cast<Uint8>(spot.alpha * 255));
                SDL_Rect oilRect = { spot.x, spot.y, spot.size, spot.size };
                SDL_RenderCopy(renderer, oilTexture, nullptr, &oilRect);
            }
        }
        // Reset alpha mod
        SDL_SetTextureAlphaMod(oilTexture, 255);
    }
    
    // Full blackout overlay
    if (isBlackout) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Completely black
        SDL_Rect fullScreen = {0, 0, 800, 600};
        SDL_RenderFillRect(renderer, &fullScreen);
        
        // Re-render submarine on top of blackout
        submarine.render(renderer);
        
        // Display oil slick warning message
        TTF_Font* warningFont = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 18);
        if (warningFont) {
            SDL_Color warningColor = {255, 255, 0, 255}; // Yellow text
            SDL_Surface* warningSurf = TTF_RenderText_Blended_Wrapped(warningFont, "Be careful,\nyou're in an\noil slick!", warningColor, 130);
            if (warningSurf) {
                SDL_Texture* warningTex = SDL_CreateTextureFromSurface(renderer, warningSurf);
                if (warningTex) {
                    SDL_Rect warningRect = {
                        660, // Position to the right of scoreboard
                        95,  // Just below scoreboard
                        warningSurf->w,
                        warningSurf->h
                    };
                    SDL_RenderCopy(renderer, warningTex, NULL, &warningRect);
                    SDL_DestroyTexture(warningTex);
                }
                SDL_FreeSurface(warningSurf);
            }
            TTF_CloseFont(warningFont);
        }
    }
}
