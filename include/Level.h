#pragma once
#include <SDL.h>
#include <vector>
#include "Litter.h"
#include "Enemies.h"
#include "Submarine.h"
#include "Scoreboard.h"

// Base Level class
class Level {
public:
    Level(SDL_Renderer* renderer,
          const std::vector<SDL_Texture*>& litterTextures,
          const std::vector<SDL_Texture*>& enemyTextures,
          const std::vector<float>& enemySpeeds,
          const std::vector<int>& enemyWidths,
          const std::vector<int>& enemyHeights);
    virtual ~Level();

    virtual void update(Submarine& submarine, Scoreboard& scoreboard, int& lives, bool& gameOver);
    virtual void render();
    virtual void renderBlackoutEffects(Submarine& submarine);
    virtual void reset();
    void setOilTexture(SDL_Texture* oilTex);
    void calmEnemies(float subX, float subY, float radius);
    std::vector<Litter>& getLitterItems() { return litterItems; }
    void setLitterItems(const std::vector<Litter>& litter) { litterItems = litter; }
    std::vector<Enemies>& getEnemyItems() { return enemyItems; }
    void setEnemyItems(const std::vector<Enemies>& enemies) { enemyItems = enemies; }

protected:
    SDL_Renderer* renderer;
    std::vector<Litter> litterItems;
    std::vector<Enemies> enemyItems;
    std::vector<SDL_Texture*> enemyTextures;
    std::vector<float> enemySpeeds;
    std::vector<int> enemyWidths;
    std::vector<int> enemyHeights;
    int spawnTimer;
    int spawnInterval;
    int maxActiveEnemies;
    
    // Oil blackout system (for Level 3)
    SDL_Texture* oilTexture;
    struct OilSpot { int x, y, size; int spawnFrame; float alpha; };
    std::vector<OilSpot> oilSpots;
    int blackoutNext;
    int warningFrameCounter;
    bool isBlackout;
    bool isWarning;
    int blackoutCounter;
    int blackoutInterval;
    int blackoutWarning;
    int blackoutDuration;
    int blackoutWidth;
    bool isBlackoutFading;
    
    virtual void updateEnemies(Submarine& submarine, int& lives, bool& gameOver);
    virtual void updateBlackoutMechanic();
};

// Level 1: Only litter, no animals
class Level1 : public Level {
public:
    Level1(SDL_Renderer* renderer,
           const std::vector<SDL_Texture*>& litterTextures,
           const std::vector<SDL_Texture*>& enemyTextures,
           const std::vector<float>& enemySpeeds,
           const std::vector<int>& enemyWidths,
           const std::vector<int>& enemyHeights);
    
    void update(Submarine& submarine, Scoreboard& scoreboard, int& lives, bool& gameOver) override;
};

// Level 2: Litter + Animals
class Level2 : public Level {
public:
    Level2(SDL_Renderer* renderer,
           const std::vector<SDL_Texture*>& litterTextures,
           const std::vector<SDL_Texture*>& enemyTextures,
           const std::vector<float>& enemySpeeds,
           const std::vector<int>& enemyWidths,
           const std::vector<int>& enemyHeights);
};

// Level 3: Litter + Animals + Oil blackout mechanics
class Level3 : public Level {
public:
    Level3(SDL_Renderer* renderer,
           const std::vector<SDL_Texture*>& litterTextures,
           const std::vector<SDL_Texture*>& enemyTextures,
           const std::vector<float>& enemySpeeds,
           const std::vector<int>& enemyWidths,
           const std::vector<int>& enemyHeights);
    
    void update(Submarine& submarine, Scoreboard& scoreboard, int& lives, bool& gameOver) override;
    void renderBlackoutEffects(Submarine& submarine) override;
};

// Level 4: Everything from Level 3 (can be extended later)
class Level4 : public Level3 {
public:
    Level4(SDL_Renderer* renderer,
           const std::vector<SDL_Texture*>& litterTextures,
           const std::vector<SDL_Texture*>& enemyTextures,
           const std::vector<float>& enemySpeeds,
           const std::vector<int>& enemyWidths,
           const std::vector<int>& enemyHeights);
};
