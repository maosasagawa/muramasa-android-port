#include "FontRenderer.hpp"
#include <png.h>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace
{
string EncodeUtf8(uint32_t Codepoint)
{
    string Result;
    if (Codepoint <= 0x7F)
        Result += static_cast<char>(Codepoint);
    else if (Codepoint <= 0x7FF)
    {
        Result += static_cast<char>(0xC0 | (Codepoint >> 6));
        Result += static_cast<char>(0x80 | (Codepoint & 0x3F));
    }
    else if (Codepoint <= 0xFFFF)
    {
        Result += static_cast<char>(0xE0 | (Codepoint >> 12));
        Result += static_cast<char>(0x80 | ((Codepoint >> 6) & 0x3F));
        Result += static_cast<char>(0x80 | (Codepoint & 0x3F));
    }
    else
    {
        Result += static_cast<char>(0xF0 | (Codepoint >> 18));
        Result += static_cast<char>(0x80 | ((Codepoint >> 12) & 0x3F));
        Result += static_cast<char>(0x80 | ((Codepoint >> 6) & 0x3F));
        Result += static_cast<char>(0x80 | (Codepoint & 0x3F));
    }
    return Result;
}

bool WritePng(const string& Path, int Width, int Height, const vector<uint8_t>& Pixels)
{
    const filesystem::path OutputPath(Path);
    if (!OutputPath.parent_path().empty())
        filesystem::create_directories(OutputPath.parent_path());

    png_image Image = {};
    Image.version = PNG_IMAGE_VERSION;
    Image.width = Width;
    Image.height = Height;
    Image.format = PNG_FORMAT_RGBA;
    if (!png_image_write_to_file(&Image, Path.c_str(), 0, Pixels.data(), 0, nullptr))
    {
        cerr << "failed to write " << Path << ": " << Image.message << '\n';
        png_image_free(&Image);
        return false;
    }
    png_image_free(&Image);
    return true;
}

void RenderSize(const string& Text, int Size, const string& OutputPrefix)
{
    int RawWidth = 0, RawHeight = 0;
    vector<FontGlyphMetrics> Metrics;
    uint8_t* Raw = FontRenderText(Text.c_str(), Size, 0, &RawWidth, &RawHeight,
                                  true, &Metrics);
    if (!Raw)
        throw runtime_error("font rendering failed; set NP_FONT to a readable font file");

    const int DisplayWidth = RawHeight;
    const int DisplayHeight = RawWidth;
    vector<uint8_t> DisplayPixels(static_cast<size_t>(DisplayWidth) * DisplayHeight * 4);
    for (int Y = 0; Y < RawHeight; ++Y)
        for (int X = 0; X < RawWidth; ++X)
        {
            const int DisplayX = RawHeight - 1 - Y;
            const int DisplayY = X;
            const size_t Source = (static_cast<size_t>(Y) * RawWidth + X) * 4;
            const size_t Destination =
                (static_cast<size_t>(DisplayY) * DisplayWidth + DisplayX) * 4;
            for (int Channel = 0; Channel < 4; ++Channel)
                DisplayPixels[Destination + Channel] = Raw[Source + Channel];
        }
    std::free(Raw);

    const string OutputPath = OutputPrefix + "_" + to_string(Size) + ".png";
    if (!WritePng(OutputPath, DisplayWidth, DisplayHeight, DisplayPixels))
        throw runtime_error("PNG output failed");
    cout << "size=" << Size << " output=" << OutputPath
         << " dimensions=" << DisplayWidth << "x" << DisplayHeight << '\n';

    cout << fixed << setprecision(2);
    for (const FontGlyphMetrics& Metric : Metrics)
    {
        const int CellX = RawHeight - (Metric.CellY + Metric.CellHeight);
        const int CellY = Metric.CellX;
        const int CellWidth = Metric.CellHeight;
        const int CellHeight = Metric.CellWidth;
        const int InkX = Metric.InkWidth ? RawHeight - (Metric.InkY + Metric.InkHeight) : 0;
        const int InkY = Metric.InkX;
        const int InkWidth = Metric.InkHeight;
        const int InkHeight = Metric.InkWidth;
        const double CentroidX = RawHeight - 1 - Metric.CentroidY;
        const double CentroidY = Metric.CentroidX;
        cout << "  glyph=" << EncodeUtf8(Metric.Codepoint)
             << " U+" << hex << uppercase << Metric.Codepoint << dec << nouppercase
             << " cell=(" << CellX << ',' << CellY << ',' << CellWidth << ',' << CellHeight << ')'
             << " ink=(" << InkX << ',' << InkY << ',' << InkWidth << ',' << InkHeight << ')'
             << " centroid=(" << CentroidX << ',' << CentroidY << ")\n";
    }
}
}

int main(int argc, char** argv)
{
    try
    {
        const string Text = argc > 1 ? argv[1] : u8"一。、，．！？「」『』（）……——ー～国.";
        const string OutputPrefix = argc > 2 ? argv[2] : "fonttest";
        RenderSize(Text, 20, OutputPrefix);
        RenderSize(Text, 24, OutputPrefix);
    }
    catch (const exception& Error)
    {
        cerr << Error.what() << '\n';
        return 1;
    }
    return 0;
}
