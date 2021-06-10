/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
// TODO: Refactor the asset system to be more portable and implement Gfx code to have the ability to change the backend of Graphic subclasses.
#include "Video/Animation.h"
#include "Video/Render.h"
#include "Asset/Image.h"

using namespace Shiro;
using namespace std;

namespace Shiro {
    struct AnimationGraphic : public Graphic {
        AnimationGraphic() = delete;

        AnimationGraphic(
            SDL_Texture* const frame,
            const int x,
            const int y,
            const Uint32 rgbaMod
        );

        void draw(const Screen& screen) const;

        SDL_Texture* frame;
        const int x;
        const int y;
        const Uint32 rgbaMod;
    };

    struct AnimationEntity::Impl {
        Impl() = delete;

        Impl(
            AssetManager& mgr,
            const filesystem::path& frames,
            const size_t layerNum,
            const size_t numFrames,
            const size_t frameMultiplier,
            const shared_ptr<AnimationGraphic> graphic
        );

        AssetManager& mgr;
        const filesystem::path frames;
        const size_t layerNum;
        size_t counter;
        const size_t numFrames;
        const size_t frameMultiplier;
        const shared_ptr<AnimationGraphic> graphic;
    };
}

AnimationGraphic::AnimationGraphic(
    SDL_Texture* const frame,
    const int x,
    const int y,
    const Uint32 rgbaMod
) :
    frame(frame),
    x(x),
    y(y),
    rgbaMod(rgbaMod) {}

void AnimationGraphic::draw(const Screen& screen) const {
    SDL_Rect dest;
    dest.x = x;
    dest.y = y;
    SDL_QueryTexture(frame, NULL, NULL, &dest.w, &dest.h);

    SDL_SetTextureColorMod(frame, R(rgbaMod), G(rgbaMod), B(rgbaMod));
    SDL_SetTextureAlphaMod(frame, A(rgbaMod));
    Shiro::RenderCopy(screen, frame, NULL, &dest);
    SDL_SetTextureAlphaMod(frame, 255);
    SDL_SetTextureColorMod(frame, 255, 255, 255);
}

AnimationEntity::Impl::Impl(
    AssetManager& mgr,
    const filesystem::path& frames,
    const size_t layerNum,
    const size_t numFrames,
    const size_t frameMultiplier,
    const shared_ptr<AnimationGraphic> graphic
) :
    mgr(mgr),
    frames(frames),
    layerNum(layerNum),
    counter(0u),
    numFrames(numFrames),
    frameMultiplier(frameMultiplier),
    graphic(graphic) {}

AnimationEntity::AnimationEntity(
    AssetManager& mgr,
    const filesystem::path& frames,
    const size_t layerNum,
    const int x,
    const int y,
    const size_t numFrames,
    const size_t frameMultiplier,
    const Uint32 rgbaMod
) :
    implPtr(make_shared<AnimationEntity::Impl>(
        mgr,
        frames,
        layerNum,
        numFrames,
        frameMultiplier,
        make_shared<AnimationGraphic>(
            nullptr,
            x,
            y,
            rgbaMod
        )
    )) {}

bool AnimationEntity::update(Layers& layers) {
    implPtr->graphic->frame = ImageAsset::get(implPtr->mgr, implPtr->frames, implPtr->counter / implPtr->frameMultiplier).getTexture();
    layers.push(implPtr->layerNum, implPtr->graphic);
    if (++implPtr->counter < implPtr->frameMultiplier * implPtr->numFrames) {
        return true;
    }
    else {
        return false;
    }
}
