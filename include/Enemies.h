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
    int width, height;
    float detectionRadius = 250.0f; 
    int hitBlinkTimer = 0;  
    bool falling = false;   
    float fallSpeed = 0;    

    Enemies(SDL_Texture* tex, float startX, float startY, float moveSpeed, int w = 90, int h = 90);
    void update(float subX, float subY); 
    void render(SDL_Renderer* renderer);
    bool checkCollision(const SDL_Rect& subRect);
    void startHitBlink();  
    void startFalling();   
};