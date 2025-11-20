#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>

class Messages {
public:
    Messages(SDL_Renderer* renderer);
    ~Messages();

    void loadMessageList(const std::vector<std::string>& msgs);
    void start();
    void update();
    void render();
    bool finished() const;

private:
    SDL_Renderer* renderer;
    TTF_Font* font;

    std::vector<std::string> messageList;
    int currentIndex;

    SDL_Texture* currentMessage;


    float alpha;
    Uint32 startTime;
    Uint32 duration;

    bool active;

    // ---- NEW RADIO POPUP ANIMATION ----
    enum State { SLIDE_IN, VISIBLE, SLIDE_OUT, DONE };
    State state;

    float x;          // current X position for slide animation
    float targetX;    // final visible X
    float speed;      // slide speed
    int popupWidth;   // text box width
    int popupHeight;  // text box height


    SDL_Texture* createTexture(const std::string& text, Uint8 alphaValue);
};
