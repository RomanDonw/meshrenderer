#ifndef GLMESH_H
#define GLMESH_H

#include <stddef.h>
#include <glad.h>

#include "mesh.h"

struct GLMeshSurface
{
    size_t namesize;
    unsigned char *name; // not zero-terminated string.

    GLuint VAO, VBO_VERTEX, VBO_UV, EBO;
    size_t triscount;
} typedef GLMeshSurface;

struct GLMesh
{
    GLMeshSurface *surfaces;
    size_t surfacescount;
} typedef GLMesh;

GLMesh *glmesh_frommesh(const Mesh *mesh);
void glmesh_free(GLMesh *mesh);

#endif