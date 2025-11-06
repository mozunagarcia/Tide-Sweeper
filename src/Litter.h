#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <cstdlib>
#include <iostream>

class Litter {
public:
    SDL_Texture* texture;  // Texture for the litter
    float x, y;            // Position
    float speed;           // Movement speed toward the left
    bool active;           // Whether it's visible / collectible
    int respawnTimer;      // Countdown before reappearing

    // Constructor
    Litter(SDL_Texture* tex, float startX, float startY, float moveSpeed)
        : texture(tex), x(startX), y(startY), speed(moveSpeed), active(true), respawnTimer(0) {}

    // Update movement + respawn logic
    // Returns true if this litter went off the left edge (missed) and wrapped
    bool update() {
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

    // Draw litter if active
    void render(SDL_Renderer* renderer) {
        if (!active) return;
        SDL_Rect dest = { static_cast<int>(x), static_cast<int>(y), 90, 90 };
        SDL_RenderCopy(renderer, texture, nullptr, &dest);
    }

    // Check collision with submarine
    bool checkCollision(const SDL_Rect& subRect) {
        if (!active) return false;
        SDL_Rect litterRect = { static_cast<int>(x), static_cast<int>(y), 64, 64 };
        return (subRect.x < litterRect.x + litterRect.w &&
                subRect.x + subRect.w > litterRect.x &&
                subRect.y < litterRect.y + litterRect.h &&
                subRect.y + subRect.h > litterRect.y);
    }

    // When collected → deactivate + start respawn timer
    void collect() {
        active = false;
        respawnTimer = 120; // ~2 seconds (120 frames at 60fps)
    }
};
