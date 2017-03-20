/*
    GpuWrapper.h nachocpol@gmail.com
*/

#ifndef GPU_WRAPPER
#define GPU_WRAPPER

#include <tygra/Image.hpp>
#include <scene/scene_fwd.hpp>
#include <tygra/WindowViewDelegate.hpp>
#include <tgl/tgl.h>
#include <tsl/shapes.hpp>
#include <glm/glm.hpp>

#include <vector>
#include <memory>

namespace gpu
{
    // 5 directional + 20 point + 5 spot
    static const unsigned int k_MaxLights = 30;
    static const unsigned int k_MaxDirect = 5;
    static const unsigned int k_MaxPoint = 20;
    static const unsigned int k_MaxSpot = 5;

    class GpuMaterial
    {
    public:
        GpuMaterial ();
        ~GpuMaterial();
        void Use    ();

        GLuint id;
    };

    class GpuMesh
    {
    public:
        GpuMesh     ();
        ~GpuMesh    ();
        void Init   (scene::Mesh curMesh);
        void Init   (float r,int sub);
		void Init	(float r,float h,int s);
        void Draw   ();

        GLuint id;
        unsigned int meshId; // id used by scene
        std::vector<GLuint> indices;
    };

    class GpuFrameBuffer
    {
    public:
        GpuFrameBuffer  ();
        ~GpuFrameBuffer ();
        void Init       (glm::vec2 sSize);
        void Resize     (glm::vec2 sSize);
        void Enable     ();
        void Disable    ();
        void Draw       ();

        GLuint id;
        GLuint colorTexture;
        GLuint depthTexture;
        gpu::GpuMaterial fboMat;
        GpuMesh rectanleMesh;
    };

    class GpuInstancedMesh
    {
    public:
        GpuInstancedMesh();
        ~GpuInstancedMesh();
        void Init(std::vector<glm::mat4>& iM,std::vector<glm::vec4> &iCol,GpuMesh m);
        void Draw();

        GpuMesh mesh;
        GLuint modelsId;
        GLuint colorsId;
        GLuint iCount;
    };

    class GpuTexture
    {
    public:
        GpuTexture();
        ~GpuTexture();
        void Init(const char* path);

        GLuint id;
    };

    class GpuTextureCube
    {
    public:
        GpuTextureCube();
        ~GpuTextureCube();
        void Init(std::vector<const char*>& files);

        GLuint id;
    };

    class GpuGBuffer
    {
    public:
        GpuGBuffer();
        ~GpuGBuffer();
        void Init(glm::vec2 screenS);
        void Resize(glm::vec2 screenS);
        void DrawDebug();
        void Enable();
        void Disable();

        GLuint id =  -1;
        GLuint pId = -1;    // positions
        GLuint nId = -1;    // normals
        GLuint aId = -1;    // albedo
        GLuint dId = -1;    // depth/stencil

        GpuMaterial rectMat;
        GpuMesh pRect;
        GpuMesh nRect;
        GpuMesh aRect;
    };

    class GpuShadowMap
    {
    public:
        GpuShadowMap();
        ~GpuShadowMap();
        void Init(glm::vec2 size);
        void Resize(glm::vec2 size);
        glm::vec2 s;
        GLuint id;
        GLuint DepthId;
        unsigned int LightId = -1;
    };

    void InitMaterial(GpuMaterial* mat, const char* vsPath, const char* fsPath);
    
    //rect:bl,br,tr,tl
    //full screen: -1,-1  1,-1  1,1  -1,1
    int InitScreenRect(std::vector<glm::vec2> rect,std::vector<GLuint>* outIndex);
    
    glm::vec3 ToGlmVec3(scene::Vector3& in);

    // Generates a texture cubemap object (to-do:bool to
    // add mipmaps or not).
    unsigned int CreateCubeMap(std::vector<const char*>& files);

    static const std::vector<glm::vec2> fullScreenRect =
    {
        { -1.0f,-1.0f },    //bl
        {  1.0f,-1.0f },    //br
        {  1.0f, 1.0f },    //tr
        { -1.0f, 1.0f },    //tl
    };

    struct GpuLightPass
    {
        void Enable()
        {
            glClear(GL_COLOR_BUFFER_BIT);
            glDepthMask(GL_FALSE);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_ONE, GL_ONE);
        }

        void Disable()
        {
            glDepthMask(GL_TRUE);
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
        }
    };

    enum LightType
    {
        kDefault = 0,
        kDirectional = 1,
        kPoint = 2,
        kSpot = 3
    };

    struct Light
    {
        Light()
        {
            position = glm::vec3(0.0f);
            direction = glm::vec3(0.0f);
            color = glm::vec3(0.0f);
        }
        glm::vec3 position;
        glm::vec3 direction;
        glm::vec3 color;
        float angle = 0.0;
        float range = 0.0f;
        int type = 0;
        bool castShadow = false;
    };

    struct SSAO
    {
        std::vector<glm::vec3> Kernel;
        std::vector<glm::vec3> Noise;
        void Init();
        float Lerp(float a,float b,float f);

        GLuint NoiseTexId = -1;
    };

    struct Camera
    {
        glm::vec3 Position;
        float FovRad = 0.0f;
        float Near = 0.0f;
        float Far = 0.0f;
        float Aspect = 0.0f;
    };

    struct RenderingCache
    {
        glm::mat4 Projection;
        glm::mat4 View;
    };
}

#endif