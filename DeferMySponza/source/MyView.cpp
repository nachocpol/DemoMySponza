#include "MyView.hpp"

#include <scene/scene.hpp>
#include <tygra/FileHelper.hpp>
#include <tsl/shapes.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cassert>
#include <string>

#include "nvToolsExt.h"

MyView::MyView()
{
}

MyView::~MyView() 
{
}

void MyView::setScene(const scene::Context * scene)
{
    mScene = scene;
}

void MyView::windowViewWillStart(tygra::Window * window)
{
    assert(mScene != nullptr);

    // Load all the meshes
    auto geoBuilder = scene::GeometryBuilder();
    for each (scene::Mesh mesh in geoBuilder.getAllMeshes())
    {
        gpu::GpuMesh gpuMesh;
        gpuMesh.Init(mesh);
        gpuMesh.meshId = mesh.getId();
        mMeshes.push_back(gpuMesh);
    }

    // Init the main material
    gpu::InitMaterial(&mFillGBufMat, "resource:///deferred1.vs", "resource:///deferred1.fs");

    // Init matrix UBO
    glGenBuffers(1, &mUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, mUbo);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, mUbo, 0, 2 * sizeof(glm::mat4));
    
    // Init deferred meshes and materials
    // Again, note to stupid Nacho, if a material is not initialized,
    // log an error if we try to use it ....!!!!!!!
    gpu::InitMaterial(&mDirMat, "resource:///deferredDirect.vs", "resource:///deferredDirect.fs");
    mDirMesh.id = gpu::InitScreenRect(gpu::fullScreenRect,&mDirMesh.indices);
    
    gpu::InitMaterial(&mPoinMat, "resource:///deferredPoint.vs", "resource:///deferredPoint.fs");
    mPoinMesh.Init(1.0f, 10.0f);

    gpu::InitMaterial(&mSpotMat, "resource:///deferredSpot.vs", "resource:///deferredSpot.fs");
	mSpotMesh.Init(1.0f, 1.0f, 10);

    // Init screen fbo
    mScreenFbo.Init(glm::vec2(0, 0));
    gpu::InitMaterial(&mScreenFbo.fboMat, "resource:///fbo.vs", "resource:///fbo.fs");

    // Init ssao
    mSsao.Init();

    mSsaoFbo.Init(glm::vec2(0));
    gpu::InitMaterial(&mSsaoFbo.fboMat, "resource:///ssao.vs", "resource:///ssao.fs");

    mSsaoBlurFbo.Init(glm::vec2(0));
    gpu::InitMaterial(&mSsaoBlurFbo.fboMat, "resource:///ssaoBlur.vs", "resource:///ssaoBlur.fs");

    // Init env map probe
    std::vector<const char*> cmFiles =
    {
        "../demo/img/sky/right.png",
        "../demo/img/sky/left.png",
        "../demo/img/sky/up.png",
        "../demo/img/sky/down.png",
        "../demo/img/sky/back.png",
        "../demo/img/sky/front.png"
    };
    mEnvProbe.Init(cmFiles);

    // Init fxaa
    mFxaa.Init(glm::vec2(0));
    gpu::InitMaterial(&mFxaa.fboMat, "resource:///fxaa.vs", "resource:///fxaa.fs");

    // Init compose fbo
    mComposeFbo.Init(glm::vec2(0));
    gpu::InitMaterial(&mComposeFbo.fboMat, "resource:///compose.vs", "resource:///compose.fs");

    // Init dof
    mDofBlurV.Init(glm::vec2(0));
    gpu::InitMaterial(&mDofBlurV.fboMat, "resource:///dofblur.vs", "resource:///dofblur.fs");
    mDofBlurH.Init(glm::vec2(0));
    gpu::InitMaterial(&mDofBlurH.fboMat, "resource:///dofblur.vs", "resource:///dofblur.fs");
    mDof.Init(glm::vec2(0));
    gpu::InitMaterial(&mDof.fboMat, "resource:///dof.vs", "resource:///dof.fs");

    // Init diffuse textures
    mDiffuseTextures.resize(24);
    for (unsigned int i = 0; i < mDiffuseTextures.size(); i++)
    {
        mDiffuseTextures[i].Init(kDiffPath[i]);
    }

    // Init normal textures
    mNormalTextures.resize(24);
    for (unsigned int i = 0; i < mNormalTextures.size(); i++)
    {
        mNormalTextures[i].Init(kNormPath[i]);
    }

    // Init specular textures
    mSpecTextures.resize(24);
    for (unsigned int i = 0; i < mSpecTextures.size(); i++)
    {
        mSpecTextures[i].Init(kSpecPath[i]);
    }

    // Init metallic textures
    mMetTextures.resize(24);
    for (unsigned int i = 0; i < mMetTextures.size(); i++)
    {
        mMetTextures[i].Init(kMetPath[i]);
    }

    // Init shadow maps
    auto sLights = mScene->getAllSpotLights();
    for (unsigned int i = 0; i < sLights.size(); i++)
    {
        if (sLights[i].getCastShadow())
        {
            mShadowMaps.push_back(gpu::GpuShadowMap());
            mShadowMaps[mShadowMaps.size() - 1].LightId = i;
            mShadowMaps[mShadowMaps.size() - 1].Init(glm::vec2(1024.0f));
        }
    }
    gpu::InitMaterial(&mShadowMat, "resource:///shadow.vs", "resource:///shadow.fs");

    // Debug
    glGenQueries(2, mQueries);
}

void MyView::windowViewDidReset(tygra::Window * window,
                                int width,
                                int height)
{
    glViewport(0, 0, width, height);
    mWindowSize = glm::vec4(0, 0, width, height);

    // Resize geobuffer
    if (mGBuffer.id == -1)
    {
        mGBuffer.Init(glm::vec2(width, height));
    }
    else
    {
        mGBuffer.Resize(glm::vec2(width, height));
    }
    
    // Resize fbos
    // Note for stupid Nacho: add a check so if you are
    // trying to resize something that hasnt been created 
    // logs something!!!!!! sdnfojdsanfodisano
    glm::vec2 nSize = glm::vec2(width, height);
    mScreenFbo.Resize(nSize);
    mSsaoFbo.Resize(nSize);
    mSsaoBlurFbo.Resize(nSize);
    mFxaa.Resize(nSize);
    mComposeFbo.Resize(nSize);
    mDofBlurV.Resize(nSize);
    mDofBlurH.Resize(nSize);
    mDof.Resize(nSize);

    // Shadow resize
    //mTestShadow.Resize(nSize);

    // Make a projection matrix
    float aspect = (float)width / (float)height;
    mRenderCamera.Aspect = aspect;
    mRenderCamera.Near = 2.0f;
    mRenderCamera.Far = 1000.0f;
    mRenderCamera.FovRad = glm::radians(80.0f);
    glm::mat4 projection;
    projection = glm::perspectiveRH(
        mRenderCamera.FovRad, 
        mRenderCamera.Aspect,
        mRenderCamera.Near, mRenderCamera.Far);
    mRenderingCache.Projection = projection;

    // Store it in the ubo
    if (mUbo == -1)
    {
        printf("ERROR:Matrix ubo is not initialized.\n");
        return;
    }
    glBindBuffer(GL_UNIFORM_BUFFER, mUbo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void MyView::windowViewDidStop(tygra::Window * window)
{
}

void MyView::windowViewRender(tygra::Window * window)
{
    assert(mScene != nullptr);

    mDt = mScene->getTimeInSeconds() - mStartTime;
    printf("FPS:%i\n", (int)(1.0f / mDt));

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.f, 0.f, 0.0f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw shadowmaps
    for (unsigned int i = 0; i < mShadowMaps.size(); i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mShadowMaps[i].id);
        glClear(GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, mShadowMaps[i].s.x, mShadowMaps[i].s.y);
        {
            mShadowMat.Use();
            SetPassShadow(mScene->getAllSpotLights()[mShadowMaps[i].LightId]);
            for (unsigned int j = 0; j<mMeshes.size(); j++)
            {
                auto meshInstances = mScene->getInstancesByMeshId(mMeshes[j].meshId);
                for (unsigned int i = 0; i < meshInstances.size(); i++)
                {
                    SetObjectShadow(meshInstances[i]);
                    mMeshes[j].Draw();
                }
            }
            ResetPass();
        }
        glViewport(0, 0, mWindowSize.z, mWindowSize.w);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Fill the geometry buffer
    glBindFramebuffer(GL_FRAMEBUFFER, mGBuffer.id);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    {
        mFillGBufMat.Use();
        SetPass();
        glUniform1fv(glGetUniformLocation(mFillGBufMat.id, "near"), 1, &mRenderCamera.Near);
        glUniform1fv(glGetUniformLocation(mFillGBufMat.id, "far"), 1, &mRenderCamera.Far);
        for (unsigned int j=0;j<mMeshes.size();j++)
        {
            auto meshInstances = mScene->getInstancesByMeshId(mMeshes[j].meshId);
            for (unsigned int i = 0; i < meshInstances.size(); i++)
            {
                SetObject(meshInstances[i],j);
                mMeshes[j].Draw();
            }
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Lights
    std::vector<gpu::Light> lights;
    lights.resize(gpu::k_MaxLights);
    // Directionals
    {
        auto dLight = mScene->getAllDirectionalLights();
        for (unsigned int i = 0; i < dLight.size(); i++)
        {
            if (i >= gpu::k_MaxDirect)
            {
                printf("INFO:Maximun number of directional lights reached,ignoring rest.\n");
                break;
            }
            gpu::Light dL;
            dL.position = glm::vec3();
            dL.direction = gpu::ToGlmVec3(dLight[i].getDirection());
            dL.color =  gpu::ToGlmVec3(dLight[i].getIntensity());
            dL.angle = 0.0f;
            dL.range = 0.0f;
            dL.type = gpu::kDirectional;

            lights[i] = dL;
        }
    }
    // Points
    {
        auto pLights = mScene->getAllPointLights();
        for (unsigned int i = 0; i < pLights.size(); i++)
        {
            if (i >= gpu::k_MaxPoint)
            {
                printf("INFO:Maximun number of point lights reached,ignoring rest.\n");
                break;
            }
            gpu::Light dL;
            dL.position = gpu::ToGlmVec3(pLights[i].getPosition());
            dL.direction = glm::vec3();
            dL.color = gpu::ToGlmVec3(pLights[i].getIntensity());
            dL.angle = 0.0f;
            dL.range = pLights[i].getRange();
            dL.type = gpu::kPoint;

            lights[gpu::k_MaxDirect + i] = dL;
        }
    }
    // Spot
    {
        auto sLights = mScene->getAllSpotLights();
        for (unsigned int i = 0; i < sLights.size(); i++)
        {
            if (i >= gpu::k_MaxSpot)
            {
                printf("INFO:Maximum number of spot lights reached,ignoring rest.\n");
                break;
            }
            gpu::Light sL;
            sL.position = gpu::ToGlmVec3(sLights[i].getPosition());
            sL.direction = gpu::ToGlmVec3(sLights[i].getDirection());
            sL.color = gpu::ToGlmVec3(sLights[i].getIntensity());
            /////////////sL.angle = glm::radians(sLights[i].getConeAngleDegrees());
			sL.angle = 0.75f;
            sL.range = sLights[i].getRange();
            sL.type = gpu::kSpot;
            sL.castShadow = sLights[i].getCastShadow();

            lights[gpu::k_MaxPoint + gpu::k_MaxDirect + i] = sL;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, mScreenFbo.id);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    unsigned int mId = -1;
    unsigned int tmp = -1;

    // Directional lights
    {
        mDirMat.Use();
        mId = mDirMat.id;

        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, mGBuffer.pId);
        tmp = glGetUniformLocation(mId, "positions");
        glUniform1i(tmp, 1);

        glActiveTexture(GL_TEXTURE0 + 2);
        glBindTexture(GL_TEXTURE_2D, mGBuffer.nId);
        tmp = glGetUniformLocation(mId, "normals");
        glUniform1i(tmp, 2);

        glActiveTexture(GL_TEXTURE0 + 3);
        glBindTexture(GL_TEXTURE_2D, mGBuffer.aId);
        glUniform1i(glGetUniformLocation(mId, "albedot"), 3);

        glUniform3fv(glGetUniformLocation(mId, "camPos"), 1, &mRenderCamera.Position.x);

        for (unsigned int i = 0; i < gpu::k_MaxDirect; i++)
        {
            // If its an invalid light, just stop, there are no
            // more lights of this type
            if (lights[i].type == gpu::kDefault)break;
            std::string n = std::to_string(i);

            // Setup light uniform
            glUniform3fv(glGetUniformLocation(mId, ("lights[" + n + "].position").c_str()), 1, &lights[i].position.x);
            glUniform3fv(glGetUniformLocation(mId, ("lights[" + n + "].direction").c_str()), 1, &lights[i].direction.x);
            glUniform3fv(glGetUniformLocation(mId, ("lights[" + n + "].color").c_str()), 1, &lights[i].color.x);
            glUniform1fv(glGetUniformLocation(mId, ("lights[" + n + "].angle").c_str()), 1, &lights[i].angle);
            glUniform1fv(glGetUniformLocation(mId, ("lights[" + n + "].range").c_str()), 1, &lights[i].range);
            glUniform1iv(glGetUniformLocation(mId, ("lights[" + n + "].type").c_str()), 1, &lights[i].type);
        }
        // Draw directional light + ambient volume
        mDirMesh.Draw();
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    // Point lights
    {
        mPoinMat.Use();
        mId = mPoinMat.id;

        glCullFace(GL_FRONT);

        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, mGBuffer.pId);
        tmp = glGetUniformLocation(mId, "positions");
        glUniform1i(tmp, 0);

        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, mGBuffer.nId);
        tmp = glGetUniformLocation(mId, "normals");
        glUniform1i(tmp, 1);

        glActiveTexture(GL_TEXTURE0 + 2);
        glBindTexture(GL_TEXTURE_2D, mGBuffer.aId);
        glUniform1i(glGetUniformLocation(mId, "albedot"), 2);
        for (unsigned int i = gpu::k_MaxDirect; i < gpu::k_MaxPoint + gpu::k_MaxDirect; i++)
        {
            // If its an invalid light, just stop, there are no
            // more lights of this type
            if (lights[i].type == gpu::kDefault)break;

            // Setup light uniform
            glUniform3fv(glGetUniformLocation(mId, "light.position"), 1, &lights[i].position.x);
            glUniform3fv(glGetUniformLocation(mId, "light.direction"), 1, &lights[i].direction.x);
            glUniform3fv(glGetUniformLocation(mId, "light.color"), 1, &lights[i].color.x);
            glUniform1fv(glGetUniformLocation(mId, "light.angle"), 1, &lights[i].angle);
            glUniform1fv(glGetUniformLocation(mId, "light.range"), 1, &lights[i].range);
            glUniform1iv(glGetUniformLocation(mId, "light.type"), 1, &lights[i].type);

            glm::mat4 m;
            m = glm::translate(m, lights[i].position);
            m = glm::scale(m, glm::vec3(lights[i].range));
            glUniformMatrix4fv(glGetUniformLocation(mId, "model"), 1, GL_FALSE, &m[0][0]);

            glUniform4fv(glGetUniformLocation(mId, "wSize"), 1, &mWindowSize.x);
            glUniform3fv(glGetUniformLocation(mId, "camPos"), 1, &mRenderCamera.Position.x);

            // Draw light volume
            mPoinMesh.Draw();
        }
    }

    // Spot lights
    {
        mSpotMat.Use();
        mId = mSpotMat.id;

        glCullFace(GL_FRONT);

        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, mGBuffer.pId);
        tmp = glGetUniformLocation(mId, "positions");
        glUniform1i(tmp, 0);

        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, mGBuffer.nId);
        tmp = glGetUniformLocation(mId, "normals");
        glUniform1i(tmp, 1);

        glActiveTexture(GL_TEXTURE0 + 2);
        glBindTexture(GL_TEXTURE_2D, mGBuffer.aId);
        glUniform1i(glGetUniformLocation(mId, "albedot"), 2);
        for (unsigned int i = gpu::k_MaxPoint + gpu::k_MaxDirect; i < gpu::k_MaxSpot + gpu::k_MaxPoint + gpu::k_MaxDirect; i++)
        {
            // If its an invalid light, just stop, there are no
            // more lights of this type
            if (lights[i].type == gpu::kDefault)break;

            // Setup light uniform
            glUniform3fv(glGetUniformLocation(mId, "light.position"), 1, &lights[i].position.x);
            glUniform3fv(glGetUniformLocation(mId, "light.direction"), 1, &lights[i].direction.x);
            glUniform3fv(glGetUniformLocation(mId, "light.color"), 1, &lights[i].color.x);
            glUniform1fv(glGetUniformLocation(mId, "light.angle"), 1, &lights[i].angle);
            glUniform1fv(glGetUniformLocation(mId, "light.range"), 1, &lights[i].range);
            glUniform1iv(glGetUniformLocation(mId, "light.type"), 1, &lights[i].type);
			
			// Matrix to orientate the cone mesh
			glm::mat4 r = glm::lookAt(glm::vec3(0.0f), lights[i].direction, glm::vec3(0.0f, 1.0f, 0.0f));
			r = glm::inverse(r);

			/*
					Apply the range
					Apply the aperture fancy Pythagorean
				|\							a = range
				| \							angle = spot aperture
			   b|  \ c
				|	\
				|----\	<- angle here :3
				a
				tan angle = b / a
				tan angle * a = b
			*/
			float b = lights[i].range * tan(lights[i].angle);
			glm::mat4 s;
			s = glm::scale(s, glm::vec3(b, b, lights[i].range));

            glm::mat4 t;
			t = glm::translate(t, lights[i].position);

			glm::mat4 m = t * r * s * glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0, -1.0f));
	
						
			glUniformMatrix4fv(glGetUniformLocation(mId, "model"), 1, GL_FALSE, &m[0][0]);

            glUniform4fv(glGetUniformLocation(mId, "wSize"), 1, &mWindowSize.x);
            glUniform3fv(glGetUniformLocation(mId, "camPos"), 1, &mRenderCamera.Position.x);

            // Setup shadow information only if the light
            // casts shadows
            int value = 1;
            if (lights[i].castShadow)
            {
                glUniform1iv(glGetUniformLocation(mId, "castShadow"), 1, &value);

                // Light view matrix
                auto light = mScene->getAllSpotLights()[i - (gpu::k_MaxPoint + gpu::k_MaxDirect)];
                
                glm::mat4 lightView;
                glm::vec3 lPos(gpu::ToGlmVec3(light.getPosition()));
                glm::vec3 lDir(gpu::ToGlmVec3(light.getDirection()));
                lightView = glm::lookAt(lPos, lPos + lDir, glm::vec3(0.0f, 1.0f, 0.0f));

                glm::mat4 lightProj;
                lightProj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 300.0f);

                auto lightMatrix = lightProj * lightView;

                unsigned int id = glGetUniformLocation(mId, "lightMatrix");
                glUniformMatrix4fv(id, 1, GL_FALSE, &lightMatrix[0][0]);

                // Search our shadow map, Hack...
                for (unsigned int j = 0; j < mShadowMaps.size(); j++)
                {
                    if (mShadowMaps[j].LightId == (i - (gpu::k_MaxPoint + gpu::k_MaxDirect)))
                    {
                        glActiveTexture(GL_TEXTURE0 + 3);
                        glBindTexture(GL_TEXTURE_2D, mShadowMaps[j].DepthId);
                        tmp = glGetUniformLocation(mId, "shadowT");
                        glUniform1i(tmp, 3);
                        break;
                    }
                }
                //glActiveTexture(GL_TEXTURE0 + 3);
                //glBindTexture(GL_TEXTURE_2D, mTestShadow.DepthId);
                //glBindTexture(GL_TEXTURE_2D, mShadowMaps[i - (gpu::k_MaxPoint + gpu::k_MaxDirect)].DepthId);
                //tmp = glGetUniformLocation(mId, "shadowT");
                //glUniform1i(tmp, 3);
            }
            else
            {
                value = 0;
                glUniform1iv(glGetUniformLocation(mId, "castShadow"), 1, &value);
            }

            // Draw light volume
            // TO-DO:use the cone instead of the sphere 
            mSpotMesh.Draw();
        }
    }

	// Timing start
	//------------------------------------------------
	//glQueryCounter(mQueries[0], GL_TIMESTAMP);
	//------------------------------------------------

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glCullFace(GL_BACK);

    mSsaoFbo.Enable();
    {
        glUseProgram(mSsaoFbo.fboMat.id);

        mId = mSsaoFbo.fboMat.id;

        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, mGBuffer.pId);
        tmp = glGetUniformLocation(mId, "gPositionDepth");
        glUniform1i(tmp, 0);

        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, mGBuffer.nId);
        tmp = glGetUniformLocation(mId, "gNormal");
        glUniform1i(tmp, 1);

        glActiveTexture(GL_TEXTURE0 + 2);
        glBindTexture(GL_TEXTURE_2D, mSsao.NoiseTexId);
        glUniform1i(glGetUniformLocation(mId, "texNoise"), 2);

        for (GLuint i = 0; i < 64; ++i)
            glUniform3fv(glGetUniformLocation(mId, ("samples[" + std::to_string(i) + "]").c_str()), 1, &mSsao.Kernel[i][0]);

        glUniform2f(glGetUniformLocation(mId, "wSize"), mWindowSize.z, mWindowSize.w);

        mSsaoFbo.rectanleMesh.Draw();
    }
    mSsaoFbo.Disable();
    
    // SSAO blur 
    mSsaoBlurFbo.Enable();
    {
        int old = mSsaoBlurFbo.colorTexture;
        mSsaoBlurFbo.colorTexture = mSsaoFbo.colorTexture;

        mSsaoBlurFbo.fboMat.Use();

        GLuint tUnit = 0;
        glActiveTexture(GL_TEXTURE0 + tUnit);
        glBindTexture(GL_TEXTURE_2D, mSsaoBlurFbo.colorTexture);
        glUniform1i(glGetUniformLocation(mSsaoBlurFbo.fboMat.id, "colorTexture"), tUnit);

        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, mGBuffer.dId);
        glUniform1i(glGetUniformLocation(mSsaoBlurFbo.fboMat.id, "depthT"), 1);

        glActiveTexture(GL_TEXTURE0 + 2);
        glBindTexture(GL_TEXTURE_2D, mGBuffer.nId);
        glUniform1i(glGetUniformLocation(mSsaoBlurFbo.fboMat.id, "normalT"), 2);

        mSsaoBlurFbo.rectanleMesh.Draw();

        mSsaoBlurFbo.colorTexture = old;
    }
    mSsaoBlurFbo.Disable();

    // Compose ao + scene
    glBindFramebuffer(GL_FRAMEBUFFER, mComposeFbo.id);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    {
        mComposeFbo.fboMat.Use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mEnvProbe.id);

        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, mScreenFbo.colorTexture);
        glUniform1i(glGetUniformLocation(mComposeFbo.fboMat.id, "scene"), 1);

        glActiveTexture(GL_TEXTURE0 + 2);
        glBindTexture(GL_TEXTURE_2D, mSsaoBlurFbo.colorTexture);
        glUniform1i(glGetUniformLocation(mComposeFbo.fboMat.id, "ambientOclu"), 2);

        glActiveTexture(GL_TEXTURE0 + 3);
        glBindTexture(GL_TEXTURE_2D, mGBuffer.nId);
        glUniform1i(glGetUniformLocation(mComposeFbo.fboMat.id, "normalT"), 3);

        mComposeFbo.rectanleMesh.Draw();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Horizontal blur
    mDofBlurV.Enable();
    {
        int oldCt = mDofBlurV.colorTexture;
        mDofBlurV.colorTexture = mComposeFbo.colorTexture;

        mDofBlurV.fboMat.Use();

        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, mDofBlurV.colorTexture);
        glUniform1i(glGetUniformLocation(mDofBlurV.fboMat.id, "colorTexture"), 0);

        glUniform1i(glGetUniformLocation(mDofBlurV.fboMat.id, "blurHorizontal"), 1);
        
        mDofBlurV.rectanleMesh.Draw();

        mDofBlurV.colorTexture = oldCt;
    }
    mDofBlurV.Disable();

    // Vertical blur
    mDofBlurH.Enable();
    {
        int oldCt = mDofBlurH.colorTexture;
        mDofBlurH.colorTexture = mDofBlurV.colorTexture;

        mDofBlurH.fboMat.Use();
        
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D,     mDofBlurH.colorTexture);
        glUniform1i(glGetUniformLocation(mDofBlurH.fboMat.id, "colorTexture"), 0);

        glUniform1i(glGetUniformLocation(mDofBlurH.fboMat.id, "blurHorizontal"), 0);
        
        mDofBlurH.rectanleMesh.Draw();

        mDofBlurH.colorTexture = oldCt;
    }
    mDofBlurH.Disable();

    // Dof
    mDof.Enable();
    {
        int oldCt = mDof.colorTexture;
        mDof.colorTexture = mComposeFbo.colorTexture;

        mDof.fboMat.Use();

        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, mDof.colorTexture);
        glUniform1i(glGetUniformLocation(mDof.fboMat.id, "colorTexture"), 0);

        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, mDofBlurH.colorTexture);
        glUniform1i(glGetUniformLocation(mDof.fboMat.id, "colorBlurTexture"), 1);

        glActiveTexture(GL_TEXTURE0 + 2);
        glBindTexture(GL_TEXTURE_2D, mGBuffer.dId);
        glUniform1i(glGetUniformLocation(mDof.fboMat.id, "depthTexture"), 2);

        glUniform1fv(glGetUniformLocation(mDof.fboMat.id, "far"), 1, &mRenderCamera.Far);
        glUniform1fv(glGetUniformLocation(mDof.fboMat.id, "near"), 1, &mRenderCamera.Near);

        mDof.rectanleMesh.Draw();

        mDof.colorTexture = oldCt;
    }
    mDof.Disable();

    // Debug G-Buffer 
    //mGBuffer.DrawDebug();

    // Draws with FXAA
    mFxaa.colorTexture = mDof.colorTexture;
    mFxaa.Draw();
    
    // Get time at end frame
    mStartTime = mScene->getTimeInSeconds();

	// Timming end
	//------------------------------------------------
	/*
    glQueryCounter(mQueries[1], GL_TIMESTAMP);
	while (!mDone)
	{
		glGetQueryObjectiv(mQueries[1], GL_QUERY_RESULT_AVAILABLE, &mDone);
	}
	glGetQueryObjectui64v(mQueries[0], GL_QUERY_RESULT, &mTimeStart);
	glGetQueryObjectui64v(mQueries[1], GL_QUERY_RESULT, &mTimeEnd);
	float e = ((float)mTimeEnd - (float)mTimeStart) / 1000000.0f;
	printf("%f\n", e);
    */
	//------------------------------------------------
}

void MyView::SetPass()
{
    // Make a view matrix
    glm::mat4 view;
    glm::vec3 cPos(mScene->getCamera().getPosition().x, mScene->getCamera().getPosition().y, mScene->getCamera().getPosition().z);
    glm::vec3 cDir(mScene->getCamera().getDirection().x, mScene->getCamera().getDirection().y, mScene->getCamera().getDirection().z);
    view = glm::lookAt(cPos, cPos + cDir, glm::vec3(0.0f, 1.0f, 0.0f));

    // Setup view
    mRenderCamera.Position = gpu::ToGlmVec3(mScene->getCamera().getPosition());
    mRenderingCache.View = view;

    glBindBuffer(GL_UNIFORM_BUFFER, mUbo);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void MyView::SetObject(int iId,unsigned int meshId)
{
    // Set model
    auto tMatrix = mScene->getInstanceById(iId).getTransformationMatrix();
    glm::mat4x3 model_4x3;
    model_4x3 = glm::make_mat4x3(&tMatrix.m00);
    glm::mat4 model = glm::mat4(model_4x3);
    glUniformMatrix4fv(glGetUniformLocation(mFillGBufMat.id, "model"), 1, GL_FALSE,&model[0][0]);

    unsigned int matId = mScene->getInstanceById(iId).getMaterialId();
    auto mat = mScene->getMaterialById(matId);

    auto color = gpu::ToGlmVec3(mat.getDiffuseColour());
    glUniform3fv(glGetUniformLocation(mFillGBufMat.id, "albedoColor"), 1, &color.x);

    // Eye pos
    glUniform3fv(glGetUniformLocation(mFillGBufMat.id, "eyePos"), 1, &mRenderCamera.Position.x);

    // Set diffuse texture
    int texId = kMatDiffuseId[meshId];
    int useDiff = texId != -1;
    glUniform1iv(glGetUniformLocation(mFillGBufMat.id, "useDiffTexture"), 1, &useDiff);
    if (useDiff)
    {
        int id = mDiffuseTextures[texId].id;
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, id);
        glUniform1i(glGetUniformLocation(mFillGBufMat.id, "difTexture"), 0);
    }

    // Set normal texture
    int nId = -1;
    if(texId != -1) mNormalTextures[texId].id;
    int useNorm = texId != -1 && nId != -1;
    glUniform1iv(glGetUniformLocation(mFillGBufMat.id, "useNormalTexture"), 1, &useNorm);
    if (useNorm)
    {
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, nId);
        glUniform1i(glGetUniformLocation(mFillGBufMat.id, "normTexture"), 1);
    }

    // Set specular texture
    int sId = -1;
    if(texId != -1)sId = mSpecTextures[texId].id;
    int useSpec = texId != -1 && sId != -1;
    glUniform1iv(glGetUniformLocation(mFillGBufMat.id, "useSpecTexture"), 1, &useSpec);
    if (useSpec)
    {
        glActiveTexture(GL_TEXTURE0 + 2);
        glBindTexture(GL_TEXTURE_2D, sId);
        glUniform1i(glGetUniformLocation(mFillGBufMat.id, "specTexture"), 2);
    }

    // Set met textures
    int metId = -1;
    if(texId != -1) metId = mMetTextures[texId].id;
    int useMet = texId != -1 && metId != -1;
    glUniform1iv(glGetUniformLocation(mFillGBufMat.id, "useMetTexture"), 1, &useMet);
    if (useMet)
    {
        glActiveTexture(GL_TEXTURE0 + 3);
        glBindTexture(GL_TEXTURE_2D, metId);
        glUniform1i(glGetUniformLocation(mFillGBufMat.id, "metTexture"), 3);
    }
}

void MyView::SetPassShadow(scene::SpotLight light)
{
    // Light view matrix
    glm::mat4 lightView;
    glm::vec3 lPos(gpu::ToGlmVec3(light.getPosition()));
    glm::vec3 lDir(gpu::ToGlmVec3(light.getDirection()));
    lightView = glm::lookAt(lPos, lPos + lDir, glm::vec3(0.0f, 1.0f, 0.0f));

    // Light projection
    glm::mat4 lightProj;
    lightProj = glm::perspective(glm::radians(45.0f), mWindowSize.z / mWindowSize.w, 0.1f, 300.0f);

    // Setup uniform buffer for the light data
    glBindBuffer(GL_UNIFORM_BUFFER, mUbo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &lightProj[0][0]);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &lightView[0][0]);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

}

void MyView::SetObjectShadow(int iId)
{
    // Set model
    auto tMatrix = mScene->getInstanceById(iId).getTransformationMatrix();
    glm::mat4x3 model_4x3;
    model_4x3 = glm::make_mat4x3(&tMatrix.m00);
    glm::mat4 model = glm::mat4(model_4x3);
    glUniformMatrix4fv(glGetUniformLocation(mShadowMat.id, "model"), 1, GL_FALSE, &model[0][0]);
}

void MyView::ResetPass()
{
    glBindBuffer(GL_UNIFORM_BUFFER, mUbo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &mRenderingCache.Projection[0][0]);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &mRenderingCache.View[0][0]);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
