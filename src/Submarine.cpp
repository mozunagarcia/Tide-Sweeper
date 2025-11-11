#include "Submarine.h"
#include <SDL.h>

Submarine::Submarine(SDL_Texture* tex, int x, int y, int w, int h)
    : texture(tex) {
    rect.x = x; rect.y = y; rect.w = w; rect.h = h;
}

Submarine::~Submarine() {
    if (texture) SDL_DestroyTexture(texture);
}

void Submarine::setPosition(int x, int y) {
    rect.x = x; rect.y = y;
}

void Submarine::moveBy(int dx, int dy) {
    rect.x += dx; rect.y += dy;
}

void Submarine::clamp(int minX, int maxX, int minY, int maxY) {
    if (rect.x < minX) rect.x = minX;
    if (rect.x > maxX) rect.x = maxX;
    if (rect.y < minY) rect.y = minY;
    if (rect.y > maxY) rect.y = maxY;
}

void Submarine::render(SDL_Renderer* renderer) {
    SDL_RenderCopyEx(renderer, texture, nullptr, &rect, 0, nullptr, SDL_FLIP_HORIZONTAL);
}

SDL_Rect Submarine::getRect() const {
    return rect;
}
