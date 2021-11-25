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
    void compile_from_file(const std::string& vertFile, const std::string& fragFile);
};

typedef std::shared_ptr<ShaderProgram> pShaderProgram;

// 将一个矩形材质绘制到全屏
class QuadShader : public ShaderProgram {
   public:
    QuadShader();
    void configure(pTexture texture);

    const static int storeId = 1;
};