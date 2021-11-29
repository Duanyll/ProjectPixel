#include "pch.h"
#include "Terrain.h"

#include "../utils/Utils.h"
#include "../renderer/AssetsHub.h"
#include <cctype>

BoxStackTerrain::BoxStackTerrain(int xSize, int zSize, const std::string& data)
    : xSize(xSize), zSize(zSize) {
    heightMap.reserve(xSize * zSize);
    for (const auto& i : data) {
        if (isdigit(i)) {
            heightMap.push_back(i - '0');
        }
    }
    assert(heightMap.size() == xSize * zSize);
}

int BoxStackTerrain::test_point(glm::vec3 point) {
    int x = floor(point.x);
    int z = floor(point.z);
    if (x < 0 || x >= xSize || z < 0 || z >= zSize) return 1;
    int height = heightMap[z * xSize + x];
    return dcmp(float(height), point.y);
}

bool BoxStackTerrain::test_line_intersection(glm::vec3 from, glm::vec3 to,
                                             glm::vec3& point) {
    // TODO: implement line intersection test
    return false;
}

Tile BoxStackTerrain::get_tile(glm::vec3 point) {
    int x = floor(point.x);
    int y = floor(point.y);
    int z = floor(point.z);
    if (x < 0 || x >= xSize || z < 0 || z >= zSize) return Tile::Air;
    if (y < 0) return Tile::Stone;
    int height = heightMap[z * xSize + x];
    return (y < height) ? Tile::Box : Tile::Air;
}

Material BoxStackTerrain::create_material() {
    auto diffuseMatrix = std::make_shared<TextureMatrix>(16, 16, 4, 4);
    auto specularMatrix = std::make_shared<TextureMatrix>(16, 16, 4, 4);
    diffuseMatrix->load({AssetsHub::get_texture_2d("terrain-stone-diffuse"),
                         AssetsHub::get_texture_2d("terrain-planks-diffuse")});
    specularMatrix->load(
        {AssetsHub::get_texture_2d("terrain-stone-specular"),
         AssetsHub::get_texture_2d("terrain-planks-specular")});
    return {diffuseMatrix, specularMatrix,
            AssetsHub::get_texture_2d("no-emission"), 32};
}

struct vertex {
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 tpos;
};

pVAO BoxStackTerrain::create_vao(Material material) {
    std::vector<vertex> res;
    auto face = [&](glm::vec3 A, glm::vec3 B, glm::vec3 C, glm::vec3 D,
                    glm::vec3 norm, const std::vector<glm::vec2>& texPos) {
        res.insert(res.end(), {{A, norm, texPos[3]},
                               {D, norm, texPos[0]},
                               {C, norm, texPos[1]},
                               {A, norm, texPos[3]},
                               {C, norm, texPos[1]},
                               {B, norm, texPos[2]}});
        /*res.insert(res.end(), {{A, norm, {0.0f, 1.0f}},
                               {D, norm, {0.0f, 0.0f}},
                               {C, norm, {1.0f, 0.0f}},
                               {A, norm, {0.0f, 1.0f}},
                               {C, norm, {1.0f, 0.0f}},
                               {B, norm, {1.0f, 1.0f}}});*/
    };
    auto getHeight = [&](int x, int z) {
        if (x < 0 || x >= xSize || z < 0 || z >= zSize) return 0;
        return heightMap[z * xSize + x];
    };
    auto stone_texpos =
        std::dynamic_pointer_cast<TextureMatrix>(material.diffuse)
            ->query_position(0);
    auto box_texpos = std::dynamic_pointer_cast<TextureMatrix>(material.diffuse)
                          ->query_position(1);
    for (int x = 0; x < xSize; x++) {
        for (int z = 0; z < zSize; z++) {
            int height = heightMap[z * xSize + x];
            for (int y = 0; y < height; y++) {
                glm::vec3 A{x, y + 1, z};
                glm::vec3 B{x + 1, y + 1, z};
                glm::vec3 C{x + 1, y + 1, z + 1};
                glm::vec3 D{x, y + 1, z + 1};
                glm::vec3 E{x, y, z};
                glm::vec3 F{x + 1, y, z};
                glm::vec3 G{x + 1, y, z + 1};
                glm::vec3 H{x, y, z + 1};
                if (y >= getHeight(x - 1, z)) {
                    face(A, D, H, E, {-1, 0, 0}, box_texpos);
                }
                if (y >= getHeight(x, z - 1)) {
                    face(B, A, E, F, {0, 0, -1}, box_texpos);
                }
                if (y >= getHeight(x + 1, z)) {
                    face(C, B, F, G, {1, 0, 0}, box_texpos);
                }
                if (y >= getHeight(x, z + 1)) {
                    face(D, C, G, H, {0, 0, 1}, box_texpos);
                }
            }
            glm::vec3 A{x, height, z};
            glm::vec3 B{x + 1, height, z};
            glm::vec3 C{x + 1, height, z + 1};
            glm::vec3 D{x, height, z + 1};
            face(A, B, C, D, {0, 1, 0},
                 (height == 0) ? stone_texpos : box_texpos);
        }
    }
    auto vao = std::make_shared<VAO>();
    vao->load_interleave_vbo(reinterpret_cast<float*>(res.data()),
                             res.size() * sizeof(vertex), {3, 3, 2});
    return vao;
}

TerrainRenderer::TerrainRenderer(pTerrain terrain) {
    material = terrain->create_material();
    vao = terrain->create_vao(material);
}

void TerrainRenderer::render() {
    auto shader = AssetsHub::get_shader<EntityShader>();
    shader->configure(material, glm::mat4());
    vao->draw();
}
