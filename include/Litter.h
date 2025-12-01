#pragma once
#include <SDL.h>
#include <SDL_image.h>

class Litter {
public:
    SDL_Texture* texture;  // Texture for the litter
    float x, y;            // Position
    float speed;           // Movement speed toward the left
    bool active;           // Whether it's visible / collectible
    int respawnTimer;      // Countdown before reappearing

    Litter(SDL_Texture* tex, float startX, float startY, float moveSpeed)
    : Litter(tex, startX, startY, moveSpeed, 40, 40) {}


    // Constructor
    Litter(SDL_Texture* tex, float startX, float startY, float moveSpeed, int w, int h);

    // Update movement + respawn logic
    // Returns true if this litter went off the left edge (missed) and wrapped
    bool update();

    // Draw litter if active
    void render(SDL_Renderer* renderer);

    // Check collision with submarine
    bool checkCollision(const SDL_Rect& subRect);

    // When collected → deactivate + start respawn timer
    void collect();

    private:

    int width;
    int height;
};
