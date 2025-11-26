#include "TrashCluster.h"
#include <cstdlib>

TrashCluster::TrashCluster(const std::vector<SDL_Texture*>& textures, float startX, float startY, int hp)
    : x(startX), y(startY), width(120), height(120), health(hp), maxHealth(hp), active(true), 
      litterTextures(textures), hitBlinkTimer(0)
{
}

void TrashCluster::update(float scrollSpeed) {
    if (!active) return;
    
    // Move left with the scroll
    x -= scrollSpeed;
    
    // Update hit blink timer
    if (hitBlinkTimer > 0) {
        hitBlinkTimer--;
    }
    
    // Deactivate if off screen
    if (x < -width) {
        active = false;
    }
}

void TrashCluster::render(SDL_Renderer* renderer) {
    if (!active) return;
    
    // Draw as a single large debris piece that shrinks with damage
    float healthRatio = (float)health / maxHealth;
    int currentWidth = (int)(width * healthRatio);
    int currentHeight = (int)(height * healthRatio);
    
    // Use the first texture as the debris
    SDL_Texture* tex = litterTextures[0];
    SDL_Rect dest = {
        (int)x,
        (int)y,
        currentWidth,
        currentHeight
    };
    
    // Blink when hit
    if (hitBlinkTimer > 0 && (hitBlinkTimer / 3) % 2 == 0) {
        SDL_SetTextureColorMod(tex, 255, 200, 100);  // Orange flash
    } else {
        SDL_SetTextureColorMod(tex, 180, 180, 180);  // Darker/grayer for debris
    }
    
    SDL_RenderCopy(renderer, tex, nullptr, &dest);
    
    // Reset color mod
    SDL_SetTextureColorMod(tex, 255, 255, 255);
}

bool TrashCluster::checkCollision(const SDL_Rect& subRect) {
    if (!active) return false;
    
    SDL_Rect clusterRect = { (int)x, (int)y, width, height };
    return (subRect.x < clusterRect.x + clusterRect.w &&
            subRect.x + subRect.w > clusterRect.x &&
            subRect.y < clusterRect.y + clusterRect.h &&
            subRect.y + subRect.h > clusterRect.y);
}

void TrashCluster::hit() {
    health--;
    hitBlinkTimer = 15;  // Blink for 15 frames
    
    if (health <= 0) {
        active = false;
    }
}
