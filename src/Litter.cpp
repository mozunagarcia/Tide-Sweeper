#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>

class Litter {
    private:

    public:

    //litterRect can be renamed based on what we decide to name the litter objects
    bool checkCollision(const SDL_Rect& submarineRect, const SDL_Rect& litterRect) {
        return SDL_HasIntersection(&submarineRect, &litterRect);
    }

    

};