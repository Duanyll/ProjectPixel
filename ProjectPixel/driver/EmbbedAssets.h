#pragma once
#include "pch.h"
#include "VAO.h"

namespace EmbbedAssets {
extern std::unordered_map<std::string, std::string> texturePath;
extern std::unordered_map<std::string, std::vector<std::string>> skyboxPath;

pVAO load_quad_vao();
pVAO load_box_vao();
pVAO load_skybox_vao();
void load_paperman_vaos(std::unordered_map<std::string, pVAO>& data);
void load_item_vaos(std::unordered_map<std::string, pVAO>& data);
}  // namespace EmbbedAssets