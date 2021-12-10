#include "pch.h"
#include "Uniform.h"

UBO::UBO(GLuint programId, const std::string& blockName, int bindingPoint,
         const std::vector<std::string>& members)
    : blockName(blockName), bindingPoint(bindingPoint) {
    glGenBuffers(1, &id);
    glBindBuffer(GL_UNIFORM_BUFFER, id);
    glUseProgram(programId);
    GLuint blockIndex = glGetUniformBlockIndex(programId, blockName.c_str());
    glGetActiveUniformBlockiv(programId, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE,
                              &blockSize);
    glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, id);

    auto names = new const char*[members.size()];
    int idx = 0;
    for (const auto& i : members) {
        names[idx++] = i.c_str();
    }
    auto indices = new GLuint[members.size()];
    glGetUniformIndices(programId, members.size(), names, indices);
    auto offsets = new GLint[members.size()];
    glGetActiveUniformsiv(programId, members.size(), indices, GL_UNIFORM_OFFSET,
                          offsets);
    for (int i = 0; i < members.size(); i++) {
        memberOffsets[names[i]] = offsets[i];
    }
    delete[] names;
    delete[] indices;
    delete[] offsets;
}

UBO::~UBO() { glDeleteBuffers(1, &id); }

std::unordered_map<std::string, pUBO> Uniform::uboStore;
std::unordered_map<std::string, std::vector<std::string>> uniformBlockMembers;

int bindingPointCount;

void Uniform::init_members() {
    uniformBlockMembers["Camera"] = {"view", "projection", "viewPos"};
    uniformBlockMembers["Lights"] = {"useDirLight", "pointLightCount",
                                     "useSpotLight", "ambientLight"};
    auto prefix = [](std::string blockName, std::string pre,
                     std::vector<std::string> vec) -> auto {
        for (auto& i : vec) {
            uniformBlockMembers[blockName].push_back(pre + i);
        }
    };
    prefix("Lights", "dirLight.", {"direction", "diffuse", "specular"});
    for (int i = 0; i < 4; i++) {
        prefix("Lights", std::format("pointLights[{}].", i),
               {"position", "constant", "linear", "quadratic", "diffuse",
                "specular"});
    }
    prefix("Lights", "spotLight.",
           {"position", "direction", "cutOff", "outerCutOff", "constant",
            "linear", "quadratic", "diffuse", "specular"});
}

void Uniform::bind_block(GLuint programId, const std::string& blockName) {
    auto it = uboStore.find(blockName);
    if (it == uboStore.end()) {
        uboStore[blockName] =
            std::make_shared<UBO>(programId, blockName, ++bindingPointCount,
                                  uniformBlockMembers[blockName]);
    }
    glUseProgram(programId);
    GLuint point = uboStore[blockName]->bindingPoint;
    GLuint index = glGetUniformBlockIndex(programId, blockName.c_str());
    glUniformBlockBinding(programId, index, point);
}