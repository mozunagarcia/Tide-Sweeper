#pragma once
#include <string>
#include <vector>
#include "Level.h"
#include "Messages.h"
#include "Level.h"

class Level;
class Level4;
struct LevelInfo
{
    std::string zoneName;

    std::vector<std::string> radioIntro;
    std::vector<std::string> milestones;

    int milestoneIndex = 0;
    int nextScore = 30;

    std::string endMessage;
    int endScore = 0;

    std::vector<int> timeTriggers;  // seconds remaining when messages fire
    int timeIndex = 0;              // next time-trigger to play
};

class StoryManager {
public:
    StoryManager(Messages* msg);

    void reset();
    void onLevelChange(int newLevel);
    void onLevelEnd(int oldLevel);

    void onFirstAnimal();   // LEVEL 2 special
    void onOilDetected();   // LEVEL 3 special

    void setLevelPointer(Level* lvl) { currentLevelPtr = lvl; }

    void update(int score, int level, int timeRemaining);
    void renderLevelChange(SDL_Renderer* renderer);

    bool animalMessagePlayed = false;
    bool oilMessagePlayed = false;
    bool endMessagePlayed = false;

private:
    Messages* messages;
    std::vector<LevelInfo> levels;

    int currentLevel = 1;
    Level* currentLevelPtr = nullptr; // <-- pointer stored here

    std::string lvlChangeText;
    Uint32 lvlChangeStart = 0;
    bool lvlChangeActive = false;
    int lvlChangeDuration = 3500;
};
