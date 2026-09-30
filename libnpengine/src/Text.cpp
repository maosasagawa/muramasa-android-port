/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2014-2016,2018 Mislav Blažević <krofnica996@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * */
#include "Text.hpp"
#include "Playable.hpp"
#define yyparse xmlparse
#define yy_scan_bytes xml_scan_bytes
#define yy_delete_buffer xml_delete_buffer
#include "flex.hpp"
#include "FontRenderer.hpp"

// For use in bison
TextParser::Text* pText;

string Text::dFont;
int32_t Text::dSize;
uint32_t Text::dInColor;
uint32_t Text::dOutColor;
int32_t Text::dWeight;
string Text::dAlign;

class VoiceMgr
{
public:
    VoiceMgr() : pVoice(nullptr)
    {
    }

    ~VoiceMgr()
    {
        delete pVoice;
    }

    void SetVoice(const string& Filename)
    {
        delete pVoice;
        pVoice = new Playable(sResourceMgr->GetResource(Filename + ".ogg"));
        pVoice->Play();
    }

private:
    Playable* pVoice;
} sVoiceMgr;

Text::Text() : Index(0), LayoutWidth(-1), Size(dSize), Color(dInColor)
{
}

Text::~Text()
{
}

void Text::CreateFromXML(const string& XML)
{
    ::pText = this;
    YY_BUFFER_STATE buffer = yy_scan_bytes(XML.c_str(), XML.size());
    try
    {
        yyparse();
    }
    catch (const std::exception& e)
    {
        /* Muramasa's <PRE> text doesn't fit the grammar; strip markup and
           render it as one plain line instead of dying. */
        string Plain;
        bool InAngle = false, InSquare = false;
        for (char c : XML)
        {
            if (c == '<') { InAngle = true; continue; }
            if (InAngle) { if (c == '>') InAngle = false; continue; }
            if (c == '[') { InSquare = true; continue; }
            if (InSquare) { if (c == ']') InSquare = false; continue; }
            Plain += c;
        }
        TextParser::Line L;
        TextParser::StringSegment Seg;
        Seg.Segment = Plain;
        L.StringSegs.push_back(Seg);
        Lines.push_back(L);
        if (getenv("NP_TEXT_DBG"))
            fprintf(stderr, "[text] xml parse failed (%s), fallback used\n", e.what());
    }
    yy_delete_buffer(buffer);
}

void Text::CreateFromString(const string& String)
{
    string Plain;
    bool InTag = false;
    for (char c : String)
    {
        if (c == '<')
        {
            InTag = true;
            continue;
        }
        if (InTag)
        {
            if (c == '>')
                InTag = false;
            continue;
        }
        Plain += c;
    }
    SetString(Plain);
}

void Text::SetCharacterSize(uint32_t Size)
{
    this->Size = Size;
}

void Text::SetColor(uint32_t Color)
{
    this->Color = Color;
    if (!CurString.empty())
        SetString(CurString);
}

void Text::SetWrap(int32_t Width)
{
    LayoutWidth = Width;
}

bool Text::Advance()
{
    if (Index == Lines.size())
        return false;

    TextParser::Line& CurrLine = Lines[Index];

    // [HACK]
    string String;
    for (size_t i = 0; i < CurrLine.StringSegs.size(); ++i)
        String += CurrLine.StringSegs[CurrLine.StringSegs.size() - i - 1].Segment;
    SetString(String);

    if (!CurrLine.VoiceAttrs.empty())
        sVoiceMgr.SetVoice(CurrLine.VoiceAttrs[TextParser::ATTR_SRC]);
    Index++;
    return true;
}

void Text::Reflow(int32_t W)
{
    LayoutWidth = W;
    if (!CurString.empty())
        SetString(CurString);
}

void Text::SetVertical(bool V)
{
    if (V == Vertical)
        return;
    Vertical = V;
    if (!CurString.empty())
        SetString(CurString);
}

void Text::SetString(const string& String)
{
    CurString = String;
    int W = 0, H = 0;
    uint8_t* pData = FontRenderText(String.c_str(), Size, LayoutWidth, &W, &H, Vertical);
    if (!pData)
        return;
    uint32_t RenderColor = Color ? Color : 0xFFFFFFFF;
    uint8_t R = (RenderColor >> 16) & 0xFF;
    uint8_t G = (RenderColor >> 8) & 0xFF;
    uint8_t B = RenderColor & 0xFF;
    uint8_t A = (RenderColor >> 24) & 0xFF;
    size_t PixelCount = static_cast<size_t>(W) * H;
    for (size_t i = 0; i < PixelCount; ++i)
    {
        size_t Pixel = i * 4;
        pData[Pixel + 0] = R;
        pData[Pixel + 1] = G;
        pData[Pixel + 2] = B;
        pData[Pixel + 3] = pData[Pixel + 3] * A / 255;
    }
    Width = W;
    Height = H;
    Create(pData, GL_RGBA, Width, Height);
    free(pData);
}

void Text::Request(int32_t State)
{
}
