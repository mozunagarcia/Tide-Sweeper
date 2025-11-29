#include <gtest/gtest.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "../include/Submarine.h"
#include "../include/Litter.h"
#include "../include/ScoreDisplay.hpp"

// Test fixture that initializes SDL for rendering tests
class RenderingTest : public ::testing::Test {
protected:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    
    void SetUp() override {
        // Initialize SDL
        ASSERT_EQ(SDL_Init(SDL_INIT_VIDEO), 0) << "SDL_Init failed: " << SDL_GetError();
        
        // Initialize SDL_image
        int imgFlags = IMG_INIT_PNG;
        ASSERT_TRUE(IMG_Init(imgFlags) & imgFlags) << "SDL_image init failed: " << IMG_GetError();
        
        // Initialize SDL_ttf
        ASSERT_EQ(TTF_Init(), 0) << "SDL_ttf init failed: " << TTF_GetError();
        
        // Create a window (hidden for testing)
        window = SDL_CreateWindow(
            "Test Window",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            800, 600,
            SDL_WINDOW_HIDDEN
        );
        ASSERT_NE(window, nullptr) << "Window creation failed: " << SDL_GetError();
        
        // Create a renderer
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
        ASSERT_NE(renderer, nullptr) << "Renderer creation failed: " << SDL_GetError();
    }
    
    void TearDown() override {
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
    }
};

TEST_F(RenderingTest, RendererInitialization) {
    // Test that renderer is valid
    EXPECT_NE(renderer, nullptr);
    
    // Test that we can set render draw color
    EXPECT_EQ(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255), 0);
    
    // Test that we can clear the renderer
    EXPECT_EQ(SDL_RenderClear(renderer), 0);
}

TEST_F(RenderingTest, SubmarineRendering) {
    // Create a simple texture for testing (1x1 pixel)
    SDL_Surface* surface = SDL_CreateRGBSurface(0, 1, 1, 32, 0, 0, 0, 0);
    ASSERT_NE(surface, nullptr);
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    ASSERT_NE(texture, nullptr);
    
    // Create submarine
    Submarine sub(texture, 100, 200, 64, 64);
    
    // Test that rendering doesn't crash
    EXPECT_NO_THROW(sub.render(renderer));
    
    SDL_DestroyTexture(texture);
}

TEST_F(RenderingTest, LitterRendering) {
    // Create a simple texture for testing
    SDL_Surface* surface = SDL_CreateRGBSurface(0, 1, 1, 32, 0, 0, 0, 0);
    ASSERT_NE(surface, nullptr);
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    ASSERT_NE(texture, nullptr);
    
    // Create litter
    Litter litter(texture, 300.0f, 200.0f, 5.0f);
    litter.active = true;
    
    // Test that rendering doesn't crash
    EXPECT_NO_THROW(litter.render(renderer));
    
    SDL_DestroyTexture(texture);
}

TEST_F(RenderingTest, ScoreDisplayRendering) {
    // Create ScoreDisplay
    ScoreDisplay scoreDisplay(renderer, 10, 10, 200, 50);
    
    // Set a score
    scoreDisplay.setScore(100);
    EXPECT_EQ(scoreDisplay.getScore(), 100);
    
    // Test that rendering doesn't crash
    EXPECT_NO_THROW(scoreDisplay.render());
}

TEST_F(RenderingTest, RenderClearAndPresent) {
    // Test full render cycle
    EXPECT_EQ(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255), 0);
    EXPECT_EQ(SDL_RenderClear(renderer), 0);
    
    // Draw a simple rectangle
    SDL_Rect rect = {100, 100, 50, 50};
    EXPECT_EQ(SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255), 0);
    EXPECT_EQ(SDL_RenderFillRect(renderer, &rect), 0);
    
    // Present shouldn't crash
    EXPECT_NO_THROW(SDL_RenderPresent(renderer));
}

TEST_F(RenderingTest, TextureCreation) {
    // Test texture creation from surface
    SDL_Surface* surface = SDL_CreateRGBSurface(0, 64, 64, 32, 0, 0, 0, 0);
    ASSERT_NE(surface, nullptr);
    
    // Fill surface with color
    SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 255, 0, 0));
    
    // Create texture
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    ASSERT_NE(texture, nullptr);
    
    // Test texture properties
    int w, h;
    EXPECT_EQ(SDL_QueryTexture(texture, nullptr, nullptr, &w, &h), 0);
    EXPECT_EQ(w, 64);
    EXPECT_EQ(h, 64);
    
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

TEST_F(RenderingTest, MultipleObjectsRendering) {
    // Create textures
    SDL_Surface* surface = SDL_CreateRGBSurface(0, 32, 32, 32, 0, 0, 0, 0);
    ASSERT_NE(surface, nullptr);
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    ASSERT_NE(texture, nullptr);
    
    // Create multiple objects
    Submarine sub(texture, 50, 300, 64, 64);
    Litter litter1(texture, 200.0f, 150.0f, 5.0f);
    Litter litter2(texture, 400.0f, 250.0f, 5.0f);
    litter1.active = true;
    litter2.active = true;
    
    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Render all objects - should not crash
    EXPECT_NO_THROW({
        sub.render(renderer);
        litter1.render(renderer);
        litter2.render(renderer);
        SDL_RenderPresent(renderer);
    });
    
    SDL_DestroyTexture(texture);
}

TEST_F(RenderingTest, RenderWithBlendMode) {
    // Test rendering with different blend modes
    SDL_Surface* surface = SDL_CreateRGBSurface(0, 32, 32, 32, 0, 0, 0, 0);
    ASSERT_NE(surface, nullptr);
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    ASSERT_NE(texture, nullptr);
    
    // Test different blend modes
    EXPECT_EQ(SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE), 0);
    EXPECT_EQ(SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND), 0);
    EXPECT_EQ(SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_ADD), 0);
    
    SDL_DestroyTexture(texture);
}
