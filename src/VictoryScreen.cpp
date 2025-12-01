#include "VictoryScreen.h"
#include <iostream>

VictoryScreen::VictoryScreen(SDL_Renderer* renderer)
    : renderer(renderer), hoveredIndex(-1)
{
    fontLarge = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 48);
    fontSmall = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 28);

    if (!fontLarge || !fontSmall) {
        std::cerr << "VictoryScreen Font Load Error: "
                  << TTF_GetError() << std::endl;
    }

    int bw = 220;
    int bh = 50;

    restartBtn = { { 90,  430, bw, bh }, "Restart" };
    menuBtn    = { { 290, 430, bw, bh }, "Main Menu" };
    exitBtn    = { { 490, 430, bw, bh }, "Exit" };
}

VictoryScreen::~VictoryScreen() {
    if (fontLarge) TTF_CloseFont(fontLarge);
    if (fontSmall) TTF_CloseFont(fontSmall);
}

bool VictoryScreen::isInside(const SDL_Rect& r, int x, int y) {
    return (x >= r.x && x <= r.x + r.w &&
            y >= r.y && y <= r.y + r.h);
}

void VictoryScreen::render(const std::string& fact, float countdownRatio, int finalScore)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect bg = {0, 0, 800, 600};
    SDL_RenderFillRect(renderer, &bg);

    SDL_Color white = {255,255,255};
    SDL_Color gold = {255,215,0};

    // Title
    SDL_Surface* t = TTF_RenderText_Blended(fontLarge, "Mission Accomplished!", gold);
    SDL_Texture* tt = SDL_CreateTextureFromSurface(renderer, t);
    SDL_Rect tr = { (800 - t->w)/2, 60, t->w, t->h };
    SDL_RenderCopy(renderer, tt, NULL, &tr);
    SDL_FreeSurface(t);
    SDL_DestroyTexture(tt);

    // Score
    char scoreText[64];
    snprintf(scoreText, sizeof(scoreText), "Final Score: %d", finalScore);
    SDL_Surface* s = TTF_RenderText_Blended(fontSmall, scoreText, gold);
    SDL_Texture* st = SDL_CreateTextureFromSurface(renderer, s);
    SDL_Rect sr = { (800 - s->w)/2, 120, s->w, s->h };
    SDL_RenderCopy(renderer, st, NULL, &sr);
    SDL_FreeSurface(s);
    SDL_DestroyTexture(st);

    // Fact
    SDL_Surface* f = TTF_RenderText_Blended_Wrapped(fontSmall, fact.c_str(), white, 700);
    SDL_Texture* ft = SDL_CreateTextureFromSurface(renderer, f);
    SDL_Rect fr = { 50, 150, f->w, f->h };
    SDL_RenderCopy(renderer, ft, NULL, &fr);
    SDL_FreeSurface(f);
    SDL_DestroyTexture(ft);

    // Countdown bar
    int barWidth = 700;
    int barHeight = 8;
    int barX = 50;
    int barY = fr.y + fr.h + 20;

    SDL_Rect barBG = { barX, barY, barWidth, barHeight };
    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 200);
    SDL_RenderFillRect(renderer, &barBG);

    SDL_Rect barFG = { barX, barY, (int)(barWidth * countdownRatio), barHeight };
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);  // Gold color for victory
    SDL_RenderFillRect(renderer, &barFG);

    // Buttons
    auto drawBtn = [&](Button& b, int index) {
        if (index == hoveredIndex)
            SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);  // Gold highlight
        else
            SDL_SetRenderDrawColor(renderer, 0, 80, 160, 255);

        SDL_RenderFillRect(renderer, &b.rect);

        SDL_Surface* ts = TTF_RenderText_Blended(fontSmall, b.label.c_str(), white);
        SDL_Texture* tt2 = SDL_CreateTextureFromSurface(renderer, ts);
        SDL_Rect lr = {
            b.rect.x + (b.rect.w - ts->w)/2,
            b.rect.y + (b.rect.h - ts->h)/2,
            ts->w, ts->h
        };
        SDL_RenderCopy(renderer, tt2, NULL, &lr);
        SDL_FreeSurface(ts);
        SDL_DestroyTexture(tt2);
    };

    drawBtn(restartBtn, 0);
    drawBtn(menuBtn, 1);
    drawBtn(exitBtn, 2);
}

std::string VictoryScreen::run(const std::vector<std::string>& facts, int finalScore)
{
    hoveredIndex = -1;

    int factIndex = 0;

    const float autoTime = 7.0f;  // seconds per fact
    auto lastSwitch = std::chrono::steady_clock::now();

    while (true) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {

            if (e.type == SDL_QUIT)
                return "exit";

            if (e.type == SDL_KEYDOWN) {

                if (e.key.keysym.sym == SDLK_RIGHT) {
                    factIndex = (factIndex + 1) % facts.size();
                    lastSwitch = std::chrono::steady_clock::now();
                }

                if (e.key.keysym.sym == SDLK_LEFT) {
                    factIndex = (factIndex - 1 + facts.size()) % facts.size();
                    lastSwitch = std::chrono::steady_clock::now();
                }
            }

            if (e.type == SDL_MOUSEMOTION) {
                int mx = e.motion.x;
                int my = e.motion.y;
                hoveredIndex = -1;

                if (isInside(restartBtn.rect, mx, my)) hoveredIndex = 0;
                else if (isInside(menuBtn.rect, mx, my)) hoveredIndex = 1;
                else if (isInside(exitBtn.rect, mx, my)) hoveredIndex = 2;
            }

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x;
                int my = e.button.y;

                if (isInside(restartBtn.rect, mx, my))
                    return "restart";
                if (isInside(menuBtn.rect, mx, my))
                    return "menu";
                if (isInside(exitBtn.rect, mx, my))
                    return "exit";
            }
        }

        // ----- AUTO-SCROLL TIMER -----
        auto now = std::chrono::steady_clock::now();
        float elapsed =
            std::chrono::duration<float>(now - lastSwitch).count();

        if (elapsed >= autoTime) {
            factIndex = (factIndex + 1) % facts.size();
            lastSwitch = now;
            elapsed = 0.0f;
        }

        float countdownRatio = 1.0f - (elapsed / autoTime);
        if (countdownRatio < 0) countdownRatio = 0;

        render(facts[factIndex], countdownRatio, finalScore);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}
