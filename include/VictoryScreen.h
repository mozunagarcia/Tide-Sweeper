#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>
#include <chrono>


class VictoryScreen {
public:
    VictoryScreen(SDL_Renderer* renderer);
    ~VictoryScreen();

    // returns: "restart", "menu", "exit"
    std::string run(const std::vector<std::string>& facts, int finalScore);

private:
    SDL_Renderer* renderer;
    TTF_Font* fontLarge;
    TTF_Font* fontSmall;

    struct Button {
        SDL_Rect rect;
        std::string label;
    };

    Button restartBtn;
    Button menuBtn;
    Button exitBtn;
    int hoveredIndex; // -1 = none

    void render(const std::string& fact, float countdownRatio, int finalScore);

    bool isInside(const SDL_Rect& r, int x, int y);
};
