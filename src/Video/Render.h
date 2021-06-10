#pragma once
#include "SDL.h"
#include "Video/Screen.h"

using namespace Shiro;

namespace Shiro {
    namespace Render {
        enum class Alignment {
            topLeft,
            bottomLeft,
            topRight,
            bottomRight
        };
    }

    int RenderCopy(const Screen& screen, SDL_Texture *tex, const SDL_Rect *srcrect, SDL_Rect *dstrect);
    int RenderCopyOuter(const Screen& screen, SDL_Texture *tex, const SDL_Rect *srcrect, SDL_Rect *dstrect, Render::Alignment alignment);
    int RenderFillRect(const Screen& screen, SDL_Rect *rect);
    int RenderFillRectOuter(const Screen& screen, SDL_Rect *rect, Render::Alignment alignment);
}