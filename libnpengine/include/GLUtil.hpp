#ifndef GL_UTIL_HPP
#define GL_UTIL_HPP

#ifdef NP_GLES2
#include <GLES2/gl2.h>
#else
#include <GL/glew.h>
#endif

void GLInit(int Width, int Height);
void GLInitViewport(int SurfaceW, int SurfaceH);
// Letterbox mapping between SDL window pixels and game coordinates.
void GLSurfaceToGame(int& X, int& Y);
void GLGameToSurface(int& X, int& Y);
void GLDrawQuad(GLuint TextureID, const float* Xs, const float* Ys);
void GLSetColor(float R, float G, float B, float A);

// Builds a draw program from a fragment shader source. Attaches the
// shared quad vertex shader so every program has a vertex stage.
// gl_TexCoord[0].xy in the source is rewritten to the vUV varying.
GLuint GLMakeProgram(const char* FragSrc);

// Switches the ortho projection to an FBO-sized bottom-up space.
void GLSetOrtho(int Width, int Height);
void GLResetOrtho();

#endif
