#include "StoryManager.h"
#include <SDL_ttf.h>
#include <iostream>

StoryManager::StoryManager(Messages* msg)
    : messages(msg)
{
    levels.resize(5);


    // LEVEL 1 (3 messages)
    levels[1].zoneName = "Entering Zone: Coastal Litter Zone";
    levels[1].radioIntro = { "Scanning conditions. Low threat. Begin your sweep." };
    levels[1].milestones = {
        "Currents shifting. Adjust your heading."
    };
    levels[1].endMessage = "Picking up early wildlife activity. Proceed to next zone.";

    // LEVEL 2 (5 messages)
    // Intro + first animal + milestone + end

    levels[2].zoneName = "Entering Zone: Wildlife Disturbance";
    levels[2].radioIntro = {
        "Animal patterns are erratic. Stay alert."
    };
    levels[2].milestones = {
        "Debris density rising ahead. Navigate with care."
    };
    levels[2].endMessage = "Disturbance levels climbing. Source still unknown.";

    // LEVEL 3 (5 messages)
    // Intro + first oil + milestone + end

    levels[3].zoneName = "Entering Zone: Oil Spill Corridor";
    levels[3].radioIntro = {
        "Oil traces confirmed. Visibility will drop."
    };
    levels[3].milestones = {
        "Heavy contaminants approaching. Adjust your course."
    };
    levels[3].endMessage = "Oil spread increasing. Situation becoming unstable.";

    // LEVEL 4 (timer-driven)

    levels[4].zoneName = "Entering Zone: Illegal Dumping Grounds";
    levels[4].radioIntro = {
        "We are close to the source. Stay focused."
    };

    // Timer milestones trigger 
    levels[4].timeTriggers = { 25, 20, 15, 10, 5 };

    levels[4].milestones = {
        "Debris surge detected, sweep fast!",
        "Collect, collect, collect!",
        "Halfway there, debris flooding in!",
        "Getting close, keep sweeping!",
        "Final push, grab everything you can!"
    };

    levels[4].endMessage = "Cleanup complete. Returning to safer waters.";

}

void StoryManager::reset()
{
    for (int i = 0; i < levels.size(); i++)
    {
        levels[i].milestoneIndex = 0;

        // Default milestone timing
        levels[i].nextScore = 30;
    }

    // Custom milestone timings
    levels[2].nextScore = 60;   // level 2 milestone AFTER first animal
    levels[3].nextScore = 90;   // level 3 milestone AFTER oil warning


    animalMessagePlayed  = false;
    oilMessagePlayed     = false;
    endMessagePlayed     = false;

    lvlChangeActive = false;
    currentLevel = 1;
    
    levels[1].endScore = 80;
    levels[2].endScore = 180;
    levels[3].endScore = 380;
    levels[4].endScore = 1000;
}

void StoryManager::onLevelChange(int newLevel)
{
    currentLevel = newLevel;

    animalMessagePlayed = false;
    oilMessagePlayed = false;
    endMessagePlayed = false;

    auto& L = levels[newLevel];

    lvlChangeText = L.zoneName;
    lvlChangeStart = SDL_GetTicks();
    lvlChangeActive = true;


    messages->setStyle(MessageStyle::RADIO);
    messages->queueMessage(L.radioIntro[0]);
}

void StoryManager::onLevelEnd(int oldLevel)
{
    auto& L = levels[oldLevel];

    lvlChangeText = L.endMessage;
    lvlChangeStart = SDL_GetTicks();
    lvlChangeActive = true;
}

void StoryManager::onFirstAnimal()
{
    if (animalMessagePlayed) return;
    if (messages->isTypewriting()) return;


    animalMessagePlayed = true;

    messages->setStyle(MessageStyle::RADIO);
    messages->queuePriorityMessage("Marine life detected, maintain distance.");
}

void StoryManager::onOilDetected()
{
    if (oilMessagePlayed) return;

    oilMessagePlayed = true;

    messages->setStyle(MessageStyle::RADIO);
    messages->queuePriorityMessage("Oil slick detected, visibility compromised.");
}

void StoryManager::update(int score, int level, int timeRemaining)
{
    auto& L = levels[level];

    // LEVEL 4 TIMER-BASED TRIGGERS
    if (level == 4)
    {
        Level4* lv4 = dynamic_cast<Level4*>(currentLevelPtr);

        if (lv4)
        {
            int timeLeftSec = lv4->getStormTimer() / 60;

            if (L.timeIndex < (int)L.timeTriggers.size() &&
                timeLeftSec <= L.timeTriggers[L.timeIndex])
            {
                messages->setStyle(MessageStyle::RADIO);
                messages->queueMessage(L.milestones[L.timeIndex]);

                L.timeIndex++;
            }
        }
    }


    // NORMAL SCORE TRIGGERS (levels 1–3)
    if (level != 4)        // prevent overlap
    {
        if (score >= L.nextScore &&
            L.milestoneIndex < (int)L.milestones.size())
        {
            messages->setStyle(MessageStyle::RADIO);
            messages->queueMessage(L.milestones[L.milestoneIndex]);

            L.milestoneIndex++;
            L.nextScore += 30;
        }

        if (!endMessagePlayed && score >= L.endScore)
        {
            endMessagePlayed = true;

            messages->setStyle(MessageStyle::RADIO);
            messages->queueMessage(L.endMessage);
        }
    }
}


void StoryManager::renderLevelChange(SDL_Renderer* renderer)
{
    if (!lvlChangeActive || lvlChangeText.empty()) return;

    Uint32 now = SDL_GetTicks();
    if (now - lvlChangeStart > lvlChangeDuration)
    {
        lvlChangeActive = false;
        return;
    }

    TTF_Font* font = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 22);
    if (!font) return;

    SDL_Color white = {255,255,255};
    SDL_Surface* surf = TTF_RenderText_Blended(font, lvlChangeText.c_str(), white);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);

    int w = surf->w;
    int h = surf->h;

    SDL_Rect bg = { 20, 460, w + 40, h + 20 };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 170);
    SDL_RenderFillRect(renderer, &bg);

    SDL_Rect txt = { bg.x + 20, bg.y + 10, w, h };

    // TIMING
    Uint32 elapsed  = now - lvlChangeStart;

    // FULLY VISIBLE FOR FIRST 3 SECONDS
    if (elapsed < 3000)  
    {
        SDL_SetTextureAlphaMod(tex, 255);   // fully visible
        SDL_RenderCopy(renderer, tex, NULL, &txt);
    }
    else
    {
        // FINAL 0.5 SECOND FLICKER 
        // Flickers every 60ms
        bool flickerOn = ((now / 60) % 2) == 0;

        SDL_SetTextureAlphaMod(tex, flickerOn ? 255 : 0);

        if (flickerOn)
            SDL_RenderCopy(renderer, tex, NULL, &txt);
    }

    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
    TTF_CloseFont(font);
}
