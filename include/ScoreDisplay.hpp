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
        int getLevel() const;
        void resetLevel();
        
    private:
        SDL_Renderer* renderer;
        SDL_Rect scoreRect;
        int score;
        int level;
        TTF_Font* font;
        SDL_Texture* scoreTexture;
        SDL_Texture* levelTexture;
        void renderScore();
        void renderLevel();
        void updateTextTexture();
        void updateLevelTexture();
};

#endif 