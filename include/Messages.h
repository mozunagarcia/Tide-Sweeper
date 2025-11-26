#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <string>

enum class MessageStyle {
    RADIO,
    CUTSCENE
};

class Messages {
public:
    Messages(SDL_Renderer* renderer);
    ~Messages();

    void loadMessageList(const std::vector<std::string>& msgs);
    void start();
    void update();
    void render();
    void setStyle(MessageStyle s);

    bool isActive() const { return active; }

private:
    SDL_Texture* createTexture(const std::string& text);
    void startTypewriter(const std::string& text);

    SDL_Renderer* renderer;
    TTF_Font* font;

    MessageStyle style = MessageStyle::RADIO;

    SDL_Texture* radioTexture = nullptr;

    // --- Radio typewriter ---
    std::string fullText;
    std::string visibleText;
    Uint32 typeStart = 0;
    int charIndex = 0;
    int charsPerSecond = 40;
    bool typewriterActive = false;

    int radioW = 380;
    int radioH = 80;

    // --- Cutscene ---
    std::vector<std::string> messageList;
    SDL_Texture* currentMessage = nullptr;
    int currentIndex = -1;

    float alpha = 0;
    Uint32 startTime = 0;
    Uint32 duration = 5000;

    bool active = false;

    enum State { SLIDE_IN, VISIBLE, SLIDE_OUT, DONE };
    State state = DONE;

    float x = -400;
    float targetX = 20;
    float speed = 12.0f;

    int popupWidth = 380;
    int popupHeight = 110;
};
