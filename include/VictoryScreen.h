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

    std::string run(int finalScore);
    

private:
    SDL_Renderer* renderer;
    TTF_Font* fontLarge;
    TTF_Font* fontSmall;

    TTF_Font* fontTitle;     
    TTF_Font* fontStats;    
    TTF_Font* fontHeader;    
    TTF_Font* fontBody;      

    SDL_Texture* bgTexture = nullptr;


    struct Button {
        SDL_Rect rect;
        std::string label;
    };

    Button restartBtn;
    Button menuBtn;
    Button exitBtn;
    int hoveredIndex;

    // TYPEWRITER VARIABLES
    std::string fullText;
    std::string typedText;
    float typeTimer;
    int typeIndex;
    float charsPerSecond;


    void render(float countdownRatio, int finalScore);
    bool isInside(const SDL_Rect& r, int x, int y);
};
