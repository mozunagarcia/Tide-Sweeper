#include "Messages.h"
#include <SDL_ttf.h>

Messages::Messages(SDL_Renderer* renderer) 
    : renderer(renderer), font(nullptr), currentMessage(nullptr), displayTimer(0)
{
    font = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 24);
}

Messages::~Messages() {
    if (currentMessage) SDL_DestroyTexture(currentMessage);
    if (font) TTF_CloseFont(font);
}

void Messages::showFact(const char* text) {
    if (!font) return;

    if (currentMessage) {
        SDL_DestroyTexture(currentMessage);
        currentMessage = nullptr;
    }

    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, textColor);
    if (surface) {
        currentMessage = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        displayTimer = 180; // Show for ~3 seconds at 60fps
    }
}