#pragma once
#include <SDL.h>
#include <vector>

// Giant cluster of trash that needs multiple hits to break
class TrashCluster {
public:
    float x, y;
    int width, height;
    int health;  // Number of hits needed to destroy
    int maxHealth;
    bool active;
    std::vector<SDL_Texture*> litterTextures;
    
    TrashCluster(const std::vector<SDL_Texture*>& textures, float startX, float startY, int hp);
    
    void update(float scrollSpeed);
    void render(SDL_Renderer* renderer);
    bool checkCollision(const SDL_Rect& subRect);
    void hit();  // Take damage
    bool isActive() const { return active; }
    
private:
    int hitBlinkTimer;
};
