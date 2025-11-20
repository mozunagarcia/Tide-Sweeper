#include "Level.h"
#include <SDL.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cmath>

Level::Level(SDL_Renderer* renderer_, const std::vector<SDL_Texture*>& litterTextures,
             const std::vector<SDL_Texture*>& enemyTextures_, const std::vector<float>& enemySpeeds_,
             const std::vector<int>& enemyWidths_, const std::vector<int>& enemyHeights_)
    : renderer(renderer_), enemyTextures(enemyTextures_), enemySpeeds(enemySpeeds_), 
      enemyWidths(enemyWidths_), enemyHeights(enemyHeights_), spawnTimer(0)
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
    float subX = subRect.x + subRect.w / 2.0f;  // Submarine center X
    float subY = subRect.y + subRect.h / 2.0f;  // Submarine center Y
    
    for (auto it = enemyItems.begin(); it != enemyItems.end();) {
        if (it->x < -100 || (it->y > 600 && it->falling)) {  // Remove if off screen
            it = enemyItems.erase(it);
        } else {
            it->update(subX, subY);  // Pass submarine position
            if (it->checkCollision(submarine.getRect()) && !it->falling) {
                lives--;
                submarine.startHitBlink();  // Make submarine blink red
                it->startHitBlink();        // Make enemy blink red
                it->startFalling();         // Make enemy fall off screen
                if (lives <= 0) gameOver = true;
                ++it;
            } else ++it;
        }
    }
}

void Level::render() {
    for (auto& litter : litterItems) litter.render(renderer);
    for (auto& e : enemyItems) e.render(renderer);
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
