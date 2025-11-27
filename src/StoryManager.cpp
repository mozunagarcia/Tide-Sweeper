#include "StoryManager.h"
#include <SDL_ttf.h>
#include <iostream>

StoryManager::StoryManager(Messages* msg)
    : messages(msg)
{
    levels.resize(5);

    // LEVEL 1 — Tutorial

    levels[1].zoneName = "Entering Zone: Coastal Trash Vortex";
    levels[1].radioIntro = {
        "This area is safe, perfect for training.",
    };
    levels[1].milestones = {
        "Current shift detected, adjust your route.",
        "Handling improving, keep steady."
    };
    levels[1].endMessage = "Tutorial complete. Wildlife signals detected.";

    levels[1].intro.lines = {
        "Welcome, operator.",
        "Unusual current patterns are destabilizing the region.",
        "Your task is to investigate each affected area.",
        "Use this zone to learn navigation and debris collection."
    };
    levels[1].intro.playsBeforeLevel = true;



    // LEVEL 2 — Wildlife Disturbance
    levels[2].zoneName = "Entering Zone: Wildlife Disturbance Front";
    levels[2].radioIntro = {
        "Animals are fleeing deeper waters.",
        "Something is disturbing their migration patterns."
    };
    levels[2].milestones = {
        "Sharp movement detected, keep distance.",
        "Large creature ahead, maintain respectful spacing.",
        "Debris concentration rising, navigate carefully."
    };
    levels[2].endMessage = "Disturbances intensifying... cause still unknown.";




    // LEVEL 3 — Oil Spill Corridor
    levels[3].zoneName = "Entering Zone: Oil Spill Corridor";
    levels[3].radioIntro = {
        "We've traced the source of the wildlife panic.",
        "A damaged pipeline is leaking oil into the deep zone.",
        "Visibility will be reduced, proceed cautiously."
    };
    levels[3].milestones = {
        "Oil density rising, watch your surroundings.",
        "Contaminated debris incoming, adjust course.",
        "Marine life struggling, tread carefully."
    };
    levels[3].endMessage = "Oil spread worsening, conditions unstable.";




    // LEVEL 4 — Illegal Dumping Grounds (Final)
    levels[4].zoneName = "Entering Zone: Illegal Dumping Grounds";
    levels[4].radioIntro = {
        "This is it, the source of the contamination.",
        "Unauthorized dumping systems detected ahead.",
        "Shut them down before more damage occurs."
    };
    levels[4].milestones = {
        "Hazard barrels detected, void collisions.",
        "Active dumping overhead, watch for falling debris.",
        "Toxic surge detected, hold steady."
    };
    levels[4].endMessage = "Dumping operations disabled, contamination declining.";

    levels[4].outro.lines = {
        "Sector secured.",
        "Currents stabilizing and wildlife returning.",
        "Mission complete, excellent work."
    };
    levels[4].outro.playsAfterLevel = true;
}



void StoryManager::reset()
{
    for (auto& L : levels)
    {
        L.nextScore = 30;
        L.milestoneIndex = 0;
    }
    lvlChangeActive = false;
    currentLevel = 1;
}




// CUTSCENE (slides from left)
void StoryManager::playCutscene(const Cutscene& cs)
{
    if (cs.lines.empty()) return;

    messages->setStyle(MessageStyle::CUTSCENE);
    messages->loadMessageList(cs.lines);
    messages->start();
}




// LEVEL START
void StoryManager::onLevelChange(int newLevel)
{
    currentLevel = newLevel;
    auto& L = levels[newLevel];

    // HUD (top-left)
    lvlChangeText = L.zoneName;
    lvlChangeStart = SDL_GetTicks();
    lvlChangeActive = true;

    // Play cutscene if this level has one
    if (L.intro.playsBeforeLevel)
        playCutscene(L.intro);

    // Radio intro
    if (!L.radioIntro.empty())
    {
        messages->setStyle(MessageStyle::RADIO);
        messages->loadMessageList(L.radioIntro);
        messages->start();
    }
}



// LEVEL END

void StoryManager::onLevelEnd(int oldLevel)
{
    auto& L = levels[oldLevel];

    // HUD (top-left)
    lvlChangeText = L.endMessage;
    lvlChangeStart = SDL_GetTicks();
    lvlChangeActive = true;

    // Cutscene for final mission
    if (L.outro.playsAfterLevel)
        playCutscene(L.outro);

    // Radio end message
    if (!L.endMessage.empty())
    {
        messages->setStyle(MessageStyle::RADIO);
        messages->loadMessageList({ L.endMessage });
        messages->start();
    }
}



//
// MID-LEVEL MILESTONES
//
void StoryManager::update(int score, int level)
{
    auto& L = levels[level];

    if (score >= L.nextScore &&
        L.milestoneIndex < (int)L.milestones.size())
    {
        messages->setStyle(MessageStyle::RADIO);
        messages->loadMessageList({ L.milestones[L.milestoneIndex] });
        messages->start();

        L.milestoneIndex++;
        L.nextScore += 30;
    }
}



// HUD ZONE NAME / END MESSAGE BANNER
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

    //  POSITION HUD ABOVE RADIO
    
    int radioY = 500;     // radio panel Y
    int padding = 10;     // spacing between HUD and radio

    int hudY = radioY - (h + 20) - padding;

    SDL_Rect bg = { 20, hudY, w + 40, h + 20 };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 170);
    SDL_RenderFillRect(renderer, &bg);

    SDL_Rect txt = { bg.x + 20, bg.y + 10, w, h };
    SDL_RenderCopy(renderer, tex, NULL, &txt);

    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
    TTF_CloseFont(font);
}

