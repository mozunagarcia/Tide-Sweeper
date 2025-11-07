#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <cstdlib>
#include <iostream>

class Enemies {
    public: 
        SDL_Texture* texture; 
        float  x, y; 
        float speed;        
        bool active;
        int respawnTimer;

        Enemies(SDL_Texture* tex, float startX, float startY, float moveSpeed)
        : texture(tex), x(startX), y(startY), speed(moveSpeed), active(true), respawnTimer(0) {}

        void update() {
            if (!active) return;
            
            // Move left
            x -= speed;
            
            // Don't wrap around anymore - the main loop will handle cleanup
            // when enemies go off screen

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
        SDL_Rect enemiesRect = { static_cast<int>(x), static_cast<int>(y), 64, 64 };
        return (subRect.x < enemiesRect.x + enemiesRect.w &&
                subRect.x + subRect.w > enemiesRect.x &&
                subRect.y < enemiesRect.y + enemiesRect.h &&
                subRect.y + subRect.h > enemiesRect.y);
    }
};