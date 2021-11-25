#include "pch.h"
#include "AssetsHub.h"

#include "EmbbedAssets.h"

std::unordered_map<int, pShaderProgram> AssetsHub::shaderStore;
std::unordered_map<std::string, pVAO> vaoStore;
std::unordered_map<std::string, pTexture> texture2dStore;
std::unordered_map<std::string, pCubeTexture> skyboxStore;

void AssetsHub::load_all() {
    register_shader<QuadShader>();
    register_shader<TextShader>();

    vaoStore["quad"] = load_quad_vao();

    for (const auto& i : texturePath) {
        texture2dStore[i.first] = std::make_shared<Texture>(i.second, false);
    }

    for (const auto& i : skyboxPath) {
        skyboxStore[i.first] = std::make_shared<CubeTexture>(i.second);
    }
}

pVAO AssetsHub::get_vao(const std::string& key) { return vaoStore[key]; }

void AssetsHub::register_vao(const std::string& key, pVAO vao) {
    vaoStore[key] = vao;
}

pTexture AssetsHub::get_texture_2d(const std::string& key) {
    return texture2dStore[key];
}

void AssetsHub::register_texture_2d(const std::string& key, pTexture texture) {
    texture2dStore[key] = texture;
}

pCubeTexture AssetsHub::get_skybox(const std::string& key) {
    return skyboxStore[key];
}

void AssetsHub::register_skybox(const std::string& key, pCubeTexture texture) {
    skyboxStore[key] = texture;
}
