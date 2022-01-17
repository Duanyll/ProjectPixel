#include "pch.h"
#include "ShaderProgram.h"

#include "Uniform.h"
#include "Light.h"

#include <fstream>
#include <sstream>

void ShaderProgram::compile_from_file(const std::string& vertFile,
                                      const std::string& fragFile) {
    // 1. 从文件路径中获取顶点/片段着色器
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // 保证ifstream对象可以抛出异常：
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        // 打开文件
        vShaderFile.open(vertFile);
        fShaderFile.open(fragFile);
        std::stringstream vShaderStream, fShaderStream;
        // 读取文件的缓冲内容到数据流中
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // 关闭文件处理器
        vShaderFile.close();
        fShaderFile.close();
        // 转换数据流到string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    } catch (std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // 2. 编译着色器
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // 顶点着色器
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    // 打印编译错误（如果有的话）
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    };

    // 片段着色器
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    // 打印编译错误（如果有的话）
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    };

    // 着色器程序
    id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);
    // 打印连接错误（如果有的话）
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(id, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }

    // 删除着色器，它们已经链接到我们的程序中了，已经不再需要了
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

ShaderProgram::~ShaderProgram() { glDeleteProgram(id); }

GLuint currentShader;
void ShaderProgram::use() {
    if (currentShader != id) {
        glUseProgram(id);
        currentShader = id;
    }
}

void ShaderProgram::set_uniform(const std::string& name, int value) {
    use();
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void ShaderProgram::set_uniform(const std::string& name, float value) {
    use();
    glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void ShaderProgram::set_uniform(const std::string& name, glm::vec3 value) {
    use();
    glUniform3fv(glGetUniformLocation(id, name.c_str()), 1,
                 glm::value_ptr(value));
}

void ShaderProgram::set_uniform(const std::string& name, glm::mat3 value) {
    use();
    glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, false,
                       glm::value_ptr(value));
}

void ShaderProgram::set_uniform(const std::string& name, glm::mat4 value) {
    use();
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, false,
                       glm::value_ptr(value));
}

void ShaderProgram::bind_texture(const std::string& name, int slot,
                                 pTexture texture) {
    use();
    set_uniform(name, slot);
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(texture->get_type(), texture->id);
}

void ShaderProgram::bind_texture(GLint location, int slot, pTexture texture) {
    use();
    glUniform1i(location, slot);
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(texture->get_type(), texture->id);
}

BlitShader::BlitShader() {
    compile_from_file("shaders/quad.vert", "shaders/blit.frag");
    use();
    screenTexture_pos = glGetUniformLocation(id, "screenTexture");
}

void BlitShader::configure(pTexture texture) {
    use();
    bind_texture(screenTexture_pos, 0, texture);
}

TextShader::TextShader() {
    compile_from_file("shaders/text.vert", "shaders/text.frag");
    use();
    Uniform::bind_block(id, "Screen");    
    textColor_pos = glGetUniformLocation(id, "textColor");
    text_pos = glGetUniformLocation(id, "text");
}

void TextShader::configure(glm::vec3 textColor) { 
    use(); 
    glUniform3fv(textColor_pos, 1, glm::value_ptr(textColor));
    glUniform1i(text_pos, 0);
}

SkyboxShader::SkyboxShader() {
    compile_from_file("shaders/skybox.vert", "shaders/skybox.frag");
    use();
    Uniform::bind_block(id, "Camera");
    skybox_pos = glGetUniformLocation(id, "skybox");
}

void SkyboxShader::configure(pCubeTexture texture) {
    use();
    bind_texture(skybox_pos, 0, texture);
}

EntityShader::EntityShader() {
    compile_from_file("shaders/entity.vert", "shaders/entity.frag");
    use();
    Uniform::bind_block(id, "Camera");
    Uniform::bind_block(id, "Lights");

    diffuse_pos = glGetUniformLocation(id, "material.diffuse");
    specular_pos = glGetUniformLocation(id, "material.specular");
    emission_pos = glGetUniformLocation(id, "material.emission");
    shininess_pos = glGetUniformLocation(id, "material.shininess");
    model_pos = glGetUniformLocation(id, "model");
    normal_pos = glGetUniformLocation(id, "normal");
    dirLightDepth_pos = glGetUniformLocation(id, "dirLightDepth");
    spotLightDepth_pos = glGetUniformLocation(id, "spotLightDepth");
}

void EntityShader::configure(const Material& material, glm::mat4 model) {
    use();
    bind_texture(diffuse_pos, 0, material.diffuse);
    bind_texture(specular_pos, 1, material.specular);
    bind_texture(emission_pos, 2, material.emission);
    bind_texture(dirLightDepth_pos, 3, Lights.dirLight.depthMap);
    bind_texture(spotLightDepth_pos, 4, Lights.spotLight.depthMap);
    glUniform1f(shininess_pos, material.shininess);
    glUniformMatrix4fv(model_pos, 1, false, glm::value_ptr(model));
    glUniformMatrix3fv(
        normal_pos, 1, false,
        glm::value_ptr(glm::mat3(glm::transpose(glm::inverse(model)))));
}

HUDShader::HUDShader() {
    compile_from_file("shaders/hud.vert", "shaders/hud.frag");
    use();
    Uniform::bind_block(id, "Screen");
    img_pos = glGetUniformLocation(id, "img");
}

void HUDShader::configure(pTexture img) {
    use();
    bind_texture(img_pos, 0, img);
}

DepthShader::DepthShader() {
    compile_from_file("shaders/depth.vert", "shaders/empty.frag");
    use();
    Uniform::bind_block(id, "LightSpace");
    model_pos = glGetUniformLocation(id, "model");
}

void DepthShader::configure(glm::mat4 model) {
    use();
    glUniformMatrix4fv(model_pos, 1, false, glm::value_ptr(model));
}

ParticleShader::ParticleShader() {
    compile_from_file("shaders/entity.vert", "shaders/particle.frag");
    use();
    Uniform::bind_block(id, "Camera");
    Uniform::bind_block(id, "Lights");

    diffuse_pos = glGetUniformLocation(id, "material.diffuse");
    emission_pos = glGetUniformLocation(id, "material.emission");
    color_pos = glGetUniformLocation(id, "material.color");
    model_pos = glGetUniformLocation(id, "model");
    normal_pos = glGetUniformLocation(id, "normal");
    dirLightDepth_pos = glGetUniformLocation(id, "dirLightDepth");
    spotLightDepth_pos = glGetUniformLocation(id, "spotLightDepth");
}

void ParticleShader::configure(pTexture diffuse, pTexture emission,
                               glm::mat4 model, glm::vec4 color) {
    use();
    bind_texture(diffuse_pos, 0, diffuse);
    bind_texture(emission_pos, 1, emission);
    bind_texture(dirLightDepth_pos, 2, Lights.dirLight.depthMap);
    bind_texture(spotLightDepth_pos, 3, Lights.spotLight.depthMap);
    glUniformMatrix4fv(model_pos, 1, false, glm::value_ptr(model));
    glUniformMatrix3fv(
        normal_pos, 1, false,
        glm::value_ptr(glm::mat3(glm::transpose(glm::inverse(model)))));
    glUniform4fv(color_pos, 1, glm::value_ptr(color));
}

GaussianBlurShader::GaussianBlurShader() {
    compile_from_file("shaders/quad.vert", "shaders/gaussian-blur.frag");
    use();

    screenTexture_pos = glGetUniformLocation(id, "screenTexture");
    resolution_pos = glGetUniformLocation(id, "resolution");
    direction_pos = glGetUniformLocation(id, "direction");
}

void GaussianBlurShader::configure(pTexture texture, glm::vec2 resolution,
                                   glm::vec2 direction) {
    use();
    bind_texture(screenTexture_pos, 0, texture);
    glUniform2fv(resolution_pos, 1, glm::value_ptr(resolution));
    glUniform2fv(direction_pos, 1, glm::value_ptr(direction));
}

SingleColorShader::SingleColorShader() {
    compile_from_file("shaders/quad.vert", "shaders/single-color.frag");
    use();
    
    color_pos = glGetUniformLocation(id, "color");
}

void SingleColorShader::configure(glm::vec4 color) {
    use();
    glUniform4fv(color_pos, 1, glm::value_ptr(color));
}
