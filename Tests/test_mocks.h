#pragma once
#include <gmock/gmock.h>
#include <SDL.h>

// Mock Renderer to track render calls
class MockRenderer {
public:
    MOCK_METHOD(int, RenderCopy, (SDL_Texture*, const SDL_Rect*, const SDL_Rect*));
    MOCK_METHOD(int, RenderFillRect, (const SDL_Rect*));
    MOCK_METHOD(int, SetRenderDrawColor, (Uint8, Uint8, Uint8, Uint8));
    MOCK_METHOD(int, RenderClear, ());
    MOCK_METHOD(void, RenderPresent, ());
    
    int callCount = 0;
    
    void recordCall() { callCount++; }
    int getCallCount() const { return callCount; }
    void resetCallCount() { callCount = 0; }
};

// Mock Texture to control texture properties
class MockTexture {
public:
    MOCK_METHOD(int, QueryTexture, (Uint32*, int*, int*, int*));
    
    int width = 64;
    int height = 64;
    
    void setDimensions(int w, int h) {
        width = w;
        height = h;
    }
    
    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

// Stub for collision detection that always returns a fixed result
class CollisionStub {
public:
    bool shouldCollide = true;
    
    bool checkCollision(const SDL_Rect& a, const SDL_Rect& b) {
        return shouldCollide;
    }
    
    void setCollisionResult(bool result) {
        shouldCollide = result;
    }
};

// Stub for score that provides controlled score values
class ScoreStub {
public:
    int fixedScore = 100;
    int incrementAmount = 10;
    
    int getScore() const { return fixedScore; }
    
    void addScore(int amount) {
        fixedScore += amount;
    }
    
    void setScore(int score) {
        fixedScore = score;
    }
    
    int getIncrementAmount() const { return incrementAmount; }
};

// Stub for position/movement
class PositionStub {
public:
    float x = 0.0f;
    float y = 0.0f;
    float speed = 5.0f;
    
    void setPosition(float newX, float newY) {
        x = newX;
        y = newY;
    }
    
    void move(float dx, float dy) {
        x += dx;
        y += dy;
    }
    
    float getX() const { return x; }
    float getY() const { return y; }
    float getSpeed() const { return speed; }
};

// Stub timer for testing time-based mechanics
class TimerStub {
public:
    int currentTime = 0;
    int deltaTime = 16; // 16ms = ~60fps
    
    void advance(int frames = 1) {
        currentTime += deltaTime * frames;
    }
    
    int getTime() const { return currentTime; }
    int getDelta() const { return deltaTime; }
    
    void reset() { currentTime = 0; }
    void setDelta(int dt) { deltaTime = dt; }
};

// Fake SDL_Rect factory for testing
class RectFactory {
public:
    static SDL_Rect create(int x, int y, int w, int h) {
        return SDL_Rect{x, y, w, h};
    }
    
    static SDL_Rect createSubmarine(int x = 100, int y = 200) {
        return SDL_Rect{x, y, 64, 64};
    }
    
    static SDL_Rect createLitter(int x = 300, int y = 150) {
        return SDL_Rect{x, y, 32, 32};
    }
    
    static SDL_Rect createEnemy(int x = 500, int y = 250) {
        return SDL_Rect{x, y, 48, 48};
    }
};
