/*
 * Minimal shader-based 2D renderer replacing the fixed-function pipeline.
 * Runs on desktop GL (GLSL 110) and OpenGL ES 2.0 (GLSL 100).
 * */
#include "GLUtil.hpp"
#include <cstdio>
#include <cstring>
#include <string>

namespace
{
GLuint ProgramID = 0;
GLuint SharedVS = 0;
GLint LocMVP = -1, LocColor = -1, LocTex = -1;
GLint LocPos = -1, LocUV = -1;
GLuint VBOTexCoords = 0;
float CurColor[4] = {1, 1, 1, 1};
int WinW = 0, WinH = 0;
float ViewScale = 1.0f;
int ViewOffX = 0, ViewOffY = 0;

const char* VS =
#ifdef NP_GLES2
    "#version 100\n"
#else
    "#version 110\n"
#endif
    "attribute vec2 aPos;\n"
    "attribute vec2 aUV;\n"
    "uniform mat4 uMVP;\n"
    "varying vec2 vUV;\n"
    "void main()\n"
    "{\n"
    "    vUV = aUV;\n"
    "    gl_Position = uMVP * vec4(aPos, 0.0, 1.0);\n"
    "}\n";

const char* DefaultFS =
#ifdef NP_GLES2
    "#version 100\n"
    "precision mediump float;\n"
#else
    "#version 110\n"
#endif
    "varying vec2 vUV;\n"
    "uniform sampler2D uTex;\n"
    "uniform vec4 uColor;\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = texture2D(uTex, vUV) * uColor;\n"
    "}\n";

GLuint CompileShader(GLenum Type, const char* Src)
{
    GLuint Shader = glCreateShader(Type);
    glShaderSource(Shader, 1, &Src, nullptr);
    glCompileShader(Shader);
    GLint Status = GL_FALSE;
    glGetShaderiv(Shader, GL_COMPILE_STATUS, &Status);
    if (Status != GL_TRUE)
    {
        char Log[1024];
        glGetShaderInfoLog(Shader, sizeof(Log), nullptr, Log);
        fprintf(stderr, "[gl] shader error: %s\n", Log);
    }
    return Shader;
}

GLuint LinkProgram(GLuint VSID, GLuint FSID)
{
    GLuint Prog = glCreateProgram();
    glAttachShader(Prog, VSID);
    glAttachShader(Prog, FSID);
    // Attribute indices are fixed so all programs share the same VBO layout.
    glBindAttribLocation(Prog, 0, "aPos");
    glBindAttribLocation(Prog, 1, "aUV");
    glLinkProgram(Prog);
    GLint Status = GL_FALSE;
    glGetProgramiv(Prog, GL_LINK_STATUS, &Status);
    if (Status != GL_TRUE)
    {
        char Log[512];
        glGetProgramInfoLog(Prog, sizeof(Log), nullptr, Log);
        fprintf(stderr, "[gl] link error: %s\n", Log);
    }
    return Prog;
}

float Ortho[16] =
{
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

void ComputeOrtho(int Width, int Height, bool TopDown)
{
    memset(Ortho, 0, sizeof(Ortho));
    Ortho[0]  = 2.0f / Width;
    Ortho[5]  = TopDown ? -2.0f / Height : 2.0f / Height;
    Ortho[10] = -1.0f;
    Ortho[12] = -1.0f;
    Ortho[13] = TopDown ? 1.0f : -1.0f;
    Ortho[15] = 1.0f;
}
}

void GLInit(int Width, int Height)
{
    WinW = Width;
    WinH = Height;

    SharedVS = CompileShader(GL_VERTEX_SHADER, VS);
    GLuint FSID = CompileShader(GL_FRAGMENT_SHADER, DefaultFS);
    ProgramID = LinkProgram(SharedVS, FSID);
    glDeleteShader(FSID);

    LocMVP = glGetUniformLocation(ProgramID, "uMVP");
    LocColor = glGetUniformLocation(ProgramID, "uColor");
    LocTex = glGetUniformLocation(ProgramID, "uTex");
    LocPos = glGetAttribLocation(ProgramID, "aPos");
    LocUV = glGetAttribLocation(ProgramID, "aUV");

    glGenBuffers(1, &VBOTexCoords);
    glBindBuffer(GL_ARRAY_BUFFER, VBOTexCoords);
    static const float UVs[8] = {0, 0, 1, 0, 1, 1, 0, 1};
    glBufferData(GL_ARRAY_BUFFER, sizeof(UVs), UVs, GL_STATIC_DRAW);

    glUseProgram(ProgramID);
    glUniform1i(LocTex, 0);

    ComputeOrtho(Width, Height, true);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(ViewOffX, ViewOffY, (int)(Width * ViewScale), (int)(Height * ViewScale));
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void GLSetColor(float R, float G, float B, float A)
{
    CurColor[0] = R; CurColor[1] = G; CurColor[2] = B; CurColor[3] = A;
}

void GLDrawQuad(GLuint TextureID, const float* Xs, const float* Ys)
{
    float Verts[8] = { Xs[0], Ys[0], Xs[1], Ys[1], Xs[2], Ys[2], Xs[3], Ys[3] };

    // Draw with whatever program is active (effects install their own);
    // fall back to the default program when none is bound.
    GLuint Prog = ProgramID;
    GLint Bound = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &Bound);
    if (Bound)
        Prog = (GLuint)Bound;
    else
        glUseProgram(Prog);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureID);

    GLint Loc = glGetUniformLocation(Prog, "uMVP");
    if (Loc >= 0)
        glUniformMatrix4fv(Loc, 1, GL_FALSE, Ortho);
    Loc = glGetUniformLocation(Prog, "uColor");
    if (Loc >= 0)
        glUniform4f(Loc, CurColor[0], CurColor[1], CurColor[2], CurColor[3]);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, Verts);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, VBOTexCoords);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

GLuint GLMakeProgram(const char* FragSrc)
{
#ifdef NP_GLES2
    std::string Fixed = "#version 100\nprecision mediump float;\nvarying vec2 vUV;\n";
#else
    std::string Fixed = "#version 120\nvarying vec2 vUV;\n";
#endif
    std::string Src(FragSrc);
    size_t pos;
    while ((pos = Src.find("gl_TexCoord[0].xy")) != std::string::npos)
        Src.replace(pos, 17, "vUV");
    Fixed += Src;
    GLuint FSID = CompileShader(GL_FRAGMENT_SHADER, Fixed.c_str());
    GLuint Prog = LinkProgram(SharedVS, FSID);
    glDeleteShader(FSID);
    return Prog;
}

void GLSetOrtho(int Width, int Height)
{
    glViewport(0, 0, Width, Height);
    ComputeOrtho(Width, Height, false);
}

void GLResetOrtho()
{
    glViewport(ViewOffX, ViewOffY, (int)(WinW * ViewScale), (int)(WinH * ViewScale));
    ComputeOrtho(WinW, WinH, true);
}

void GLInitViewport(int SurfaceW, int SurfaceH)
{
    if (!WinW || !WinH || SurfaceW <= 0 || SurfaceH <= 0)
        return;
    float sx = (float)SurfaceW / WinW, sy = (float)SurfaceH / WinH;
    ViewScale = sx < sy ? sx : sy;
    ViewOffX = (int)((SurfaceW - WinW * ViewScale) / 2);
    ViewOffY = (int)((SurfaceH - WinH * ViewScale) / 2);
    glViewport(ViewOffX, ViewOffY, (int)(WinW * ViewScale), (int)(WinH * ViewScale));
}

void GLSurfaceToGame(int& X, int& Y)
{
    X = (int)((X - ViewOffX) / ViewScale);
    Y = (int)((Y - ViewOffY) / ViewScale);
    if (X < 0) X = 0; else if (X >= WinW) X = WinW - 1;
    if (Y < 0) Y = 0; else if (Y >= WinH) Y = WinH - 1;
}

void GLGameToSurface(int& X, int& Y)
{
    X = (int)(X * ViewScale) + ViewOffX;
    Y = (int)(Y * ViewScale) + ViewOffY;
}
