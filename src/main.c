#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stb_image.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <ctype.h>
#include <time.h>

#include "vec2.h"
#include "vec3.h"
#include "mesh.h"
#include "glmesh.h"
#include "mat4.h"
#include "shaders.h"
#include "util.h"

static char loadGLtexture(GLuint *texture, const char *file);

#define GLDEBUG() (printf("OpenGL error %u at %llu:%s in function %s\n", glGetError(), __LINE__, __FILE__, __func__))

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

static GLint u_proj = -1, modelprog = 0;
static struct
{
    unsigned int width, height;
    GLuint texcolor, rbdepth;
} ppfb = {0}; // frame buffer related data for post-processing.
static unsigned int winwidth = 1, winheight = 1;
static void onresize(GLFWwindow *window, int width, int height)
{
    winwidth = width > 0 ? width : 1;
    winheight = height > 0 ? height : 1;

    if (u_proj >= 0 && modelprog)
    {
        glUseProgram(modelprog);
        mat4f projm;
        mat4f_perspective(projm, RAD(45), (float)winwidth / (float)winheight, 0.01, 1000);
        glUniformMatrix4fv(u_proj, 1, GL_TRUE, projm);
    }

    if (!ppfb.width || !ppfb.height)
    {
        if (ppfb.texcolor)
        {
            glBindTexture(GL_TEXTURE_2D, ppfb.texcolor);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, winwidth, winheight, 0, GL_RGB, GL_FLOAT, NULL);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        if (ppfb.rbdepth)
        {
            glBindRenderbuffer(GL_RENDERBUFFER, ppfb.rbdepth);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, winwidth, winheight);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }
    }
}

#define DEFAULT_FLY_SPEED 2.0
static float flyspeed = DEFAULT_FLY_SPEED;
static void onscroll(GLFWwindow *w, double x, double y)
{
    float s = flyspeed + y / 5;
    flyspeed = s > 0 ? s : 0;
}

static const char *memallocfailstr = "memory allocation failed";

int main(int argc, char *argv[])
{
    const char *meshpath = NULL, *texspath = NULL;
    bool downscale = false, flipv = false;
    unsigned int noisetexwidth = 800, noisetexheight = 600;
    {
        static const struct option longopts[] =
        {
            { .name = "framebufferSize", .has_arg = required_argument, .val = 256, .flag = NULL },
            { .name = "noiseTextureSize", .has_arg = required_argument, .val = 257, .flag = NULL },
            {0}
        };
        int p;
        while ((p = getopt_long(argc, argv, "m:dt:vf:", longopts, NULL)) != -1)
        {
            switch (p)
            {
                case 'm':
                    meshpath = optarg;
                    break;

                case 'd':
                    downscale = true;
                    break;

                case 't':
                    texspath = optarg;
                    break;

                case 'v':
                    flipv = true;
                    break;

                case 256:
                    if (sscanf(optarg, "%u,%u", &ppfb.width, &ppfb.height) < 2)
                    { puts("failed parsing option --framebufferSize"); return 1; }
                    break;

                case 257:
                    if (sscanf(optarg, "%u,%u", &noisetexwidth, &noisetexheight) < 2)
                    { puts("failed parsing option --noiseTextureSize"); return 1; }
                    break;
            }
        }
    }

    if (!meshpath)
    {
        puts("specify path to mesh through -m parameter");
        return 1;
    }

    if (!texspath)
    {
        puts("specify path to textures folder through -t parameter");
        return 1;
    }

    if (!noisetexwidth || !noisetexheight)
    {
        puts("noise texture size must be positive non-zero value");
        return 1;
    }

    // ==========================================================================================

    if (!glfwInit()) { puts("failed to initialize GLFW"); return 1; }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *w;
    if (!(w = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Mesh renderer", NULL, NULL))) { puts("error creating window"); goto errorquit_glfw; }
    glfwMakeContextCurrent(w);

    if (!gladLoadGL(glfwGetProcAddress)) { puts("error loading OpenGL context through GLAD"); goto errorquit_glfw;  }
    glfwSetScrollCallback(w, onscroll);

    // ==========================================================================================

    char *log;
    modelprog = glCreateProgram();
    if (shprog_addshader(modelprog, GL_VERTEX_SHADER, modelshaders.vertex, &log))
    {
        if (log) { printf("failed compiling vertex shader for model shader program:\n%s\n", log); free(log); }
        else puts(memallocfailstr);
        goto errorquit_glfw;
    }
    if (shprog_addshader(modelprog, GL_FRAGMENT_SHADER, modelshaders.fragment, &log))
    {
        if (log) { printf("failed compiling fragment shader for model shader program:\n%s\n", log); free(log); }
        else puts(memallocfailstr);
        goto errorquit_glfw;
    }
    if (shprog_link(modelprog, &log))
    {
        if (log) { printf("failed linking model shader program:\n%s\n", log); free(log); }
        else puts(memallocfailstr);
        goto errorquit_glfw;
    }

    glUseProgram(modelprog);

    u_proj = glGetUniformLocation(modelprog, "proj");
    GLint u_view = glGetUniformLocation(modelprog, "view");

    glUniform1ui(glGetUniformLocation(modelprog, "downscale"), downscale);
    glUniform1ui(glGetUniformLocation(modelprog, "flipv"), flipv);
    glUniform1i(glGetUniformLocation(modelprog, "albedo"), 0);

    // ==========================================================================================

    GLuint screenprog = glCreateProgram();
    if (shprog_addshader(screenprog, GL_VERTEX_SHADER, screenshaders.vertex, &log))
    {
        if (log) { printf("failed compiling vertex shader for screen shader program:\n%s\n", log); free(log); }
        else puts(memallocfailstr);
        goto errorquit_glfw;
    }
    if (shprog_addshader(screenprog, GL_FRAGMENT_SHADER, screenshaders.fragment, &log))
    {
        if (log) { printf("failed compiling fragment shader for screen shader program:\n%s\n", log); free(log); }
        else puts(memallocfailstr);
        goto errorquit_glfw;
    }
    if (shprog_link(screenprog, &log))
    {
        if (log) { printf("failed linking screen shader program:\n%s\n", log); free(log); }
        else puts(memallocfailstr);
        goto errorquit_glfw;
    }

    glUseProgram(screenprog);

    GLint u_time = glGetUniformLocation(screenprog, "time");

    glUniform1i(glGetUniformLocation(screenprog, "screen"), 0);
    glUniform1i(glGetUniformLocation(screenprog, "noise"), 1);

    // ==========================================================================================

    Mesh *mesh = loadmesh(meshpath);
    if (!mesh) { puts("error loading mesh"); goto errorquit_glfw; }

    GLMesh *glmesh = glmesh_frommesh(mesh);
    freemesh(mesh);
    if (!glmesh) { puts("error creating (translating) OpenGL mesh from generic mesh"); goto errorquit_glfw; }

    // ==========================================================================================

    GLuint *texs = malloc(glmesh->surfacescount * sizeof(GLuint));
    if (!texs) { puts(memallocfailstr); glmesh_free(glmesh); goto errorquit_glfw; }
    memset(texs, 0, glmesh->surfacescount * sizeof(GLuint));

    {
        size_t texspathlen = strlen(texspath);
        size_t texpathsize = texspathlen + 256 + 4;
        char *texpath = malloc(texpathsize);
        if (!texpath) { puts(memallocfailstr); glmesh_free(glmesh); goto errorquit_glfw; }

        memcpy(texpath, texspath, texspathlen);

        for (size_t i = 0; i < glmesh->surfacescount; i++)
        {
            size_t namesize = glmesh->surfaces[i].namesize > 255 ? 255 : glmesh->surfaces[i].namesize;
            for (size_t j = 0; j < namesize; j++)
            {
                *(texpath + texspathlen + j) = /*tolower*/(*(glmesh->surfaces[i].name + j));
            }
            
            memcpy(texpath + texspathlen + namesize, ".png", 4);
            *(texpath + texspathlen + namesize + 4) = 0;

            if (loadGLtexture(texs + i, texpath)) printf("failed to load texture %s\n", texpath);
        }

        free(texpath);
    }

    // ==========================================================================================

    GLuint tex;
    if (loadGLtexture(&tex, "res/C3A2A_TRANS1.png"))
    {
        puts("failed loading texture res/C3A2A_TRANS1.png");
        free(texs);
        glmesh_free(glmesh);
        goto errorquit_glfw;
    }

    // ==========================================================================================

    #if 0
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    static const float vertices[] =
    {
        -0.5, -0.5, -5,
        0, 0.5, -5,
        0.5, -0.5, -5
    };
    static const int indices[] =
    {
        0, 1, 2
    };

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, NULL);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    #endif

    struct
    {
        GLuint VAO, VBO, EBO;
    } ndcquad;
    {
        glGenVertexArrays(1, &ndcquad.VAO);
        glBindVertexArray(ndcquad.VAO);
        
        static const float verts[] =
        {
            -1, -1,
            -1, 1,
            1, -1,
            1, 1
        };
        glGenBuffers(1, &ndcquad.VBO);
        glBindBuffer(GL_ARRAY_BUFFER, ndcquad.VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
        glEnableVertexAttribArray(0);

        static const unsigned int inds[] =
        {
            3, 1, 0,
            0, 2, 3
        };
        glGenBuffers(1, &ndcquad.EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ndcquad.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(inds), inds, GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    // ==========================================================================================

    float *noisetexdata = malloc(noisetexwidth * noisetexheight * sizeof(float));
    if (!noisetexdata) { puts(memallocfailstr); goto errorquit_glfw; }

    GLuint noisetex;
    glGenTextures(1, &noisetex);
    glBindTexture(GL_TEXTURE_2D, noisetex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, noisetexwidth, noisetexheight, 0, GL_RED, GL_FLOAT, noisetexdata);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // ==========================================================================================

    GLuint fb;
    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    glGenTextures(1, &ppfb.texcolor);
    glBindTexture(GL_TEXTURE_2D, ppfb.texcolor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, ppfb.width ? ppfb.width : 1, ppfb.height ? ppfb.height : 1, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ppfb.texcolor, 0);

    glGenRenderbuffers(1, &ppfb.rbdepth);
    glBindRenderbuffer(GL_RENDERBUFFER, ppfb.rbdepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, ppfb.width ? ppfb.width : 1, ppfb.height ? ppfb.height : 1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ppfb.rbdepth);

    printf("Framebuffer complete status: %s.\n", glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE ? "yes" : "no");

    // ==========================================================================================
    {
        int width, height;
        glfwGetFramebufferSize(w, &width, &height);

        glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPos(w, width / 2.0, height / 2.0);

        glfwSetFramebufferSizeCallback(w, onresize);
        onresize(w, width, height);
    }

    if (ppfb.width && ppfb.height) printf("Framebuffer size (WxH): %ux%u.\n", ppfb.width, ppfb.height);
    else puts("Framebuffer size (WxH): (aligned to window).");
    printf("Noise texture size (WxH): %ux%u\n", noisetexwidth, noisetexheight);

    srand(time(NULL));
    glClearColor(0, 0, 0, 1);
    glCullFace(GL_BACK);
    double lastframetime = 0, lastmousex = winwidth / 2.0, lastmousey = winheight / 2.0;
    glfwSetTime(0);
    while (!glfwWindowShouldClose(w))
    {
        double currentframetime = glfwGetTime();
        double deltatime = currentframetime - lastframetime;
        lastframetime = currentframetime;

        glUseProgram(modelprog);
        
        {
            static vec3f campos = {0, 0, 0};
            static vec2f camrot = {0, 0};

            {
                double x, y;
                glfwGetCursorPos(w, &x, &y);
                double deltax = x - lastmousex;
                double deltay = y - lastmousey;
                lastmousex = x;
                lastmousey = y;

                camrot[1] += deltax * 0.005;
                camrot[0] += deltay * 0.005;

                camrot[1] = fmodf(camrot[1], 2 * PI);
                camrot[0] = fmodf(camrot[0], 2 * PI);

                if (camrot[0] > RAD(80)) camrot[0] = RAD(80);
                else if (camrot[0] < -RAD(80)) camrot[0] = -RAD(80);
            }

            mat4f mat;
            {
                mat4f_rotatey(mat, camrot[1]);
                mat4f t;
                mat4f_rotatex(t, camrot[0]);
                mat4f_mulm2(mat, t);
            }

            vec4f front = {0, 0, -1, 0}, right = {1, 0, 0, 0}, up = {0, 1, 0, 0};
            mat4f_mulv2(front, mat);
            mat4f_mulv2(right, mat);
            mat4f_mulv2(up, mat);

            vec3f_norm2(front);
            vec3f_norm2(right);
            vec3f_norm2(up);

            {
                if (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) flyspeed = DEFAULT_FLY_SPEED;

                float speedfactor;
                if (glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) speedfactor = 3;
                else if (glfwGetKey(w, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) speedfactor = 0.3;
                else speedfactor = 1;

                float factor = 5 * deltatime * speedfactor * flyspeed;

                static const vec3f worldup = {0, 1, 0};
                vec3f vec3;
                if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS) { vec3f_muls(vec3, front, factor); vec3f_sub2(campos, vec3); }
                if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS) { vec3f_muls(vec3, front, factor); vec3f_add2(campos, vec3); }
                if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS) { vec3f_muls(vec3, right, factor); vec3f_sub2(campos, vec3); }
                if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS) { vec3f_muls(vec3, right, factor); vec3f_add2(campos, vec3); }
                if (glfwGetKey(w, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) { vec3f_muls(vec3, worldup, factor); vec3f_sub2(campos, vec3); }
                if (glfwGetKey(w, GLFW_KEY_SPACE) == GLFW_PRESS) { vec3f_muls(vec3, worldup, factor); vec3f_add2(campos, vec3); }
            }

            mat4f_lookat(mat, campos, right, up, front);
            glUniformMatrix4fv(u_view, 1, GL_TRUE, mat);
        }

        // ==========================================================================================

        glBindFramebuffer(GL_FRAMEBUFFER, fb);
        glDisable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, ppfb.width ? ppfb.width : winwidth, ppfb.height ? ppfb.height : winheight);

        for (size_t i = 0; i < glmesh->surfacescount; i++)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texs[i] ? texs[i] : tex);

            glBindVertexArray(glmesh->surfaces[i].VAO);
            glDrawElements(GL_TRIANGLES, glmesh->surfaces[i].triscount * 3, GL_UNSIGNED_INT, NULL);
        }

        // ======================================================

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(screenprog);
        glEnable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glViewport(0, 0, winwidth, winheight);

        glUniform1f(u_time, currentframetime);

        for (size_t j = 0; j < noisetexheight; j++)
            for (size_t i = 0; i < noisetexwidth; i++)
                noisetexdata[j * noisetexwidth + i] = rand() / (float)RAND_MAX;
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, noisetex);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, noisetexwidth, noisetexheight, GL_RED, GL_FLOAT, noisetexdata);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ppfb.texcolor);

        glBindVertexArray(ndcquad.VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

        // ======================================================

        glfwSwapBuffers(w);
        glfwPollEvents();
    }

    glmesh_free(glmesh);

    glfwTerminate();
    return 0;
    
    errorquit_glfw:
        glfwTerminate();
    return 1;
}

static char loadGLtexture(GLuint *texture, const char *file)
{
    int x, y, ch;
    void *data = stbi_load(file, &x, &y, &ch, 0);
    if (!data) return 1;

    if (x <= 0 || y <= 0 || ch > 4 || ch < 3) { stbi_image_free(data); return 1; }

    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

    glTexImage2D(GL_TEXTURE_2D, 0, ch == 4 ? GL_RGBA8 : GL_RGB8, x, y, 0, ch == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);
    return 0;
}