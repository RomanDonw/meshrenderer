#include "util.h"

#include <stdlib.h>
#include <stb_image.h>

char shprog_addshader(GLuint program, GLenum type, const char *source, char ** restrict log)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        if (length <= 0) { *log = NULL; return 2; }

        if (!(*log = malloc(length))) return 1;
        glGetShaderInfoLog(shader, length, NULL, *log);
        return 1;
    }

    glAttachShader(program, shader);
    return 0;
}

char shprog_link(GLuint program, char ** restrict log)
{
    glLinkProgram(program);
    
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status)
    {
        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        if (length > 0)
        {
            if (!(*log = malloc(length))) return 1;
            glGetProgramInfoLog(program, length, NULL, *log);
            return 1;
        }
        *log = NULL;
        return 2;
    }

    return 0;
}

char loadGLtexture(GLuint *texture, const char *file, bool genmipmap)
{
    int x, y, ch;
    void *data = stbi_load(file, &x, &y, &ch, 0);
    if (!data) return 1;

    if (x <= 0 || y <= 0 || ch > 4 || ch < 3) { stbi_image_free(data); return 1; }

    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

    glTexImage2D(GL_TEXTURE_2D, 0, ch == 4 ? GL_RGBA8 : GL_RGB8, x, y, 0, ch == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    
    if (genmipmap) glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, genmipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);
    return 0;
}

void fb_init(Framebuffer * restrict fb, unsigned int width, unsigned int height, bool depthrendbuff, unsigned int MSAAsamples)
{
    glGenFramebuffers(1, &fb->fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb->fb);

    if (MSAAsamples)
    {
        glGenTextures(1, &fb->texcolor);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fb->texcolor);

        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAAsamples, GL_RGB32F, width, height, GL_TRUE);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, fb->texcolor, 0);

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    }
    else
    {
        glGenTextures(1, &fb->texcolor);
        glBindTexture(GL_TEXTURE_2D, fb->texcolor);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb->texcolor, 0);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (depthrendbuff)
    {
        glGenRenderbuffers(1, &fb->rbdepth);
        glBindRenderbuffer(GL_RENDERBUFFER, fb->rbdepth);

        if (MSAAsamples) glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAAsamples, GL_DEPTH_COMPONENT32F, width, height);
        else glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fb->rbdepth);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
    else fb->rbdepth = 0;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    fb->MSAAsamples = MSAAsamples;
}

bool fb_checkcomplete(const Framebuffer * restrict fb)
{ return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE; }

void fb_resize(const Framebuffer * restrict fb, unsigned int width, unsigned int height)
{
    if (fb->MSAAsamples)
    {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fb->texcolor);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, fb->MSAAsamples, GL_RGB32F, width, height, GL_TRUE);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, fb->texcolor);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (fb->rbdepth)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, fb->rbdepth);
        if (fb->MSAAsamples) glRenderbufferStorageMultisample(GL_RENDERBUFFER, fb->MSAAsamples, GL_DEPTH_COMPONENT32F, width, height);
        else glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
}

void fb_free(Framebuffer * restrict fb)
{
    if (fb->rbdepth) glDeleteRenderbuffers(1, &fb->rbdepth);
    glDeleteTextures(1, &fb->texcolor);
    glDeleteFramebuffers(1, &fb->fb);
}