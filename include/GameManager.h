#pragma once
#include <SDL.h>
#include <SDL_mixer.h>
#include <vector>

#include "Level.h"
#include "Submarine.h"
#include "Scoreboard.h"
#include "Messages.h"
#include "Menu.hpp"
#include "StoryManager.h"


class GameManager {
public:
    GameManager(SDL_Window* window, SDL_Renderer* renderer);
    ~GameManager();

    // Runs the main game loop; returns when the program should exit
    void run();
    bool isRunning() const { return running; }

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    Level* level;
    Submarine* submarine;
    Scoreboard* scoreboard;
    Messages* messages;
    Messages* msgManager;   // Story/message system
    StoryManager* storyManager;
    Menu* menu;
    bool running;
    bool startGame;
    Mix_Music* backgroundMusic;
    Mix_Chunk* levelCompleteSound;

    std::vector<std::string> facts;   // Fact strings used in pause + game over
};
