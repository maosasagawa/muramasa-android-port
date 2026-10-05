#ifndef FONTRENDERER_HPP
#define FONTRENDERER_HPP

#include <cstdint>
#include <vector>

struct FontGlyphMetrics
{
    uint32_t Codepoint;
    int CellX, CellY, CellWidth, CellHeight;
    int InkX, InkY, InkWidth, InkHeight;
    double CentroidX, CentroidY;
};

// Renders UTF-8 text to an RGBA pixel buffer (white glyphs, coverage alpha).
// WrapWidth <= 0 disables wrapping. Caller owns the returned buffer.
uint8_t* FontRenderText(const char* String, int Size, int WrapWidth, int* OutW, int* OutH,
                        bool Vertical = false, std::vector<FontGlyphMetrics>* Metrics = nullptr);

#endif
