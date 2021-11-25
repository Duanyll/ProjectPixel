#include "pch.h"
#include "ShaderProgram.h"

#include <fstream>
#include <sstream>

void ShaderProgram::compile_from_file(const std::string& vertFile,
                                      const std::string& fragFile) {
    // 1. ���ļ�·���л�ȡ����/Ƭ����ɫ��
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // ��֤ifstream��������׳��쳣��
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        // ���ļ�
        vShaderFile.open(vertFile);
        fShaderFile.open(fragFile);
        std::stringstream vShaderStream, fShaderStream;
        // ��ȡ�ļ��Ļ������ݵ���������
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // �ر��ļ�������
        vShaderFile.close();
        fShaderFile.close();
        // ת����������string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    } catch (std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // 2. ������ɫ��
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // ������ɫ��
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    // ��ӡ�����������еĻ���
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    };

    // Ƭ����ɫ��
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    // ��ӡ�����������еĻ���
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    };

    // ��ɫ������
    id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);
    // ��ӡ���Ӵ�������еĻ���
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(id, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }

    // ɾ����ɫ���������Ѿ����ӵ����ǵĳ������ˣ��Ѿ�������Ҫ��
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

QuadShader::QuadShader() {
    compile_from_file("shaders/quad.vert", "shaders/quad.frag");
}

void QuadShader::configure(pTexture texture) {
    use();
    bind_texture("screenTexture", 0, texture);
}
