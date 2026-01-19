#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../include/Litter.h"
#include "test_mocks.h"

using ::testing::Return;
using ::testing::_;

class LitterTest : public ::testing::Test {
protected:
    void SetUp() override {
        texture = nullptr;
    }

    SDL_Texture* texture;
};

//  ASSERTION TESTS 

TEST_F(LitterTest, LitterCreation) {
    Litter litter(texture, 100.0f, 150.0f, 5.0f);
    
    EXPECT_FLOAT_EQ(litter.x, 100.0f);
    EXPECT_FLOAT_EQ(litter.y, 150.0f);
    EXPECT_FLOAT_EQ(litter.speed, 5.0f);
    EXPECT_TRUE(litter.active);
}

TEST_F(LitterTest, LitterMovement) {
    Litter litter(texture, 100.0f, 150.0f, 5.0f);
    
    float initialX = litter.x;
    litter.update();
    
    // Litter should move left (decrease X)
    EXPECT_LT(litter.x, initialX);
}

TEST_F(LitterTest, LitterCollection) {
    Litter litter(texture, 100.0f, 150.0f, 5.0f);
    
    EXPECT_TRUE(litter.active);
    EXPECT_EQ(litter.respawnTimer, 0);
    
    litter.collect();
    
    EXPECT_FALSE(litter.active);
    EXPECT_GT(litter.respawnTimer, 0);
}

TEST_F(LitterTest, RespawnTimer) {
    Litter litter(texture, 100.0f, 150.0f, 5.0f);
    
    litter.collect();
    int initialTimer = litter.respawnTimer;
    
    EXPECT_GT(initialTimer, 0);
    
    // Update should decrement respawn timer when inactive
    litter.update();
    EXPECT_LT(litter.respawnTimer, initialTimer);
}

TEST_F(LitterTest, CollisionDetection) {
    Litter litter(texture, 100.0f, 100.0f, 5.0f);
    litter.active = true;
    
    // Overlapping submarine
    SDL_Rect subRect = {90, 90, 64, 64};
    EXPECT_TRUE(litter.checkCollision(subRect));
    
    // Non-overlapping submarine
    SDL_Rect farSubRect = {500, 500, 64, 64};
    EXPECT_FALSE(litter.checkCollision(farSubRect));
}

TEST_F(LitterTest, NoCollisionWhenInactive) {
    Litter litter(texture, 100.0f, 100.0f, 5.0f);
    litter.active = false;
    
    SDL_Rect subRect = {90, 90, 64, 64};
    EXPECT_FALSE(litter.checkCollision(subRect));
}

//  STUB TESTS 

TEST(LitterStubTest, PositionStubSimulatesLitterMovement) {
    PositionStub stub;
    stub.setPosition(800.0f, 300.0f); // Start at right edge
    
    // Simulate litter moving left at 5.0 speed
    for (int frame = 0; frame < 60; frame++) { // 1 second
        stub.move(-5.0f, 0.0f);
    }
    
    EXPECT_FLOAT_EQ(stub.getX(), 500.0f); // 800 - (60 * 5)
}

TEST(LitterStubTest, TimerStubSimulatesRespawn) {
    TimerStub timer;
    int respawnTime = 180; // 3 seconds at 60fps
    
    // Simulate respawn countdown
    for (int i = 0; i < respawnTime; i++) {
        timer.advance(1);
    }
    
    EXPECT_EQ(timer.getTime(), respawnTime * 16); // 180 frames * 16ms
}

TEST(LitterStubTest, CollisionStubOverridesDetection) {
    CollisionStub stub;
    
    // Force collision to always be true
    stub.setCollisionResult(true);
    
    SDL_Rect rect1 = {0, 0, 10, 10};
    SDL_Rect rect2 = {1000, 1000, 10, 10}; // Far apart
    
    EXPECT_TRUE(stub.checkCollision(rect1, rect2));
}

//  MOCK TESTS 

TEST(LitterMockTest, Level4SpawnRateSimulation) {
    TimerStub timer;
    int spawnInterval = 10; // Level 4 spawns every 10 frames
    int spawnCount = 0;
    int frameCount = 0;
    
    // Simulate 1 second of gameplay
    for (int i = 0; i < 60; i++) {
        frameCount++;
        if (frameCount % spawnInterval == 0) {
            spawnCount++;
        }
        timer.advance(1);
    }
    
    EXPECT_EQ(spawnCount, 6); // Should spawn 6 times in 60 frames (every 10 frames)
}

TEST(LitterMockTest, MultipleSpeedVariations) {
    PositionStub slow;
    PositionStub medium;
    PositionStub fast;
    
    slow.speed = 3.0f;
    medium.speed = 5.0f;
    fast.speed = 13.0f; // Level 4 speed
    
    slow.setPosition(800.0f, 100.0f);
    medium.setPosition(800.0f, 200.0f);
    fast.setPosition(800.0f, 300.0f);
    
    // Move for 10 frames
    for (int i = 0; i < 10; i++) {
        slow.move(-slow.getSpeed(), 0.0f);
        medium.move(-medium.getSpeed(), 0.0f);
        fast.move(-fast.getSpeed(), 0.0f);
    }
    
    EXPECT_FLOAT_EQ(slow.getX(), 770.0f);    // 800 - 30
    EXPECT_FLOAT_EQ(medium.getX(), 750.0f);  // 800 - 50
    EXPECT_FLOAT_EQ(fast.getX(), 670.0f);    // 800 - 130
}

//  INTEGRATION TESTS WITH STUBS 

TEST(LitterIntegrationTest, OffScreenDetectionStub) {
    PositionStub stub;
    stub.setPosition(800.0f, 300.0f);
    
    // Move left until off-screen
    while (stub.getX() > -50.0f) {
        stub.move(-5.0f, 0.0f);
    }
    
    EXPECT_LT(stub.getX(), 0.0f); // Should be off left side of screen
}

TEST(LitterIntegrationTest, RespawnCycleSimulation) {
    bool active = true;
    int respawnTimer = 0;
    TimerStub timer;
    int respawnDuration = 120; // 2 seconds
    
    // Collect litter
    active = false;
    respawnTimer = respawnDuration;
    
    // Countdown
    while (respawnTimer > 0) {
        timer.advance(1);
        respawnTimer--;
    }
    
    // Should reactivate
    active = true;
    EXPECT_TRUE(active);
    EXPECT_EQ(respawnTimer, 0);
}
