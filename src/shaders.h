#ifndef SHADERS_H
#define SHADERS_H

struct shadersourcepair
{
    const char *vertex, *fragment;
} typedef shadersourcepair;

extern const shadersourcepair modelshaders, screenshaders;

#endif
