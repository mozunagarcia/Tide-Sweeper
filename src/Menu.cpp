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
      selectedIndex(0),
      showInstructions(false),
      hoveredIndex(-1)
{
    // --- Load font ---
    font = TTF_OpenFont("assets/fonts/OpenSans.ttf", 48);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
    }

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

    // --- Define menu options ---
    items = {"Start Game", "Instructions", "Quit"};
}


// Destructor
Menu::~Menu() {
    if (menuBackgroundTexture) SDL_DestroyTexture(menuBackgroundTexture);
    if (instructionsBackgroundTexture) SDL_DestroyTexture(instructionsBackgroundTexture);
    if (font) TTF_CloseFont(font);

}

// Handle clicks & quit events
void Menu::handleEvent(const SDL_Event& e, bool& running, bool& startGame) {
    if (e.type == SDL_QUIT) {
        running = false;
        return;
    }

    // Handle mouse movement
    if (e.type == SDL_MOUSEMOTION) {
        int x = e.motion.x;
        int y = e.motion.y;
        hoveredIndex = -1; // default to none

        // Check if mouse is over any menu item
        for (int i = 0; i < items.size(); ++i) {

            int totalMenuHeight = static_cast<int>(items.size()) * 80;
            int startY = (WINDOW_HEIGHT - totalMenuHeight) / 2;
            int itemY = startY + i * 80;
            int itemX = (WINDOW_WIDTH / 2) - 150;

            int itemW = 300;
            int itemH = 60;

            if (x >= itemX && x <= itemX + itemW && y >= itemY && y <= itemY + itemH) {
                hoveredIndex = i;
                break;
            }
        }
    }

    // Handle mouse clicks
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (hoveredIndex != -1) {
            if (items[hoveredIndex] == "Start Game") {
                startGame = true;
            } else if (items[hoveredIndex] == "Instructions") {
                showInstructions = true;
            } else if (items[hoveredIndex] == "Quit") {
                running = false;
            }
        }
    }

    // Handle ESC key to go back or quit
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
        if (showInstructions) showInstructions = false;
        else running = false;
    }
}

// Render main menu or instructions screen
void Menu::render() {
    SDL_SetRenderDrawColor(renderer, 10, 25, 60, 255);  // deep ocean blue
    SDL_RenderClear(renderer);

    if (showInstructions)
        renderInstructions();
    else
        renderMainMenu();
}

void Menu::renderMainMenu() {

    if (menuBackgroundTexture) {
    SDL_RenderCopy(renderer, menuBackgroundTexture, nullptr, nullptr);
    } else {
        SDL_SetRenderDrawColor(renderer, 10, 25, 60, 255);
        SDL_RenderClear(renderer);
    }
    // Vertical layout settings
    const int buttonSpacing = 110;  // distance between button centers
    const int paddingX = 25;        // horizontal padding around text
    const int paddingY = 15;        // vertical padding around text

    int totalMenuHeight = static_cast<int>(items.size()) * buttonSpacing;
    int startY = (WINDOW_HEIGHT - totalMenuHeight) / 2;

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
    TTF_Font* smallFont = TTF_OpenFont("assets/fonts/OpenSans.ttf", 28); // Smaller font size
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



