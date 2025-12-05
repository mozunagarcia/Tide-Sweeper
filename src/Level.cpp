#include "Level.h"
#include <SDL.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <SDL_ttf.h>

// Base Level Class Implementation
Level::Level(SDL_Renderer* renderer_, const std::vector<SDL_Texture*>& litterTextures,
             const std::vector<SDL_Texture*>& enemyTextures_, const std::vector<float>& enemySpeeds_,
             const std::vector<int>& enemyWidths_, const std::vector<int>& enemyHeights_)
    : renderer(renderer_), enemyTextures(enemyTextures_), enemySpeeds(enemySpeeds_), 
      enemyWidths(enemyWidths_), enemyHeights(enemyHeights_), spawnTimer(0),
      spawnInterval(120), maxActiveEnemies(2), animalCollisionSound(nullptr),
      oilTexture(nullptr), blackoutNext(0), warningFrameCounter(0), 
      isBlackout(false), isWarning(false), blackoutCounter(0),
      blackoutInterval(600), blackoutWarning(120), blackoutDuration(300), blackoutWidth(0),
      isBlackoutFading(false), isBlackoutFullyCovered(false), fullCoverCounter(0)
{
    std::vector<int> litterWidths;
    std::vector<int> litterHeights;

    for (auto tex : litterTextures) {
        int w = 0, h = 0;
        SDL_QueryTexture(tex, NULL, NULL, &w, &h);

        // Scale down here if you want universal smaller sizes
        float scale = 0.15f;  
        w = int(w * scale);
        h = int(h * scale);

        litterWidths.push_back(w);
        litterHeights.push_back(h);

    }

    // Create litter from provided textures using the original initial positions/speeds
    if (litterTextures.size() >= 7) {
        litterItems.emplace_back(Litter(litterTextures[0], 200, 300, 1.5f, litterWidths[0], litterHeights[0]));
        litterItems.emplace_back(Litter(litterTextures[1], 500, 400, 2.0f, litterWidths[1], litterHeights[1]));
        litterItems.emplace_back(Litter(litterTextures[2], 650, 250, 1.8f, litterWidths[2], litterHeights[2]));
        litterItems.emplace_back(Litter(litterTextures[3], 350, 200, 1.3f, litterWidths[3], litterHeights[3]));
        litterItems.emplace_back(Litter(litterTextures[4], 700, 500, 2.2f, litterWidths[4], litterHeights[4]));
        litterItems.emplace_back(Litter(litterTextures[5], 100, 450, 1.6f, litterWidths[5], litterHeights[5]));
        litterItems.emplace_back(Litter(litterTextures[6], 400, 350, 1.9f, litterWidths[6], litterHeights[6]));
    } else {
        for (int i = 0; i < litterTextures.size(); i++) {
            litterItems.emplace_back(
                Litter(litterTextures[i],
                    850,
                    rand() % 500 + 50,
                    1.5f,
                    litterWidths[i],
                    litterHeights[i])
            );
        }
    }
}

Level::~Level() {}

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
    isBlackoutFullyCovered = false;
    fullCoverCounter = 0;
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
            float startX, startY;
            
            // Octopus spawns from bottom, others from right
            if (randomIndex == 2) {
                startX = rand() % 700 + 50;  // Random X position across screen
                startY = 600;  // Start from bottom
            } else {
                startX = 850;  // Start from right
                startY = rand() % 500 + 50;  // Random Y position
            }
            
            enemyItems.emplace_back(enemyTextures[randomIndex], startX, startY, enemySpeeds[randomIndex],
                                   enemyWidths[randomIndex], enemyHeights[randomIndex], randomIndex);
        }
    }

    // Update enemies
    SDL_Rect subRect = submarine.getRect();
    float subX = subRect.x + subRect.w / 2.0f;
    float subY = subRect.y + subRect.h / 2.0f;
    
    for (auto it = enemyItems.begin(); it != enemyItems.end();) {
        // Remove enemies that go off screen
        // Octopus removed when going off top, others when going off left
        bool offScreen = (it->enemyType == 2 && it->y < -100) || 
                        (it->enemyType != 2 && it->x < -100) || 
                        (it->y > 600 && it->falling);
        
        if (offScreen) {
            it = enemyItems.erase(it);
        } else {
            it->update(subX, subY);
            if (it->checkCollision(submarine.getRect()) && !it->falling) {
                lives--;
                submarine.startHitBlink();
                it->startHitBlink();
                it->startFalling();
                if (animalCollisionSound) {
                    Mix_PlayChannel(-1, animalCollisionSound, 0);
                }
                if (lives <= 0) gameOver = true;
                ++it;
            } else ++it;
        }
    }
}

// Default: no blackout mechanic (only in Level 3)
void Level::updateBlackoutMechanic() {}

void Level::render() {
    for (auto& litter : litterItems) litter.render(renderer);
    for (auto& e : enemyItems) e.render(renderer);
}

// Default: no blackout effects (only in Level 3)
void Level::renderBlackoutEffects(Submarine& submarine) {}

void Level::calmEnemies(float subX, float subY, float radius) {
    for (auto& enemy : enemyItems) {
        if (!enemy.active || enemy.calmed) continue;
        
        // Calculate distance from submarine to enemy center
        float enemyCenterX = enemy.x + enemy.width / 2.0f;
        float enemyCenterY = enemy.y + enemy.height / 2.0f;
        float dx = enemyCenterX - subX;
        float dy = enemyCenterY - subY;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        // If enemy is within calm radius, deflect it slightly away
        if (distance <= radius && distance > 0) {
            // Normalize the direction vector
            float dirX = dx / distance;
            float dirY = dy / distance;
            
            // Set up gradual deflection
            enemy.deflecting = true;
            enemy.deflectDirX = dirX;
            enemy.deflectDirY = dirY;
            enemy.deflectTimer = 30;  // 30 frames for smooth deflection
            
            // Set enemy to calmed state (will move left after deflection)
            enemy.calmed = true;
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

void Level2::updateEnemies(Submarine& submarine, int& lives, bool& gameOver) {
    // Spawn enemies periodically (excluding octopuses)
    spawnTimer++;
    if (spawnTimer >= spawnInterval) {
        spawnTimer = 0;
        int activeCount = 0;
        for (const auto& enemy : enemyItems) if (enemy.active) activeCount++;
        if (activeCount < maxActiveEnemies && !enemyTextures.empty()) {
            // Exclude octopus (index 2) from Level 2
            int randomIndex;
            do {
                randomIndex = rand() % enemyTextures.size();
            } while (randomIndex == 2);  // Skip octopus
            
            float startX = 850;  // Start from right
            float startY = rand() % 500 + 50;  // Random Y position
            
            enemyItems.emplace_back(enemyTextures[randomIndex], startX, startY, enemySpeeds[randomIndex],
                                   enemyWidths[randomIndex], enemyHeights[randomIndex], randomIndex);
        }
    }

    // Update enemies (same as base class)
    SDL_Rect subRect = submarine.getRect();
    float subX = subRect.x + subRect.w / 2.0f;
    float subY = subRect.y + subRect.h / 2.0f;
    
    for (auto it = enemyItems.begin(); it != enemyItems.end();) {
        bool offScreen = it->x < -100 || (it->y > 600 && it->falling);
        
        if (offScreen) {
            it = enemyItems.erase(it);
        } else {
            it->update(subX, subY);
            if (it->checkCollision(submarine.getRect()) && !it->falling) {
                lives--;
                submarine.startHitBlink();
                it->startHitBlink();
                it->startFalling();
                if (animalCollisionSound) {
                    Mix_PlayChannel(-1, animalCollisionSound, 0);
                }
                if (lives <= 0) gameOver = true;
                ++it;
            } else ++it;
        }
    }
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
        blackoutCounter++; // Continue incrementing for wave animation
        // Blackout is receding from the left edge
        // Allow it to go below 0 so waves disappear completely beyond right edge
        if (blackoutWidth > -100) {
            blackoutWidth -= 2; // Slower fade out: 800/400 = 2 pixels per frame
        } else {
            // Completely faded out
            isBlackoutFading = false;
            isBlackout = false;
            blackoutCounter = 0;
            blackoutNext = 0;
        }
    } else if (isBlackout) {
        blackoutCounter++;
        // Expand blackout from right edge (takes about 400 frames to fully cover screen)
        // Add extra width (900) to allow waves to disappear completely beyond left edge
        if (blackoutWidth < 900) {
            blackoutWidth += 2; // 800/400 = 2 pixels per frame
        } else if (!isBlackoutFullyCovered) {
            // Blackout has reached full width, wait for waves to settle
            isBlackoutFullyCovered = true;
            fullCoverCounter = 0;
        } else {
            // Count frames while fully covered (wait ~60 frames for waves to disappear)
            fullCoverCounter++;
        }
        
        if (isBlackoutFullyCovered && fullCoverCounter >= 60 && blackoutCounter >= blackoutDuration) {
            // Start fading phase only after waves have settled
            isBlackoutFading = true;
            isBlackoutFullyCovered = false;
            // oilSpots.clear();
        }
    } else if (isWarning) {
        // In warning phase, update oil spots appearance
        warningFrameCounter++;
        
        // After warning period, trigger blackout
        if (warningFrameCounter >= blackoutWarning) {
            isWarning = false;
            isBlackout = true;
            blackoutCounter = 0;
            blackoutWidth = 0; // Start with width 0 for expansion effect
        }
    } else {
        // Check if it's time to start warning/blackout cycle
        if (blackoutNext >= blackoutInterval) {
            isWarning = true;
            warningFrameCounter = 0;
            blackoutNext = 0;  // Reset counter for next cycle
        }
    }
    
    // Level 3 specific: Create ink splotches near octopuses
    for (auto& enemy : enemyItems) {
        // Only create ink for octopuses (type 2)
        if (enemy.enemyType == 2 && enemy.active) {
            // Random chance to spawn ink (about 5% per frame)
            if (rand() % 100 < 5) {
                OilSpot inkSpot;
                // Spawn ink near the octopus position
                inkSpot.x = static_cast<int>(enemy.x) - 50 + (rand() % 100);
                inkSpot.y = static_cast<int>(enemy.y) - 50 + (rand() % 100);
                inkSpot.size = 250 + (rand() % 150);  // Random size 250-400 (bigger)
                inkSpot.spawnFrame = 0;
                inkSpot.alpha = 0.0f;
                oilSpots.push_back(inkSpot);
            }
        }
    }
    
    // Fade in and fade out ink spots
    for (auto it = oilSpots.begin(); it != oilSpots.end();) {
        it->spawnFrame++;
        
        // Fade in for first 20 frames
        if (it->spawnFrame < 20) {
            it->alpha += 0.05f;
            if (it->alpha > 1.0f) it->alpha = 1.0f;
            ++it;
        }
        // Stay visible for 60 frames
        else if (it->spawnFrame < 80) {
            ++it;
        }
        // Fade out for next 20 frames
        else if (it->spawnFrame < 100) {
            it->alpha -= 0.05f;
            if (it->alpha < 0.0f) it->alpha = 0.0f;
            ++it;
        }
        // Remove after fully faded
        else {
            it = oilSpots.erase(it);
        }
    }
}

void Level3::renderBlackoutEffects(Submarine& submarine) {
    // Show ink spots with fade-in effect
    if (oilTexture) {
        for (const auto& spot : oilSpots) {
            if (spot.alpha > 0.0f) {
                SDL_SetTextureAlphaMod(oilTexture, static_cast<Uint8>(spot.alpha * 240));  // Max 240 for darker ink
                SDL_Rect inkRect = { spot.x, spot.y, spot.size, spot.size };
                SDL_RenderCopy(renderer, oilTexture, nullptr, &inkRect);
            }
        }
        // Reset alpha mod
        SDL_SetTextureAlphaMod(oilTexture, 240);
    }
    
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
    }
}

bool Level3::isPositionInBlackout(int x, int y) {
    // No blackout if not active
    if (!isBlackout && !isBlackoutFading) {
        return false;
    }
    
    // If fully covered (no waves visible), entire screen is in blackout
    if (blackoutWidth >= 800 && !isBlackoutFading) {
        return true;
    }
    
    // Calculate wave offset for this y position
    float wave1 = sin((y * 0.05f) + (blackoutCounter * 0.03f)) * 25.0f;
    float wave2 = sin((y * 0.15f) + (blackoutCounter * 0.05f)) * 15.0f;
    float wave3 = sin((y * 0.08f) - (blackoutCounter * 0.02f)) * 10.0f;
    float wave = wave1 + wave2 + wave3;
    
    if (isBlackoutFading) {
        // During fade: blackout is from x=0 to blackoutWidth+wave
        int xEnd = blackoutWidth + static_cast<int>(wave);
        return x <= xEnd;
    } else {
        // During expansion: blackout is from 800-blackoutWidth+wave to right edge
        int xStart = 800 - blackoutWidth + static_cast<int>(wave);
        return x >= xStart;
    }
}

// ============================================================================
// Level 4: Superstorm Surge - Final level with timer
// ============================================================================

Level4::Level4(SDL_Renderer* renderer,
               const std::vector<SDL_Texture*>& litterTextures,
               const std::vector<SDL_Texture*>& enemyTextures,
               const std::vector<float>& enemySpeeds,
               const std::vector<int>& enemyWidths,
               const std::vector<int>& enemyHeights)
    : Level3(renderer, litterTextures, enemyTextures, enemySpeeds, enemyWidths, enemyHeights),
      stormTimer(1800),  // 60 seconds at 60 FPS
      stormPulseCounter(0),
      litterSpeedMultiplier(5.0f),
      scrollOffset(0),
      litterSpawnTimer(0),
      storedLitterTextures(litterTextures)
      //scrollSpeed(0.65f)  // Scroll feed for level 4
{
    // More enemies for increased challenge in final level
    maxActiveEnemies = 4;  // 4 enemies on screen
    spawnInterval = 120;   // spawns every 2 seconds

    float scale = 0.15f;        // choose your litter scale
    for (auto tex : storedLitterTextures) {
        int w = 0, h = 0;
        SDL_QueryTexture(tex, NULL, NULL, &w, &h);
        scaledWidths.push_back(int(w * scale));
        scaledHeights.push_back(int(h * scale));
    }
    
    // Clear all litter from base class and Level 3
    litterItems.clear();
 }

void Level4::update(Submarine& submarine, Scoreboard& scoreboard, int& lives, bool& gameOver) {
    // Decrease timer
    if (stormTimer > 0) {
        stormTimer--;
    }
    
    // Spawn new litter from the right side continuously
    litterSpawnTimer++;
    if (litterSpawnTimer >= 10 && storedLitterTextures.size() >= 7) {  // Spawn every 0.17 seconds 
        litterSpawnTimer = 0;
        // Spawn 2-3 pieces of litter at once for higher density
        int spawnCount = 2 + (rand() % 2);  // 2 or 3 items
        for (int i = 0; i < spawnCount; i++) {
            int texIndex = rand() % 7;
            int randomY = 50 + (rand() % 500);  // Keep within visible area
            float randomSpeed = 4.0f;// 1.5f + (rand() % 15) / 10.0f;  // Speed between 1.5 and 3.0
            int randomX = 850 + (rand() % 100);  // Slight variation in spawn position
            
            int w = scaledWidths[texIndex];
            int h = scaledHeights[texIndex];

            litterItems.emplace_back(
                Litter(storedLitterTextures[texIndex], randomX, randomY, randomSpeed, w, h)
            );
        }
    }
    
    // Update litter - continuous flow from right to left
    for (auto& litter : litterItems) {
        if (!litter.active) {
            continue;
        }
        
        // Move the litter left (continuous flow)
        litter.x -= litter.speed;
        
        // Remove litter that goes off the left side
        if (litter.x < -100) {
            litter.active = false;
        }
        
        // Check collision with submarine
        if (litter.active && litter.checkCollision(submarine.getRect())) {
            litter.active = false;
            scoreboard.setScore(scoreboard.getScore() + 10);
        }
    }
    
    // Update enemies (with increased spawn rate from constructor)
    updateEnemies(submarine, lives, gameOver);
}

// Override to disable ink/oil mechanics in Level 4
void Level4::updateBlackoutMechanic() {
    // No ink mechanics in final level
}

void Level4::updateEnemies(Submarine& submarine, int& lives, bool& gameOver) {
    // Spawn enemies periodically (excluding octopuses and sharks)
    spawnTimer++;
    if (spawnTimer >= spawnInterval) {
        spawnTimer = 0;
        int activeCount = 0;
        for (const auto& enemy : enemyItems) if (enemy.active) activeCount++;
        if (activeCount < maxActiveEnemies && !enemyTextures.empty()) {
            // Exclude octopus (index 2) and shark (index 4) from Level 4
            int randomIndex;
            do {
                randomIndex = rand() % enemyTextures.size();
            } while (randomIndex == 2 || randomIndex == 4);  // Skip octopus and shark
            
            float startX = 850;  // Start from right
            float startY = rand() % 500 + 50;  // Random Y position
            
            // Speed up enemies to match fast litter flow 
            float fastSpeed = enemySpeeds[randomIndex] * 3.0f;
            enemyItems.emplace_back(enemyTextures[randomIndex], startX, startY, fastSpeed,
                                   enemyWidths[randomIndex], enemyHeights[randomIndex], randomIndex);
        }
    }

    // Update enemies (same as base class)
    SDL_Rect subRect = submarine.getRect();
    float subX = subRect.x + subRect.w / 2.0f;
    float subY = subRect.y + subRect.h / 2.0f;
    
    for (auto it = enemyItems.begin(); it != enemyItems.end();) {
        bool offScreen = it->x < -100 || (it->y > 600 && it->falling);
        
        if (offScreen) {
            it = enemyItems.erase(it);
        } else {
            it->update(subX, subY);
            if (it->checkCollision(submarine.getRect()) && !it->falling) {
                lives--;
                submarine.startHitBlink();
                it->startHitBlink();
                it->startFalling();
                if (animalCollisionSound) {
                    Mix_PlayChannel(-1, animalCollisionSound, 0);
                }
                if (lives <= 0) gameOver = true;
                ++it;
            } else ++it;
        }
    }
}

void Level4::render() {
    // Render regular litter and enemies
    Level::render();
}

void Level4::renderBlackoutEffects(Submarine& submarine) {
    
    // Storm timer
    TTF_Font* timerFont = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 24);
    if (timerFont) {
        int seconds = stormTimer / 60;
        int minutes = seconds / 60;
        seconds = seconds % 60;
        
        char timerText[32];
        snprintf(timerText, sizeof(timerText), "Timer: %d:%02d", minutes, seconds);
        
        // Timer color: green -> yellow -> red as time runs out
        SDL_Color timerColor;
        if (stormTimer > 1800) {  // > 30 seconds
            timerColor = {0, 255, 0, 255};  // Green
        } else if (stormTimer > 600) {  // > 10 seconds
            timerColor = {255, 255, 0, 255};  // Yellow
        } else {
            timerColor = {255, 0, 0, 255};  // Red
        }
        
        SDL_Surface* timerSurf = TTF_RenderText_Blended(timerFont, timerText, timerColor);
        if (timerSurf) {
            SDL_Texture* timerTex = SDL_CreateTextureFromSurface(renderer, timerSurf);
            if (timerTex) {
                SDL_Rect timerRect = {
                    10,
                    50,
                    timerSurf->w,
                    timerSurf->h
                };
                SDL_RenderCopy(renderer, timerTex, NULL, &timerRect);
                SDL_DestroyTexture(timerTex);
            }
            SDL_FreeSurface(timerSurf);
        }
        TTF_CloseFont(timerFont);
    }
}
