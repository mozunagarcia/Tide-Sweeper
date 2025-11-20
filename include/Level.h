#pragma once
#include <SDL.h>
#include <vector>
#include "Litter.h"
#include "Enemies.h"
#include "Submarine.h"
#include "Scoreboard.h"

class Level {
public:
    Level(SDL_Renderer* renderer,
          const std::vector<SDL_Texture*>& litterTextures,
          const std::vector<SDL_Texture*>& enemyTextures,
          const std::vector<float>& enemySpeeds);
    ~Level();

    void update(Submarine& submarine, Scoreboard& scoreboard, int& lives, bool& gameOver, int currentLevel);
    void render();
    void renderBlackoutEffects(int currentLevel, Submarine& submarine);
    void reset();
    void setOilTexture(SDL_Texture* oilTex);

private:
    SDL_Renderer* renderer;
    std::vector<Litter> litterItems;
    std::vector<Enemies> enemyItems;
    std::vector<SDL_Texture*> enemyTextures;
    std::vector<float> enemySpeeds;
    int spawnTimer;
    const int spawnInterval = 120;
    const int maxActiveEnemies = 2;
    
    // Level 3 blackout system MAYBE DO A CLASS FOR THIS
    SDL_Texture* oilTexture;
    struct OilSpot { int x, y, size; int spawnFrame; float alpha; };
    std::vector<OilSpot> oilSpots;
    int blackoutNext;
    int warningFrameCounter;
    bool isBlackout;
    bool isWarning;
    int blackoutCounter;
    const int blackoutInterval = 600;  // 10 seconds at 60 FPS
    const int blackoutWarning = 120;   // 2 seconds warning
    const int blackoutDuration = 300;  // 5 seconds at 60 FPS
    
    void updateBlackoutMechanic(int currentLevel);
};
