#include "ScoreDisplay.hpp"
#include <cassert>
#include <iostream>


int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    if (TTF_Init() < 0) {
        std::cerr << "SDL_ttf init failed: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Score Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 400, 200, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    ScoreDisplay scoreDisplay(renderer, 200, 20, 180, 60);

    // Test iniziale
    assert(scoreDisplay.getScore() == 0);
    std::cout << "Test iniziale OK\n";

    // Test aggiornamento
    scoreDisplay.setScore(42);
    assert(scoreDisplay.getScore() == 42);
    std::cout << "Test aggiornamento OK\n";

    // Test incremento
    scoreDisplay.setScore(scoreDisplay.getScore() + 8);
    assert(scoreDisplay.getScore() == 50);
    std::cout << "Test incremento OK\n";

    // Mostra il rendering per 2 secondi
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);
    scoreDisplay.render();
    SDL_RenderPresent(renderer);
    SDL_Delay(2000);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    std::cout << "Tutti i test sono passati!\n";
    return 0;
}
