#pragma once

#include "pch.h"
#include "AssetsHub.h"

namespace RenderMethods {
void fullscreen_quad(pTexture texture);
void skybox(pCubeTexture texture = AssetsHub::get_skybox());
}