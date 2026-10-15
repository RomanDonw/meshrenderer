#ifndef MESH_H
#define MESH_H

#include <stddef.h>

#include "vec2.h"
#include "vec3.h"

typedef unsigned int tri[3];

struct Surface
{
    size_t namesize;
    unsigned char *name; // not zero-terminated string.

    vec3f *poss;
    vec2f *uvs;
    size_t verticescount;

    tri *tris;
    size_t triscount;
} typedef Surface;

struct Mesh
{
    Surface *surfaces;
    size_t surfacescount;
} typedef Mesh;

// allocated in the heap, requires freeing. returns NULL on error.
Mesh *loadmesh(const char *file);
void freemesh(Mesh *mesh);

#endif