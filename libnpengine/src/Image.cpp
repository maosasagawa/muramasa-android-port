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
#include <iostream>
#ifndef NP_GLES2
#include <GL/glew.h>
#endif
#include "GLUtil.hpp"
#include "Image.hpp"
#include "ResourceMgr.hpp"
#include "Window.hpp"
#include "fscommon.hpp"
#include <jpeglib.h>
#include <png.h>
#include <new>
#include <algorithm>

Image::Image() : Format(-1), Width(0), Height(0), pPixels(0)
{
}

Image::~Image()
{
    delete[] pPixels;
}

void Image::LoadColor(int Width, int Height, uint32_t Color)
{
    this->Width = Width;
    this->Height = Height;
    pPixels = new uint8_t[Width * Height * 4];
    // Color is 0xAABBGGRR-style packed uint32 (AARRGGBB); store RGBA-order pixels.
    uint8_t R = (Color >> 16) & 0xFF, G = (Color >> 8) & 0xFF,
          B = Color & 0xFF, A = (Color >> 24) & 0xFF;
    for (int i = 0; i < Width * Height; ++i)
    {
        pPixels[i * 4 + 0] = R;
        pPixels[i * 4 + 1] = G;
        pPixels[i * 4 + 2] = B;
        pPixels[i * 4 + 3] = A;
    }
    Format = GL_RGBA;
}

void Image::LoadImage(const string& Filename, bool Mask)
{
    uint32_t Size;
    uint8_t* pData = (uint8_t*)sResourceMgr->Read(Filename, Size);
    if (!pData)
        pData = (uint8_t*)fs::ReadFile(Filename, Size);
    if (!pData)
        return;

    if (Size >= 2 && pData[0] == 0xFF && pData[1] == 0xD8)
    {
        pPixels = LoadJPEG(pData, Size);
    }
    else if (Size >= 4 && pData[0] == 0x89 && pData[1] == 'P' &&
             pData[2] == 'N' && pData[3] == 'G')
    {
        pPixels = LoadPNG(pData, Size, Mask ? PNG_FORMAT_GRAY : PNG_FORMAT_RGBA);
        Format = Mask ? GL_LUMINANCE : GL_RGBA;
    }
    else
        cout << Filename << " is neither .jpg nor .png!" << endl;

    delete[] pData;
}

void Image::LoadScreen(Window* pWindow)
{
    Format = GL_RGBA;
    Width = pWindow->WIDTH;
    Height = pWindow->HEIGHT;
    pPixels = new uint8_t[Width * Height * 4];
    pWindow->DrawTextures(0);
#ifndef NP_GLES2
    glReadBuffer(GL_BACK);
#endif
    glReadPixels(0, 0, Width, Height, GL_RGBA, GL_UNSIGNED_BYTE, pPixels);
    size_t RowBytes = static_cast<size_t>(Width) * 4;
    for (int y = 0; y < Height / 2; ++y)
    {
        uint8_t* Top = pPixels + static_cast<size_t>(y) * RowBytes;
        uint8_t* Bottom = pPixels + static_cast<size_t>(Height - 1 - y) * RowBytes;
        for (size_t x = 0; x < RowBytes; ++x)
            std::swap(Top[x], Bottom[x]);
    }
}

uint8_t* Image::LoadPNG(uint8_t* pMem, uint32_t Size, uint8_t Format)
{
    png_image png;
    memset(&png, 0, sizeof(png_image));
    png.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_memory(&png, pMem, Size))
        return nullptr;

    png.format = Format;
    uint8_t* pData = new (nothrow) uint8_t[PNG_IMAGE_SIZE(png)];
    if (!pData)
    {
        png_image_free(&png);
        return nullptr;
    }

    if (!png_image_finish_read(&png, NULL, pData, 0, NULL))
    {
        delete[] pData;
        return nullptr;
    }

    Width = png.width;
    Height = png.height;
    return pData;
}

uint8_t* Image::LoadJPEG(uint8_t* pMem, uint32_t Size)
{
    struct jpeg_decompress_struct jpeg;
    struct jpeg_error_mgr err;

    jpeg.err = jpeg_std_error(&err);
    jpeg_create_decompress(&jpeg);
    jpeg_mem_src(&jpeg, pMem, Size);
    jpeg_read_header(&jpeg, 1);
    jpeg_start_decompress(&jpeg);

    Width = jpeg.output_width;
    Height = jpeg.output_height;
    jpeg.out_color_space = JCS_EXT_RGBX;
    Format = GL_RGBA;

    uint8_t* data = new uint8_t[Width * Height * 4];
    for (int y = 0; y < Height; ++y)
    {
        uint8_t* ptr = data + Width * 4 * y;
        jpeg_read_scanlines(&jpeg, &ptr, 1);
    }

    jpeg_finish_decompress(&jpeg);
    jpeg_destroy_decompress(&jpeg);
    return data;
}
