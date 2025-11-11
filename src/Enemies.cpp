#include "Enemies.h"

Enemies::Enemies(SDL_Texture* tex, float startX, float startY, float moveSpeed)
    : texture(tex), x(startX), y(startY), speed(moveSpeed), active(true), respawnTimer(0)
{
}

void Enemies::update() {
    if (!active) return;
    
    // Move left
    x -= speed;
    
    // Don't wrap around anymore - the main loop will handle cleanup
    // when enemies go off screen
}

void Enemies::render(SDL_Renderer* renderer) {
    if (!active) return;
    SDL_Rect dest = { static_cast<int>(x), static_cast<int>(y), 90, 90 };
    SDL_RenderCopy(renderer, texture, nullptr, &dest);
}

bool Enemies::checkCollision(const SDL_Rect& subRect) {
    if (!active) return false;
    SDL_Rect enemiesRect = { static_cast<int>(x), static_cast<int>(y), 64, 64 };
    return (subRect.x < enemiesRect.x + enemiesRect.w &&
            subRect.x + subRect.w > enemiesRect.x &&
            subRect.y < enemiesRect.y + enemiesRect.h &&
            subRect.y + subRect.h > enemiesRect.y);
}