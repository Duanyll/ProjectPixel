#pragma once
#include "pch.h"
#include "../utils/Utils.h"

class Texture {
   public:
    NO_COPY_CLASS(Texture)
    Texture(const std::string& filePath, bool flipped = false);
    ~Texture();
    GLuint id = 0;
    int width = 0, height = 0, channels = 0;

    inline virtual GLenum get_type() const { return GL_TEXTURE_2D; }

   protected:
    inline Texture() {}
};

typedef std::shared_ptr<Texture> pTexture;

class CubeTexture : public Texture {
   public:
    CubeTexture(const std::vector<std::string>& facesPath);
    inline virtual GLenum get_type() const { return GL_TEXTURE_CUBE_MAP; }
};

typedef std::shared_ptr<CubeTexture> pCubeTexture;

struct EntityMaterial {
    pTexture diffuse;
    pTexture specular;
    pTexture emission;
    float shininess = 32;
};

class FrameBufferTexture : public Texture {
   public:
    FrameBufferTexture(int width, int height, bool isPixelized = true);
    ~FrameBufferTexture();
    inline virtual GLenum get_type() const { return GL_TEXTURE_2D; }

    GLuint rbo = 0;
    GLuint bufferId = 0;

    void drawInside(std::function<void()> draw);
};