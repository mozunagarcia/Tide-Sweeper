#pragma once
#include <SDL.h>
#include <SDL_ttf.h>

class Messages {
public:
    Messages(SDL_Renderer* renderer);
    ~Messages();
    void showFact(const char* text);

private:
    SDL_Renderer* renderer;
    TTF_Font* font;
    SDL_Texture* currentMessage;
    int displayTimer;
};
