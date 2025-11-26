#pragma once
#include <string>
#include <vector>
#include <SDL.h>
#include "Messages.h"

// ------------------------------------------------------
// CUTSCENE DATA (only Level 1 intro + Level 4 outro use it)
// ------------------------------------------------------
struct Cutscene {
    std::vector<std::string> lines;
    bool playsBeforeLevel = false;
    bool playsAfterLevel  = false;
};

// ------------------------------------------------------
// PER-LEVEL STORY DATA
// ------------------------------------------------------
struct LevelData {
    std::string zoneName;                // Top-left HUD banner text

    std::vector<std::string> radioIntro; // Start-of-level radio messages
    std::vector<std::string> milestones; // Score-based radio messages
    std::string endMessage;              // End-of-level radio message

    Cutscene intro;                      // Level 1 only
    Cutscene outro;                      // Level 4 only

    int nextScore = 30;                  // Score threshold for next milestone
    int milestoneIndex = 0;              // Which milestone to show next
};

// ------------------------------------------------------
// STORY MANAGER
// ------------------------------------------------------
class StoryManager
{
public:
    StoryManager(Messages* msg);

    void reset();
    void onLevelChange(int newLevel);
    void onLevelEnd(int oldLevel);
    void update(int score, int currentLevel);

    void renderLevelChange(SDL_Renderer* renderer);

private:
    void playCutscene(const Cutscene& cs);

    Messages* messages;                  // Radio + cutscene system
    std::vector<LevelData> levels;       // Level 1–4 story data

    int currentLevel = 1;

    // HUD zone banner data
    bool lvlChangeActive = false;
    std::string lvlChangeText = "";
    Uint32 lvlChangeStart = 0;
    Uint32 lvlChangeDuration = 2600;     // HUD stays visible 2.6 seconds
};
