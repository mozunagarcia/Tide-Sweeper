#include "Messages.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h> 
#include <iostream>

Messages::Messages(SDL_Renderer* renderer)
    : renderer(renderer)
{
    font = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 22);
    if (!font) {
        std::cerr << "Failed to load message font OpenSans.ttf\n";
    }

    radioTexture = IMG_LoadTexture(renderer, "Assets/Radio.png");
    if (!radioTexture)
        std::cout << "Failed to load Radio.png\n";
}

Messages::~Messages() {
    if (currentMessage) SDL_DestroyTexture(currentMessage);
    if (font) TTF_CloseFont(font);
    if (radioTexture) SDL_DestroyTexture(radioTexture);

}

void Messages::setStyle(MessageStyle s) {
    style = s;
    typewriterActive = false;
    visibleText.clear();
    fullText.clear();
}

void Messages::loadMessageList(const std::vector<std::string>& msgs) {
    messageList = msgs;
    currentIndex = -1;
}

void Messages::start() {
    if (messageList.empty()) return;
    active = true;

    if (style == MessageStyle::RADIO) {
        startTypewriter(messageList[0]);
        return;
    }

    // Cutscene mode
    currentIndex = 0;
    alpha = 0;
    x = -400;
    startTime = SDL_GetTicks();
    state = SLIDE_IN;

    if (currentMessage) SDL_DestroyTexture(currentMessage);
    currentMessage = createTexture(messageList[currentIndex]);
}

void Messages::startTypewriter(const std::string& text) {
    style = MessageStyle::RADIO;
    fullText = text;
    visibleText = "";
    charIndex = 0;
    typewriterActive = true;
    typeStart = SDL_GetTicks();

    // measure text
    int w, h;
    if (TTF_SizeText(font, text.c_str(), &w, &h) == 0) {
        radioW = w + 40;
        radioH = h + 30;
    }
}

SDL_Texture* Messages::createTexture(const std::string& text) {
    SDL_Color white = {255, 255, 255};
    SDL_Surface* surf = TTF_RenderText_Blended_Wrapped(font, text.c_str(), white, 480);
    if (!surf) return nullptr;

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
}

void Messages::update() {
    if (!active) return;

    // ---------------------------------------
    // RADIO TYPEWRITER
    // ---------------------------------------
    if (style == MessageStyle::RADIO) {
        if (!typewriterActive) return;

        Uint32 now = SDL_GetTicks();
        float elapsed = (now - typeStart) / 1000.0f;

        int shouldShow = int(elapsed * charsPerSecond);
        if (shouldShow > charIndex &&
            charIndex < (int)fullText.size())
        {
            charIndex = shouldShow;
            visibleText = fullText.substr(0, charIndex);
        }

        if (charIndex >= (int)fullText.size())
            typewriterActive = false;

        return;
    }

    // ---------------------------------------
    // CUTSCENE SLIDE-IN ANIMATION
    // ---------------------------------------
    Uint32 now = SDL_GetTicks();
    Uint32 elapsed = now - startTime;
    const Uint32 fadeTime = 400;

    switch (state) {
    case SLIDE_IN:
        x += speed;
        if (x >= targetX) {
            x = targetX;
            startTime = now;
            state = VISIBLE;
        }
        break;

    case VISIBLE:
        if (elapsed < fadeTime)
            alpha = (elapsed / float(fadeTime)) * 255;
        else if (elapsed < duration - fadeTime)
            alpha = 255;
        else if (elapsed < duration)
            alpha = 255 - ((elapsed - (duration - fadeTime)) / float(fadeTime)) * 255;
        else
            state = SLIDE_OUT;
        break;

    case SLIDE_OUT:
        x -= speed;
        if (x <= -400) {
            currentIndex++;
            if (currentIndex >= (int)messageList.size()) {
                active = false;
                state = DONE;
                return;
            }

            if (currentMessage) SDL_DestroyTexture(currentMessage);
            currentMessage = createTexture(messageList[currentIndex]);
            startTime = now;
            x = -400;
            alpha = 0;
            state = SLIDE_IN;
        }
        break;

    default:
        break;
    }

    if (currentMessage)
        SDL_SetTextureAlphaMod(currentMessage, Uint8(alpha));
}

void Messages::render() {
    if (!active) return;

    // ---------------------------------------
    // RADIO FIXED PANEL
    // ---------------------------------------
    if (style == MessageStyle::RADIO) {
        int xPos = 80;
        int yPos = 500;

        // --- RADIO SPRITE ---
    if (radioTexture) {
        SDL_Rect radioRect = { 10, yPos - 8, 70, 70 }; // adjust to size you want
        SDL_RenderCopy(renderer, radioTexture, NULL, &radioRect);
    }

        SDL_Rect box = { xPos, yPos, radioW, radioH };
        SDL_SetRenderDrawColor(renderer, 10, 10, 40, 220);
        SDL_RenderFillRect(renderer, &box);

        SDL_SetRenderDrawColor(renderer, 80, 160, 255, 255);
        SDL_RenderDrawRect(renderer, &box);

        SDL_Color white = {255,255,255};
        SDL_Surface* surf =
            TTF_RenderText_Blended_Wrapped(font, visibleText.c_str(), white, radioW - 20);

        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect txt = { xPos + 10, yPos + 10, surf->w, surf->h };
            SDL_RenderCopy(renderer, tex, NULL, &txt);

            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
        }
        return;
    }

    // ---------------------------------------
    // CUTSCENE
    // ---------------------------------------
    if (currentMessage) {
        SDL_Rect rect = { int(x), 450, 380, 110 };
        SDL_RenderCopy(renderer, currentMessage, NULL, &rect);
    }
}
