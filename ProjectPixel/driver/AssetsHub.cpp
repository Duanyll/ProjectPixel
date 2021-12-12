#include "pch.h"
#include "AssetsHub.h"

#include "Uniform.h"
#include "EmbbedAssets.h"
#include "../game/Instructions.h"

std::unordered_map<int, pShaderProgram> AssetsHub::shaderStore;
std::unordered_map<std::string, pVAO> vaoStore;
std::unordered_map<std::string, pTexture> texture2dStore;
std::unordered_map<std::string, pCubeTexture> skyboxStore;
std::unordered_map<std::string, Material> materialStore;

void AssetsHub::load_all() {
    using namespace EmbbedAssets;
    Uniform::init_members();
    register_shader<QuadShader>();
    register_shader<TextShader>();
    register_shader<SkyboxShader>();
    register_shader<EntityShader>();
    register_shader<HUDShader>();

    vaoStore["quad"] = load_quad_vao();
    vaoStore["box"] = load_box_vao();
    vaoStore["skybox"] = load_skybox_vao();
    load_paperman_vaos(vaoStore);
    load_item_vaos(vaoStore);

    for (const auto& i : texturePath) {
        texture2dStore[i.first] = std::make_shared<Texture>(i.second, false);
    }

    for (const auto& i : skyboxPath) {
        skyboxStore[i.first] = std::make_shared<CubeTexture>(i.second);
    }

    for (const auto& key : materialPresets) {
        auto diffuseKey = key;
        auto specularKey = key + "-specular";
        auto emissionKey = key + "-emission";

        auto diffuse = texture2dStore[diffuseKey];

        pTexture specular;
        auto specIt = texture2dStore.find(specularKey);
        if (specIt != texture2dStore.end()) {
            specular = specIt->second;
        } else {
            specular = texture2dStore["no-specular"];
        }

        pTexture emission;
        auto emisIt = texture2dStore.find(emissionKey);
        if (emisIt != texture2dStore.end()) {
            emission = emisIt->second;
        } else {
            emission = texture2dStore["no-emission"];
        }

        materialStore[key] = {diffuse, specular, emission, 32};
    }
}

pVAO AssetsHub::get_vao(const std::string& key) { return vaoStore[key]; }

void AssetsHub::register_vao(const std::string& key, pVAO vao) {
    vaoStore[key] = vao;
}

pTexture AssetsHub::get_texture_2d(const std::string& key) {
    return texture2dStore.at(key);
}

void AssetsHub::register_texture_2d(const std::string& key, pTexture texture) {
    texture2dStore[key] = texture;
}

pCubeTexture AssetsHub::get_skybox(const std::string& key) {
    return skyboxStore.at(key);
}

void AssetsHub::register_skybox(const std::string& key, pCubeTexture texture) {
    skyboxStore[key] = texture;
}

std::unordered_map<ItemType, std::string> itemKey{
    {ItemType::DiamondSword, "diamond-sword"},
    {ItemType::DiamondAxe, "diamond-axe"},
    {ItemType::Bow, "bow"},
    {ItemType::LifePotion, "life-potion"},
    {ItemType::Arrow, "arrow"}};
std::string AssetsHub::get_item_resid(ItemType type) { return itemKey.at(type); }

Material AssetsHub::get_material(const std::string& key) {
    return materialStore.at(key);
}
