#include "Menu.hpp"
#include <SDL_image.h>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// Constructor
Menu::Menu(SDL_Renderer* renderer)
    : renderer(renderer),
      font(nullptr),
      menuBackgroundTexture(nullptr),
      instructionsBackgroundTexture(nullptr),
      menuMusic(nullptr),
      selectedIndex(0),
      // showInstructions(false),
      hoveredIndex(-1),
      briefingActive(false),
      nameEntryActive(false),
      choiceActive(false),
      briefingFinished(false),
      currentMessageIndex(0),
      selectedChoice(-1),
      playerName(""),
      isTyping(false),
      fullMessage(""),
      visibleMessage(""),
      lastCharTime(0),
      typingSpeedMs(25)
{

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Initialize SDL_mixer for menu music
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer could not initialize! Mix_Error: " << Mix_GetError() << std::endl;
    } else {
        // Load menu background music
        menuMusic = Mix_LoadMUS("Assets/music/background_music.mp3");
        if (!menuMusic) {
            menuMusic = Mix_LoadMUS("Assets/music/background_music.wav");
            if (!menuMusic) {
                std::cerr << "Failed to load menu music! Mix_Error: " << Mix_GetError() << std::endl;
            }
        }
        
        // Play menu music on loop
        if (menuMusic) {
            Mix_PlayMusic(menuMusic, -1);
            Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
        }
    }

    chatFont = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 22);
    if (!chatFont) {
        std::cerr << "Failed to load UI font: " << TTF_GetError() << std::endl;
    }

    titleFont = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 50);
    if (!titleFont) {
        std::cerr << "Failed to load title font: " << TTF_GetError() << std::endl;
    }
        
    // --- Load font ---
    font = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 40);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
    }

    if (!chatFont) std::cerr << "UI font failed: " << TTF_GetError() << "\n";

    // CREATE CHAT
    chat = new ChatUI(renderer, chatFont);

    chat->loadSonar("Assets/sonar.png");
    // chat->loadChatBackground("Assets/backgrounds/chat_background.png");


    // --- Load main menu background ---
    SDL_Surface* menuSurface = IMG_Load("Assets/backgrounds/menu_background.png");
    if (!menuSurface) {
        std::cerr << "Failed to load menu background: " << IMG_GetError() << std::endl;
    } else {
        menuBackgroundTexture = SDL_CreateTextureFromSurface(renderer, menuSurface);
        SDL_FreeSurface(menuSurface);
    }

    // --- Load instructions background ---
    SDL_Surface* instructionsSurface = IMG_Load("Assets/backgrounds/instructions_background.png");
    if (!instructionsSurface) {
        std::cerr << "Failed to load instructions background: " << IMG_GetError() << std::endl;
    } else {
        instructionsBackgroundTexture = SDL_CreateTextureFromSurface(renderer, instructionsSurface);
        SDL_FreeSurface(instructionsSurface);
    }

    SDL_Surface* bgSurf = IMG_Load("Assets/backgrounds/chat_background.png");
    if (!bgSurf) {
        std::cerr << "Failed to load chat BG: " << IMG_GetError() << std::endl;
    } else {
        chatBGTexture = SDL_CreateTextureFromSurface(renderer, bgSurf);
        SDL_FreeSurface(bgSurf);
    }


    // --- Define menu options ---
    items = {"Start Game", "Instructions", "Quit"};
}


// Destructor
Menu::~Menu() {
    if (menuBackgroundTexture) SDL_DestroyTexture(menuBackgroundTexture);
    if (instructionsBackgroundTexture) SDL_DestroyTexture(instructionsBackgroundTexture);
    if (font) TTF_CloseFont(font);
    
    // Stop and free menu music
    if (menuMusic) {
        Mix_HaltMusic();
        Mix_FreeMusic(menuMusic);
        menuMusic = nullptr;
    }
    delete chat;

}

// Handle clicks & quit events
void Menu::handleEvent(const SDL_Event& e, bool& running, bool& startGame) {

    // If briefing active and it just finished
    if (briefingActive && chat->briefingDone) {
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mx = e.button.x;
            int my = e.button.y;

            // Check if they clicked the "Return to Menu" button
            SDL_Rect r = chat->getStartButtonRect();
            if (mx >= r.x && mx <= r.x + r.w &&
                my >= r.y && my <= r.y + r.h)
            {
                // Exit chat
                briefingActive = false;
                chat->reset();  // optional cleanup
                return;
            }
        }

        // Still render chat and block menu events
        chat->handleEvent(e);
        return;
    }


    // Global quit
    if (e.type == SDL_QUIT) {
        running = false;
        return;
    }
    // If briefing is active, ChatUI handles all clicks
    if (briefingActive) {
        chat->handleEvent(e);
        return;
    }

    // MAIN MENU LOGIC

    // Hover detection
    if (e.type == SDL_MOUSEMOTION) {
        int mx = e.motion.x;
        int my = e.motion.y;
        hoveredIndex = -1;

        const int buttonSpacing = 90;
        const int paddingX = 25;
        const int paddingY = 15;

        int totalMenuHeight = (int)items.size() * buttonSpacing;
        int startY = (WINDOW_HEIGHT - totalMenuHeight) / 2 + 60;

        for (int i = 0; i < items.size(); i++) {
            SDL_Surface* ts = TTF_RenderText_Blended(font, items[i].c_str(), {255,255,255});

        if (!ts) {
            std::cerr << "TTF_RenderText_Blended failed: " << TTF_GetError() << std::endl;
            continue;  // prevent crash
        }

        int textW = ts->w;
        int textH = ts->h;

        SDL_FreeSurface(ts);


            int xPos = (WINDOW_WIDTH - textW) / 2;
            int yPos = startY + i * buttonSpacing;

            SDL_Rect rect = {
                xPos - paddingX,
                yPos - paddingY,
                textW + paddingX * 2,
                textH + paddingY * 2
            };

            if (mx >= rect.x && mx <= rect.x + rect.w &&
                my >= rect.y && my <= rect.y + rect.h)
            {
                hoveredIndex = i;
                break;
            }
        }
    }

    // Menu clicks
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (hoveredIndex != -1) {
            if (items[hoveredIndex] == "Start Game") {
                startGame = true;
            }
            else if (items[hoveredIndex] == "Instructions") {
                startBriefing();
            }
            else if (items[hoveredIndex] == "Quit") {
                running = false;
            }
        }
    }
}
 

// Render main menu or instructions screen
void Menu::render() 
{
    if (briefingActive) {
    // Draw the ORANGE background behind everything
    if (chatBGTexture)
        SDL_RenderCopy(renderer, chatBGTexture, NULL, NULL);
    else {
        SDL_SetRenderDrawColor(renderer, 10, 25, 60, 255);
        SDL_RenderClear(renderer);
    }

    // Draw the chat UI in front
    chat->update();
    chat->render();
    return;
}


    // ----------------------------
    // OTHERWISE, normal main menu
    // ----------------------------

    SDL_RenderCopy(renderer, menuBackgroundTexture, NULL, NULL);

    if (showInstructions) {
        renderInstructions();
    } else {
        renderMainMenu();
    }
}


void Menu::renderMainMenu() {

    if (menuBackgroundTexture) {
    SDL_RenderCopy(renderer, menuBackgroundTexture, nullptr, nullptr);
    } else {
        SDL_SetRenderDrawColor(renderer, 10, 25, 60, 255);
        SDL_RenderClear(renderer);
    }

    // --- DARKEN OVERLAY ---
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 40);   // adjust 120 for desired effect
    SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(renderer, &overlay);

    // ---- WELCOME TITLE ----
    float t = SDL_GetTicks() / 1000.0f;
    float speed = 1.0f;       
    float amplitude = 12.0f;

    float phase = fmod(t * speed, 2.0f);
    float wave = (phase < 1.0f)
        ? amplitude * phase
        : amplitude * (2.0f - phase);

   int titleY = 120 + (int)wave;


    // --- Glow background for title ---
    SDL_Color glowColor = {80, 160, 255, 180}; // ocean blue glow
    SDL_Surface* glowSurf = TTF_RenderText_Blended(titleFont, "Welcome to TideSweepers", glowColor);
    SDL_Texture* glowTex = SDL_CreateTextureFromSurface(renderer, glowSurf);

    SDL_Rect glowRect = {
        (WINDOW_WIDTH - glowSurf->w) / 2 - 3,
        titleY - 3,
        glowSurf->w + 6,
        glowSurf->h + 6
    };

    SDL_RenderCopy(renderer, glowTex, nullptr, &glowRect);

    SDL_FreeSurface(glowSurf);
    SDL_DestroyTexture(glowTex);

    if (titleFont) {
        SDL_Color titleColor = {255, 255, 255, 255}; // bright white
        
        SDL_Surface* surf = TTF_RenderText_Blended(titleFont, "Welcome to TideSweepers", titleColor);
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);

        SDL_Rect pos = {
            (WINDOW_WIDTH - surf->w) / 2,
            titleY,  // <-- adjust this to move up or down
            surf->w,
            surf->h
        };

        SDL_RenderCopy(renderer, tex, nullptr, &pos);

        SDL_FreeSurface(surf);
        SDL_DestroyTexture(tex);
    }

    // Vertical layout settings
    const int buttonSpacing = 90;  // distance between button centers
    const int paddingX = 25;        // horizontal padding around text
    const int paddingY = 15;        // vertical padding around text

    int totalMenuHeight = static_cast<int>(items.size()) * buttonSpacing;
    int startY = (WINDOW_HEIGHT - totalMenuHeight) / 2 + 60;

    for (int i = 0; i < items.size(); ++i) {
        // --- Text color ---
        SDL_Color color = {220, 240, 255, 255}; // soft white-blue text
        if (i == hoveredIndex) color = {255, 210, 80, 255}; // warm gold when hovered

        SDL_Surface* textSurface = TTF_RenderText_Blended(font, items[i].c_str(), color);
        SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurface);

        int textW, textH;
        SDL_QueryTexture(textTex, nullptr, nullptr, &textW, &textH);

        // --- Centered positions ---
        int xPos = (WINDOW_WIDTH - textW) / 2;
        int yPos = startY + i * buttonSpacing;

        // --- Button background box (smaller and consistent) ---
        SDL_Rect buttonRect = {
            xPos - paddingX,
            yPos - paddingY,
            textW + paddingX * 2,
            textH + paddingY * 2
        };

        // --- Button background color ---
        if (i == hoveredIndex)
            SDL_SetRenderDrawColor(renderer, 100, 200, 255, 160);  // aqua glow
        else
            SDL_SetRenderDrawColor(renderer, 25, 55, 100, 180);    // dark navy

        SDL_RenderFillRect(renderer, &buttonRect);

        // --- Border ---
        SDL_SetRenderDrawColor(renderer, 180, 230, 255, 200);
        SDL_RenderDrawRect(renderer, &buttonRect);

        // --- Render text centered in button ---
        SDL_Rect dst = {xPos, yPos, textW, textH};
        SDL_RenderCopy(renderer, textTex, nullptr, &dst);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTex);
        }
}


void Menu::renderInstructions() {
    // --- Background setup ---
    if (instructionsBackgroundTexture) {
    SDL_RenderCopy(renderer, instructionsBackgroundTexture, nullptr, nullptr);
    } else {
    SDL_SetRenderDrawColor(renderer, 15, 40, 90, 255);
    SDL_RenderClear(renderer);
    }


    // --- Use a smaller font for instructions text ---
    TTF_Font* smallFont = TTF_OpenFont("Assets/fonts/OpenSans.ttf", 28); // Smaller font size
    if (!smallFont) {
        std::cerr << "Failed to load small font: " << TTF_GetError() << std::endl;
        return;
    }

    // --- Text color ---
    SDL_Color white = {255, 255, 255, 255};

    // --- Cute intro text ---
    SDL_Surface* textSurface = TTF_RenderText_Blended_Wrapped(
        smallFont,
        "The sea needs your help!\n\n"
        "Use the arrow keys to steer your submarine through the waves.\n\n"
        "Swim close to litter to collect it and keep the ocean clean!\n\n"
        "Press ESC anytime to get back to the main menu.",
        white, 600);

    SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurface);

    int textW = 0, textH = 0;
    SDL_QueryTexture(textTex, nullptr, nullptr, &textW, &textH);

    // --- Center the text nicely on screen ---
    SDL_Rect dst = {
        (WINDOW_WIDTH - textW) / 2,
        (WINDOW_HEIGHT - textH) / 2,
        textW,
        textH
    };

    // --- Render to screen ---
    SDL_RenderCopy(renderer, textTex, nullptr, &dst);

    // --- Cleanup ---
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTex);
    TTF_CloseFont(smallFont); //  Close small font after use
}

void Menu::startBriefing() {
    briefingActive = true;
    chat->startBriefing("Pilot");
}


void Menu::renderNameEntry() {

    // Sonar background
    SDL_SetRenderDrawColor(renderer, 5, 20, 40, 255);
    SDL_RenderFillRect(renderer, &radioRect);

    SDL_SetRenderDrawColor(renderer, 0, 40, 70, 255);
    for (int y = radioRect.y; y < radioRect.y + radioRect.h; y += 6) {
        SDL_RenderDrawLine(renderer, radioRect.x, y, radioRect.x + radioRect.w, y);
    }

    // Prompt text
    SDL_Rect promptRect = { radioRect.x + 20, radioRect.y + 20, 600, 40 };
    SDL_Texture* promptTex = renderText("Enter your pilot name:", sonarTextColor, promptRect);
    if (promptTex) {
        SDL_RenderCopy(renderer, promptTex, NULL, &promptRect);
        SDL_DestroyTexture(promptTex);
    }

    // Typed name (SAFE even if empty)
    SDL_Rect nameRect = { radioRect.x + 20, radioRect.y + 80, 600, 40 };
    SDL_Texture* nameTex = renderText(playerName, sonarTextColor, nameRect);
    if (nameTex) {
        SDL_RenderCopy(renderer, nameTex, NULL, &nameRect);
        SDL_DestroyTexture(nameTex);
    }

    // Hint text
    SDL_Rect hintRect = { radioRect.x + 20, radioRect.y + 150, 600, 40 };
    SDL_Texture* hintTex = renderText("(Press ENTER to continue)", sonarHintColor, hintRect);
    if (hintTex) {
        SDL_RenderCopy(renderer, hintTex, NULL, &hintRect);
        SDL_DestroyTexture(hintTex);
    }
}


void Menu::renderBriefingChoices() {

std::cout << "[Menu] startBriefing() triggered\n";

    SDL_SetRenderDrawColor(renderer, 5, 20, 40, 255);
    SDL_RenderFillRect(renderer, &radioRect);

    SDL_SetRenderDrawColor(renderer, 0, 40, 70, 255);
    for (int y = radioRect.y; y < radioRect.y + radioRect.h; y += 6) {
        SDL_RenderDrawLine(renderer, radioRect.x, y, radioRect.x + radioRect.w, y);
    }

    std::string q = "[CMD] Understood, Pilot " + playerName + ". Your response?";

    SDL_Rect qRect = { radioRect.x + 20, radioRect.y + 20, 640, 40 };
    SDL_Texture* qTex = renderText(q, sonarTextColor, qRect, true);
    if (qTex) { SDL_RenderCopy(renderer, qTex, NULL, &qRect); SDL_DestroyTexture(qTex); }

    SDL_Rect aRect = { radioRect.x + 20, radioRect.y + 120, 640, 40 };
    SDL_Texture* aTex = renderText("A) " + choiceA, sonarTextColor, aRect, true);
    if (aTex) { SDL_RenderCopy(renderer, aTex, NULL, &aRect); SDL_DestroyTexture(aTex); }

    SDL_Rect bRect = { radioRect.x + 20, radioRect.y + 180, 640, 40 };
    SDL_Texture* bTex = renderText("B) " + choiceB, sonarTextColor, bRect, true);
    if (bTex) { SDL_RenderCopy(renderer, bTex, NULL, &bRect); SDL_DestroyTexture(bTex); }
}



void Menu::renderBriefing() {

    if (currentMessageIndex >= (int)briefingMessages.size()) return;
    if (!chatFont) return;

    // Typewriter update
    if (isTyping) {
        Uint32 now = SDL_GetTicks();
        if (now - lastCharTime > typingSpeedMs) {
            if (visibleMessage.size() < fullMessage.size()) {
                if (visibleMessage.size() < fullMessage.size()) {
                    visibleMessage += fullMessage[visibleMessage.size()];
                }
                lastCharTime = now;
            } else {
                isTyping = false;
            }
        }
    }

    // Sonar radio panel background
    SDL_SetRenderDrawColor(renderer, 5, 20, 40, 255);
    SDL_RenderFillRect(renderer, &radioRect);

    // sonar scanlines
    SDL_SetRenderDrawColor(renderer, 0, 40, 70, 255);
    for (int y = radioRect.y; y < radioRect.y + radioRect.h; y += 6) {
        SDL_RenderDrawLine(renderer, radioRect.x, y, radioRect.x + radioRect.w, y);
    }

    // Message text
    SDL_Rect msgRect = { radioRect.x + 20, radioRect.y + 20, radioRect.w - 40, 200 };
    SDL_Texture* msgTex = renderText(visibleMessage, sonarTextColor, msgRect, true);
    if (msgTex) {
        SDL_RenderCopy(renderer, msgTex, NULL, &msgRect);
        SDL_DestroyTexture(msgTex);
    }

    // Continue indicator
    if (!isTyping) {
        SDL_Rect r = { radioRect.x + 20, radioRect.y + radioRect.h - 40, 400, 40 };
        SDL_Texture* t = renderText("► Click to continue", sonarHintColor, r);
        if (t) {
            SDL_RenderCopy(renderer, t, NULL, &r);
            SDL_DestroyTexture(t);
        }
    }
}


SDL_Texture* Menu::renderText(const std::string& msg, SDL_Color color, SDL_Rect& dst, bool wrap)
{
    if (!chatFont) return nullptr;

    if (msg.empty()) return nullptr;

    SDL_Surface* surf = nullptr;

    if (wrap) {
        int wrapWidth = dst.w;   // use width only
        surf = TTF_RenderText_Blended_Wrapped(chatFont, msg.c_str(), color, wrapWidth);
        dst.h = surf->h;
    } else {
        surf = TTF_RenderText_Blended(chatFont, msg.c_str(), color);
    }

    if (!surf) return nullptr;

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    dst.w = surf->w;
    dst.h = surf->h;

    SDL_FreeSurface(surf);
    return tex;
}


