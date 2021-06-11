#pragma once
#include "types.h"
namespace Shiro {
    struct KeyFlags {
        bool operator==(const KeyFlags& cmp) const;
        bool operator!=(const KeyFlags& cmp) const;

        u8 left;
        u8 right;
        u8 up;
        u8 down;
        u8 a;
        u8 b;
        u8 c;
        u8 d;
        u8 start;
        u8 escape;
    };
}