#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>

// Collision function
bool checkCollision(const SDL_Rect& submarineRect, const SDL_Rect& litterRect) {
    return SDL_HasIntersection(&submarineRect, &litterRect);
}

int main() {
    SDL_Rect sub = {100, 100, 50, 50};     // submarine position (x=100, y=100)
    SDL_Rect litter = {100, 100, 50, 50};  // litter overlapping sub to ensure collision detection
    SDL_Rect farLitter = {300, 300, 30, 30}; // litter far away from sub

    std::cout << "Test 1 (overlap): "
              << (checkCollision(sub, litter) ? "PASS" : "FAIL")
              << std::endl;

    std::cout << "Test 2 (no overlap): "    // Makes sure no collision is detected with far away litter 
              << (!checkCollision(sub, farLitter) ? "PASS" : "FAIL")
              << std::endl;

    SDL_Rect edgeTouch = {150, 100, 20, 20}; // touches submarine edge (should fail) might change this 
    std::cout << "Test 3 (edge touch): "     // later if it looks ugly in-game
              << (checkCollision(sub, edgeTouch) ? "PASS" : "FAIL")
              << std::endl;

    return 0;
}


// This is if we wat to include edge touching as collision
// SDL_HasIntersection does not count edge touching as collision because it only uses < and > comparisons

/*
bool checkCollision2(const SDL_Rect& submarineRect, const SDL_Rect& litterRect) {
    return (a.x <= b.x + b.w &&  // left edge <= right edge of b
            a.x + a.w >= b.x &&  // right edge >= left edge of b
            a.y <= b.y + b.h &&  // top edge <= bottom edge of b
            a.y + a.h >= b.y);   // bottom edge >= top edge of b
}
*/