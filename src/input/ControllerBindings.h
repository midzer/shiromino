#pragma once
#include "types.h"
#include "PDINI.h"
#include "SDL.h"
#include <string>
namespace Shiro {
    // TODO: Rename this to JoystickBindings, then create a new GameControllerBindings, which would provide support for the "game controller" SDL2 API (XInput style controllers).
    class ControllerBindings {
    public:
        static constexpr i16 MAXIMUM_DEAD_ZONE = std::numeric_limits<i16>::max();
        ControllerBindings();
        void read(PDINI::INI& ini);
        std::string name;
        int controllerIndex;
        SDL_JoystickID controllerID;
        struct Buttons {
            Buttons();
            int left;
            int right;
            int up;
            int down;
            int start;
            int a;
            int b;
            int c;
            int d;
            int escape;
        } buttons;
        struct Axes {
            Axes();
            int x;
            int right;
            int y;
            int down;
            // TODO: Axis bindings for non-directional inputs.
        } axes;
        int hatIndex;
        float deadZone;
    };
};