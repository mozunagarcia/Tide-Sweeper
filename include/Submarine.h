#pragma once
#include <SDL.h>

class Submarine {
public:
    Submarine(SDL_Texture* tex, int x, int y, int w, int h);
    ~Submarine();

    void setPosition(int x, int y);
    void moveBy(int dx, int dy);
    void clamp(int minX, int maxX, int minY, int maxY);
    void render(SDL_Renderer* renderer);
    SDL_Rect getRect() const;

private:
    SDL_Texture* texture;
    SDL_Rect rect;
};
