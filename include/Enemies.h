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
    float detectionRadius = 200.0f; 
    int hitBlinkTimer = 0;  
    bool falling = false;   
    float fallSpeed = 0;    
    int enemyType = 0;      // 0=Swordfish, 1=Eel, 2=Octopus, 3=Angler, 4=Shark
    bool calmed = false;    
    bool deflecting = false; 
    float deflectDirX = 0;   
    float deflectDirY = 0;
    int deflectTimer = 0;   
    bool facingRight = false; 

    Enemies(SDL_Texture* tex, float startX, float startY, float moveSpeed, int w = 90, int h = 90, int type = 0);
    void update(float subX, float subY); 
    void render(SDL_Renderer* renderer);
    bool checkCollision(const SDL_Rect& subRect);
    void startHitBlink();  
    void startFalling();   
};