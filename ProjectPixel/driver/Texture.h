#pragma once
#include "pch.h"
#include "../utils/Utils.h"

class Texture {
   public:
    NO_COPY_CLASS(Texture)
    Texture(const std::string& filePath, bool flipped = false);
    Texture(int width, int height, bool isPixelized = true);
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

struct Material {
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
    GLuint fbo = 0;

    void draw_inside(std::function<void()> draw);

   protected:
    inline FrameBufferTexture() {}
};

class OutlineFrameBufferTexture : public FrameBufferTexture {
   public:
    OutlineFrameBufferTexture(int width, int height, bool isPixelized = true);
    ~OutlineFrameBufferTexture();
    
    void clear_outline();
    void draw_and_mark_outline(std::function<void()> draw);
    void draw_outline(glm::vec3 color);

    std::shared_ptr<Texture> texOutline1, texOutline2;
    GLuint fboOutline1 = 0, fboOutline2 = 0;
};

class TextureMatrix : public FrameBufferTexture {
   public:
    TextureMatrix(int subWidth, int subHeight, int column, int row,
                  bool isPixelized = true);
    int subWidth, subHeight, column, row;
    void load(const std::vector<pTexture>& subTextures);
    /*
    * 3 --- 2
    * 0 --- 1
    */
    std::vector<glm::vec2> query_position(int subId);
};

class DepthMap : public FrameBufferTexture {
   public:
    DepthMap(int width, int height);
};
typedef std::shared_ptr<DepthMap> pDepthMap;