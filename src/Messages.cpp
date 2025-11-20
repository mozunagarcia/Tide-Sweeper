#include "Messages.h"
#include <SDL_ttf.h>

Messages::Messages(SDL_Renderer* renderer)
    : renderer(renderer), font(nullptr), currentMessage(nullptr),
      currentIndex(-1), alpha(0), startTime(0), duration(5000),
      active(false), state(DONE),
      x(-400), targetX(20), speed(12.0f),
      popupWidth(380), popupHeight(110)
{
    font = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 22);
}


Messages::~Messages() {
    if (currentMessage) SDL_DestroyTexture(currentMessage);
    if (font) TTF_CloseFont(font);
}

void Messages::loadMessageList(const std::vector<std::string>& msgs) {
    messageList = msgs;
    currentIndex = -1;
}

void Messages::start() {
    if (messageList.empty()) return;

    currentIndex = 0;
    alpha = 0;
    active = true;

    state = SLIDE_IN;
    x = -popupWidth;        // start offscreen
    startTime = SDL_GetTicks();

    if (currentMessage) SDL_DestroyTexture(currentMessage);
    currentMessage = createTexture(messageList[currentIndex], 0);
}


SDL_Texture* Messages::createTexture(const std::string& text, Uint8 alphaValue)
{
    SDL_Color color = {255, 255, 255, alphaValue};

    const int wrap = 480; // max width of text before wrapping

    SDL_Surface* surface = TTF_RenderText_Blended_Wrapped(font, text.c_str(), color, wrap);

    popupWidth  = surface->w;
    popupHeight = surface->h;

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    return tex;
}


void Messages::update() {
    if (!active || currentIndex < 0) return;

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
            // Fade in
            if (elapsed < fadeTime)
                alpha = (elapsed / (float)fadeTime) * 255;

            // Fully visible
            else if (elapsed < duration - fadeTime)
                alpha = 255;

            // Fade out trigger
            else if (elapsed < duration)
                alpha = 255 - ((elapsed - (duration - fadeTime)) / (float)fadeTime) * 255;

            // Time to slide out
            else {
                state = SLIDE_OUT;
            }
            break;

        case SLIDE_OUT:
            x -= speed;
            if (x <= -popupWidth) {

                currentIndex++;
                if (currentIndex >= (int)messageList.size()) {
                    state = DONE;
                    active = false;
                    return;
                }

                // start next message
                if (currentMessage) SDL_DestroyTexture(currentMessage);
                currentMessage = createTexture(messageList[currentIndex], 0);

                alpha = 0;
                x = -popupWidth;
                startTime = now;
                state = SLIDE_IN;
            }
            break;

        default:
            break;
    }

    SDL_SetTextureAlphaMod(currentMessage, Uint8(alpha));
}


void Messages::render() {
    if (!active || !currentMessage) return;

    int paddingX = 40;
    int paddingY = 25;

    int boxW = popupWidth  + paddingX;
    int boxH = popupHeight + paddingY;

    int headerH = 28;
    int totalH = headerH + boxH;

    int y = 500;  // vertical position

    // --- Background panel ---
    SDL_Rect bg = { (int)x, y, boxW, totalH };
    SDL_SetRenderDrawColor(renderer, 10, 10, 40, 220);
    SDL_RenderFillRect(renderer, &bg);

    // --- Header ---
    SDL_Color white = {255,255,255};
    SDL_Surface* headSurf = TTF_RenderText_Blended(font, "Radio", white);
    SDL_Texture* headTex = SDL_CreateTextureFromSurface(renderer, headSurf);

    SDL_Rect headRect = { (int)x + 12, y + 4, headSurf->w, headSurf->h };
    SDL_RenderCopy(renderer, headTex, NULL, &headRect);

    SDL_FreeSurface(headSurf);
    SDL_DestroyTexture(headTex);

    // --- Message text ---
    SDL_Rect textRect = {
        (int)x + paddingX/2,
        y + headerH + paddingY/2,
        popupWidth,
        popupHeight
    };
    SDL_RenderCopy(renderer, currentMessage, NULL, &textRect);

    SDL_SetRenderDrawColor(renderer, 100,150,255,255);
    SDL_RenderDrawRect(renderer, &bg);
}