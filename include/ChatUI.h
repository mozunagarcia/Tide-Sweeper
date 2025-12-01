#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <string>
#include <vector>

struct ChatMessage {
    std::string sender;
    std::string fullText;
    std::string visibleText;
    bool fromCommander = true;

    // OPTIONAL player responses for this message
    std::vector<std::string> playerResponses; 
};

struct Button {
    SDL_Rect rect;
    std::string text;
};

class ChatUI
{
public:
    ChatUI(SDL_Renderer* renderer, TTF_Font* chatFont);
    ~ChatUI();

    void loadAvatars(const std::string& commanderPath,
                     const std::string& pilotPath);

    void loadSonar(const std::string& sonarPath);

    void loadChatBackground(const std::string& path);



    void startBriefing(const std::string& playerName);
    void update();
    void handleEvent(const SDL_Event& e);
    void render();

    SDL_Rect chatRect { 100, 80, 600, 420 };

    bool briefingDone = false;             
    SDL_Rect getStartButtonRect() const;   
    void reset();                           

private:
    SDL_Renderer* renderer = nullptr;
    TTF_Font* chatFont = nullptr;

    // Script system
    std::vector<ChatMessage> script;
    int scriptIndex = 0;

    ChatMessage current;

    bool typingActive = false;
    Uint32 typingSpeed = 22;
    Uint32 lastCharTime = 0;

    SDL_Texture* commanderAvatar = nullptr;
    SDL_Texture* pilotAvatar = nullptr;

    SDL_Texture* chatBackground = nullptr;


    // Colors
    SDL_Color commanderNameColor { 180, 210, 255, 255 };
    SDL_Color pilotNameColor { 200, 255, 235, 255 };

    SDL_Color commanderBubbleColor { 60, 125, 160, 230 };
    SDL_Color pilotBubbleColor { 70, 150, 130, 255 };

    TTF_Font* briefFont = nullptr;


    // Active response buttons
    std::vector<Button> responseButtons;

    // helpers
   void renderText(const std::string& text, SDL_Color color,
                SDL_Rect& rect, bool wrap = false, TTF_Font* overrideFont = nullptr);


    void renderBubble(int x, int y, int w, int h, SDL_Color color);
    int calculateBubbleHeight(const std::string& text, int maxWidth);

    // internal
    void showNextMessage();
    void spawnResponseButtons();
    bool pendingAutoAdvance = false;
    Uint32 autoAdvanceTime = 0;  // NEW
    Uint32 autoAdvanceDelay = 900; // 0.5s delay (change if desired)
    
    bool showStartButton = false;
    Button startButton;

    int hoveredButton = -1;
    bool startButtonHovered = false;

    SDL_Texture* sonarSprite = nullptr;
    SDL_Rect sonarRect;



};
