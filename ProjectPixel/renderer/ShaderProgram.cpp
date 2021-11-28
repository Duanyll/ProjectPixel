#include "pch.h"
#include "ShaderProgram.h"

#include "Uniform.h"

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

QuadShader::QuadShader() {
    compile_from_file("shaders/quad.vert", "shaders/quad.frag");
    use();
    screenTexture_pos = glGetUniformLocation(id, "screenTexture");
}

void QuadShader::configure(pTexture texture) {
    use();
    bind_texture(screenTexture_pos, 0, texture);
}

TextShader::TextShader() {
    compile_from_file("shaders/text.vert", "shaders/text.frag");
    use();
    projection_pos = glGetUniformLocation(id, "projection2d");
    textColor_pos = glGetUniformLocation(id, "textColor");
    text_pos = glGetUniformLocation(id, "text");
}

void TextShader::configure(glm::mat4 projection, glm::vec3 textColor) { 
    use(); 
    glUniformMatrix4fv(projection_pos, 1, false, glm::value_ptr(projection));
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
}

void EntityShader::configure(const EntityMaterial& material, glm::mat4 model) {
    use();
    bind_texture(diffuse_pos, 0, material.diffuse);
    bind_texture(specular_pos, 1, material.specular);
    bind_texture(emission_pos, 2, material.emission);
    glUniform1f(shininess_pos, material.shininess);
    glUniformMatrix4fv(model_pos, 1, false, glm::value_ptr(model));
    glUniformMatrix3fv(
        normal_pos, 1, false,
        glm::value_ptr(glm::mat3(glm::transpose(glm::inverse(model)))));
}