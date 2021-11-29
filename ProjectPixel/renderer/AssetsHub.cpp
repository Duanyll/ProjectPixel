#include "pch.h"
#include "AssetsHub.h"

#include "Uniform.h"
#include "EmbbedAssets.h"

std::unordered_map<int, pShaderProgram> AssetsHub::shaderStore;
std::unordered_map<std::string, pVAO> vaoStore;
std::unordered_map<std::string, pTexture> texture2dStore;
std::unordered_map<std::string, pCubeTexture> skyboxStore;

void AssetsHub::load_all() {
    using namespace EmbbedAssets;
    Uniform::init_members();
    register_shader<QuadShader>();
    register_shader<TextShader>();
    register_shader<SkyboxShader>();
    register_shader<EntityShader>();

    vaoStore["quad"] = load_quad_vao();
    vaoStore["box"] = load_box_vao();
    vaoStore["skybox"] = load_skybox_vao();
    load_paperman_vaos(vaoStore);

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
    auto& res = texture2dStore[key];
    assert(res);
    return res;
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
