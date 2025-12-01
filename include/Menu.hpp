#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <string>
#include <vector>
#include <iostream>
#include "ChatUI.h"

class Menu {
public:
    Menu(SDL_Renderer* renderer);
    ~Menu();

    void handleEvent(const SDL_Event& e, bool& running, bool& startGame);
    void render();
    void startBriefing();
    void renderBriefing();
    void renderNameEntry();
    void renderBriefingChoices();
    ChatUI* chat = nullptr;




private:
    SDL_Renderer* renderer;
    TTF_Font* font;
    TTF_Font* titleFont;
    TTF_Font* chatFont;
    SDL_Texture* menuBackgroundTexture;
    SDL_Texture* instructionsBackgroundTexture;
    Mix_Music* menuMusic;

    int selectedIndex; // for keyboard navigation
    int hoveredIndex; 
    bool showInstructions;

    // --- Briefing System ---
    bool briefingActive = false;
    bool nameEntryActive = false;
    bool choiceActive = false;
    bool briefingFinished = false;

        // --- Typing animation ---
    bool isTyping = false;
    std::string fullMessage = "";
    std::string visibleMessage = "";
    Uint32 lastCharTime = 0;
    int typingSpeedMs = 25; // lower = faster typing

    std::string playerName = "";
    std::vector<std::string> briefingMessages;
    int currentMessageIndex = 0;

    // Player choice text
    std::string choiceA = "Understood. Beginning deployment.";
    std::string choiceB = "Copy that. Prepping systems now.";
    int selectedChoice = -1; 


    // Menu item list
    std::vector<std::string> items;

    void renderMainMenu();
    void renderInstructions();

    // Sonar radio UI elements (only used during briefing)
    SDL_Rect radioRect = { 60, 140, 680, 260 };
    SDL_Color sonarTextColor = { 0, 255, 255, 255 };
    SDL_Color sonarHintColor = { 0, 180, 180, 255 };


    // Safe text renderer used ONLY for radio screens
    SDL_Texture* renderText(const std::string& msg, SDL_Color color, SDL_Rect& dst, bool wrap = false);
    SDL_Texture* chatBGTexture = nullptr;

};