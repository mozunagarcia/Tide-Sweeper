#include "ScoreDisplay.hpp"
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

ScoreDisplay::ScoreDisplay(SDL_Renderer* renderer, int x, int y, int width, int height) 
    : renderer(renderer), score(0), textTexture(nullptr) {
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

    updateTextTexture();
}

ScoreDisplay::~ScoreDisplay() {
    if (textTexture) {
        SDL_DestroyTexture(textTexture);
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

    // Render the score text
    renderScore();
}

void ScoreDisplay::setScore(int newScore) {
    if (score != newScore) {
        score = newScore;
        updateTextTexture();
    }
}

int ScoreDisplay::getScore() const {
    return score;
}

void ScoreDisplay::updateTextTexture() {
    if (textTexture) {
        SDL_DestroyTexture(textTexture);
        textTexture = nullptr;
    }

    if (!font) {
        return;
    }

    // Create text surface
    SDL_Color textColor = {255, 255, 255, 255}; // White text
    std::string scoreText = "Score: " + std::to_string(score);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
    
    if (textSurface) {
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FreeSurface(textSurface);
    }
}

void ScoreDisplay::renderScore() {
    if (!textTexture) {
        return;
    }

    // Get the dimensions of the text texture
    int textWidth, textHeight;
    SDL_QueryTexture(textTexture, NULL, NULL, &textWidth, &textHeight);

    // Center the text in the score rectangle
    SDL_Rect textRect = {
        scoreRect.x + (scoreRect.w - textWidth) / 2,
        scoreRect.y + (scoreRect.h - textHeight) / 2,
        textWidth,
        textHeight
    };

    // Render the text
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
}