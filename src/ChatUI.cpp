#include "ChatUI.h"
#include <iostream>

ChatUI::ChatUI(SDL_Renderer* renderer, TTF_Font* chatFont)
    : renderer(renderer), chatFont(chatFont)
{
    briefFont = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 28); 
    if (!briefFont) {
        std::cerr << "Failed to load title font: " << TTF_GetError() << std::endl;
    }
}

ChatUI::~ChatUI()
{
    if (commanderAvatar) SDL_DestroyTexture(commanderAvatar);
    if (pilotAvatar) SDL_DestroyTexture(pilotAvatar);
    if (sonarSprite) SDL_DestroyTexture(sonarSprite);   

}

void ChatUI::loadAvatars(const std::string& commanderPath,
                         const std::string& pilotPath)
{
    SDL_Surface* cSurf = IMG_Load(commanderPath.c_str());
    if (cSurf) {
        commanderAvatar = SDL_CreateTextureFromSurface(renderer, cSurf);
        SDL_FreeSurface(cSurf);
    }

    SDL_Surface* pSurf = IMG_Load(pilotPath.c_str());
    if (pSurf) {
        pilotAvatar = SDL_CreateTextureFromSurface(renderer, pSurf);
        SDL_FreeSurface(pSurf);
    }
}

void ChatUI::startBriefing(const std::string& playerName)
{
    script.clear();
    scriptIndex = 0;

    auto make = [&](const std::string& sender,
                    const std::string& msg,
                    bool fromCommander,
                    std::initializer_list<std::string> responses = {})
    {
        ChatMessage m;
        m.sender = sender;
        m.fullText = msg;
        m.visibleText = "";
        m.fromCommander = fromCommander;

        for (auto& r : responses)
            m.playerResponses.push_back(r);

        script.push_back(m);
    };

    // SCRIPT CONTENT

    make("Commander", "Pilot, this is Command. Do you copy?", true,
        { "Loud and clear.", "Copy that." });

    make("Commander",
        "Good. You have been assigned to the TideSweepers cleanup fleet.",
        true);

    make("Commander",
        "Your first mission takes you into the Coastal Litter Zone.",
        true,
        { "Why that zone?", "Understood." });

    make("Commander",
        "Debris levels are rising... fast.",
        true);

    make("Commander",
        "Wildlife signals show unusual disturbance patterns.",
        true,
        { "Predators?", "Toxic runoff?", "Illegal activity?" });

    make("Commander",
        "Perhaps.. We suspect something suspicious is happening upstream.",
        true);

    make("Commander",
        "Your job is to investigate and document everything.",
        true,
        { "On it.", "I'll handle it." });

    make("Commander",
     "Before you dive in, remember: \n"
     "Use Arrow keys to navigate your sub.",
     true);

    make("Commander",
     "You might encounter scared wildlife: \n"
     "Space sends a Calm Pulse.",
     true);


    make("Commander",
     "If the currents get overwhelming: \n" 
     "Press Esc to pause your mission.",
     true);


    make("Commander",
        "When you're ready... begin your sweep.",
        true,
        { "Moving out.", "Roger." });

    current = script[0];
    typingActive = true;
    current.visibleText = "";
    lastCharTime = SDL_GetTicks();
    responseButtons.clear();
}



void ChatUI::update()
{
    if (!typingActive) return;

    Uint32 now = SDL_GetTicks();
    if (now - lastCharTime < typingSpeed)
        return;

    if (current.visibleText.size() < current.fullText.size())
    {
        current.visibleText.push_back(current.fullText[current.visibleText.size()]);
        lastCharTime = now;
        return;
    }

    // Typing finished
    typingActive = false;

    // If this is a pilot message, start auto-advance AFTER it finishes typing
    if (!current.fromCommander)
    {
        pendingAutoAdvance = true;
        autoAdvanceTime = SDL_GetTicks() + autoAdvanceDelay;
        return; // wait for delay
    }

    // Commander message with choices → show buttons
    if (!current.playerResponses.empty())
    {
        spawnResponseButtons();
    }

}


void ChatUI::handleEvent(const SDL_Event& e)
{
    // Hover detection for buttons
    if (e.type == SDL_MOUSEMOTION)
    {
        int mx = e.motion.x;
        int my = e.motion.y;

        // Reset hover states
        hoveredButton = -1;
        startButtonHovered = false;

        // Hover for choice buttons
        for (int i = 0; i < responseButtons.size(); i++)
        {
            SDL_Rect& r = responseButtons[i].rect;
            if (mx >= r.x && mx <= r.x + r.w &&
                my >= r.y && my <= r.y + r.h)
            {
                hoveredButton = i;
                break;
            }
        }

        // Hover for RETURN TO MENU button
        if (showStartButton)
        {
            SDL_Rect& r = startButton.rect;
            if (mx >= r.x && mx <= r.x + r.w &&
                my >= r.y && my <= r.y + r.h)
            {
                startButtonHovered = true;
            }
        }
    }

    bool clicked =
        (e.type == SDL_MOUSEBUTTONDOWN) ||
        (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE);

    if (!clicked) return;

    int mx = (e.type == SDL_MOUSEBUTTONDOWN) ? e.button.x : -1;
    int my = (e.type == SDL_MOUSEBUTTONDOWN) ? e.button.y : -1;

    // Finish typing instantly
    if (typingActive)
    {
        current.visibleText = current.fullText;
        typingActive = false;

        if (!current.playerResponses.empty())
            spawnResponseButtons();

        return;
    }

    // Handle button clicks
    if (!responseButtons.empty())
    {
        for (auto& b : responseButtons)
        {
            if (mx >= b.rect.x && mx <= b.rect.x + b.rect.w &&
                my >= b.rect.y && my <= b.rect.y + b.rect.h)
            {
                responseButtons.clear();

                ChatMessage reply;
                reply.sender = "You";
                reply.fullText = b.text;
                reply.visibleText = "";
                reply.fromCommander = false;

                current = reply;
                typingActive = true;
                lastCharTime = SDL_GetTicks();

                if (b.text == "Moving out." || b.text == "Roger.") {
                    pendingAutoAdvance = false;
                    showStartButton = true;

                    startButton.text = "Return to Menu";
                    int bw = 200;
                    int bh = 40;

                    startButton.rect = {
                        chatRect.x + (chatRect.w - bw) / 2,
                        chatRect.y + chatRect.h + 20,
                        bw,
                        bh
                    };

                    briefingDone = true;
                    return;
                }
            }
        }
        return;
    }

    // Go to next scripted message
    showNextMessage();
}



void ChatUI::render()
{
    if (!renderer) return;

    // CHAT BACKGROUND 
    SDL_SetRenderDrawColor(renderer, 10, 20, 40, 255);
    SDL_RenderFillRect(renderer, &chatRect);

    // TITLE BAR 
    const int titleHeight = 40;
    SDL_Rect titleRect = { chatRect.x, chatRect.y, chatRect.w, titleHeight };
    SDL_SetRenderDrawColor(renderer, 20, 35, 60, 255);
    SDL_RenderFillRect(renderer, &titleRect);

    SDL_Color titleColor = {200, 220, 255, 255};
   
    // Measure text height first 
    SDL_Surface* tempSurf = TTF_RenderText_Blended(briefFont, "Mission Briefing", titleColor);
    int textW = tempSurf ? tempSurf->w : 0;
    int textH = tempSurf ? tempSurf->h : 0;
    if (tempSurf) SDL_FreeSurface(tempSurf);

    // Center vertically inside title bar
    const int briefHeight = 40;
    int centeredY = chatRect.y + (briefHeight - textH) / 2;

    // Now draw it centered vertically
        SDL_Rect titleTextRect = {
        chatRect.x + (chatRect.w - textW) / 2,
        centeredY,
        textW,
        textH
    };

    renderText("Mission Briefing", titleColor, titleTextRect, false, briefFont);

    // SONAR SPRITE 

    if (sonarSprite)
    {
        SDL_RenderCopy(renderer, sonarSprite, nullptr, &sonarRect);
    }


    // MESSAGE: CENTERED BUBBLE

    int contentTop = chatRect.y + titleHeight;
    int contentHeight = chatRect.h - titleHeight;

    int bubbleWidth = chatRect.w - 80; 
    int bubbleHeight = calculateBubbleHeight(current.visibleText, bubbleWidth);

    // Center bubble

    int verticalOffset = 145;   

    int bubbleX = chatRect.x + (chatRect.w - bubbleWidth) / 2;
    int bubbleY = contentTop + (contentHeight - bubbleHeight) / 2 + verticalOffset;

    // Sender name above bubble, right-aligned
    SDL_Color nameColor = current.fromCommander ?
        commanderNameColor : pilotNameColor;

    std::string nameToShow = current.sender;

    // Measure name width
    SDL_Surface* nameSurf = TTF_RenderText_Blended(chatFont, nameToShow.c_str(), nameColor);
    int nameW = nameSurf ? nameSurf->w : 0;
    int nameH = nameSurf ? nameSurf->h : 0;
    if (nameSurf) SDL_FreeSurface(nameSurf);

    // Sender name aligned to left of bubble
    int namePadding = 2;
    SDL_Rect nameRect {
        bubbleX + namePadding,   // LEFT aligned to bubble
        bubbleY - nameH - 2,     // above bubble
        nameW,
        nameH
    };


    renderText(nameToShow, nameColor, nameRect, false);

    // Draw bubble
    SDL_Color bubbleColor = current.fromCommander ?
        commanderBubbleColor : pilotBubbleColor;

    renderBubble(bubbleX, bubbleY, bubbleWidth, bubbleHeight, bubbleColor);

    SDL_Rect textRect {
        bubbleX + 12,
        bubbleY + 10,
        bubbleWidth - 24,
        bubbleHeight - 20
    };

    renderText(current.visibleText, SDL_Color{255,255,255,255}, textRect, true);

    // RESPONSE BUTTONS 
    SDL_Color white = {255,255,255,255};

    for (int i = 0; i < responseButtons.size(); i++)
    {
        Button& b = responseButtons[i];

        bool isHovered = (i == hoveredButton);

        SDL_Color drawColor = isHovered
            ? SDL_Color{  140, 200, 255, 250 }   // lighter blue on hover
            : SDL_Color{ 60, 120, 200, 250 };  // normal color

        SDL_SetRenderDrawColor(renderer, drawColor.r, drawColor.g, drawColor.b, drawColor.a);
        SDL_RenderFillRect(renderer, &b.rect);

        SDL_Rect tr = { b.rect.x + 10, b.rect.y + 10, b.rect.w - 20, b.rect.h - 20 };
        renderText(b.text, white, tr, false);
    }

    // START BUTTON (Return to Menu) 
    if (showStartButton)
    {
    SDL_Color normal = { 60, 120, 200, 250 };
        SDL_Color hover  = { 140, 200, 255, 250 };

        SDL_Color draw = startButtonHovered ? hover : normal;

        SDL_SetRenderDrawColor(renderer, draw.r, draw.g, draw.b, draw.a);
        SDL_RenderFillRect(renderer, &startButton.rect);

        SDL_Rect tr = { startButton.rect.x + 12,
                        startButton.rect.y + 8,
                        startButton.rect.w - 24,
                        startButton.rect.h - 16 };

        renderText(startButton.text, SDL_Color{255,255,255,255}, tr, false);
    }


}



void ChatUI::renderText(const std::string& text, SDL_Color color,
                        SDL_Rect& rect, bool wrap, TTF_Font* overrideFont)
{
    if (text.empty()) return;

    // Use override font if provided, else fallback to chatFont
    TTF_Font* fontToUse = overrideFont ? overrideFont : chatFont;
    if (!fontToUse) return;

    SDL_Surface* surf = nullptr;

    if (wrap) {
        int wrapWidth = rect.w;
        surf = TTF_RenderText_Blended_Wrapped(fontToUse, text.c_str(), color, wrapWidth);
    } else {
        surf = TTF_RenderText_Blended(fontToUse, text.c_str(), color);
    }

    if (!surf) return;

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_Rect dst = { rect.x, rect.y, surf->w, surf->h };

    SDL_FreeSurface(surf);

    if (tex) {
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);
    }
}




void ChatUI::renderBubble(int x, int y, int w, int h, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect bubble = { x, y, w, h };
    SDL_RenderFillRect(renderer, &bubble);
}

int ChatUI::calculateBubbleHeight(const std::string& text, int maxWidth)
{
    if (!chatFont) return 40;

    if (text.empty()) return 24;

    SDL_Surface* surf =
        TTF_RenderText_Blended_Wrapped(chatFont,
                                       text.c_str(),
                                       SDL_Color{255,255,255,255},
                                       maxWidth);

    if (!surf) return 40;

    int h = surf->h;
    SDL_FreeSurface(surf);

    return h + 20;
}

void ChatUI::showNextMessage()
{
    responseButtons.clear(); // remove old buttons

    if (scriptIndex + 1 < script.size())
    {
        scriptIndex++;
        current = script[scriptIndex];
        current.visibleText = "";
        typingActive = true;
        lastCharTime = SDL_GetTicks();
    }
}

void ChatUI::spawnResponseButtons()
{
    responseButtons.clear();

    // horizontal layout
    int padding = 20;

    int totalButtons = current.playerResponses.size();
    int buttonWidth = 200;
    int buttonHeight = 40;

    // Total width of all buttons including padding
    int totalWidth = totalButtons * buttonWidth + (totalButtons - 1) * padding;

    // Center them horizontally under the chat box
    int startX = chatRect.x + (chatRect.w - totalWidth) / 2;
    int y = chatRect.y + chatRect.h + 20;  // Below the chat box

    int x = startX;

    for (auto& r : current.playerResponses)
    {
        Button b;
        b.text = r;
        b.rect = { x, y, buttonWidth, buttonHeight };

        responseButtons.push_back(b);

        x += buttonWidth + padding; // move to the right for next button
    }
}

SDL_Rect ChatUI::getStartButtonRect() const {
    return startButton.rect;
}

void ChatUI::reset()
{
    script.clear();
    responseButtons.clear();
    showStartButton = false;
    typingActive = false;
    pendingAutoAdvance = false;
    briefingDone = false;
    current.visibleText.clear();
}

void ChatUI::loadSonar(const std::string& sonarPath)
{
    SDL_Surface* sSurf = IMG_Load(sonarPath.c_str());
    if (sSurf)
    {
        sonarSprite = SDL_CreateTextureFromSurface(renderer, sSurf);
        SDL_FreeSurface(sSurf);

        const int briefHeight = 40;

        // WIDE SONAR SIZE
        const int sonarWidth  = 400;   
        const int sonarHeight = 250;

        sonarRect.w = sonarWidth;
        sonarRect.h = sonarHeight;

        // Center horizontally
        sonarRect.x = chatRect.x + (chatRect.w - sonarWidth) / 2;

        // Place below title bar
        sonarRect.y = chatRect.y + briefHeight + 20;
    }
}

void ChatUI::loadChatBackground(const std::string& path)
{
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf) {
        std::cerr << "Failed to load chat background: " << IMG_GetError() << std::endl;
        return;
    }

    chatBackground = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
}
