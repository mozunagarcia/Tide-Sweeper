#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>
#include <chrono>


class GameOverScreen {
public:
    GameOverScreen(SDL_Renderer* renderer);
    ~GameOverScreen();

    // returns: "restart", "menu", "exit", "resume"
    std::string run(const std::string& title,
                    const std::vector<std::string>& facts);

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
    Button resumeBtn;
    int hoveredIndex; // -1 = none

    void render(const std::string& title,
            const std::string& fact,
            bool showResume,
            float countdownRatio);

    bool isInside(const SDL_Rect& r, int x, int y);
};
