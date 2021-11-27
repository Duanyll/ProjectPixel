#pragma once

#include "pch.h"
#include "AssetsHub.h"

namespace RenderMethods {
void fullscreen_quad(pTexture texture);
void skybox(pCubeTexture texture = AssetsHub::get_skybox());
void paperman_standing(glm::vec3 position, float facing, float head_yaw,
                       float head_pitch, EntityMaterial material,
                       bool is_slim = true);
}  // namespace RenderMethods