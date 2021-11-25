#pragma once

#include "pch.h"
#include "VAO.h"
#include "Texture.h"
#include "ShaderProgram.h"

namespace AssetsHub {
void load_all();
pVAO get_vao(const std::string& key);
void register_vao(const std::string& key, pVAO vao);
pTexture get_texture_2d(const std::string& key);
void register_texture_2d(const std::string& key, pTexture texture);
pCubeTexture get_skybox(const std::string& key = "default");
void register_skybox(const std::string& key, pCubeTexture texture);

extern std::unordered_map<int, pShaderProgram> shaderStore;
template <typename T>
std::shared_ptr<T> get_shader() {
    return std::static_pointer_cast<T>(shaderStore[T::storeId]);
}
template <typename T>
void register_shader() {
    shaderStore[T::storeId] = std::static_pointer_cast<ShaderProgram>(std::make_shared<T>());
}
}  // namespace AssetsHub