#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../include/Submarine.h"
#include "test_mocks.h"

using ::testing::Return;
using ::testing::_;

class SubmarineTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Mock texture (nullptr for unit tests)
        texture = nullptr;
    }

    SDL_Texture* texture;
};

//  ASSERTION TESTS 

TEST_F(SubmarineTest, InitialPosition) {
    Submarine sub(texture, 100, 200, 64, 64);
    
    SDL_Rect rect = sub.getRect();
    EXPECT_EQ(rect.x, 100);
    EXPECT_EQ(rect.y, 200);
    EXPECT_EQ(rect.w, 64);
    EXPECT_EQ(rect.h, 64);
}

TEST_F(SubmarineTest, SetPosition) {
    Submarine sub(texture, 100, 200, 64, 64);
    
    sub.setPosition(150, 250);
    SDL_Rect rect = sub.getRect();
    
    EXPECT_EQ(rect.x, 150);
    EXPECT_EQ(rect.y, 250);
}

TEST_F(SubmarineTest, MoveBy) {
    Submarine sub(texture, 100, 200, 64, 64);
    
    sub.moveBy(50, -30);
    SDL_Rect rect = sub.getRect();
    
    EXPECT_EQ(rect.x, 150);
    EXPECT_EQ(rect.y, 170);
}

TEST_F(SubmarineTest, Clamping) {
    Submarine sub(texture, 100, 200, 64, 64);
    
    // Move outside bounds
    sub.setPosition(-50, -50);
    sub.clamp(0, 800, 0, 600);
    
    SDL_Rect rect = sub.getRect();
    EXPECT_GE(rect.x, 0);
    EXPECT_GE(rect.y, 0);
}

TEST_F(SubmarineTest, InvulnerabilityAfterHit) {
    Submarine sub(texture, 100, 200, 64, 64);
    
    EXPECT_FALSE(sub.isInvulnerable());
    
    sub.startHitBlink();
    EXPECT_TRUE(sub.isInvulnerable());
}

TEST_F(SubmarineTest, BlinkUpdate) {
    Submarine sub(texture, 100, 200, 64, 64);
    
    sub.startHitBlink();
    EXPECT_TRUE(sub.isInvulnerable());
    
    // Update blink many times to expire invulnerability
    for (int i = 0; i < 200; i++) {
        sub.updateBlink();
    }
    
    EXPECT_FALSE(sub.isInvulnerable());
}

//  STUB TESTS 

TEST(SubmarineStubTest, PositionStubTracksMovement) {
    PositionStub stub;
    
    stub.setPosition(100.0f, 200.0f);
    EXPECT_FLOAT_EQ(stub.getX(), 100.0f);
    EXPECT_FLOAT_EQ(stub.getY(), 200.0f);
    
    stub.move(50.0f, -30.0f);
    EXPECT_FLOAT_EQ(stub.getX(), 150.0f);
    EXPECT_FLOAT_EQ(stub.getY(), 170.0f);
}

TEST(SubmarineStubTest, PositionStubIncrementalMovement) {
    PositionStub stub;
    stub.setPosition(0.0f, 0.0f);
    
    // Simulate frame-by-frame movement
    for (int i = 0; i < 10; i++) {
        stub.move(stub.getSpeed(), 0.0f);
    }
    
    EXPECT_FLOAT_EQ(stub.getX(), 50.0f); // 10 frames * 5.0 speed
}

TEST(SubmarineStubTest, MockTextureReturnsControlledSize) {
    MockTexture mockTex;
    mockTex.setDimensions(128, 96);
    
    EXPECT_EQ(mockTex.getWidth(), 128);
    EXPECT_EQ(mockTex.getHeight(), 96);
}

//  INTEGRATION WITH STUBS 

TEST(SubmarineIntegrationTest, MovementWithinBounds) {
    PositionStub stub;
    stub.setPosition(100.0f, 100.0f);
    
    // Move right
    stub.move(50.0f, 0.0f);
    EXPECT_LE(stub.getX(), 800.0f); // Should be within screen width
    
    // Move down
    stub.move(0.0f, 50.0f);
    EXPECT_LE(stub.getY(), 600.0f); // Should be within screen height
}

TEST(SubmarineIntegrationTest, TimedInvulnerability) {
    TimerStub timer;
    int invulnerabilityDuration = 180; // 3 seconds at 60fps
    int invulnerabilityTimer = invulnerabilityDuration;
    bool isInvulnerable = true;
    
    // Simulate frames passing
    while (invulnerabilityTimer > 0) {
        timer.advance(1);
        invulnerabilityTimer--;
    }
    
    EXPECT_EQ(invulnerabilityTimer, 0);
    isInvulnerable = (invulnerabilityTimer > 0);
    EXPECT_FALSE(isInvulnerable);
}
