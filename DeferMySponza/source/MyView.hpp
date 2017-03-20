#pragma once

#include <scene/scene_fwd.hpp>
#include <tygra/WindowViewDelegate.hpp>
#include <tgl/tgl.h>

#include <glm/glm.hpp>

#include "GpuWrapper.h"

#include <vector>
#include <memory>

class MyView : public tygra::WindowViewDelegate
{
public:

    MyView();

    ~MyView();

    void setScene(const scene::Context * scene);

private:

    void windowViewWillStart(tygra::Window * window) override;

    void windowViewDidReset(tygra::Window * window,
                            int width,
                            int height) override;

    void windowViewDidStop(tygra::Window * window) override;

    void windowViewRender(tygra::Window * window) override;

    const scene::Context * mScene;

    // Screen size
    glm::vec4 mWindowSize;

    // Camera
    gpu::Camera mRenderCamera;

    // Rendering cache
    gpu::RenderingCache mRenderingCache;

    // Holds all the meshes in the scene
    std::vector<gpu::GpuMesh> mMeshes;

    // Material used to fill the gbuffer
    gpu::GpuMaterial mFillGBufMat;

    // Material used to render the shadows
    //gpu::GpuShadowMap mTestShadow;
    std::vector<gpu::GpuShadowMap> mShadowMaps;
    gpu::GpuMaterial mShadowMat;

    // The G-Buffer
    gpu::GpuGBuffer mGBuffer;

    // Enviroment probe
    gpu::GpuTextureCube mEnvProbe;

    // Deferred directional
    gpu::GpuMaterial mDirMat;
    gpu::GpuMesh mDirMesh;
    // Deferred point
    gpu::GpuMaterial mPoinMat;
    gpu::GpuMesh mPoinMesh;
    // Deferred spot
    gpu::GpuMaterial mSpotMat;
	gpu::GpuMesh mSpotMesh;
    // we will use the sphere for now

    // Screen fbo (can be used for post effects)
    gpu::GpuFrameBuffer mScreenFbo;

    // Holds projection and view matrix
    GLuint mUbo = -1;

    // Holds ssao kernel and noise
    gpu::SSAO mSsao;

    // SSAO fbo
    gpu::GpuFrameBuffer mSsaoFbo;

    // SSAO blur
    gpu::GpuFrameBuffer mSsaoBlurFbo;

    // FXAA
    gpu::GpuFrameBuffer mFxaa;

    // Compose (ssao * ambient) + sceneColor
    gpu::GpuFrameBuffer mComposeFbo;

    // DOF 
    gpu::GpuFrameBuffer mDofBlurV;
    gpu::GpuFrameBuffer mDofBlurH;
    gpu::GpuFrameBuffer mDof;

    // Delta time 
    float mStartTime = 0.0f;
    float mDt = 0.0f;

    // Sets information for the rendering pass (mvp,fog,eye pos etc)
    void SetPass();

    // Sets information for the next rendered object (model, material properties etc)
    void SetObject(int iId,unsigned int meshId);

    // Sets information for shadow pass
    void SetPassShadow(scene::SpotLight light);
    // void SetPassShadow(scene::PointLight light);
    // void SetPassShadow(scene::DirectionalLight light);

    // Configures object to be rendered in the shadow pass (model)
    void SetObjectShadow(int iId);

    // Resets the state of the rendering pass (view & projection)
    void ResetPass();

    std::vector<gpu::GpuTexture> mDiffuseTextures;
    std::vector<gpu::GpuTexture> mNormalTextures;
    std::vector<gpu::GpuTexture> mSpecTextures;
    std::vector<gpu::GpuTexture> mMetTextures;

    const char* kDiffPath[24] = 
    {
        "../demo/img/sponza/Background_Albedo.jpg",            //0
        "../demo/img/sponza/Lion_Albedo.jpg",                  //1
        "../demo/img/sponza/Sponza_Bricks_a_Albedo.jpg",      //2
        "../demo/img/sponza/Sponza_Arch_diffuse.jpg",          //3
        "../demo/img/sponza/Sponza_Ceiling_diffuse.jpg",       //4
        "../demo/img/sponza/Sponza_Column_a_diffuse.jpg",      //5
        "../demo/img/sponza/Sponza_Column_b_diffuse.jpg",      //6
        "../demo/img/sponza/Sponza_Column_c_diffuse.jpg",      //7
        "../demo/img/sponza/Sponza_Curtain_Blue_diffuse.jpg",  //8
        "../demo/img/sponza/Sponza_Curtain_Red_diffuse.jpg",  //9
        "../demo/img/sponza/Sponza_Curtain_Green_diffuse.jpg", //10
        "../demo/img/sponza/Sponza_Details_diffuse.jpg",       //11
        "../demo/img/sponza/Sponza_Fabric_Blue_diffuse.jpg",   //12
        "../demo/img/sponza/Sponza_Fabric_Red_diffuse.jpg",    //13
        "../demo/img/sponza/Sponza_Fabric_Green_diffuse.jpg",  //14
        "../demo/img/sponza/Sponza_FlagPole_diffuse.jpg",      //15
        "../demo/img/sponza/Sponza_Floor_diffuse.jpg",         //16
        "../demo/img/sponza/Sponza_Roof_diffuse.jpg",          //17
        "../demo/img/sponza/Sponza_Thorn_diffuse.jpg",         //18
        "../demo/img/sponza/Vase_diffuse.jpg",                 //19
        "../demo/img/sponza/Vase_diffuse.jpg",                 //20
        "../demo/img/sponza/VaseRound_diffuse.jpg",            //21
        "../demo/img/sponza/ChainTexture_Albedo.jpg",          //22
        "../demo/img/sponza/VaseRound_diffuse.jpg"             //23 check this
    };

    int kMatDiffuseId[30] =
    {
        19,
        4,
        18,
        1,
        21,
        18,
        0,
        15,
        18,
        2,
        18,
        16,
        12, //up curtains
        17, 
        15,
        18,
        2,
        2,  
        18,
        22,
        23,
        18,
        18,
        18,
        9,  // down curtains
        4,
        18,
        -1,
        -1,
        -1
    };

    const char* kNormPath[24] =
    {
        "../demo/img/sponza/Background_Normal.jpg",            //0
        "../demo/img/sponza/Lion_Normal.jpg",                  //1
        "../demo/img/sponza/Sponza_Bricks_a_Normal.jpg",      //2
        "../demo/img/sponza/Sponza_Arch_normal.jpg",          //3
        "../demo/img/sponza/Sponza_Ceiling_normal.jpg",       //4
        "../demo/img/sponza/Sponza_Column_a_normal.jpg",      //5
        "../demo/img/sponza/Sponza_Column_b_normal.jpg",      //6
        "../demo/img/sponza/Sponza_Column_c_normal.jpg",      //7
        "../demo/img/sponza/Sponza_Curtain_Blue_normal.jpg",  //8
        "../demo/img/sponza/Sponza_Curtain_Blue_normal.jpg",  //9
        "../demo/img/sponza/Sponza_Curtain_Green_normal.jpg", //10
        "../demo/img/sponza/Sponza_Details_normal.jpg",       //11
        "../demo/img/sponza/Sponza_Fabric_Blue_normal.jpg",   //12
        "../demo/img/sponza/Sponza_Fabric_Red_normal.jpg",    //13
        "../demo/img/sponza/Sponza_Fabric_Green_normal.jpg",  //14
        "../demo/img/sponza/Sponza_FlagPole_normal.jpg",      //15
        "../demo/img/sponza/Sponza_Floor_normal.jpg",         //16
        "../demo/img/sponza/Sponza_Roof_normal.jpg",          //17
        "../demo/img/sponza/Sponza_Thorn_normal.jpg",         //18
        "../demo/img/sponza/Vase_normal.jpg",                 //19
        "../demo/img/sponza/Vase_normal.jpg",                  //20
        "../demo/img/sponza/VaseRound_normal.jpg",             //21
        "../demo/img/sponza/ChainTexture_Normal.jpg",          //22
        "../demo/img/sponza/VaseRound_normal.jpg"            //23 check this
    };

    const char* kSpecPath[24] =
    {
        "../demo/img/sponza/Background_Roughness.jpg",            //0
        "../demo/img/sponza/Lion_Roughness.jpg",                  //1
        "../demo/img/sponza/Sponza_Bricks_a_Roughness.jpg",      //2
        "../demo/img/sponza/Sponza_Arch_roughness.jpg",          //3
        "../demo/img/sponza/Sponza_Ceiling_roughness.jpg",       //4
        "../demo/img/sponza/Sponza_Column_a_roughness.jpg",      //5
        "../demo/img/sponza/Sponza_Column_b_roughness.jpg",      //6
        "../demo/img/sponza/Sponza_Column_c_roughness.jpg",      //7
        "../demo/img/sponza/Sponza_Curtain_roughness.jpg",  //8
        "../demo/img/sponza/Sponza_Curtain_roughness.jpg",  //9
        "../demo/img/sponza/Sponza_Curtain_roughness.jpg", //10
        "../demo/img/sponza/Sponza_Details_roughness.jpg",       //11
        "../demo/img/sponza/Sponza_Fabric_roughness.jpg",   //12
        "../demo/img/sponza/Sponza_Fabric_roughness.jpg",    //13
        "../demo/img/sponza/Sponza_Fabric_roughness.jpg",  //14
        "../demo/img/sponza/Sponza_FlagPole_roughness.jpg",      //15
        "../demo/img/sponza/Sponza_Floor_roughness.jpg",         //16
        "../demo/img/sponza/Sponza_Roof_roughness.jpg",          //17
        "../demo/img/sponza/Sponza_Thorn_roughness.jpg",         //18
        "../demo/img/sponza/Vase_roughness.jpg",                 //19
        "../demo/img/sponza/Vase_roughness.jpg",                  //20
        "../demo/img/sponza/VaseRound_roughness.jpg",             //21
        "../demo/img/sponza/ChainTexture_Roughness.jpg",          //22
        "../demo/img/sponza/VaseRound_roughness.jpg"            //23 check this
    };

    const char* kMetPath[24] =
    {
        "../demo/img/sponza/Dielectric_metallic.jpg",            //0
        "../demo/img/sponza/Dielectric_metallic.jpg",                  //1
        "../demo/img/sponza/Dielectric_metallic.jpg",      //2
        "../demo/img/sponza/Dielectric_metallic.jpg",          //3
        "../demo/img/sponza/Dielectric_metallic.jpg",       //4
        "../demo/img/sponza/Dielectric_metallic.jpg",      //5
        "../demo/img/sponza/Dielectric_metallic.jpg",      //6
        "../demo/img/sponza/Dielectric_metallic.jpg",      //7
        "../demo/img/sponza/Sponza_Curtain_metallic.jpg",  //8
        "../demo/img/sponza/Sponza_Curtain_metallic.jpg",  //9
        "../demo/img/sponza/Sponza_Curtain_metallic.jpg", //10
        "../demo/img/sponza/Sponza_Details_metallic.jpg",       //11
        "../demo/img/sponza/Sponza_Fabric_metallic.jpg",   //12
        "../demo/img/sponza/Sponza_Fabric_metallic.jpg",    //13
        "../demo/img/sponza/Sponza_Fabric_metallic.jpg",  //14
        "../demo/img/sponza/Metallic_metallic.jpg",      //15
        "../demo/img/sponza/Dielectric_metallic.jpg",         //16
        "../demo/img/sponza/Dielectric_metallic.jpg",          //17
        "../demo/img/sponza/Dielectric_metallic.jpg",         //18
        "../demo/img/sponza/Dielectric_metallic.jpg",                 //19
        "../demo/img/sponza/Dielectric_metallic.jpg",                  //20
        "../demo/img/sponza/Dielectric_metallic.jpg",             //21
        "../demo/img/sponza/ChainTexture_Metallic.jpg",          //22
        "../demo/img/sponza/Dielectric_metallic.jpg"            //23 check this
    };

    // Debuging
    GLuint mQueries[2];
    GLuint64 mTimeStart = 0;
    GLuint64 mTimeEnd = 0;
    int mDone = 0;
};
