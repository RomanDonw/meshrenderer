#include "mesh.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

Mesh *loadmesh(const char *file)
{
    FILE *f = fopen(file, "rb");
    if (!f) return NULL;

    char sig[4];
    if (fread(sig, sizeof(sig), 1, f) < 1) goto errorquit_afteropenfile;
    if (memcmp(sig, "MESH", 4)) goto errorquit_afteropenfile;

    uint16_t ver;
    if (fread(&ver, sizeof(ver), 1, f) < 1) goto errorquit_afteropenfile;
    if (ver != 1) goto errorquit_afteropenfile;
    
    uint16_t surfcountminusone;
    if (fread(&surfcountminusone, sizeof(surfcountminusone), 1, f) < 1) goto errorquit_afteropenfile;

    // ================================

    Mesh *ret = malloc(sizeof(Mesh));
    if (!ret) goto errorquit_afteropenfile;
    memset(ret, 0, sizeof(Mesh));
    
    for (; ret->surfacescount < (size_t)surfcountminusone + 1; ret->surfacescount++)
    {
        uint8_t namesize;
        if (fread(&namesize, sizeof(namesize), 1, f) < 1) goto errorquit_afterallocmesh;
        void *name = NULL;
        if (namesize)
        {
            if (!(name = malloc(namesize))) goto errorquit_afterallocmesh;
            if (fread(name, namesize, 1, f) < 1) goto errorquit_afterallocname;
        }

        size_t objscount;
        {
            uint32_t objcountminusone;
            if (fread(&objcountminusone, sizeof(objcountminusone), 1, f) < 1) goto errorquit_afterallocname;
            objscount = (size_t)objcountminusone + 1;
        }

        vec3f *poss = malloc(objscount * sizeof(vec3f));
        if (!poss) goto errorquit_afterallocname;
        vec2f *uvs = malloc(objscount * sizeof(vec2f));
        if (!uvs) goto errorquit_afterallocposs;

        for (size_t i = 0; i < objscount; i++)
        {
            if (fread(poss + i, sizeof(vec3f), 1, f) < 1 ||
                fread(uvs + i, sizeof(vec2f), 1, f) < 1) goto errorquit_afterallocuvs;
        }

        // ================================

        size_t triscount;
        {
            uint32_t triscountminusone;
            if (fread(&triscountminusone, sizeof(triscountminusone), 1, f) < 1) goto errorquit_afterallocuvs;
            triscount = (size_t)triscountminusone + 1;
        }
        
        tri *tris = malloc(triscount * sizeof(tri));
        if (!tris) goto errorquit_afterallocuvs;

        for (size_t i = 0; i < triscount; i++) if (fread(tris + i, sizeof(tri), 1, f) < 1) goto errorquit_afteralloctris;

        {
            void *newptr = realloc(ret->surfaces, (ret->surfacescount + 1) * sizeof(Surface));
            if (!newptr) goto errorquit_afteralloctris;
            ret->surfaces = newptr;
        }

        // ================================

        Surface *s = ret->surfaces + ret->surfacescount;
        s->name = name;
        s->namesize = namesize;
        s->poss = poss;
        s->uvs = uvs;
        s->tris = tris;
        s->verticescount = objscount;
        s->triscount = triscount;

        continue;
        // ================================
        errorquit_afteralloctris:
            free(tris);
        errorquit_afterallocuvs:
            free(uvs);
        errorquit_afterallocposs:
            free(poss);
        errorquit_afterallocname:
            free(name);
        goto errorquit_afterallocmesh;    
    }

    return ret;

    errorquit_afterallocmesh:
        freemesh(ret);
    errorquit_afteropenfile:
        fclose(f);
    return NULL;
}

void freemesh(Mesh *mesh)
{
    if (!mesh) return;

    for (size_t i = 0; i < mesh->surfacescount; i++)
    {
        free(mesh->surfaces[i].name);
        free(mesh->surfaces[i].poss);
        free(mesh->surfaces[i].uvs);
        free(mesh->surfaces[i].tris);
    }
    
    free(mesh->surfaces);
    free(mesh);
}