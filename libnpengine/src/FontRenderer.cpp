/*
 * stb_truetype-based text layout + rasterizer.
 * Replaces the Pango/Cairo stack so the engine has no font-service
 * dependency (needed for the Android port).
 * */
#include "FontRenderer.hpp"
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <vector>
#include <string>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

namespace
{
stbtt_fontinfo FontInfo;
std::vector<unsigned char> FontData;
bool FontReady = false;

const char* FontCandidates[] =
{
    "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
    "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
    "/usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc",
    "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
};

bool LoadFont()
{
    if (FontReady)
        return true;

    const char* Path = getenv("NP_FONT");
    if (!Path)
        for (const char* C : FontCandidates)
            if (FILE* f = fopen(C, "rb"))
            {
                fclose(f);
                Path = C;
                break;
            }
    if (!Path)
        return false;

    FILE* f = fopen(Path, "rb");
    if (!f)
        return false;
    fseek(f, 0, SEEK_END);
    long Size = ftell(f);
    fseek(f, 0, SEEK_SET);
    FontData.resize(Size);
    fread(FontData.data(), 1, Size, f);
    fclose(f);

    int Offset = stbtt_GetFontOffsetForIndex(FontData.data(), 0);
    if (Offset < 0 || !stbtt_InitFont(&FontInfo, FontData.data(), Offset))
        return false;
    FontReady = true;
    fprintf(stderr, "[font] loaded %s\n", Path);
    return true;
}

// UTF-8 -> codepoints
std::vector<uint32_t> Decode(const char* Str)
{
    std::vector<uint32_t> Out;
    const unsigned char* s = (const unsigned char*)Str;
    while (*s)
    {
        uint32_t cp;
        if (*s < 0x80) { cp = *s++; }
        else if ((*s & 0xE0) == 0xC0) { cp = *s & 0x1F; if (s[1]) cp = (cp << 6) | (s[1] & 0x3F), s += 2; else { cp = 0xFFFD; s++; } }
        else if ((*s & 0xF0) == 0xE0) { cp = *s & 0x0F; if (s[1] && s[2]) cp = ((cp << 6) | (s[1] & 0x3F)), cp = (cp << 6) | (s[2] & 0x3F), s += 3; else { cp = 0xFFFD; s++; } }
        else if ((*s & 0xF8) == 0xF0) { cp = *s & 0x07; if (s[1] && s[2] && s[3]) cp = ((cp << 6) | (s[1] & 0x3F)), cp = (cp << 6) | (s[2] & 0x3F), cp = (cp << 6) | (s[3] & 0x3F), s += 4; else { cp = 0xFFFD; s++; } }
        else { cp = 0xFFFD; s++; }
        Out.push_back(cp);
    }
    return Out;
}

bool IsTopRightVerticalPunctuation(uint32_t cp)
{
    return cp == 0x3001 || cp == 0x3002 || cp == 0xFF0C || cp == 0xFF0E;
}

bool IsSidewaysVerticalGlyph(uint32_t cp)
{
    switch (cp)
    {
        case 0x0028: case 0x0029:
        case 0x2013: case 0x2014: case 0x2015:
        case 0x2025: case 0x2026:
        case 0x2212:
        case 0x3008: case 0x3009:
        case 0x300A: case 0x300B:
        case 0x300C: case 0x300D:
        case 0x300E: case 0x300F:
        case 0x3010: case 0x3011:
        case 0x301C:
        case 0x30FC:
        case 0xFF08: case 0xFF09:
        case 0xFF0D: case 0xFF5E:
            return true;
        default:
            return false;
    }
}

bool IsUprightVerticalPunctuation(uint32_t cp)
{
    return cp == 0xFF01 || cp == 0xFF1F || cp == '!' || cp == '?';
}
}

// Returns an RGBA (white, alpha=coverage) buffer; caller frees.
uint8_t* FontRenderText(const char* String, int Size, int WrapWidth, int* OutW, int* OutH, bool Vertical)
{
    *OutW = *OutH = 0;
    if (!LoadFont() || !String || !*String)
        return nullptr;

    float Scale = stbtt_ScaleForPixelHeight(&FontInfo, Size);
    int Ascent, Descent, LineGap;
    stbtt_GetFontVMetrics(&FontInfo, &Ascent, &Descent, &LineGap);
    int LineHeight = (int)((Ascent - Descent + LineGap) * Scale) + 1;

    std::vector<uint32_t> Cps = Decode(String);

    // Layout pass: per-codepoint greedy wrap (CJK breaks anywhere).
    struct Glyph { uint32_t cp; int x, y, adv; };
    std::vector<Glyph> Glyphs;
    int X = 0, Y = 0, MaxX = 0;
    for (uint32_t cp : Cps)
    {
        if (cp == '\n')
        {
            X = 0;
            Y += LineHeight;
            continue;
        }
        int Adv, Lsb;
        int GlyphIdx = stbtt_FindGlyphIndex(&FontInfo, cp);
        if (!GlyphIdx) GlyphIdx = stbtt_FindGlyphIndex(&FontInfo, 0xFFFD);
        stbtt_GetGlyphHMetrics(&FontInfo, GlyphIdx, &Adv, &Lsb);
        int AdvPx = (int)(Adv * Scale) + 1;
        if (WrapWidth > 0 && X > 0 && X + AdvPx > WrapWidth)
        {
            X = 0;
            Y += LineHeight;
        }
        Glyphs.push_back({cp, X, Y, AdvPx});
        X += AdvPx;
        if (X > MaxX) MaxX = X;
    }

    int W = MaxX ? MaxX : 1, H = (Y + LineHeight) ? (Y + LineHeight) : 1;
    uint8_t* Buf = (uint8_t*)calloc(4 * W * H, 1);
    if (!Buf)
        return nullptr;

    for (const Glyph& g : Glyphs)
    {
        int w, h, xoff, yoff;
        unsigned char* Bitmap = stbtt_GetCodepointBitmap(&FontInfo, Scale, Scale,
                                                        g.cp, &w, &h, &xoff, &yoff);
        if (!Bitmap)
            continue;
        bool SidewaysVertical = Vertical && IsSidewaysVerticalGlyph(g.cp);
        bool UprightVertical = Vertical && IsUprightVerticalPunctuation(g.cp);
        double InkCenterX = (w - 1) * 0.5, InkCenterY = (h - 1) * 0.5;
        if (Vertical && (IsTopRightVerticalPunctuation(g.cp) ||
                         UprightVertical || SidewaysVertical))
        {
            double InkWeight = 0, InkX = 0, InkY = 0;
            for (int row = 0; row < h; ++row)
            {
                for (int col = 0; col < w; ++col)
                {
                    unsigned char Cov = Bitmap[row * w + col];
                    InkWeight += Cov;
                    InkX += col * Cov;
                    InkY += row * Cov;
                }
            }
            if (InkWeight)
            {
                InkCenterX = InkX / InkWeight;
                InkCenterY = InkY / InkWeight;
            }
        }
        int Baseline = (int)(Ascent * Scale);
        int PX = g.x + xoff, PY = g.y + Baseline + yoff;
        if (Vertical)
        {
            if (IsTopRightVerticalPunctuation(g.cp))
            {
                double TargetX = g.x + (g.adv - 1) * 0.25;
                double TargetY = g.y + (LineHeight - 1) * 0.5 - Size * 0.2;
                PX = (int)lround(TargetX - InkCenterY);
                PY = (int)lround(TargetY - (w - 1 - InkCenterX));
            }
            else if (IsUprightVerticalPunctuation(g.cp))
            {
                double TargetX = g.x + (g.adv - 1) * 0.5;
                double TargetY = g.y + (LineHeight - 1) * 0.5;
                PX = (int)lround(TargetX - InkCenterY);
                PY = (int)lround(TargetY - (w - 1 - InkCenterX));
            }
            else if (SidewaysVertical)
            {
                double TargetX = g.x + (g.adv - 1) * 0.5;
                double TargetY = g.y + (LineHeight - 1) * 0.5;
                PX = (int)lround(TargetX - InkCenterX);
                PY = (int)lround(TargetY - InkCenterY);
            }
            else
            {
                PX += w / 2 - h / 2;
                PY += h / 2 - w / 2;
            }
        }
        for (int row = 0; row < h; ++row)
        {
            for (int col = 0; col < w; ++col)
            {
                int Dx, Dy;
                if (Vertical && !SidewaysVertical)
                {
                    // rotate glyph 90 deg CCW about its ink center
                    Dx = PX + row;
                    Dy = PY + (w - 1 - col);
                }
                else
                {
                    Dx = PX + col;
                    Dy = PY + row;
                }
                if (Dy < 0 || Dy >= H || Dx < 0 || Dx >= W) continue;
                uint8_t Cov = Bitmap[row * w + col];
                uint8_t* Px = Buf + 4 * (Dy * W + Dx);
                // white text; alpha = coverage (straight alpha blending)
                Px[0] = Px[1] = Px[2] = 255;
                int A = Cov + Px[3];
                Px[3] = A > 255 ? 255 : A;
            }
        }
        stbtt_FreeBitmap(Bitmap, nullptr);
    }

    *OutW = W;
    *OutH = H;
    return Buf;
}
