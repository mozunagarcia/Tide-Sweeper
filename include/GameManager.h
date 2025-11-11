#pragma once
#include <SDL.h>
#include "Level.h"
#include "Submarine.h"
#include "Scoreboard.h"
#include "Messages.h"
#include "Menu.hpp"

class GameManager {
public:
    GameManager(SDL_Window* window, SDL_Renderer* renderer);
    ~GameManager();

    // Runs the main game loop; returns when the program should exit
    void run();

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    Level* level;
    Submarine* submarine;
    Scoreboard* scoreboard;
    Messages* messages;
    Menu* menu;
    bool running;
    bool startGame;
};
