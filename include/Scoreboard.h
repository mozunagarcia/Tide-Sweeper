#pragma once
#include "ScoreDisplay.hpp"

class Scoreboard : public ScoreDisplay {
public:
    Scoreboard(SDL_Renderer* renderer, int x, int y, int width, int height);
};
