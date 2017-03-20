/*
    GpuWrapper.cc nachocpol@gmail.com
*/

#include "GpuWrapper.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <scene/scene.hpp>
#include <tygra/FileHelper.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cassert>

#include <random>

#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF

namespace gpu
{
    // GpuMaterial

    GpuMaterial::GpuMaterial()
    {
        
    }

    GpuMaterial::~GpuMaterial()
    {
        glDeleteProgram(id);
    }

    void GpuMaterial::Use()
    {
        glUseProgram(id);
    }

    // GpuMesh
    GpuMesh::GpuMesh()
    {

    }

    GpuMesh::~GpuMesh()
    {
        //glDeleteVertexArrays(1, &id);
    }

    void GpuMesh::Init(scene::Mesh curMesh)
    {
        GLuint singleSize = 0;
        singleSize = curMesh.getPositionArray().size();

        std::vector<GLfloat> vertex;
        for (unsigned int i = 0; i < curMesh.getPositionArray().size(); i++)
        {
            vertex.push_back(curMesh.getPositionArray()[i].x);
            vertex.push_back(curMesh.getPositionArray()[i].y);
            vertex.push_back(curMesh.getPositionArray()[i].z);
        }
        for (unsigned int j = 0; j < curMesh.getNormalArray().size(); j++)
        {
            vertex.push_back(curMesh.getNormalArray()[j].x);
            vertex.push_back(curMesh.getNormalArray()[j].y);
            vertex.push_back(curMesh.getNormalArray()[j].z);
        }
        for (unsigned int k = 0; k < curMesh.getTextureCoordinateArray().size(); k++)
        {
            vertex.push_back(curMesh.getTextureCoordinateArray()[k].x);
            vertex.push_back(curMesh.getTextureCoordinateArray()[k].y);
        }

        unsigned int f3vSize = sizeof(GLfloat) * 3;
        meshId = curMesh.getId();
        GLuint vbo, ebo;

        glGenVertexArrays(1, &id);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(id);
        {
            // Vertex data 
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(GLfloat), &vertex[0], GL_STATIC_DRAW);

            // Indices
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, curMesh.getElementArray().size() * sizeof(GLuint),
                &curMesh.getElementArray()[0],
                GL_STATIC_DRAW);

            // Enable position vertex array
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, f3vSize, (GLvoid*)0);

            // Enable normals vertex array
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, f3vSize, (GLvoid*)(singleSize * f3vSize));

            // Enable uvs vertex array
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)(2 * singleSize * f3vSize));
        }
        glBindVertexArray(0);

        indices = curMesh.getElementArray();
    }

    void GpuMesh::Init(float r, int sub)
    {
        auto tmp = tsl::createSpherePtr(r, sub);
        tmp = tsl::cloneIndexedMeshAsTriangleListPtr(tmp.get());

        unsigned int f3vSize = sizeof(GLfloat) * 3;
        GLuint vbo, ebo;

        glGenVertexArrays(1, &id);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(id);
        {
            // Indices
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, tmp->indexCount() * sizeof(GLuint),
                tmp->indexArray(),
                GL_STATIC_DRAW);

            // Vertex data 
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, tmp->vertexCount() * f3vSize, tmp->positionArray(), GL_STATIC_DRAW);

            // Enable position vertex array
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, f3vSize, (GLvoid*)0);

        }
        glBindVertexArray(0);

        indices.resize(tmp->indexCount());
        for(unsigned int i = 0; i < indices.size(); i++)
        {
            indices[i] = tmp->indexArray()[i];
        }
    }

	void GpuMesh::Init(float r, float h,int s)
	{
		auto tmp = tsl::createConePtr(r, h, s);
		tmp = tsl::cloneIndexedMeshAsTriangleListPtr(tmp.get());

		unsigned int f3vSize = sizeof(GLfloat) * 3;
		GLuint vbo, ebo;

		glGenVertexArrays(1, &id);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		glBindVertexArray(id);
		{
			// Indices
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, tmp->indexCount() * sizeof(GLuint),
				tmp->indexArray(),
				GL_STATIC_DRAW);

			// Vertex data 
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, tmp->vertexCount() * f3vSize, tmp->positionArray(), GL_STATIC_DRAW);

			// Enable position vertex array
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, f3vSize, (GLvoid*)0);

		}
		glBindVertexArray(0);

		indices.resize(tmp->indexCount());
		for (unsigned int i = 0; i < indices.size(); i++)
		{
			indices[i] = tmp->indexArray()[i];
		}
	}

    void GpuMesh::Draw()
    {
        glBindVertexArray(id);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    // GpuFrameBuffer

    GpuFrameBuffer::GpuFrameBuffer()
    {

    }

    GpuFrameBuffer::~GpuFrameBuffer()
    {
        glDeleteFramebuffers(1, &id);
        glDeleteTextures(1, &colorTexture);
        glDeleteTextures(1, &depthTexture);
    }

    void GpuFrameBuffer::Init(glm::vec2 sSize)
    {
        // Init frame buffer
        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);

        // Init color texture
        glGenTextures(1, &colorTexture);
        glBindTexture(GL_TEXTURE_2D, colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sSize.x, sSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

        // Init depth texture
        glGenTextures(1, &depthTexture);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, sSize.x, sSize.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
        
        // This produces an error that seems to do nothing
        // GL_INVALID_ENUM error generated. <target> enum is invalid; expected GL_FRAMEBUFFER_EXT, GL_DRAW_FRAMEBUFFER_EXT or GL_READ_FRAMEBUFFER_EXT
        //if (glCheckFramebufferStatus(GL_FRAMEBUFFER != GL_FRAMEBUFFER_COMPLETE))
        {
            //printf("Could not create the framebuffer:%i!\n", id);
        }

        rectanleMesh.id = InitScreenRect(fullScreenRect,&rectanleMesh.indices);
        Disable();
    }

    void GpuFrameBuffer::Resize(glm::vec2 sSize)
    {
        printf("LOG: Resizing GpuFrameBuffer:%i, size:%i,%i.\n",id,(int)sSize.x, (int)sSize.y);
        glBindTexture(GL_TEXTURE_2D, colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sSize.x, sSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, sSize.x, sSize.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    }

    void GpuFrameBuffer::Enable()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void GpuFrameBuffer::Disable()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void GpuFrameBuffer::Draw()
    {
        // Use the material
        fboMat.Use();

        GLuint tUnit = 0;
        glActiveTexture(GL_TEXTURE0 + tUnit);
        glBindTexture(GL_TEXTURE_2D, colorTexture);
        glUniform1i(glGetUniformLocation(fboMat.id, "colorTexture"), tUnit);

        // Draw rectagle mesh
        rectanleMesh.Draw();
    }


    // GpuInstancedMesh
    GpuInstancedMesh::GpuInstancedMesh()
    {

    }

    GpuInstancedMesh::~GpuInstancedMesh()
    {

    }

    void GpuInstancedMesh::Init(std::vector<glm::mat4>& iM,std::vector<glm::vec4> &iCol,GpuMesh m)
    {
        // Instanced model matrix
        iCount = iM.size();
        mesh = m;
        GLuint location = 3;
        glGenBuffers(1,&modelsId);
        glBindBuffer(GL_ARRAY_BUFFER, modelsId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * iM.size(), &iM[0], GL_STATIC_DRAW);

        glBindVertexArray(mesh.id);
        {
            for (int c = 0; c < 4; ++c)
            {
                glEnableVertexAttribArray(location + c);
                glVertexAttribPointer(location + c, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),(GLvoid*)(c * sizeof(glm::vec4))); 
                glVertexAttribDivisor(location + c, 1); 
            }
        }
        glBindVertexArray(0);

        // Instaced material colors
        location = 7;
        glGenBuffers(1, &colorsId);
        glBindBuffer(GL_ARRAY_BUFFER, colorsId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * iCol.size(), &iCol[0], GL_STATIC_DRAW);

        glBindVertexArray(mesh.id);
        {
            glEnableVertexAttribArray(location);
            glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (GLvoid*)0);
            glVertexAttribDivisor(location, 1);
        }
        glBindVertexArray(0);
    }

    void GpuInstancedMesh::Draw()
    {
        glBindVertexArray(mesh.id);
        {
            //glBindBuffer(GL_ARRAY_BUFFER, modelsId);
            glDrawElementsInstanced(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0, iCount);
        }
        glBindVertexArray(0);
    }

    // GpuTexture
    GpuTexture::GpuTexture():
        id(-1)
    {

    }

    GpuTexture::~GpuTexture()
    {
        glDeleteTextures(1, &id);
    }

    void GpuTexture::Init(const char* path)
    {
        int w, h, n;
        unsigned char* data;
        data = stbi_load(path, &w, &h, &n, 0);
        if (!data)
        {
            printf("ERROR:Texture:%s , does not exist!\n",path);
            return;
        }

        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        if (n == 1)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, data);
        }
        else if (n > 4)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        GLfloat maxAnisotropy;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);

        glBindTexture(GL_TEXTURE_2D, 0);
        
        stbi_image_free(data);
        //delete data;
    }

    // Cube map
    GpuTextureCube::GpuTextureCube()
    {

    }

    GpuTextureCube::~GpuTextureCube()
    {

    }

    void GpuTextureCube::Init(std::vector<const char*>& files)
    {
        id = CreateCubeMap(files);
    }

    // G-buffer
    GpuGBuffer::GpuGBuffer()
    {

    }

    GpuGBuffer::~GpuGBuffer()
    {

    }

    void GpuGBuffer::Init(glm::vec2 screenS)
    {
        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);

        // Position and depth texture 
        glGenTextures(1, &pId);
        glBindTexture(GL_TEXTURE_2D,pId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screenS.x, screenS.y, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pId, 0);

        // Normals and roughness texture
        glGenTextures(1, &nId);
        glBindTexture(GL_TEXTURE_2D, nId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenS.x, screenS.y, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, nId, 0);

        // Albedo and metallic texture
        glGenTextures(1, &aId);
        glBindTexture(GL_TEXTURE_2D, aId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenS.x, screenS.y, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, aId, 0);
            
        // Assign attachments
        GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, attachments);

        // Init depth texture
        glGenTextures(1, &dId);
        glBindTexture(GL_TEXTURE_2D, dId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, screenS.x, screenS.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dId, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER != GL_FRAMEBUFFER_COMPLETE))
        {
            printf("ERROR:Could not create the framebuffer:%i!\n", id);
        }

        // Unbind current fbo
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Create screen rects to debug gbuffer
        std::vector<glm::vec2> r =
        {
            { -1.0f,-1.0f },    //bl
            { -.50f,-1.0f },    //br
            { -.50f,-.25f },    //tr
            { -1.0f,-.25f },    //tl
        };
        pRect.id = InitScreenRect(r, &pRect.indices);
        r =
        {
            { -.50f,-1.0f },    //bl
            {  0.0f,-1.0f },    //br
            {  0.0f,-.25f },    //tr
            { -.50f,-.25f },    //tl
        };
        nRect.id = InitScreenRect(r, &nRect.indices);
        r =
        {
            {  0.0f,-1.0f },    //bl
            {  .50f,-1.0f },    //br
            {  .50f,-.25f },    //tr
            {  0.0f,-.25f },    //tl
        };
        aRect.id = InitScreenRect(r, &aRect.indices);

        InitMaterial(&rectMat, "resource:///fbo.vs", "resource:///fbo.fs");
    }

    void GpuGBuffer::Resize(glm::vec2 screenS)
    {
        printf("LOG: Resizing GpuGBuffer:%i, size:%i,%i.\n", id, (int)screenS.x, (int)screenS.y);

        // Position texture
        glBindTexture(GL_TEXTURE_2D, pId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screenS.x, screenS.y, 0, GL_RGBA, GL_FLOAT, NULL);

        // Normals texture
        glBindTexture(GL_TEXTURE_2D, nId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenS.x, screenS.y, 0, GL_RGBA, GL_FLOAT, NULL);

        // Albedo texture
        glBindTexture(GL_TEXTURE_2D, aId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenS.x, screenS.y, 0, GL_RGBA, GL_FLOAT, NULL);

        // Depth texture
        glBindTexture(GL_TEXTURE_2D, dId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, screenS.x, screenS.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }

    void GpuGBuffer::DrawDebug()
    {
        rectMat.Use();

        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, pId);
        glUniform1i(glGetUniformLocation(rectMat.id, "colorTexture"), 0);
        pRect.Draw();
        
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, nId);
        glUniform1i(glGetUniformLocation(rectMat.id, "colorTexture"), 1);
        nRect.Draw();

        glActiveTexture(GL_TEXTURE0 + 2);
        glBindTexture(GL_TEXTURE_2D, aId);
        glUniform1i(glGetUniformLocation(rectMat.id, "colorTexture"), 2);
        aRect.Draw();
    }

    void GpuGBuffer::Enable()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void GpuGBuffer::Disable()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Shadow map

    GpuShadowMap::GpuShadowMap()
    {

    }

    GpuShadowMap::~GpuShadowMap()
    {

    }

    void GpuShadowMap::Init(glm::vec2 size)
    {
        s = size;
        glGenFramebuffers(1, &id);
        glGenTextures(1, &DepthId);
        glBindTexture(GL_TEXTURE_2D, DepthId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, s.x, s.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        // Used with sampler2DShadow
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        //GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
        //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glBindFramebuffer(GL_FRAMEBUFFER, id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthId, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void GpuShadowMap::Resize(glm::vec2 size)
    {
        s = size;
        glBindTexture(GL_TEXTURE_2D, DepthId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, s.x, s.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }



    // Inits the given material
    void InitMaterial(GpuMaterial* mat, const char* vsPath, const char* fsPath)
    {
        GLuint vId, fId;
        GLint res;
        GLchar log[512];
        std::string code;
        const GLchar* vCode;
        const GLchar* fCode;

        // Vertex shader
        code = tygra::createStringFromFile(vsPath);
        vCode = code.c_str();
        vId = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vId, 1, &vCode, nullptr);
        glCompileShader(vId);
        glGetShaderiv(vId, GL_COMPILE_STATUS, &res);
        if (!res)
        {
            glGetShaderInfoLog(vId, 512, nullptr, log);
            printf("Failed to compile vertex shader:\n");
            printf("%s", log);
        }

        // Fragment shader
        code = tygra::createStringFromFile(fsPath);
        fCode = code.c_str();
        fId = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fId, 1, &fCode, nullptr);
        glCompileShader(fId);
        glGetShaderiv(fId, GL_COMPILE_STATUS, &res);
        if (!res)
        {
            glGetShaderInfoLog(fId, 512, nullptr, log);
            printf("Failed to compile fragment shader:\n");
            printf("%s", log);
        }

        // Create a program and link the shaders
        mat->id = glCreateProgram();
        glAttachShader(mat->id, vId);
        glAttachShader(mat->id, fId);
        glLinkProgram(mat->id);
        glGetProgramiv(mat->id, GL_LINK_STATUS, &res);
        if (!res)
        {
            printf("\n ERROR: Compile failed:\n  %s \n  %s. \n\n", vsPath, fsPath);
            glGetProgramInfoLog(mat->id, 512, nullptr, log);
            //printf("Failed to link the shaders:\n");
            printf("%s \n", log);
        }
        else
        {
            printf("Created a program with id:%i!\n", mat->id);
        }

        glDeleteShader(vId);
        glDeleteShader(fId);
    }
    
    int InitScreenRect(std::vector<glm::vec2> rect, std::vector<GLuint>* outIndex)
    {
        // Init rectanle mesh
        int singleNumPos = 12;
        std::vector<float> vertexData =
        {
            rect[0].x,rect[0].y,0.0f,   //bl
            rect[1].x,rect[1].y,0.0f,   //br
            rect[2].x,rect[2].y,0.0f,   //tr
            rect[3].x,rect[3].y,0.0f,   //tl

            0.0f,1.0f,
            1.0f,1.0f,
            1.0f,0.0f,
            0.0f,0.0f
        };
        std::vector<GLuint> index =
        {
            0,1,2,
            0,2,3
        };
        outIndex[0] = index;

        GLuint vao , vbo, ebo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);
        {
            // Positions - uvs
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER,
                vertexData.size() * sizeof(float),
                &vertexData[0], GL_STATIC_DRAW);

            // Indices
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                index.size() * sizeof(GLuint),
                &index[0], GL_STATIC_DRAW);

            // Positions
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);

            // Uvs
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (GLvoid*)(singleNumPos * sizeof(float)));
        }
        glBindVertexArray(0);

        return vao;
    }

    glm::vec3 ToGlmVec3(scene::Vector3 & in)
    {
        return glm::vec3(in.x, in.y, in.z);
    }

    unsigned int CreateCubeMap(std::vector<const char*>& files)
    {
        int x, y, n;

        unsigned int id = 0;
        glGenTextures(1, &id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);

        for (unsigned int i = 0; i < 6; i++)
        {
            unsigned char* imgData = stbi_load(files[i], &x, &y, &n, 0);
            if (!imgData)
            {
                printf("ERROR:Could not open:%s , skiping.\n", files[i]);
                continue;
            }
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                0, GL_RGB, x, y, 0, GL_RGB,
                GL_UNSIGNED_BYTE, imgData);
            // Clear loaded data
            stbi_image_free(imgData);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        printf("LOG:Generated texture cube:%i,size:%i,%i.\n", id, x, y);

        return id;
    }

    void SSAO::Init()
    {
        // Init kernel
        std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
        std::default_random_engine generator;
        for (GLuint i = 0; i < 64; ++i)
        {
            glm::vec3 sample(
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator)
            );
            sample = glm::normalize(sample);
            sample *= randomFloats(generator);
            GLfloat scale = GLfloat(i) / 64.0;
            scale = Lerp(0.1f, 1.0f, scale * scale);
            sample *= scale;
            //sample = glm::normalize(sample);
            Kernel.push_back(sample);
        }
        // Init noise
        for (GLuint i = 0; i < 16; i++)
        {
            glm::vec3 noise(
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 2.0 - 1.0,
                0.0f);
            Noise.push_back(noise);
        }

        // Gen noise texture
        glGenTextures(1, &NoiseTexId);
        glBindTexture(GL_TEXTURE_2D, NoiseTexId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &Noise[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    }

    float SSAO::Lerp(float a, float b, float f)
    {
        return a + f * (b - a);
    }

}