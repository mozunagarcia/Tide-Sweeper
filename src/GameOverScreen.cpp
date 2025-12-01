#include "GameOverScreen.h"
#include <iostream>

GameOverScreen::GameOverScreen(SDL_Renderer* renderer, SDL_Texture* bg)
    : renderer(renderer), background(bg), hoveredIndex(-1)
{
    fontLarge = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 48);
    fontSmall = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 30);

    if (!fontLarge || !fontSmall) {
        std::cerr << "GameOverScreen Font Load Error: "
                  << TTF_GetError() << std::endl;
    }

    int W = 800;
    int H = 600;
    int bw = 220;
    int bh = 50;

    restartBtn = { { 90,  430, bw, bh }, "Restart" };
    menuBtn    = { { 290, 430, bw, bh }, "Main Menu" };
    exitBtn    = { { 490, 430, bw, bh }, "Exit" };

    resumeBtn = { { (W - bw) / 2, 330, bw, 60 }, "Resume" };
}

GameOverScreen::~GameOverScreen() {
    if (fontLarge) TTF_CloseFont(fontLarge);
    if (fontSmall) TTF_CloseFont(fontSmall);
}

bool GameOverScreen::isInside(const SDL_Rect& r, int x, int y) {
    return (x >= r.x && x <= r.x + r.w &&
            y >= r.y && y <= r.y + r.h);
}

void GameOverScreen::render(const std::string& title,
                            const std::string& fact,
                            bool showResume,
                            float countdownRatio)
{
    //--edit--
    // ---- BACKGROUND ----
    if (background) {
        SDL_Rect full = {0, 0, 800, 600};
        SDL_RenderCopy(renderer, background, NULL, &full);

        // add optional dark overlay for readability
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 140);  
        SDL_RenderFillRect(renderer, &full);
    } 
    else {
        // fallback if no background provided
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_Rect bg = {0, 0, 800, 600};
        SDL_RenderFillRect(renderer, &bg);
    }
//--edit--

    SDL_Color white = {255,255,255};

    // ---- TITLE ----
    SDL_Surface* t = TTF_RenderText_Blended(fontLarge, title.c_str(), white);
    SDL_Texture* tt = SDL_CreateTextureFromSurface(renderer, t);
    SDL_Rect tr = { (800 - t->w)/2, 60, t->w, t->h };
    SDL_RenderCopy(renderer, tt, NULL, &tr);
    SDL_FreeSurface(t);
    SDL_DestroyTexture(tt);

    // ---- FACT ----
    SDL_Surface* f = TTF_RenderText_Blended_Wrapped(fontSmall, fact.c_str(), white, 700);
    SDL_Texture* ft = SDL_CreateTextureFromSurface(renderer, f);
    SDL_Rect fr = { 50, 150, f->w, f->h };
    SDL_RenderCopy(renderer, ft, NULL, &fr);
    SDL_FreeSurface(f);
    SDL_DestroyTexture(ft);

    // ---- COUNTDOWN BAR ----
    int barWidth = 700;
    int barHeight = 8;
    int barX = 50;
    int barY = fr.y + fr.h + 20;

    SDL_Rect barBG = { barX, barY, barWidth, barHeight };
    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 200);
    SDL_RenderFillRect(renderer, &barBG);

    SDL_Rect barFG = { barX, barY, (int)(barWidth * countdownRatio), barHeight };
    SDL_SetRenderDrawColor(renderer, 80, 180, 255, 255);
    SDL_RenderFillRect(renderer, &barFG);

    // Buttons
    auto drawBtn = [&](Button& b, int index) {
        if (index == hoveredIndex)
            SDL_SetRenderDrawColor(renderer, 100, 180, 255, 255);
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

    int spacing = 20;          // space between buttons
    int bw = restartBtn.rect.w;
    int bh = restartBtn.rect.h;
    int baseY = 430;

    // ---- RESUME BUTTON ----
    if (showResume) {
        resumeBtn.rect.x = (800 - bw) / 2;  // centered
        resumeBtn.rect.y = baseY - 100;     // stays where you like it
    }

    // ---- OTHER BUTTONS ----
    if (showResume) {
        // align Restart, Menu, Exit UNDER resume, centered as a group
        int totalWidth = 3 * bw + 2 * spacing;
        int startX = (800 - totalWidth) / 2;

        restartBtn.rect.x = startX;
        restartBtn.rect.y = baseY;

        menuBtn.rect.x = startX + bw + spacing;
        menuBtn.rect.y = baseY;

        exitBtn.rect.x = startX + 2 * (bw + spacing);
        exitBtn.rect.y = baseY;
    } else {
        // GAME OVER — no resume → just center the 3 buttons
        int totalWidth = 3 * bw + 2 * spacing;
        int startX = (800 - totalWidth) / 2;

        restartBtn.rect.x = startX;
        restartBtn.rect.y = baseY;

        menuBtn.rect.x = startX + bw + spacing;
        menuBtn.rect.y = baseY;

        exitBtn.rect.x = startX + 2 * (bw + spacing);
        exitBtn.rect.y = baseY;
    }

    // ---- NOW draw ----
    int index = 0;
    if (showResume) drawBtn(resumeBtn, index++);
    drawBtn(restartBtn, index++);
    drawBtn(menuBtn, index++);
    drawBtn(exitBtn, index++);

}

std::string GameOverScreen::run(const std::string& title,
                                const std::vector<std::string>& facts)
{
    bool showResume = (title == "Paused");
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

                int index = 0;
                if (showResume && isInside(resumeBtn.rect, mx, my))
                    hoveredIndex = index;
                if (showResume) index++;

                if (isInside(restartBtn.rect, mx, my)) hoveredIndex = index;
                index++;

                if (isInside(menuBtn.rect, mx, my)) hoveredIndex = index;
                index++;

                if (isInside(exitBtn.rect, mx, my)) hoveredIndex = index;
            }

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x;
                int my = e.button.y;

                if (showResume && isInside(resumeBtn.rect, mx, my))
                    return "resume";
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

        render(title, facts[factIndex], showResume, countdownRatio);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}
