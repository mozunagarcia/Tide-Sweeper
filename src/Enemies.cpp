#include "Enemies.h"
#include <cmath>

Enemies::Enemies(SDL_Texture* tex, float startX, float startY, float moveSpeed, int w, int h, int type)
    : texture(tex), x(startX), y(startY), speed(moveSpeed), active(true), respawnTimer(0), width(w), height(h), enemyType(type)
{
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
    
    // If calmed, move in deflection direction and ignore player
    if (calmed) {
        // If still deflecting, move in deflection direction
        if (deflecting && deflectTimer > 0) {
            x += deflectDirX * 4.0f;  // Increased deflection movement speed
            y += deflectDirY * 4.0f;
            deflectTimer--;
            
            if (deflectTimer == 0) {
                deflecting = false;  // Deflection complete
            }
            return;
        }
        
        // After deflection, move based on enemy type
        if (enemyType == 2) {
            // Octopus moves upward when calmed
            y -= speed;
            
            // Deactivate when off screen (top)
            if (y < -height) {
                active = false;
            }
        } else {
            // Other enemies move left
            x -= speed;
            
            // Update facing direction for sharks moving left
            if (enemyType == 4) {
                facingRight = false;
            }
            
            // Deactivate when off screen (left)
            if (x < -width) {
                active = false;
            }
        }
        return;
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
        
        // Update facing direction based on movement
        if (dirX > 0) {
            facingRight = true;  // Moving right
        } else if (dirX < 0) {
            facingRight = false; // Moving left
        }
        
        x += dirX * speed;
        y += dirY * speed;
    } else if (enemyType == 2) {
        // Octopus (type 2) moves upward from bottom
        y -= speed;
    } else {
        // Normal behavior: move left
        x -= speed;
        
        // Update facing direction for sharks moving left
        if (enemyType == 4) {
            facingRight = false;
        }
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
    
    // Flip shark sprite based on facing direction
    if (enemyType == 4) {
        SDL_RendererFlip flip = facingRight ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        SDL_RenderCopyEx(renderer, texture, nullptr, &dest, 0, nullptr, flip);
    } else {
        SDL_RenderCopy(renderer, texture, nullptr, &dest);
    }
    
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