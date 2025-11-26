#include "Submarine.h"
#include <SDL.h>

Submarine::Submarine(SDL_Texture* tex, int x, int y, int w, int h)
    : texture(tex) {
    rect.x = x; rect.y = y; rect.w = w; rect.h = h;
}

Submarine::~Submarine() {
    if (texture) SDL_DestroyTexture(texture);
}

void Submarine::setPosition(int x, int y) {
    rect.x = x; rect.y = y;
}

void Submarine::moveBy(int dx, int dy) {
    rect.x += dx; rect.y += dy;
    
    // Update facing direction based on horizontal movement
    if (dx > 0) {
        facingRight = true;
    } else if (dx < 0) {
        facingRight = false;
    }
}

void Submarine::clamp(int minX, int maxX, int minY, int maxY) {
    if (rect.x < minX) rect.x = minX;
    if (rect.x > maxX) rect.x = maxX;
    if (rect.y < minY) rect.y = minY;
    if (rect.y > maxY) rect.y = maxY;
}

void Submarine::render(SDL_Renderer* renderer) {
    // Blink red if recently hit (alternating every 3 frames)
    if (hitBlinkTimer > 0 && (hitBlinkTimer / 3) % 2 == 0) {
        SDL_SetTextureColorMod(texture, 255, 100, 100);  // Red tint
    } else {
        SDL_SetTextureColorMod(texture, 255, 255, 255);  // Normal color
    }
    
    // Flip based on facing direction
    SDL_RendererFlip flip = facingRight ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(renderer, texture, nullptr, &rect, 0, nullptr, flip);
    
    // Reset color mod
    SDL_SetTextureColorMod(texture, 255, 255, 255);
}

SDL_Rect Submarine::getRect() const {
    return rect;
}

void Submarine::startHitBlink() {
    hitBlinkTimer = 30;  // Blink for 30 frames (0.5 seconds at 60 FPS)
}

void Submarine::updateBlink() {
    if (hitBlinkTimer > 0) {
        hitBlinkTimer--;
    }
}

bool Submarine::isInvulnerable() const {
    return hitBlinkTimer > 0;
}
