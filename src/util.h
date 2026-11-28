#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <glad.h>

#define PI 3.14159265358979323846
#define RAD(x) (((x) / 180.0) * PI)
#define GLPRINTERROR() (printf("OpenGL error %u at %llu:%s in function %s\n", glGetError(), __LINE__, __FILE__, __func__))

char shprog_addshader(GLuint program, GLenum type, const char *source, char ** restrict log);
char shprog_link(GLuint program, char ** restrict log);

// returns 0 on success, non-zero otherwise.
char loadGLtexture(GLuint *texture, const char *file, bool genmipmap);

struct Framebuffer
{
    // === [READ ONLY FIELDS] ===
    GLuint fb, texcolor, rbdepth;
    unsigned int MSAAsamples;
} typedef Framebuffer;

void fb_init(Framebuffer * restrict fb, unsigned int width, unsigned int height, bool depthrendbuff, unsigned int MSAAsamples);
bool fb_checkcomplete(const Framebuffer * restrict fb);
void fb_resize(const Framebuffer * restrict fb, unsigned int width, unsigned int height);
void fb_free(Framebuffer * restrict fb);

#endif