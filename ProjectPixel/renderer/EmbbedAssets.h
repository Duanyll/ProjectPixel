#pragma once
#include "pch.h"
#include "VAO.h"

extern std::unordered_map<std::string, std::string> texturePath;
extern std::unordered_map<std::string, std::vector<std::string>> skyboxPath;

pVAO load_quad_vao();