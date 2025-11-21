#include "Level.h"
#include <SDL.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cmath>

// ============================================================================
// Base Level class implementation
// ============================================================================

Level::Level(SDL_Renderer* renderer_, const std::vector<SDL_Texture*>& litterTextures,
             const std::vector<SDL_Texture*>& enemyTextures_, const std::vector<float>& enemySpeeds_,
             const std::vector<int>& enemyWidths_, const std::vector<int>& enemyHeights_)
    : renderer(renderer_), enemyTextures(enemyTextures_), enemySpeeds(enemySpeeds_), 
      enemyWidths(enemyWidths_), enemyHeights(enemyHeights_), spawnTimer(0),
      spawnInterval(120), maxActiveEnemies(2),
      oilTexture(nullptr), blackoutNext(0), warningFrameCounter(0), 
      isBlackout(false), isWarning(false), blackoutCounter(0),
      blackoutInterval(600), blackoutWarning(120), blackoutDuration(300), blackoutWidth(0),
      isBlackoutFading(false)
{
    // Create litter from provided textures using the original initial positions/speeds
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
    blackoutWidth = 0;
    isBlackoutFading = false;
    // oilSpots.clear();
}

void Level::update(Submarine& submarine, Scoreboard& scoreboard, int& lives, bool& gameOver) {
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

    // Update enemies (can be overridden in derived classes)
    updateEnemies(submarine, lives, gameOver);
    
    // Update blackout mechanic (only active in Level 3)
    updateBlackoutMechanic();
}

void Level::updateEnemies(Submarine& submarine, int& lives, bool& gameOver) {
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
            enemyItems.emplace_back(enemyTextures[randomIndex], startX, startY, enemySpeeds[randomIndex],
                                   enemyWidths[randomIndex], enemyHeights[randomIndex]);
        }
    }

    // Update enemies
    SDL_Rect subRect = submarine.getRect();
    float subX = subRect.x + subRect.w / 2.0f;
    float subY = subRect.y + subRect.h / 2.0f;
    
    for (auto it = enemyItems.begin(); it != enemyItems.end();) {
        if (it->x < -100 || (it->y > 600 && it->falling)) {
            it = enemyItems.erase(it);
        } else {
            it->update(subX, subY);
            if (it->checkCollision(submarine.getRect()) && !it->falling) {
                lives--;
                submarine.startHitBlink();
                it->startHitBlink();
                it->startFalling();
                if (lives <= 0) gameOver = true;
                ++it;
            } else ++it;
        }
    }
}

void Level::updateBlackoutMechanic() {
    // Default: no blackout mechanic (only in Level 3)
}

void Level::render() {
    for (auto& litter : litterItems) litter.render(renderer);
    for (auto& e : enemyItems) e.render(renderer);
}

void Level::renderBlackoutEffects(Submarine& submarine) {
    // Default: no blackout effects (only in Level 3)
}

void Level::calmEnemies(float subX, float subY, float radius) {
    for (auto& enemy : enemyItems) {
        if (!enemy.active) continue;
        
        // Calculate distance from submarine to enemy center
        float enemyCenterX = enemy.x + enemy.width / 2.0f;
        float enemyCenterY = enemy.y + enemy.height / 2.0f;
        float dx = enemyCenterX - subX;
        float dy = enemyCenterY - subY;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        // If enemy is within calm radius, deactivate it
        if (distance <= radius) {
            enemy.active = false;
        }
    }
}

void Level::setOilTexture(SDL_Texture* oilTex) {
    oilTexture = oilTex;
}

// ============================================================================
// Level 1: Only litter, no animals
// ============================================================================

Level1::Level1(SDL_Renderer* renderer,
               const std::vector<SDL_Texture*>& litterTextures,
               const std::vector<SDL_Texture*>& enemyTextures,
               const std::vector<float>& enemySpeeds,
               const std::vector<int>& enemyWidths,
               const std::vector<int>& enemyHeights)
    : Level(renderer, litterTextures, enemyTextures, enemySpeeds, enemyWidths, enemyHeights)
{
}

void Level1::update(Submarine& submarine, Scoreboard& scoreboard, int& lives, bool& gameOver) {
    // Update litter only (no enemies in Level 1)
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
    // No enemy spawning or updates in Level 1
}

// ============================================================================
// Level 2: Litter + Animals (uses base class implementation)
// ============================================================================

Level2::Level2(SDL_Renderer* renderer,
               const std::vector<SDL_Texture*>& litterTextures,
               const std::vector<SDL_Texture*>& enemyTextures,
               const std::vector<float>& enemySpeeds,
               const std::vector<int>& enemyWidths,
               const std::vector<int>& enemyHeights)
    : Level(renderer, litterTextures, enemyTextures, enemySpeeds, enemyWidths, enemyHeights)
{
}

// ============================================================================
// Level 3: Litter + Animals + Oil blackout mechanics
// ============================================================================

Level3::Level3(SDL_Renderer* renderer,
               const std::vector<SDL_Texture*>& litterTextures,
               const std::vector<SDL_Texture*>& enemyTextures,
               const std::vector<float>& enemySpeeds,
               const std::vector<int>& enemyWidths,
               const std::vector<int>& enemyHeights)
    : Level(renderer, litterTextures, enemyTextures, enemySpeeds, enemyWidths, enemyHeights)
{
}

void Level3::update(Submarine& submarine, Scoreboard& scoreboard, int& lives, bool& gameOver) {
    // Call base class update
    Level::update(submarine, scoreboard, lives, gameOver);
    
    // Level 3 specific: Update blackout mechanic
    blackoutNext++;
    if (isBlackoutFading) {
        // Blackout is receding from the left edge
        if (blackoutWidth > 0) {
            blackoutWidth -= 2; // Slower fade out: 800/400 = 2 pixels per frame
            if (blackoutWidth < 0) blackoutWidth = 0;
        } else {
            // Completely faded out
            isBlackoutFading = false;
            isBlackout = false;
            isWarning = false;
            blackoutCounter = 0;
            blackoutNext = 0;
        }
    } else if (isBlackout) {
        blackoutCounter++;
        // Expand blackout from right edge (takes about 400 frames to fully cover screen)
        if (blackoutWidth < 800) {
            blackoutWidth += 2; // 800/400 = 2 pixels per frame
            if (blackoutWidth > 800) blackoutWidth = 800;
        }
        if (blackoutCounter >= blackoutDuration) {
            // Start fading phase
            isBlackoutFading = true;
            // oilSpots.clear();
        }
    } else if (isWarning) {
        // In warning phase, update oil spots appearance
        warningFrameCounter++;
        
        // // Update alpha for fading in oil spots
        // for (auto& spot : oilSpots) {
        //     if (warningFrameCounter >= spot.spawnFrame) {
        //         // Fade in over 15 frames
        //         if (spot.alpha < 1.0f) {
        //             spot.alpha += 0.066f;
        //             if (spot.alpha > 1.0f) spot.alpha = 1.0f;
        //         }
        //     }
        // }
        
        if (blackoutNext >= blackoutInterval + blackoutWarning) {
            // Start full blackout
            isBlackout = true;
            blackoutCounter = 0;
            blackoutWidth = 0; // Start with width 0 for expansion effect
        }
    } else {
        if (blackoutNext >= blackoutInterval) {
            // Start warning phase with oil spots
            isWarning = true;
            warningFrameCounter = 0;
            // oilSpots.clear();
            
            // // Generate 3 huge oil spots positioned to cover the entire screen
            // OilSpot spot1, spot2, spot3;
            
            // spot1.x = 50;
            // spot1.y = 50;
            // spot1.size = 600;
            // spot1.spawnFrame = 0;
            // spot1.alpha = 0.0f;
            // oilSpots.push_back(spot1);
            
            // spot2.x = 350;
            // spot2.y = 100;
            // spot2.size = 550;
            // spot2.spawnFrame = 20;
            // spot2.alpha = 0.0f;
            // oilSpots.push_back(spot2);
            
            // spot3.x = 150;
            // spot3.y = 200;
            // spot3.size = 500;
            // spot3.spawnFrame = 40;
            // spot3.alpha = 0.0f;
            // oilSpots.push_back(spot3);
        }
    }
}

void Level3::renderBlackoutEffects(Submarine& submarine) {
    // // Show oil spots during warning phase with fade-in effect
    // if (isWarning && !isBlackout && oilTexture) {
    //     for (const auto& spot : oilSpots) {
    //         if (spot.alpha > 0.0f) {
    //             SDL_SetTextureAlphaMod(oilTexture, static_cast<Uint8>(spot.alpha * 255));
    //             SDL_Rect oilRect = { spot.x, spot.y, spot.size, spot.size };
    //             SDL_RenderCopy(renderer, oilTexture, nullptr, &oilRect);
    //         }
    //     }
    //     // Reset alpha mod
    //     SDL_SetTextureAlphaMod(oilTexture, 255);
    // }
    
    // Full blackout overlay - expands from right edge with wavy border
    if (isBlackout || isBlackoutFading) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        
        // If blackout has fully expanded and not fading, just fill the entire screen
        if (blackoutWidth >= 800 && !isBlackoutFading) {
            SDL_Rect fullScreen = {0, 0, 800, 600};
            SDL_RenderFillRect(renderer, &fullScreen);
        } else {
            // Draw blackout with wavy, irregular edge using vertical strips
            // Loop through each horizontal line of the screen
            for (int y = 0; y < 600; y++) {
                // Create irregular wave pattern using multiple sine waves at different frequencies
                // Combining multiple sine waves creates a more organic, irregular pattern
                float wave1 = sin((y * 0.05f) + (blackoutCounter * 0.03f)) * 25.0f;        // Primary wave
                float wave2 = sin((y * 0.15f) + (blackoutCounter * 0.05f)) * 15.0f;       // Secondary faster wave
                float wave3 = sin((y * 0.08f) - (blackoutCounter * 0.02f)) * 10.0f;       // Tertiary wave moving opposite
                float wave = wave1 + wave2 + wave3;  // Combine all waves for irregular pattern
                
                int xStart, width;
                
                if (isBlackoutFading) {
                    // During fade: blackout recedes from left, waves on right edge
                    // blackoutWidth shrinks from 800 to 0
                    xStart = 0;
                    int xEnd = blackoutWidth + static_cast<int>(wave);
                    
                    // Clamp to screen bounds
                    if (xEnd < 0) xEnd = 0;
                    if (xEnd > 800) xEnd = 800;
                    
                    width = xEnd;
                } else {
                    // During expansion: blackout expands from right, waves on left edge
                    // Calculate the starting x position for this row, applying the wave offset
                    // 800 - blackoutWidth is the base position (right edge moving left)
                    // + wave adds the wavy variation to create the irregular edge
                    xStart = 800 - blackoutWidth + static_cast<int>(wave);
                    
                    // Clamp to screen bounds to prevent drawing outside the window
                    if (xStart < 0) xStart = 0;
                    if (xStart > 800) xStart = 800;
                    
                    // Calculate the width of the blackout strip for this row
                    width = 800 - xStart;
                }
                
                if (width > 0) {
                    // Draw a 1-pixel tall horizontal strip for this row
                    SDL_Rect stripRect = {xStart, y, width, 1};
                    SDL_RenderFillRect(renderer, &stripRect);
                }
            }
        }
        
        // Re-render submarine on top of blackout
        submarine.render(renderer);
        
        // Display oil slick warning message
        TTF_Font* warningFont = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 18);
        if (warningFont) {
            SDL_Color warningColor = {255, 255, 0, 255};
            SDL_Surface* warningSurf = TTF_RenderText_Blended_Wrapped(warningFont, "Be careful,\nyou're in an\noil slick!", warningColor, 130);
            if (warningSurf) {
                SDL_Texture* warningTex = SDL_CreateTextureFromSurface(renderer, warningSurf);
                if (warningTex) {
                    SDL_Rect warningRect = {
                        660,
                        95,
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

// ============================================================================
// Level 4: Everything from Level 3 (can be extended later)
// ============================================================================

Level4::Level4(SDL_Renderer* renderer,
               const std::vector<SDL_Texture*>& litterTextures,
               const std::vector<SDL_Texture*>& enemyTextures,
               const std::vector<float>& enemySpeeds,
               const std::vector<int>& enemyWidths,
               const std::vector<int>& enemyHeights)
    : Level3(renderer, litterTextures, enemyTextures, enemySpeeds, enemyWidths, enemyHeights)
{
}
