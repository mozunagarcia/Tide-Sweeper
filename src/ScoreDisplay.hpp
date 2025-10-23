#ifndef SCORE_DISPLAY_HPP
#define SCORE_DISPLAY_HPP

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

class ScoreDisplay {
public:
    ScoreDisplay(SDL_Renderer* renderer, int x, int y, int width, int height);
    ~ScoreDisplay();

    void render();
    void setScore(int newScore);
    int getScore() const;

private:
    SDL_Renderer* renderer;
    SDL_Rect scoreRect;
    int score;
    TTF_Font* font;
    SDL_Texture* textTexture;
    void renderScore();
    void updateTextTexture();
};

#endif // SCORE_DISPLAY_HPP