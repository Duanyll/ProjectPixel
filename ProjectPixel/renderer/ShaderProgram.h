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
class QuadShader : public ShaderProgram {
   public:
    QuadShader();
    void configure(pTexture texture);

    const static int storeId = 1;

   private:
    GLint screenTexture_pos;
};

class TextShader : public ShaderProgram {
   public:
    TextShader();
    void configure(glm::mat4 projection, glm::vec3 textColor);

    const static int storeId = 2;

   private:
    GLint projection_pos;
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
    void configure(const EntityMaterial& material, glm::mat4 model);

    const static int storeId = 4;

   private:
    GLint diffuse_pos, specular_pos, emission_pos, shininess_pos;
    GLint model_pos, normal_pos;
};