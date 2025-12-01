#include "Litter.h"
#include <cstdlib>

Litter::Litter(SDL_Texture* tex, float startX, float startY, float moveSpeed, int w, int h)
    : texture(tex), x(startX), y(startY), speed(moveSpeed), 
      active(true), respawnTimer(0), width(w), height(h)
{
}


bool Litter::update() {
    if (!active) {
        // Countdown until respawn
        if (respawnTimer > 0) {
            respawnTimer--;
        } else {
            // Reactivate once timer hits zero
            active = true;
            x = 850; // respawn just off-screen to the right
            y = rand() % 500 + 50;
        }
        return false; // no miss while inactive
    }

    // Move leftward
    x -= speed;

    // Wrap around if it goes off the left edge
    if (x < -100) {
        x = 850; // loop back to the right
        y = rand() % 500 + 50; // random vertical drift
        return true; // missed the submarine
    }
    return false;
}

void Litter::render(SDL_Renderer* renderer) {
    if (!active) return;
    SDL_Rect dest = { (int)x, (int)y, width, height };
    SDL_RenderCopy(renderer, texture, nullptr, &dest);
}

bool Litter::checkCollision(const SDL_Rect& subRect) {
    if (!active) return false;
    SDL_Rect litterRect = { (int)x, (int)y, width, height };
    return (subRect.x < litterRect.x + litterRect.w &&
            subRect.x + subRect.w > litterRect.x &&
            subRect.y < litterRect.y + litterRect.h &&
            subRect.y + subRect.h > litterRect.y);
}

void Litter::collect() {
    active = false;
    respawnTimer = 120; // ~2 seconds (120 frames at 60fps)
}