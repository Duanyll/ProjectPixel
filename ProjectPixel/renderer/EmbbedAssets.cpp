#include "pch.h"
#include "EmbbedAssets.h"

std::unordered_map<std::string, std::string> texturePath{
    {"default", "assets/container.jpg"}};
std::unordered_map<std::string, std::vector<std::string>> skyboxPath{
    {"default",
     {"assets/skybox/right.jpg", "assets/skybox/left.jpg",
      "assets/skybox/top.jpg", "assets/skybox/bottom.jpg",
      "assets/skybox/front.jpg", "assets/skybox/back.jpg"}}};

pVAO load_quad_vao() {
    auto res = std::make_shared<VAO>();
    // clang-format off
    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
};
    // clang-format on
    res->load_interleave_vbo(quadVertices, sizeof quadVertices, {2, 2});
    return res;
}
