#include "ScoreDisplay.hpp"
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

ScoreDisplay::ScoreDisplay(SDL_Renderer* renderer, int x, int y, int width, int height) 
    : renderer(renderer), score(0), level(1), scoreTexture(nullptr), levelTexture(nullptr) {
    scoreRect.x = x;
    scoreRect.y = y;
    scoreRect.w = width;
    scoreRect.h = height;

    // Initialize SDL_ttf
    if (TTF_Init() < 0) {
        SDL_Log("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return;
    }

    // Load font
    font = TTF_OpenFont("assets/fonts/OpenSans.ttf", 24);
    if (!font) {
        SDL_Log("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return;
    }

    // Initialize both textures
    updateTextTexture();
    updateLevelTexture();
}

ScoreDisplay::~ScoreDisplay() {
    if (scoreTexture) {
        SDL_DestroyTexture(scoreTexture);
    }
    if (levelTexture) {
        SDL_DestroyTexture(levelTexture);
    }
    if (font) {
        TTF_CloseFont(font);
    }
    TTF_Quit();
}

void ScoreDisplay::render() {
    // Render the background rectangle
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200); // Semi-transparent black
    SDL_RenderFillRect(renderer, &scoreRect);
    
    // Render the border
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White border
    SDL_RenderDrawRect(renderer, &scoreRect);

    // Render the score and level text
    renderScore();
    renderLevel();
}

void ScoreDisplay::setScore(int newScore) {
    if (score != newScore) {
        score = newScore;
        // Update level if score reaches 100
        if (score >= 100 && level == 1) {
            level = 2;
            updateLevelTexture();
        }
        updateTextTexture();
    }
}

int ScoreDisplay::getScore() const {
    return score;
}

void ScoreDisplay::updateTextTexture() {
    if (scoreTexture) {
        SDL_DestroyTexture(scoreTexture);
        scoreTexture = nullptr;
    }

    if (!font) {
        return;
    }

    // Create text surface
    SDL_Color textColor = {255, 255, 255, 255}; // White text
    std::string scoreText = "Score: " + std::to_string(score);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
    
    if (textSurface) {
        scoreTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FreeSurface(textSurface);
    }
}

void ScoreDisplay::updateLevelTexture() {
    if (levelTexture) {
        SDL_DestroyTexture(levelTexture);
        levelTexture = nullptr;
    }

    if (!font) {
        return;
    }

    // Create text surface
    SDL_Color textColor = {255, 255, 255, 255}; // White text
    std::string levelText = "Level: " + std::to_string(level);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, levelText.c_str(), textColor);
    
    if (textSurface) {
        levelTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FreeSurface(textSurface);
    }
}

void ScoreDisplay::renderScore() {
    if (!scoreTexture) {
        return;
    }

    // Get the dimensions of the text texture
    int textWidth, textHeight;
    SDL_QueryTexture(scoreTexture, NULL, NULL, &textWidth, &textHeight);

    // Position the score text in the upper part of the rectangle
    SDL_Rect textRect = {
        scoreRect.x + (scoreRect.w - textWidth) / 2,
        scoreRect.y + 10, // Increased padding from top
        textWidth,
        textHeight
    };

    // Render the score text
    SDL_RenderCopy(renderer, scoreTexture, NULL, &textRect);
}

void ScoreDisplay::renderLevel() {
    if (!levelTexture) {
        return;
    }

    // Get the dimensions of the level texture
    int textWidth, textHeight;
    SDL_QueryTexture(levelTexture, NULL, NULL, &textWidth, &textHeight);

    // Position the level text below the score
    SDL_Rect textRect = {
        scoreRect.x + (scoreRect.w - textWidth) / 2,
        scoreRect.y + 45, // Fixed position for level text, below score
        textWidth,
        textHeight
    };

    // Render the level text
    SDL_RenderCopy(renderer, levelTexture, NULL, &textRect);
}

int ScoreDisplay::getLevel() const {
    return level;
}

void ScoreDisplay::resetLevel() {
    level = 1;
    updateLevelTexture();
}