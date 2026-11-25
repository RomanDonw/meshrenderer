#ifndef UTIL_H
#define UTIL_H

#include <glad.h>

char shprog_addshader(GLuint program, GLenum type, const char *source, char ** restrict log);
char shprog_link(GLuint program, char ** restrict log);

#endif