#pragma once
#include "pch.h"
#include "../utils/Utils.h"
#include "Texture.h"

class ShaderProgram {
   public:
    NO_COPY_CLASS(ShaderProgram)
    ~ShaderProgram();

    GLuint id = 0;

    void use();
    void set_uniform(const std::string& name, int value);
    void set_uniform(const std::string& name, float value);
    void set_uniform(const std::string& name, glm::vec3 value);
    void set_uniform(const std::string& name, glm::mat3 value);
    void set_uniform(const std::string& name, glm::mat4 value);

    void bind_texture(const std::string& name, int slot, pTexture texture);

   protected:
    inline ShaderProgram() {}
    void compile_from_file(const std::string& vertFile,
                           const std::string& fragFile);
    void bind_texture(GLint location, int slot, pTexture texture);
};

typedef std::shared_ptr<ShaderProgram> pShaderProgram;

// 将一个矩形材质绘制到全屏
class BlitShader : public ShaderProgram {
   public:
    BlitShader();
    void configure(pTexture texture);

    const static int storeId = 1;

   private:
    GLint screenTexture_pos;
};

class TextShader : public ShaderProgram {
   public:
    TextShader();
    void configure(glm::vec3 textColor);

    const static int storeId = 2;

   private:
    GLint textColor_pos;
    GLint text_pos;
};

class SkyboxShader : public ShaderProgram {
   public:
    SkyboxShader();
    void configure(pCubeTexture texture);

    const static int storeId = 3;

   private:
    GLint skybox_pos;
};

class EntityShader : public ShaderProgram {
   public:
    EntityShader();
    void configure(const Material& material, glm::mat4 model);

    const static int storeId = 4;

    struct Vertex {
        glm::vec3 pos;
        glm::vec3 norm;
        glm::vec2 tpos;
    };

   private:
    GLint diffuse_pos, specular_pos, emission_pos, shininess_pos;
    GLint model_pos, normal_pos;
    GLint dirLightDepth_pos, spotLightDepth_pos;
};

class HUDShader : public ShaderProgram {
   public:
    HUDShader();
    void configure(pTexture texture);

    const static int storeId = 5;

   private:
    GLint img_pos;
};

class DepthShader : public ShaderProgram {
   public:
    DepthShader();
    void configure(glm::mat4 model);

    const static int storeId = 6;

   private:
    GLint model_pos;
};

class ParticleShader : public ShaderProgram {
   public:
    ParticleShader();
    void configure(pTexture diffuse, pTexture emission, glm::mat4 model,
                   glm::vec4 color);

    const static int storeId = 7;

   private:
    GLint diffuse_pos, emission_pos, color_pos;
    GLint model_pos, normal_pos;
    GLint dirLightDepth_pos, spotLightDepth_pos;
};

class GaussianBlurShader : public ShaderProgram {
   public:
    GaussianBlurShader();
    void configure(pTexture texture, glm::vec2 resolution, glm::vec2 direction);

    const static int storeId = 8;

   private:
    GLint screenTexture_pos;
    GLint resolution_pos, direction_pos;
};

class SingleColorShader : public ShaderProgram {
   public:
    SingleColorShader();
    void configure(glm::vec4 color);

    const static int storeId = 9;

   private:
    GLint color_pos;
};