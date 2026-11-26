#ifndef UTIL_H
#define UTIL_H

#include <glad.h>

#define PI 3.14159265358979323846
#define RAD(x) (((x) / 180.0) * PI)

char shprog_addshader(GLuint program, GLenum type, const char *source, char ** restrict log);
char shprog_link(GLuint program, char ** restrict log);

#endif