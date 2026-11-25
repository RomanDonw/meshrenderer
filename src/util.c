#include "util.h"

#include <stdlib.h>

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