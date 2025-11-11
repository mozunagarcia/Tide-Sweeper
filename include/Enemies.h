#pragma once
#include <SDL.h>
#include <SDL_image.h>

class Enemies {
public: 
    SDL_Texture* texture; 
    float x, y; 
    float speed;        
    bool active;
    int respawnTimer;

    Enemies(SDL_Texture* tex, float startX, float startY, float moveSpeed);
    void update();
    void render(SDL_Renderer* renderer);
    bool checkCollision(const SDL_Rect& subRect);
};