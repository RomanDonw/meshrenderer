#include "glmesh.h"

#include <stdlib.h>

GLMesh *glmesh_frommesh(const Mesh *mesh)
{
    GLMesh *ret = malloc(sizeof(GLMesh));
    if (!ret) return NULL;
    
    ret->surfacescount = mesh->surfacescount;
    ret->surfaces = malloc(ret->surfacescount * sizeof(GLMeshSurface));
    if (!ret->surfaces) { free(ret); return NULL; }
    
    for (size_t i = 0; i < ret->surfacescount; i++)
    {
        ret->surfaces[i].name = malloc(mesh->surfaces[i].namesize);
        if (!ret->surfaces[i].name) goto errorquit;
        memcpy(ret->surfaces[i].name, mesh->surfaces[i].name, mesh->surfaces[i].namesize);
        
        ret->surfaces[i].namesize = mesh->surfaces[i].namesize;
        ret->surfaces[i].triscount = mesh->surfaces[i].triscount;

        glGenVertexArrays(1, &ret->surfaces[i].VAO);
        glBindVertexArray(ret->surfaces[i].VAO);
        
        glGenBuffers(1, &ret->surfaces[i].VBO_VERTEX);
        glBindBuffer(GL_ARRAY_BUFFER, ret->surfaces[i].VBO_VERTEX);
        glBufferData(GL_ARRAY_BUFFER, mesh->surfaces[i].verticescount * sizeof(vec3f), mesh->surfaces[i].poss, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3f), NULL);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &ret->surfaces[i].VBO_UV);
        glBindBuffer(GL_ARRAY_BUFFER, ret->surfaces[i].VBO_UV);
        glBufferData(GL_ARRAY_BUFFER, mesh->surfaces[i].verticescount * sizeof(vec2f), mesh->surfaces[i].uvs, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vec2f), NULL);
        glEnableVertexAttribArray(1);
        
        glGenBuffers(1, &ret->surfaces[i].EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret->surfaces[i].EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->surfaces[i].triscount * sizeof(tri), mesh->surfaces[i].tris, GL_STATIC_DRAW);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    return ret;

    errorquit:
        glmesh_free(ret);
    return NULL;
}

void glmesh_free(GLMesh *mesh)
{
    if (!mesh) return;

    for (size_t i = 0; i < mesh->surfacescount; i++)
    {
        free(mesh->surfaces[i].name);

        glDeleteBuffers(1, &mesh->surfaces[i].EBO);
        glDeleteBuffers(1, &mesh->surfaces[i].VBO_UV);
        glDeleteBuffers(1, &mesh->surfaces[i].VBO_VERTEX);
        glDeleteVertexArrays(1, &mesh->surfaces[i].VAO);
    }

    free(mesh->surfaces);
    free(mesh);
}