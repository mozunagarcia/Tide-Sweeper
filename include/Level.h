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
          const std::vector<float>& enemySpeeds,
          const std::vector<int>& enemyWidths,
          const std::vector<int>& enemyHeights);
    ~Level();

    void update(Submarine& submarine, Scoreboard& scoreboard, int& lives, bool& gameOver);
    void render();
    void reset();
    void calmEnemies(float subX, float subY, float radius);

private:
    SDL_Renderer* renderer;
    std::vector<Litter> litterItems;
    std::vector<Enemies> enemyItems;
    std::vector<SDL_Texture*> enemyTextures;
    std::vector<float> enemySpeeds;
    std::vector<int> enemyWidths;
    std::vector<int> enemyHeights;
    int spawnTimer;
    const int spawnInterval = 120;
    const int maxActiveEnemies = 2;
};
