#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <string>
#include <vector>
#include <iostream>

class Menu {
public:
    Menu(SDL_Renderer* renderer);
    ~Menu();

    void handleEvent(const SDL_Event& e, bool& running, bool& startGame);
    void render();

private:
    SDL_Renderer* renderer;
    TTF_Font* font;
    SDL_Texture* menuBackgroundTexture;
    SDL_Texture* instructionsBackgroundTexture;
    Mix_Music* menuMusic;

    int selectedIndex; // for keyboard navigation
    int hoveredIndex; 
    bool showInstructions;

    // Menu item list
    std::vector<std::string> items;

    void renderMainMenu();
    void renderInstructions();
};
