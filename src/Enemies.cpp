#include "Enemies.h"
#include <cmath>

Enemies::Enemies(SDL_Texture* tex, float startX, float startY, float moveSpeed, int w, int h, int type)
    : texture(tex), x(startX), y(startY), speed(moveSpeed), active(true), respawnTimer(0), width(w), height(h), enemyType(type)
{
    if (type == 4) {
        width = static_cast<int>(w * 1.5f);
        height = static_cast<int>(h * 1.5f);
    }
}

void Enemies::update(float subX, float subY) {
    if (!active) return;
    
    // Update blink timer
    if (hitBlinkTimer > 0) {
        hitBlinkTimer--;
    }
    
    // If falling, apply gravity and check if off screen
    if (falling) {
        fallSpeed += 0.5f;  // Gravity acceleration
        y += fallSpeed;
        
        // Deactivate when off screen
        if (y > 600) {
            active = false;
        }
        return;  // Don't do normal movement when falling
    }
    
    // Calculate distance to submarine
    float dx = subX - x;
    float dy = subY - y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    // Only sharks (type 4) chase the submarine when in range
    if (enemyType == 4 && distance < detectionRadius && distance > 0) {
        // Normalize direction and move toward submarine
        float dirX = dx / distance;
        float dirY = dy / distance;
        x += dirX * speed;
        y += dirY * speed;
    } else if (enemyType == 2) {
        // Octopus (type 2) moves upward from bottom
        y -= speed;
    } else {
        // Normal behavior: move left
        x -= speed;
    }
}

void Enemies::render(SDL_Renderer* renderer) {
    if (!active) return;
    
    // Blink red if recently hit (alternating every 3 frames)
    if (hitBlinkTimer > 0 && (hitBlinkTimer / 3) % 2 == 0) {
        SDL_SetTextureColorMod(texture, 255, 100, 100);  // Red tint
    } else {
        SDL_SetTextureColorMod(texture, 255, 255, 255);  // Normal color (if not hit)
    }
    
    SDL_Rect dest = { static_cast<int>(x), static_cast<int>(y), width, height };
    SDL_RenderCopy(renderer, texture, nullptr, &dest);
    
    // Reset color mod
    SDL_SetTextureColorMod(texture, 255, 255, 255);
}

bool Enemies::checkCollision(const SDL_Rect& subRect) {
    if (!active) return false;
    SDL_Rect enemiesRect = { static_cast<int>(x), static_cast<int>(y), width, height };
    return (subRect.x < enemiesRect.x + enemiesRect.w &&
            subRect.x + subRect.w > enemiesRect.x &&
            subRect.y < enemiesRect.y + enemiesRect.h &&
            subRect.y + subRect.h > enemiesRect.y);
}

void Enemies::startHitBlink() {
    hitBlinkTimer = 30;  // Blink for 30 frames 
}

void Enemies::startFalling() {
    falling = true;
    fallSpeed = 0;
} 