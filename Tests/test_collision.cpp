#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <SDL.h>
#include "test_mocks.h"

using ::testing::Return;
using ::testing::_;

// Helper function for collision detection testing
bool checkCollision(SDL_Rect a, SDL_Rect b) {
    int leftA = a.x;
    int rightA = a.x + a.w;
    int topA = a.y;
    int bottomA = a.y + a.h;

    int leftB = b.x;
    int rightB = b.x + b.w;
    int topB = b.y;
    int bottomB = b.y + b.h;

    if (bottomA <= topB) return false;
    if (topA >= bottomB) return false;
    if (rightA <= leftB) return false;
    if (leftA >= rightB) return false;

    return true;
}

class CollisionTest : public ::testing::Test {
protected:
    SDL_Rect submarine;
    SDL_Rect litter;
    SDL_Rect enemy;

    void SetUp() override {
        submarine = {100, 200, 64, 64};
        litter = {150, 220, 32, 32};
        enemy = {300, 250, 48, 48};
    }
};

// ASSERTION TESTS 

TEST_F(CollisionTest, SubmarineLitterCollision) {
    // Overlapping rectangles
    SDL_Rect overlappingLitter = {120, 220, 32, 32};
    EXPECT_TRUE(checkCollision(submarine, overlappingLitter));
}

TEST_F(CollisionTest, NoCollisionWhenSeparate) {
    // Non-overlapping rectangles
    SDL_Rect farLitter = {500, 400, 32, 32};
    EXPECT_FALSE(checkCollision(submarine, farLitter));
}

TEST_F(CollisionTest, EdgeCollision) {
    // Test edge cases - just touching
    SDL_Rect edgeLitter = {164, 200, 32, 32};  // Right edge of submarine
    EXPECT_FALSE(checkCollision(submarine, edgeLitter));  // SDL collision is exclusive
}

TEST_F(CollisionTest, CompleteOverlap) {
    // One rectangle completely inside another
    SDL_Rect smallLitter = {120, 220, 10, 10};
    EXPECT_TRUE(checkCollision(submarine, smallLitter));
}

TEST_F(CollisionTest, SubmarineEnemyCollision) {
    // Test submarine-enemy collision
    SDL_Rect closeEnemy = {110, 210, 48, 48};
    EXPECT_TRUE(checkCollision(submarine, closeEnemy));
}

TEST_F(CollisionTest, MultipleCollisions) {
    // Test multiple simultaneous collisions
    SDL_Rect litter1 = {120, 220, 32, 32};
    SDL_Rect litter2 = {130, 230, 32, 32};
    
    EXPECT_TRUE(checkCollision(submarine, litter1));
    EXPECT_TRUE(checkCollision(submarine, litter2));
}

TEST_F(CollisionTest, VerticalAlignment) {
    // Objects aligned vertically but not horizontally
    SDL_Rect verticalLitter = {120, 500, 32, 32};
    EXPECT_FALSE(checkCollision(submarine, verticalLitter));
}

TEST_F(CollisionTest, HorizontalAlignment) {
    // Objects aligned horizontally but not vertically
    SDL_Rect horizontalLitter = {500, 220, 32, 32};
    EXPECT_FALSE(checkCollision(submarine, horizontalLitter));
}

// STUB TESTS

TEST(CollisionStubTest, StubAlwaysReturnsTrue) {
    CollisionStub stub;
    stub.setCollisionResult(true);
    
    SDL_Rect rect1 = {0, 0, 10, 10};
    SDL_Rect rect2 = {1000, 1000, 10, 10}; // Far apart
    
    // Stub overrides actual collision logic
    EXPECT_TRUE(stub.checkCollision(rect1, rect2));
}

TEST(CollisionStubTest, StubAlwaysReturnsFalse) {
    CollisionStub stub;
    stub.setCollisionResult(false);
    
    SDL_Rect rect1 = {0, 0, 10, 10};
    SDL_Rect rect2 = {5, 5, 10, 10}; // Overlapping
    
    // Stub overrides actual collision logic
    EXPECT_FALSE(stub.checkCollision(rect1, rect2));
}

TEST(CollisionStubTest, ToggleCollisionBehavior) {
    CollisionStub stub;
    
    SDL_Rect rect1 = {0, 0, 10, 10};
    SDL_Rect rect2 = {5, 5, 10, 10};
    
    stub.setCollisionResult(true);
    EXPECT_TRUE(stub.checkCollision(rect1, rect2));
    
    stub.setCollisionResult(false);
    EXPECT_FALSE(stub.checkCollision(rect1, rect2));
}

// FACTORY TESTS 

TEST(RectFactoryTest, CreateSubmarineRect) {
    SDL_Rect sub = RectFactory::createSubmarine(150, 250);
    
    EXPECT_EQ(sub.x, 150);
    EXPECT_EQ(sub.y, 250);
    EXPECT_EQ(sub.w, 64);
    EXPECT_EQ(sub.h, 64);
}

TEST(RectFactoryTest, CreateLitterRect) {
    SDL_Rect lit = RectFactory::createLitter(200, 100);
    
    EXPECT_EQ(lit.x, 200);
    EXPECT_EQ(lit.y, 100);
    EXPECT_EQ(lit.w, 32);
    EXPECT_EQ(lit.h, 32);
}

TEST(RectFactoryTest, DefaultPositions) {
    SDL_Rect sub = RectFactory::createSubmarine();
    SDL_Rect lit = RectFactory::createLitter();
    
    EXPECT_EQ(sub.x, 100);
    EXPECT_EQ(lit.x, 300);
}
