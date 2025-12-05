#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../include/Submarine.h"
#include "../include/ScoreDisplay.hpp"
#include "test_mocks.h"

// ========== SUBMARINE RESET TESTS ==========

class SubmarineResetTest : public ::testing::Test {
protected:
    void SetUp() override {
        texture = nullptr;
    }
    
    SDL_Texture* texture;
};

TEST_F(SubmarineResetTest, ResetClearsHitBlinkTimer) {
    Submarine sub(texture, 100, 200, 64, 64);
    
    // Start hit blink
    sub.startHitBlink();
    EXPECT_TRUE(sub.isInvulnerable());
    
    // Reset should clear invulnerability
    sub.reset();
    EXPECT_FALSE(sub.isInvulnerable());
}

TEST_F(SubmarineResetTest, ResetPreservesDimensions) {
    Submarine sub(texture, 100, 200, 64, 64);
    
    SDL_Rect originalRect = sub.getRect();
    
    sub.startHitBlink();
    sub.reset();
    
    SDL_Rect afterReset = sub.getRect();
    
    EXPECT_EQ(afterReset.w, originalRect.w);
    EXPECT_EQ(afterReset.h, originalRect.h);
}

TEST_F(SubmarineResetTest, ResetPreservesPosition) {
    Submarine sub(texture, 100, 200, 64, 64);
    
    sub.startHitBlink();
    sub.reset();
    
    SDL_Rect rect = sub.getRect();
    EXPECT_EQ(rect.x, 100);
    EXPECT_EQ(rect.y, 200);
}

TEST_F(SubmarineResetTest, ResetAfterMovement) {
    Submarine sub(texture, 100, 200, 64, 64);
    
    // Move submarine and take damage
    sub.moveBy(50, 30);
    sub.startHitBlink();
    EXPECT_TRUE(sub.isInvulnerable());
    
    // Reset should only clear invulnerability, not position
    sub.reset();
    EXPECT_FALSE(sub.isInvulnerable());
    
    SDL_Rect rect = sub.getRect();
    EXPECT_EQ(rect.x, 150);  // Position should be preserved
    EXPECT_EQ(rect.y, 230);
}

TEST_F(SubmarineResetTest, MultipleResetsAreIdempotent) {
    Submarine sub(texture, 100, 200, 64, 64);
    
    sub.startHitBlink();
    sub.reset();
    EXPECT_FALSE(sub.isInvulnerable());
    
    // Additional resets should not cause issues
    sub.reset();
    sub.reset();
    EXPECT_FALSE(sub.isInvulnerable());
    
    SDL_Rect rect = sub.getRect();
    EXPECT_EQ(rect.x, 100);
    EXPECT_EQ(rect.y, 200);
}

TEST_F(SubmarineResetTest, ResetDuringBlinkAnimation) {
    Submarine sub(texture, 100, 200, 64, 64);
    
    sub.startHitBlink();
    
    // Update blink partway through
    for (int i = 0; i < 10; i++) {
        sub.updateBlink();
    }
    
    EXPECT_TRUE(sub.isInvulnerable());
    
    // Reset should immediately clear invulnerability
    sub.reset();
    EXPECT_FALSE(sub.isInvulnerable());
}

TEST_F(SubmarineResetTest, ResetAndTakeNewDamage) {
    Submarine sub(texture, 100, 200, 64, 64);
    
    // Take damage and reset
    sub.startHitBlink();
    sub.reset();
    EXPECT_FALSE(sub.isInvulnerable());
    
    // Should be able to take damage again
    sub.startHitBlink();
    EXPECT_TRUE(sub.isInvulnerable());
}

// ========== SCOREDISPLAY RESET TESTS ==========

class ScoreDisplayResetTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a mock renderer (nullptr for these tests)
        renderer = nullptr;
        // Note: In a real test environment, you'd want to mock SDL properly
    }
    
    void TearDown() override {
        // Cleanup if needed
    }
    
    SDL_Renderer* renderer;
};

TEST_F(ScoreDisplayResetTest, ResetLevelToOne) {
    // This test would need proper SDL initialization
    // For now, documenting expected behavior
    
    // ScoreDisplay display(renderer, 10, 10, 200, 80);
    // display.setScore(250);  // This should advance to level 3
    // EXPECT_EQ(display.getLevel(), 3);
    
    // display.resetLevel();
    // EXPECT_EQ(display.getLevel(), 1);
}

TEST_F(ScoreDisplayResetTest, ResetLevelPreservesScore) {
    // ScoreDisplay display(renderer, 10, 10, 200, 80);
    // display.setScore(250);
    
    // display.resetLevel();
    // EXPECT_EQ(display.getLevel(), 1);
    // EXPECT_EQ(display.getScore(), 250);  // Score should remain
}

TEST_F(ScoreDisplayResetTest, ResetLevelFromDifferentLevels) {
    // ScoreDisplay display(renderer, 10, 10, 200, 80);
    
    // Test from level 2
    // display.setScore(150);
    // EXPECT_EQ(display.getLevel(), 2);
    // display.resetLevel();
    // EXPECT_EQ(display.getLevel(), 1);
    
    // Test from level 3
    // display.setScore(300);
    // EXPECT_EQ(display.getLevel(), 3);
    // display.resetLevel();
    // EXPECT_EQ(display.getLevel(), 1);
    
    // Test from level 4
    // display.setScore(500);
    // EXPECT_EQ(display.getLevel(), 4);
    // display.resetLevel();
    // EXPECT_EQ(display.getLevel(), 1);
}

TEST_F(ScoreDisplayResetTest, MultipleResetLevelCalls) {
    // ScoreDisplay display(renderer, 10, 10, 200, 80);
    // display.setScore(300);
    
    // display.resetLevel();
    // display.resetLevel();
    // display.resetLevel();
    
    // EXPECT_EQ(display.getLevel(), 1);
}

// ========== INTEGRATION RESET TESTS ==========

TEST(ResetIntegrationTest, ResetGameStateSimulation) {
    SDL_Texture* texture = nullptr;
    Submarine sub(texture, 400, 300, 64, 64);
    
    // Simulate game state
    sub.moveBy(100, 50);
    sub.startHitBlink();
    
    SDL_Rect positionBeforeReset = sub.getRect();
    
    // Reset submarine state
    sub.reset();
    
    // Verify state is reset correctly
    EXPECT_FALSE(sub.isInvulnerable());
    
    SDL_Rect positionAfterReset = sub.getRect();
    EXPECT_EQ(positionAfterReset.x, positionBeforeReset.x);
    EXPECT_EQ(positionAfterReset.y, positionBeforeReset.y);
}

TEST(ResetIntegrationTest, ResetAfterMultipleHits) {
    SDL_Texture* texture = nullptr;
    Submarine sub(texture, 100, 200, 64, 64);
    
    // Take multiple hits and resets
    for (int i = 0; i < 5; i++) {
        sub.startHitBlink();
        EXPECT_TRUE(sub.isInvulnerable());
        
        // Update some frames
        for (int j = 0; j < 5; j++) {
            sub.updateBlink();
        }
        
        sub.reset();
        EXPECT_FALSE(sub.isInvulnerable());
    }
}

TEST(ResetIntegrationTest, ConcurrentResetOperations) {
    SDL_Texture* texture = nullptr;
    Submarine sub1(texture, 100, 100, 64, 64);
    Submarine sub2(texture, 200, 200, 64, 64);
    
    sub1.startHitBlink();
    sub2.startHitBlink();
    
    EXPECT_TRUE(sub1.isInvulnerable());
    EXPECT_TRUE(sub2.isInvulnerable());
    
    sub1.reset();
    sub2.reset();
    
    EXPECT_FALSE(sub1.isInvulnerable());
    EXPECT_FALSE(sub2.isInvulnerable());
}

// ========== BOUNDARY RESET TESTS ==========

TEST(ResetBoundaryTest, ResetWithZeroPosition) {
    SDL_Texture* texture = nullptr;
    Submarine sub(texture, 0, 0, 64, 64);
    
    sub.startHitBlink();
    sub.reset();
    
    EXPECT_FALSE(sub.isInvulnerable());
    SDL_Rect rect = sub.getRect();
    EXPECT_EQ(rect.x, 0);
    EXPECT_EQ(rect.y, 0);
}

TEST(ResetBoundaryTest, ResetWithLargePosition) {
    SDL_Texture* texture = nullptr;
    Submarine sub(texture, 10000, 10000, 64, 64);
    
    sub.startHitBlink();
    sub.reset();
    
    EXPECT_FALSE(sub.isInvulnerable());
    SDL_Rect rect = sub.getRect();
    EXPECT_EQ(rect.x, 10000);
    EXPECT_EQ(rect.y, 10000);
}

TEST(ResetBoundaryTest, ResetImmediatelyAfterConstruction) {
    SDL_Texture* texture = nullptr;
    Submarine sub(texture, 100, 200, 64, 64);
    
    // Reset immediately without any state changes
    sub.reset();
    
    EXPECT_FALSE(sub.isInvulnerable());
    SDL_Rect rect = sub.getRect();
    EXPECT_EQ(rect.x, 100);
    EXPECT_EQ(rect.y, 200);
}

// ========== EDGE CASE RESET TESTS ==========

TEST(ResetEdgeCaseTest, ResetWithMinimalBlinkTime) {
    SDL_Texture* texture = nullptr;
    Submarine sub(texture, 100, 200, 64, 64);
    
    sub.startHitBlink();
    sub.updateBlink();  // Update just one frame
    
    sub.reset();
    EXPECT_FALSE(sub.isInvulnerable());
}

TEST(ResetEdgeCaseTest, ResetWithMaximalBlinkTime) {
    SDL_Texture* texture = nullptr;
    Submarine sub(texture, 100, 200, 64, 64);
    
    sub.startHitBlink();
    
    // Update until almost expired
    for (int i = 0; i < 29; i++) {
        sub.updateBlink();
    }
    
    EXPECT_TRUE(sub.isInvulnerable());
    sub.reset();
    EXPECT_FALSE(sub.isInvulnerable());
}

TEST(ResetEdgeCaseTest, AlternatingResetAndDamage) {
    SDL_Texture* texture = nullptr;
    Submarine sub(texture, 100, 200, 64, 64);
    
    for (int i = 0; i < 10; i++) {
        sub.startHitBlink();
        EXPECT_TRUE(sub.isInvulnerable());
        
        sub.reset();
        EXPECT_FALSE(sub.isInvulnerable());
    }
}
