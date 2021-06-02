#include "CoreState.h"
#include "Input/KeyFlags.h"
#include "game_qs.h"
#include "gfx_old.h"
#include "gfx_structures.h"
#include "Grid.h"
#include "PieceDefinition.h"
#include "QRS0.h"
#include "stringtools.h"
#include "Timer.h"
#include <cmath>
#include <filesystem>
#include <iostream>
#include "SDL.h"
#include "SDL_image.h"
#include <string>
#include <vector>
/*
int gfx_piece_colors[25] =
{
    0xD00000,
    0x0000FF,
    0xFF6000,
    0x101010,
    0xBB3CBB,
    0x00CD00,
    0x70ECEE,   // ice
    0xFF658B,
    0xD22D04,
    0x1200BF,   // dark blue
    0xC70694,
    0x1200BF,   // dark blue
    0xEFEF00,
    0xC70694,
    0xD22D04,
    0x70ECEE,   // ice
    0xFF658B,
    0x00A3A3,   // teal
    0xD00000,
    0x00A3A3,   // teal
    0x0000FF,
    0xFF6000,
    0xEFEF00,
    0xBB3CBB,
    0x00CD00,
    0x808080
};
*/

bool img_load(gfx_image *img, std::filesystem::path&& pathWithoutExtension, CoreState *cs) {
    img->tex = NULL;

    SDL_Surface *s = NULL;

    std::filesystem::path& imagePath = pathWithoutExtension.concat(".png");
    s = IMG_Load(imagePath.string().c_str());

    if(!s) {
        s = IMG_Load(imagePath.replace_extension(".jpg").string().c_str());
    }

    if(s) {
        img->tex = SDL_CreateTextureFromSurface(cs->screen.renderer, s);
        SDL_FreeSurface(s);
    }

    return img->tex != NULL;
}

void img_destroy(gfx_image *img) {
    if (img->tex) {
        SDL_DestroyTexture(img->tex);
        img->tex = nullptr;
    }
}

png_monofont *monofont_tiny = NULL;
png_monofont *monofont_small = NULL;
png_monofont *monofont_thin = NULL;
png_monofont *monofont_square = NULL;
png_monofont *monofont_fixedsys = NULL;

struct text_formatting text_fmt_create(unsigned int flags, Uint32 rgba, Uint32 outline_rgba)
{
    struct text_formatting fmt;

    fmt.rgba = rgba;
    fmt.outline_rgba = outline_rgba;

    fmt.outlined = !(flags & DRAWTEXT_NO_OUTLINE);
    fmt.shadow = flags & DRAWTEXT_SHADOW;

    fmt.size_multiplier = 1.0;
    fmt.line_spacing = 1.0;
    fmt.align = ALIGN_LEFT;
    fmt.wrap_length = 0;

    if(flags & DRAWTEXT_CENTERED)
        fmt.align = ALIGN_CENTER;
    if(flags & DRAWTEXT_ALIGN_RIGHT)
        fmt.align = ALIGN_RIGHT;

    return fmt;
}

int gfx_init(CoreState *cs)
{
    monofont_tiny = (png_monofont *)malloc(sizeof(png_monofont));
    monofont_small = (png_monofont *)malloc(sizeof(png_monofont));
    monofont_thin = (png_monofont *)malloc(sizeof(png_monofont));
    monofont_square = (png_monofont *)malloc(sizeof(png_monofont));
    monofont_fixedsys = (png_monofont *)malloc(sizeof(png_monofont));
    assert(
        monofont_tiny != nullptr &&
        monofont_small != nullptr &&
        monofont_thin != nullptr &&
        monofont_square != nullptr &&
        monofont_fixedsys != nullptr
    );

    monofont_tiny->sheet = cs->assets->font_tiny.tex;
    monofont_tiny->outline_sheet = NULL;
    monofont_tiny->char_w = 6;
    monofont_tiny->char_h = 5;

    monofont_small->sheet = cs->assets->font_small.tex;
    monofont_small->outline_sheet = NULL;
    monofont_small->char_w = 12;
    monofont_small->char_h = 10;

    monofont_thin->sheet = cs->assets->font_thin_no_outline.tex;
    monofont_thin->outline_sheet = cs->assets->font_thin_outline_only.tex;
    monofont_thin->char_w = 13;
    monofont_thin->char_h = 18;

    monofont_square->sheet = cs->assets->font_square_no_outline.tex;
    monofont_square->outline_sheet = cs->assets->font_square_outline_only.tex;
    monofont_square->char_w = 15;
    monofont_square->char_h = 16;

    monofont_fixedsys->sheet = cs->assets->font_fixedsys_excelsior.tex;
    monofont_fixedsys->outline_sheet = NULL;
    monofont_fixedsys->char_w = 8;
    monofont_fixedsys->char_h = 16;

    return 0;
}

void gfx_quit(CoreState *cs)
{
    cs->gfx_buttons.clear();

    free(monofont_tiny);
    free(monofont_small);
    free(monofont_thin);
    free(monofont_square);
    free(monofont_fixedsys);
}

/*
int gfx_brighten_texture(SDL_Texture *tex, Uint8 amt)
{

}

int gfx_darken_texture(SDL_Texture *tex, Uint8 amt)
{

}
*/

int gfx_createbutton(CoreState *cs, const char *text, int x, int y, unsigned int flags, int (*action)(CoreState *, void *), int (*deactivate_check)(CoreState *),
                     void *data, Uint32 rgba)
{
    if(!text)
        return -1;

    gfx_button b;
    b.text = text;
    b.x = x;
    b.y = y;
    b.w = 2 * 6 + 16 * (b.text.size());
    b.h = 28;
    b.flags = flags;
    b.highlighted = 0;
    b.clicked = 0;
    b.action = action;
    b.deactivate_check = deactivate_check;
    b.data = data;
    b.text_rgba_mod = rgba;

    //cs->gfx_buttons.resize(cs->gfx_buttons.size() + 1);
    cs->gfx_buttons.push_back(b);

    return 0;
}

int gfx_drawbuttons(CoreState *cs, int type)
{
    if(!cs)
        return -1;

    if(!cs->gfx_buttons.size())
        return 0;

    SDL_Texture *font = cs->assets->font.tex;
    // SDL_Texture *font_no_outline = cs->assets->font_no_outline.tex;
    SDL_Rect src = { 0, 0, 6, 28 };
    SDL_Rect dest = { 0, 0, 6, 28 };

    struct text_formatting fmt = {
        RGBA_DEFAULT,
        RGBA_OUTLINE_DEFAULT,
        true,
        false,
        1.0,
        1.0,
        ALIGN_LEFT,
        0
    };

    for (auto it = cs->gfx_buttons.begin(); it != cs->gfx_buttons.end(); it++) {
        gfx_button& b = *it;

        if(type == EMERGENCY_OVERRIDE && !(b.flags & BUTTON_EMERGENCY))
            continue;
        if(type == 0 && (b.flags & BUTTON_EMERGENCY))
            continue;

        if(b.highlighted)
        {
            if(b.clicked)
            {
                src.x = 362;
            }
            else
                src.x = 298;
        }
        else if(b.clicked)
        {
            src.x = 362;
        }
        else
            src.x = 330;

        src.w = 6;
        dest.w = 6;
        src.y = 26;
        dest.x = b.x;
        dest.y = b.y;

        if(b.highlighted)
        {
            SDL_SetTextureColorMod(font, R(b.text_rgba_mod), G(b.text_rgba_mod), B(b.text_rgba_mod));
            SDL_SetTextureAlphaMod(font, A(b.text_rgba_mod));
        }

        SDL_RenderCopy(cs->screen.renderer, font, &src, &dest);

        src.x += 6;
        dest.x += 6;
        src.w = 16;
        dest.w = 16;

        for (decltype(b.text)::size_type j = 0; j < b.text.size(); j++)
        {
            if(j)
                dest.x += 16;

            SDL_RenderCopy(cs->screen.renderer, font, &src, &dest);
        }

        src.x += 16;
        src.w = 6;
        dest.w = 6;
        dest.x += 16;

        SDL_RenderCopy(cs->screen.renderer, font, &src, &dest);

        SDL_SetTextureColorMod(font, 255, 255, 255);
        SDL_SetTextureAlphaMod(font, 255);

        if(b.highlighted || b.clicked)
        {
            fmt.outlined = false;
            fmt.shadow = true;
            fmt.rgba = 0x000000FF;
        }
        else
        {
            fmt.outlined = true;
            fmt.shadow = false;
            fmt.rgba = b.text_rgba_mod;
        }

        gfx_drawtext(cs, b.text, b.x + 6, b.y + 6, monofont_square, &fmt);
    }

    //cs->gfx_buttons.clear();

    return 0;
}

int gfx_drawqrsfield(CoreState *cs, Shiro::Grid *field, unsigned int mode, unsigned int flags, int x, int y)
{
    if(!cs || !field)
        return -1;

    SDL_Texture *tetrion_qs = Shiro::ImageAsset::get(cs->assetMgr, "tetrion_qs_white").getTexture();
    SDL_Texture *playfield_grid = Shiro::ImageAsset::get(cs->assetMgr, "playfield_grid_alt").getTexture();
    SDL_Texture *tets = Shiro::ImageAsset::get(cs->assetMgr, "tets_dark_qs").getTexture();
    SDL_Texture *tets_jeweled = Shiro::ImageAsset::get(cs->assetMgr, "tets_jeweled").getTexture();
    SDL_Texture *misc = Shiro::ImageAsset::get(cs->assetMgr, "misc").getTexture();

    SDL_Rect tdest = { x, y - 48, 274, 416 };
    SDL_Rect src = { 0, 0, 16, 16 };
    SDL_Rect dest = { 0, 0, 16, 16 };

    // SDL_Rect field_dest = { x + 16, y + 32, 16 * 12, 16 * 20 };

    qrsdata *q = (qrsdata *)cs->p1game->data;
    int use_deltas = 0;

    int i = 0;
    int j = 0;
    // int k = 0;
    int c = 0;
    // int outline = 0;

    std::string piece_str = "A";

    // int z = cs->p1game->frame_counter;

    /*
    int r = 127 + (int)(127.0 * sin(2.0 * 3.14159265358979 * ((double)(z % 3000) / 3000.0)));
    int g = 127 + (int)(127.0 * sin(2.0 * 3.14159265358979 * ((double)((z - 1000) % 3000) / 3000.0)));
    int b = 127 + (int)(127.0 * sin(2.0 * 3.14159265358979 * ((double)((z - 2000) % 3000) / 3000.0)));
    */

    // SDL_SetTextureAlphaMod(misc, 180);

    /*if(flags & TETRION_DEATH) {
        tetrion_qs = (asset_by_name(cs, "tetrion/tetrion_death"))->data;
        SDL_RenderCopy(cs->screen.renderer, tetrion_qs, NULL, &tdest);
    } else {
        SDL_SetTextureColorMod(tetrion_qs, (Uint8)r, (Uint8)g, (Uint8)b);
        SDL_RenderCopy(cs->screen.renderer, tetrion_qs, NULL, &tdest);
    }*/

    //   if(flags & GFX_G2) {
    //      tets = cs->assets->g2_tets_dark_g2.tex;
    //   }

    switch(mode)
    {
        case MODE_G1_MASTER:
        case MODE_G1_20G:
            tetrion_qs = Shiro::ImageAsset::get(cs->assetMgr, "g1_tetrion").getTexture();
            break;

        case MODE_G2_MASTER:
            tetrion_qs = Shiro::ImageAsset::get(cs->assetMgr, "g2_tetrion_master").getTexture();
            break;

        case MODE_G2_DEATH:
            tetrion_qs = Shiro::ImageAsset::get(cs->assetMgr, "g2_tetrion_death").getTexture();
            break;

        case MODE_G3_TERROR:
            tetrion_qs = Shiro::ImageAsset::get(cs->assetMgr, "g3_tetrion_terror").getTexture();
            break;

        default:
            break;
    }

    SDL_RenderCopy(cs->screen.renderer, tetrion_qs, NULL, &tdest);

    if((flags & DRAWFIELD_GRID) && !(flags & DRAWFIELD_BIG))
    {
        SDL_RenderCopy(cs->screen.renderer, playfield_grid, NULL, &tdest);
    }

    /*if(field == cs->p1game->field) {
          use_deltas = 1;
          if(!grid_cells_filled(q->field_deltas)) {
             SDL_SetRenderTarget(cs->screen.renderer, NULL);
             SDL_RenderCopy(cs->screen.renderer, q->field_tex, NULL, &field_dest);
             return 0;
          }

       SDL_SetRenderTarget(cs->screen.renderer, q->field_tex);
       SDL_SetRenderDrawColor(cs->screen.renderer, 0, 0, 0, 0);
       SDL_SetRenderDrawBlendMode(cs->screen.renderer, SDL_BLENDMODE_NONE);
       }*/

    /* if(flags & GFX_G2)
    {
        SDL_SetTextureColorMod(misc, 124, 124, 116);
    }
    else
    {
        SDL_SetTextureAlphaMod(misc, 140);
    }*/

    int logicalW = QRS_FIELD_W;
    int logicalH = QRS_FIELD_H;

    int cellSize = 16;

    if(flags & DRAWFIELD_BIG)
    {
        logicalW = q->field_w;
        logicalH = QRS_FIELD_H - 10;

        cellSize = 32;
    }

    dest.w = cellSize;
    dest.h = cellSize;

    for(i = 0; i < logicalW; i++)
    { // test feature: last 31 frames of every 91 frames make the stack shine
        for(j = QRS_FIELD_H - 20; j < logicalH; j++)
        {
            if(flags & TEN_W_TETRION)
            {
                if(!(flags & DRAWFIELD_BIG) && (i == 0 || i == 11))
                {
                    continue;
                }
            }
            /*if(use_deltas && !gridgetcell(q->field_deltas, i, j))
                continue;*/

            //c = gridgetcell(field, i, j);
            c = field->getCell(i, j);
            if(c == GRID_OOB)
                return 1;

            /*if((!c || c == -2) && use_deltas) {
                dest.x = i*16;
                dest.y = (j-2)*16;
                SDL_RenderFillRect(cs->screen.renderer, &dest);
            }*/

            if(c != -2 && c)
            {
                if(c == -5)
                {
                    src.x = 25 * 16;
                }
                else if(c == QRS_FIELD_W_LIMITER)
                {
                    if(!(IS_INBOUNDS(field->getCell(static_cast<std::size_t>(i) - 1, j))) && !(IS_INBOUNDS(field->getCell(static_cast<std::size_t>(i) + 1, j))))
                        src.x = 27 * 16;
                    else if((IS_INBOUNDS(field->getCell(static_cast<std::size_t>(i) - 1, j))) && !(IS_INBOUNDS(field->getCell(static_cast<std::size_t>(i) + 1, j))))
                        src.x = 28 * 16;
                    else if(!(IS_INBOUNDS(field->getCell(static_cast<std::size_t>(i) - 1, j))) && (IS_INBOUNDS(field->getCell(static_cast<std::size_t>(i) + 1, j))))
                        src.x = 29 * 16;
                }
                else if(c & QRS_PIECE_BRACKETS)
                {
                    src.x = 30 * 16;
                }
                else if(c & QRS_PIECE_GEM)
                {
                    if(flags & DRAWFIELD_JEWELED)
                    {
                        src.x = ((c & 0xff) - 19) * 16;
                    }
                    else
                    {
                        src.x = ((c & 0xff) - 1) * 16;
                    }

                    src.y = 0;
                    dest.x = x + 16 + (i * cellSize);
                    dest.y = y + 32 + ((j - QRS_FIELD_H + 20) * cellSize);

                    if((flags & DRAWFIELD_BIG) && (flags & TEN_W_TETRION))
                    {
                        dest.x += 16;
                    }

                    if(flags & DRAWFIELD_JEWELED)
                    {
                        SDL_RenderCopy(cs->screen.renderer, tets_jeweled, &src, &dest);
                    }
                    else
                    {
                        SDL_RenderCopy(cs->screen.renderer, tets, &src, &dest);
                    }

                    src.x = 32 * 16;
                }
                else
                {
                    if(flags & DRAWFIELD_JEWELED)
                    {
                        src.x = ((c & 0xff) - 19) * 16;
                    }
                    else
                    {
                        src.x = ((c & 0xff) - 1) * 16;
                    }
                }

                src.y = 0;
                dest.x = x + 16 + (i * cellSize);
                dest.y = y + 32 + ((j - QRS_FIELD_H + 20) * cellSize);

                if((flags & DRAWFIELD_BIG) && (flags & TEN_W_TETRION))
                {
                    dest.x += 16;
                }

                // piece_str[0] = c + 'A' - 1;

                // gfx_drawtext(cs, piece_str, dest.x, dest.y, (gfx_piece_colors[c-1] * 0x100) + 0xFF); //SDL_RenderCopy(cs->screen.renderer, tets, &src, &dest);
                if(!(flags & DRAWFIELD_INVISIBLE) || (c == QRS_FIELD_W_LIMITER))
                {
                    // this stuff should be handled more elegantly, without needing access to the qrsdata
                    if(q->state_flags & GAMESTATE_FADING)
                    {
                        if(GET_PIECE_FADE_COUNTER(c) > 10)
                        {
                            if(flags & DRAWFIELD_JEWELED)
                            {
                                SDL_RenderCopy(cs->screen.renderer, tets_jeweled, &src, &dest);
                            }
                            else
                            {
                                SDL_RenderCopy(cs->screen.renderer, tets, &src, &dest);
                            }
                        }
                        else if(GET_PIECE_FADE_COUNTER(c) > 0)
                        {
                            SDL_SetTextureAlphaMod(tets, GET_PIECE_FADE_COUNTER(c) * 25);
                            SDL_RenderCopy(cs->screen.renderer, tets, &src, &dest);
                            SDL_SetTextureAlphaMod(tets, 255);
                        }
                    }
                    else
                    {
                        if(flags & DRAWFIELD_JEWELED)
                        {
                            SDL_RenderCopy(cs->screen.renderer, tets_jeweled, &src, &dest);
                        }
                        else
                        {
                            SDL_RenderCopy(cs->screen.renderer, tets, &src, &dest);
                        }
                    }

                    if((!(c & QRS_PIECE_BRACKETS) || c < 0) && !(flags & DRAWFIELD_NO_OUTLINE))
                    {
                        Uint8 r_;
                        Uint8 g_;
                        Uint8 b_;
                        Uint8 a_;
                        SDL_GetRenderDrawColor(cs->screen.renderer, &r_, &g_, &b_, &a_);
                        SDL_SetRenderDrawColor(cs->screen.renderer, 0xFF, 0xFF, 0xFF, 0x8C);

                        SDL_Rect outlineRect = { dest.x, dest.y, cellSize, 2 };

                        c = field->getCell(i, static_cast<std::size_t>(j) - 1); // above
                        if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                        {
                            outlineRect.x = dest.x;
                            outlineRect.y = dest.y;
                            outlineRect.w = cellSize;
                            outlineRect.h = 2;

                            if(!use_deltas)
                            {
                                SDL_RenderFillRect(cs->screen.renderer, &outlineRect);
                            }
                        }

                        c = field->getCell(static_cast<std::size_t>(i) - 1, j); // left
                        if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                        {
                            outlineRect.x = dest.x;
                            outlineRect.y = dest.y;
                            outlineRect.w = 2;
                            outlineRect.h = cellSize;

                            if(!use_deltas)
                            {
                                SDL_RenderFillRect(cs->screen.renderer, &outlineRect);
                            }
                        }

                        c = field->getCell(static_cast<std::size_t>(i) + 1, j); // right
                        if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                        {
                            outlineRect.x = dest.x + cellSize - 2;
                            outlineRect.y = dest.y;
                            outlineRect.w = 2;
                            outlineRect.h = cellSize;

                            if(!use_deltas)
                            {
                                SDL_RenderFillRect(cs->screen.renderer, &outlineRect);
                            }
                        }

                        c = field->getCell(i, static_cast<std::size_t>(j) + 1); // below
                        if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                        {
                            outlineRect.x = dest.x;
                            outlineRect.y = dest.y + cellSize - 2;
                            outlineRect.w = cellSize;
                            outlineRect.h = 2;

                            if(!use_deltas)
                            {
                                SDL_RenderFillRect(cs->screen.renderer, &outlineRect);
                            }
                        }

                        SDL_SetRenderDrawColor(cs->screen.renderer, r_, g_, b_, a_);
                    }
                }

                SDL_SetTextureColorMod(tets, 255, 255, 255);
            }
        }
    }

    /*if(use_deltas) {
       SDL_SetRenderTarget(cs->screen.renderer, NULL);
       SDL_SetRenderDrawBlendMode(cs->screen.renderer, SDL_BLENDMODE_BLEND);
       SDL_RenderCopy(cs->screen.renderer, q->field_tex, NULL, &field_dest);
    }*/

    SDL_SetTextureColorMod(misc, 255, 255, 255);
    SDL_SetTextureAlphaMod(misc, 255);

    return 0;
}

int gfx_drawkeys(CoreState *cs, Shiro::KeyFlags *k, int x, int y, Uint32 rgba)
{
    if(!cs)
        return -1;

    SDL_Texture *font = cs->assets->font.tex;
    SDL_SetTextureColorMod(font, R(rgba), G(rgba), B(rgba));
    SDL_SetTextureAlphaMod(font, A(rgba));

    SDL_Rect src = { 0, 80, 16, 16 };
    SDL_Rect dest = { 0, y, 16, 16 };

    std::string text_a = "A";
    std::string text_b = "B";
    std::string text_c = "C";
    std::string text_d = "D";

    struct text_formatting fmt = {
        RGBA_DEFAULT,
        RGBA_OUTLINE_DEFAULT,
        true,
        false,
        1.0,
        1.0,
        ALIGN_LEFT,
        0
    };

    if(k->left)
    {
        SDL_SetTextureColorMod(font, 255, 255, 255);
    }
    else
    {
        SDL_SetTextureColorMod(font, 40, 40, 40);
    }

    src.x = 0;
    dest.x = x;
    SDL_RenderCopy(cs->screen.renderer, font, &src, &dest);

    if(k->right)
    {
        SDL_SetTextureColorMod(font, 255, 255, 255);
    }
    else
    {
        SDL_SetTextureColorMod(font, 40, 40, 40);
    }

    src.x = 16;
    dest.x = x + 16;
    SDL_RenderCopy(cs->screen.renderer, font, &src, &dest);

    if(k->up)
    {
        SDL_SetTextureColorMod(font, 255, 255, 255);
    }
    else
    {
        SDL_SetTextureColorMod(font, 40, 40, 40);
    }

    src.x = 32;
    dest.x = x + 32;
    SDL_RenderCopy(cs->screen.renderer, font, &src, &dest);

    if(k->down)
    {
        SDL_SetTextureColorMod(font, 255, 255, 255);
    }
    else
    {
        SDL_SetTextureColorMod(font, 40, 40, 40);
    }

    src.x = 48;
    dest.x = x + 48;
    SDL_RenderCopy(cs->screen.renderer, font, &src, &dest);

    SDL_SetTextureColorMod(font, 255, 255, 255);

    if(k->a)
        fmt.rgba = rgba;
    else
        fmt.rgba = 0x282828FF;

    gfx_drawtext(cs, text_a, x + 64, y, monofont_square, &fmt);

    if(k->b)
        fmt.rgba = rgba;
    else
        fmt.rgba = 0x282828FF;

    gfx_drawtext(cs, text_b, x + 80, y, monofont_square, &fmt);

    if(k->c)
        fmt.rgba = rgba;
    else
        fmt.rgba = 0x282828FF;

    gfx_drawtext(cs, text_c, x + 96, y, monofont_square, &fmt);

    if(k->d)
        fmt.rgba = rgba;
    else
        fmt.rgba = 0x282828FF;

    gfx_drawtext(cs, text_d, x + 112, y, monofont_square, &fmt);

    SDL_SetTextureColorMod(font, 255, 255, 255);
    SDL_SetTextureAlphaMod(font, 255);

    return 0;
}

int gfx_drawtext(CoreState *cs, std::string text, int x, int y, png_monofont *font, struct text_formatting *fmt)
{
    return gfx_drawtext_partial(cs, text, 0, text.size(), x, y, font, fmt);
}

int gfx_drawtext_partial(CoreState *cs, std::string text, int pos, std::size_t len, int x, int y, png_monofont *font, struct text_formatting *fmt)
{
    if(!cs || text == "")
        return -1;

    if(!font)
        font = monofont_fixedsys;

    struct text_formatting fmt_ = {
        RGBA_DEFAULT,
        RGBA_OUTLINE_DEFAULT,
        true,
        false,
        1.0,
        1.0,
        ALIGN_LEFT,
        0
    };

    if(!fmt)
        fmt = &fmt_;

    SDL_SetTextureColorMod(font->sheet, R(fmt->rgba), G(fmt->rgba), B(fmt->rgba));
    SDL_SetTextureAlphaMod(font->sheet, A(fmt->rgba));

    if(font->outline_sheet)
    {
        SDL_SetTextureColorMod(font->outline_sheet, R(fmt->outline_rgba), G(fmt->outline_rgba), B(fmt->outline_rgba));
        SDL_SetTextureAlphaMod(font->outline_sheet, A(fmt->outline_rgba));
    }

    SDL_Rect src = { 0, 0, (int) font->char_w, (int) font->char_h };
    SDL_Rect dest = { x, y, (int) (fmt->size_multiplier * (float) font->char_w), (int) (fmt->size_multiplier * (float) font->char_h) };

    std::size_t i = 0;

    std::size_t linefeeds = 0;

    std::size_t last_wrap_line_pos = 0;
    std::size_t last_wrap_pos = 0;

    std::vector<std::string> lines = strtools::split(text, '\n');

    bool using_target_tex = false;

    if(SDL_GetRenderTarget(cs->screen.renderer) != NULL)
        using_target_tex = true;

    for(i = pos; i < text.size() && i < len; i++)
    {
        if(i == 0)
        {
            switch(fmt->align)
            {
                default:
                case ALIGN_LEFT:
                    dest.x = x;
                    break;

                case ALIGN_RIGHT:
                    dest.x = static_cast<int>(x - (fmt->size_multiplier * (float)font->char_w) * lines[0].size());
                    break;

                case ALIGN_CENTER:
                    if (fmt->wrap_length < lines[0].size() - last_wrap_line_pos)
                        dest.x = x;
                    else
                        dest.x = x + static_cast<int>((fmt->size_multiplier * (float)font->char_w / 2.0f) * (fmt->wrap_length - (lines[0].size() - last_wrap_line_pos)));

                    break;
            }
        }

        if((fmt->wrap_length && i != 0 && (i - last_wrap_pos) % fmt->wrap_length == 0) || text[i] == '\n')
        {
            if(text[i] == '\n')
            {
                linefeeds++;
                last_wrap_line_pos = i - last_wrap_pos + last_wrap_line_pos;
                last_wrap_pos = i;
            }
            else if(i != 0 && i % fmt->wrap_length == 0)
            {
                last_wrap_line_pos = i - last_wrap_pos + last_wrap_line_pos;
                last_wrap_pos = i;
            }

            dest.y += static_cast<int>(fmt->line_spacing * fmt->size_multiplier * (float)font->char_h);

            switch(fmt->align)
            {
                default:
                case ALIGN_LEFT:
                    dest.x = x;
                    break;

                case ALIGN_RIGHT:
                    dest.x = static_cast<int>(x - (font->char_w) * lines[linefeeds].size());
                    break;

                case ALIGN_CENTER:
                    if(fmt->wrap_length < lines[linefeeds].size() - last_wrap_line_pos)
                        dest.x = x;
                    else
                        dest.x = (int)(x + (font->char_w / 2) * (fmt->wrap_length - (lines[linefeeds].size() - last_wrap_line_pos)));

                    break;
            }

            if(text[i] == '\n')
                continue;
        }

        // we draw a square behind each character if we have no outlines to use
        if(fmt->outlined && !font->outline_sheet)
        {
            src.x = 31 * font->char_w;
            src.y = 3 * font->char_h;
            SDL_SetTextureColorMod(font->sheet, R(fmt->outline_rgba), G(fmt->outline_rgba), B(fmt->outline_rgba));
            SDL_SetTextureAlphaMod(font->sheet, A(fmt->outline_rgba));

            if(using_target_tex)
            {
                SDL_SetRenderDrawColor(cs->screen.renderer, 0, 0, 0, 0);
                SDL_RenderFillRect(cs->screen.renderer, &dest);
                SDL_RenderCopy(cs->screen.renderer, font->sheet, &src, &dest);
            }
            else
            {
                SDL_RenderCopy(cs->screen.renderer, font->sheet, &src, &dest);
            }

            SDL_SetTextureColorMod(font->sheet, R(fmt->rgba), G(fmt->rgba), B(fmt->rgba));
            SDL_SetTextureAlphaMod(font->sheet, A(fmt->rgba));
        }

        src.x = font->char_w * (text[i] % 32);
        src.y = font->char_h * ((int)(text[i] / 32) - 1);
        if(src.y < 0)
        {
            src.x = 31 * font->char_w;
            src.y = 2 * font->char_h;
        }

        if(fmt->shadow)
        {
            dest.x -= static_cast<int>(2.0f * fmt->size_multiplier);
            dest.y += static_cast<int>(2.0f * fmt->size_multiplier);

            SDL_SetTextureAlphaMod(font->sheet, A(fmt->rgba) / 4);
            if(font->outline_sheet)
                SDL_SetTextureAlphaMod(font->outline_sheet, A(fmt->rgba) / 4);

            SDL_RenderCopy(cs->screen.renderer, font->sheet, &src, &dest);

            if(fmt->outlined && font->outline_sheet)
            {
                SDL_RenderCopy(cs->screen.renderer, font->outline_sheet, &src, &dest);
            }

            dest.x += static_cast<int>(2.0f * fmt->size_multiplier);
            dest.y -= static_cast<int>(2.0f * fmt->size_multiplier);

            SDL_SetTextureAlphaMod(font->sheet, A(fmt->rgba));
            if(font->outline_sheet)
                SDL_SetTextureAlphaMod(font->outline_sheet, A(fmt->rgba));
        }

        if(using_target_tex)
        {
            SDL_SetRenderDrawColor(cs->screen.renderer, 0, 0, 0, 0);
            SDL_RenderFillRect(cs->screen.renderer, &dest);
            SDL_RenderCopy(cs->screen.renderer, font->sheet, &src, &dest);

            if(fmt->outlined && font->outline_sheet)
                SDL_RenderCopy(cs->screen.renderer, font->outline_sheet, &src, &dest);
        }
        else
        {
            SDL_RenderCopy(cs->screen.renderer, font->sheet, &src, &dest);

            if(fmt->outlined && font->outline_sheet)
                SDL_RenderCopy(cs->screen.renderer, font->outline_sheet, &src, &dest);
        }

        dest.x += static_cast<int>(fmt->size_multiplier * (float)font->char_w);
    }

    SDL_SetTextureColorMod(font->sheet, 255, 255, 255);
    SDL_SetTextureAlphaMod(font->sheet, 255);

    if(font->outline_sheet)
    {
        SDL_SetTextureColorMod(font->outline_sheet, 255, 255, 255);
        SDL_SetTextureAlphaMod(font->outline_sheet, 255);
    }

    return 0;
}

int gfx_drawpiece(CoreState *cs, Shiro::Grid *field, int field_x, int field_y, Shiro::PieceDefinition& pd, unsigned int flags, int orient, int x, int y, Uint32 rgba)
{
    if(!cs)
        return -1;

    if(flags & DRAWPIECE_BRACKETS && flags & DRAWPIECE_LOCKFLASH)
    {
        return 0;
    }

    if((flags & DRAWPIECE_PREVIEW) && (flags & DRAWPIECE_BIG))
    {
        flags &= ~DRAWPIECE_BIG;
    }

    if((flags & DRAWPIECE_JEWELED) && (flags & DRAWPIECE_SMALL))
    {
        flags &= ~DRAWPIECE_JEWELED;
    }

    SDL_Texture *tets;
    SDL_Texture *tets_jeweled = Shiro::ImageAsset::get(cs->assetMgr, "tets_jeweled").getTexture();
    SDL_Texture *misc = Shiro::ImageAsset::get(cs->assetMgr, "misc").getTexture();

    //   if(flags & GFX_G2) {
    //      if(flags & DRAWPIECE_SMALL)
    //         tets = Shiro::ImageAsset::get(cs->assetMgr, "g2_tets_bright_g2_small").getTexture();
    //      else
    //         tets = Shiro::ImageAsset::get(cs->assetMgr, "g2_tets_bright_g2").getTexture();
    //   } else {
    if(flags & DRAWPIECE_SMALL)
    {
        tets = Shiro::ImageAsset::get(cs->assetMgr, "tets_bright_qs_small").getTexture();
    }
    else
    {
        tets = Shiro::ImageAsset::get(cs->assetMgr, "tets_bright_qs").getTexture();
    }
    //   }

    int size = (flags & DRAWPIECE_SMALL) ? 8 : (flags & DRAWPIECE_BIG ? 32 : 16);
    SDL_Rect src = { 0, 0, (size == 8 ? 8 : 16), (size == 8 ? 8 : 16) };
    SDL_Rect dest = { 0, 0, size, size };

    std::string piece_str = "A";
    piece_str[0] = pd.qrsID + 'A';

    Shiro::Grid *g = NULL;

    int i = 0;
    int j = 0;
    int w = int(pd.rotationTable[0].getWidth());
    int h = int(pd.rotationTable[0].getWidth());
    int c = 0;

    int cell_x = 0;
    int cell_y = 0;

    g = &pd.rotationTable[orient & 3];
    src.x = pd.qrsID * (size == 8 ? 8 : 16);
    if(flags & DRAWPIECE_JEWELED)
    {
        src.x -= 18 * 16;
    }

    /*if(flags & DRAWPIECE_IPREVIEW && !(flags & GFX_G2)) {
       y += 8;
    }*/

    SDL_SetTextureColorMod(tets, R(rgba), G(rgba), B(rgba));
    SDL_SetTextureAlphaMod(tets, A(rgba));

    for(i = 0; i < w; i++)
    {
        for(j = 0; j < h; j++)
        {
            if(g->getCell(i, j) && (y + (j * size) > (field_y + 16) || flags & DRAWPIECE_PREVIEW))
            {
                dest.x = x + 16 + ((i - 1) * size);

                if(flags & DRAWPIECE_PREVIEW)
                {
                    if(w == 4)
                    {
                        dest.y = y + ((j + 1) * size);
                    }
                    else
                    {
                        dest.y = y + (j * size);
                    }
                }
                else
                {
                    dest.y = y + 16 + ((j - 1) * size);
                }

                if(flags & DRAWPIECE_BRACKETS || pd.flags & Shiro::PDBRACKETS)
                    src.x = 30 * (size == 8 ? 8 : 16);

                if(flags & DRAWPIECE_LOCKFLASH && !(flags & DRAWPIECE_BRACKETS) && !(pd.flags & Shiro::PDBRACKETS))
                {
                    src.x = 26 * (size == 8 ? 8 : 16);
                    cell_x = (x - field_x - 16) / size + i;
                    cell_y = (y - field_y - 32) / size + j + QRS_FIELD_H - 20;

                    if(field->getCell(cell_x, cell_y) > 0 || flags & DRAWPIECE_PREVIEW)
                    {
                        SDL_RenderCopy(cs->screen.renderer, tets, &src, &dest);

                        if(!(flags & DRAWPIECE_PREVIEW))
                        {
                            c = field->getCell(cell_x, static_cast<std::size_t>(cell_y) - 1); // above, left, right, below
                            if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                            {
                                src.x = 0;
                                src.y = 48;

                                SDL_RenderCopy(cs->screen.renderer, misc, &src, &dest);
                            }

                            c = field->getCell(static_cast<std::size_t>(cell_x) - 1, cell_y); // above, left, right, below
                            if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                            {
                                src.x = 16;
                                src.y = 48;

                                SDL_RenderCopy(cs->screen.renderer, misc, &src, &dest);
                            }

                            c = field->getCell(static_cast<std::size_t>(cell_x) + 1, cell_y); // above, left, right, below
                            if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                            {
                                src.x = 32;
                                src.y = 48;

                                SDL_RenderCopy(cs->screen.renderer, misc, &src, &dest);
                            }

                            c = field->getCell(cell_x, static_cast<std::size_t>(cell_y) + 1); // above, left, right, below
                            if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                            {
                                src.x = 48;
                                src.y = 48;

                                SDL_RenderCopy(cs->screen.renderer, misc, &src, &dest);
                            }

                            src.y = 0;
                        }
                    }
                }
                else
                {
                    // gfx_drawtext(cs, piece_str, dest.x, dest.y, (gfx_piece_colors[pd->qrs_id] * 0x100) + A(rgba)); //SDL_RenderCopy(cs->screen.renderer, tets, &src, &dest);
                    if(flags & DRAWPIECE_JEWELED)
                    {
                        SDL_RenderCopy(cs->screen.renderer, tets_jeweled, &src, &dest);
                    }
                    else
                    {
                        SDL_RenderCopy(cs->screen.renderer, tets, &src, &dest);
                    }
                }
            }
        }
    }

    SDL_SetTextureColorMod(tets, 255, 255, 255);
    SDL_SetTextureAlphaMod(tets, 255);

    return 0;
}

int gfx_drawtimer(CoreState *cs, Shiro::Timer *t, int x, Uint32 rgba)
{
    SDL_Texture *font = cs->assets->font.tex;
    qrsdata *q = (qrsdata *)cs->p1game->data;
    int y = q->fieldPos->second;

    SDL_Rect src = { 0, 96, 20, 32 };
    SDL_Rect dest = { x, 26 * 16 + 8 - QRS_FIELD_Y + y, 20, 32 };

    uint64_t min = t->min();
    uint64_t sec = t->sec() % 60;
    uint64_t csec = t->csec() % 100;

    int i = 0;
    // int n = 0;

    int digits[6];

    digits[0] = int(min / 10);
    digits[1] = int(min % 10);
    digits[2] = int(sec / 10);
    digits[3] = int(sec % 10);
    digits[4] = int(csec / 10);
    digits[5] = int(csec % 10);

    SDL_SetTextureColorMod(font, R(rgba), G(rgba), B(rgba));
    SDL_SetTextureAlphaMod(font, A(rgba));

    for(i = 0; i < 6; i++)
    {
        src.x = digits[i] * 20;
        SDL_RenderCopy(cs->screen.renderer, font, &src, &dest);
        dest.x += 20;

        if(i == 1 || i == 3)
        {
            src.x = 200; // colon character offset
            SDL_RenderCopy(cs->screen.renderer, font, &src, &dest);
            dest.x += 20;
        }
    }

    SDL_SetTextureColorMod(font, 255, 255, 255);
    SDL_SetTextureAlphaMod(font, 255);

    return 0;
}