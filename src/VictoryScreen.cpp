#include "VictoryScreen.h"
#include <SDL_image.h>
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

    charsPerSecond = 55;

    fontTitle  = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 52);
    fontStats  = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 32);
    fontHeader = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 32);
    fontBody   = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 24);

    if (!fontTitle || !fontStats || !fontHeader || !fontBody) {
    std::cerr << "VictoryScreen font failed to load: " << TTF_GetError() << std::endl;
    }

    bgTexture = IMG_LoadTexture(renderer, "Assets/backgrounds/victory_background.png");
    if (!bgTexture) {
        std::cerr << "Failed to load victory background: " << IMG_GetError() << std::endl;
    }

}

VictoryScreen::~VictoryScreen() {
    if (fontLarge) TTF_CloseFont(fontLarge);
    if (fontSmall) TTF_CloseFont(fontSmall);

    if (fontTitle)  TTF_CloseFont(fontTitle);
    if (fontStats)  TTF_CloseFont(fontStats);
    if (fontHeader) TTF_CloseFont(fontHeader);
    if (fontBody)   TTF_CloseFont(fontBody);

    if (bgTexture) SDL_DestroyTexture(bgTexture);

}


bool VictoryScreen::isInside(const SDL_Rect& r, int x, int y) {
    return (x >= r.x && x <= r.x + r.w &&
            y >= r.y && y <= r.y + r.h);
}


void VictoryScreen::render(float countdownRatio, int finalScore)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Draw background
    if (bgTexture) {
        SDL_Rect dest = {0, 0, 800, 600};
        SDL_RenderCopy(renderer, bgTexture, NULL, &dest);
    } else {
        // fallback if failed to load
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect bg = {0, 0, 800, 600};
        SDL_RenderFillRect(renderer, &bg);
    }

    // Transparent Victory Panel 
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 120);   // semi-transparent black
    SDL_Rect panel = { 0, 0, 800, 600 };
    SDL_RenderFillRect(renderer, &panel);


    SDL_Color white = {255,255,255};
    SDL_Color gold  = {255,215,0};

    int y;
    // STATIC elements — fixed positions
    int closingLineY = 435;
    int buttonY      = 485;

    // CENTERING + WIDTH CONTROL
    const int wrapWidth = 600;                     // typewriter width control
    const int centerX   = (800 - wrapWidth) / 2;   // left edge of centered block


    // 1. TITLE (large, centered)
    {
        y = 50;
        SDL_Surface* surf = TTF_RenderText_Blended(fontTitle, "Mission Successful!", gold);
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_Rect r = { (800 - surf->w)/2, y, surf->w, surf->h };
        SDL_RenderCopy(renderer, tex, NULL, &r);

        int statsY = y + surf->h - 60;

        y = statsY + 70; // move rest of layout below stats

        SDL_FreeSurface(surf);
        SDL_DestroyTexture(tex);

    }


    // STATS BLOCK — YOU CONTROL SIZE AND POSITION

    {
            std::string score = "Final Score: " + std::to_string(finalScore);
        std::string rank  = "Rank Earned: TideSweeper Hero";

        SDL_Surface* s1 = TTF_RenderText_Blended(fontStats, score.c_str(), white);
        SDL_Surface* s2 = TTF_RenderText_Blended(fontStats, rank.c_str(), white);

        SDL_Texture* t1 = SDL_CreateTextureFromSurface(renderer, s1);
        SDL_Texture* t2 = SDL_CreateTextureFromSurface(renderer, s2);

        // CENTER HORIZONTALLY
        int scoreX = (800 - s1->w) / 2;
        int rankX  = (800 - s2->w) / 2;

        SDL_Rect r1 = { scoreX, y, s1->w, s1->h };
        SDL_Rect r2 = { rankX,  y + s1->h + 10, s2->w, s2->h };

        SDL_RenderCopy(renderer, t1, nullptr, &r1);
        SDL_RenderCopy(renderer, t2, nullptr, &r2);

        y = r2.y + r2.h + 40;

        SDL_FreeSurface(s1);
        SDL_FreeSurface(s2);
        SDL_DestroyTexture(t1);
        SDL_DestroyTexture(t2);
    }


    // TYPEWRITER BODY TEXT
    {
        if (!typedText.empty()) {
            SDL_Surface* surf = TTF_RenderText_Blended_Wrapped(fontBody, typedText.c_str(), white, wrapWidth);
            if (!surf) {
                std::cerr << "TTF Error: " << TTF_GetError() << std::endl;
                return;
            }
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);

            SDL_Rect r = {
                centerX,   // <-- horizontally centered
                y - 20,
                surf->w,
                surf->h
            };

            SDL_RenderCopy(renderer, tex, NULL, &r);

            y += surf->h + 40;

            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
        }

    }


    // CLOSING LINE (centered, medium size)
    {
        SDL_Surface* surf = TTF_RenderText_Blended(fontStats, "Excellent work out there.", white);
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);

        SDL_Rect r = {
        centerX + (wrapWidth - surf->w)/2,  // <-- centered inside same block width
        closingLineY,
        surf->w,
        surf->h
        
        };


        SDL_RenderCopy(renderer, tex, NULL, &r);

        SDL_FreeSurface(surf);
        SDL_DestroyTexture(tex);
    }



    // BUTTONS (MOVE THEM DOWN HERE)
        restartBtn.rect.y = buttonY;
        menuBtn.rect.y    = buttonY;
        exitBtn.rect.y    = buttonY;

        int totalWidth =
        restartBtn.rect.w +
        menuBtn.rect.w +
        exitBtn.rect.w +
        40; // spacing

        int startX = (800 - totalWidth) / 2;

        restartBtn.rect.x = startX;
        menuBtn.rect.x    = startX + restartBtn.rect.w + 20;
        exitBtn.rect.x    = menuBtn.rect.x + menuBtn.rect.w + 20;


        auto drawBtn = [&](Button& b, int index) {
            SDL_SetRenderDrawColor(renderer,
                (index == hoveredIndex ? 255 : 0),
                (index == hoveredIndex ? 215 : 80),
                (index == hoveredIndex ? 0   : 160),
                255);
            SDL_RenderFillRect(renderer, &b.rect);

            SDL_Surface* txt = TTF_RenderText_Blended(fontStats, b.label.c_str(), white);
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, txt);
            SDL_Rect lr = {
                b.rect.x + (b.rect.w - txt->w)/2,
                b.rect.y + (b.rect.h - txt->h)/2,
                txt->w, txt->h
            };
            SDL_RenderCopy(renderer, tex, NULL, &lr);

            SDL_FreeSurface(txt);
            SDL_DestroyTexture(tex);
        };

        drawBtn(restartBtn, 0);
        drawBtn(menuBtn, 1);
        drawBtn(exitBtn, 2);

    // FOOTER CREDIT (tiny text at the bottom)
    {
        SDL_Color gray = {200, 200, 200};

        SDL_Surface* creditSurf =
            TTF_RenderText_Blended(fontBody, "Created by Laura, Mari, and Sara", gray);

        if (creditSurf) {
            SDL_Texture* creditTex =
                SDL_CreateTextureFromSurface(renderer, creditSurf);

            SDL_Rect creditRect = {
                (800 - creditSurf->w) / 2,  // centered
                550,                        // bottom of window
                creditSurf->w,
                creditSurf->h
            };

            SDL_RenderCopy(renderer, creditTex, nullptr, &creditRect);

            SDL_FreeSurface(creditSurf);
            SDL_DestroyTexture(creditTex);
        }
    }

}




std::string VictoryScreen::run(int finalScore)
{
    hoveredIndex = -1;

    fullText =
        "Pilot, this is Command.\n\n"
        "Sweep successful. You identified the dumping site and confirmed illegal "
        "activity in the region. The report has been sent to the Coastal Authority "
        "and cleanup operations are already underway.\n";


    typedText = "";
    typeTimer = 0.0f;
    typeIndex = 0;

    auto lastFrame = std::chrono::steady_clock::now();

    while (true) {

        SDL_Event e;
        while (SDL_PollEvent(&e)) {

            if (e.type == SDL_QUIT)
                return "exit";

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

                if (isInside(restartBtn.rect, mx, my)) return "restart";
                if (isInside(menuBtn.rect, mx, my))    return "menu";
                if (isInside(exitBtn.rect, mx, my))    return "exit";
            }
        }

        // Typewriter progression
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - lastFrame).count();
        lastFrame = now;

        typeTimer += dt;
        int charsToShow = (int)(typeTimer * charsPerSecond);

        if (charsToShow > typeIndex) {
            typeIndex = charsToShow;
            if (typeIndex > (int)fullText.size())
                typeIndex = (int)fullText.size();

            typedText = fullText.substr(0, typeIndex);
        }

        // Render
        render(1.0f, finalScore);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }
}
