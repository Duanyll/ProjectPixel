#pragma once

#include "pch.h"
#include "../utils/Utils.h"

class UBO {
   public:
    std::unordered_map<std::string, size_t> memberOffsets;
    GLuint id = 0;
    int bindingPoint = 0;
    std::string blockName;
    GLint blockSize = 0;
    NO_COPY_CLASS(UBO)
    UBO(GLuint programId, const std::string& blockName, int bindingPoint,
        const std::vector<std::string>& members);
    ~UBO();

    template<typename T>
    void set_data(const std::string& member, T data) {
        glBindBuffer(GL_UNIFORM_BUFFER, id);
        glBufferSubData(GL_UNIFORM_BUFFER, memberOffsets[member], sizeof(T),
                        &data);
    }
};

typedef std::shared_ptr<UBO> pUBO;

namespace Uniform {
void bind_block(GLuint programId, const std::string& blockName);
extern std::unordered_map<std::string, pUBO> uboStore;
template<typename T>
void set_data(const std::string& blockName, const std::string& memberName,
    T data) {
    uboStore[blockName]->set_data(memberName, data);
}
}