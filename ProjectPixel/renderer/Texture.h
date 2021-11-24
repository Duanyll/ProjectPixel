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

    inline virtual GLenum get_type() { return GL_TEXTURE_2D; }

   protected:
    inline Texture() {}
};

typedef std::shared_ptr<Texture> pTexture;

class CubeTexture : public Texture {
   public:
    CubeTexture(const std::vector<std::string&> facesPath);
    inline virtual GLenum get_type() { return GL_TEXTURE_CUBE_MAP; }
};

typedef std::shared_ptr<CubeTexture> pCubeTexture;