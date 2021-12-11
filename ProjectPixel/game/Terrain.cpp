#include "pch.h"
#include "Terrain.h"

#include "../utils/Utils.h"
#include "../driver/AssetsHub.h"

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

void BoxStackTerrain::get_bounding_boxes(glm::vec3 point,
                                         std::vector<TileBoundingBox>& res) {
    int x = floor(point.x);
    int y = floor(point.y);
    int z = floor(point.z);
    if (get_tile({x, y, z}) != Tile::Air) {
        res.push_back({{x, y, z}, {1, 1, 1}});
    }
}

Tile BoxStackTerrain::get_tile(glm::vec3 point) {
    int x = floor(point.x);
    int y = floor(point.y);
    int z = floor(point.z);
    if (x < 0 || x >= xSize || z < 0 || z >= zSize) return Tile::Stone;
    if (y < 0) return Tile::Stone;
    int height = heightMap[z * xSize + x];
    return (y < height) ? Tile::Box : Tile::Air;
}

Material BoxStackTerrain::create_material() {
    auto diffuseMatrix = std::make_shared<TextureMatrix>(16, 16, 4, 4);
    auto specularMatrix = std::make_shared<TextureMatrix>(16, 16, 4, 4);
    auto emissionMatrix = std::make_shared<TextureMatrix>(16, 16, 4, 4);

    std::vector<Material> tiles{AssetsHub::get_material("stone"),
                                AssetsHub::get_material("planks")};

    std::vector<pTexture> diffuse, specular, emission;
    for (auto& i : tiles) {
        diffuse.push_back(i.diffuse);
        specular.push_back(i.specular);
        emission.push_back(i.emission);
    }

    diffuseMatrix->load(diffuse);
    specularMatrix->load(specular);
    emissionMatrix->load(emission);
    return {diffuseMatrix, specularMatrix, emissionMatrix, 32};
}

pVAO BoxStackTerrain::create_vao(Material material) {
    std::vector<EntityShader::Vertex> res;
    auto face = [&](glm::vec3 A, glm::vec3 B, glm::vec3 C, glm::vec3 D,
                    glm::vec3 norm, const std::vector<glm::vec2>& texPos) {
        res.insert(res.end(), {{A, norm, texPos[3]},
                               {D, norm, texPos[0]},
                               {C, norm, texPos[1]},
                               {A, norm, texPos[3]},
                               {C, norm, texPos[1]},
                               {B, norm, texPos[2]}});
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
                             res.size() * sizeof(EntityShader::Vertex),
                             {3, 3, 2});
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

bool ITerrain::test_point_intersection(glm::vec3 point) {
    std::vector<TileBoundingBox> boxes;
    get_bounding_boxes(point, boxes);
    get_bounding_boxes({point.x, point.y - 1, point.z}, boxes);
    for (auto& i : boxes) {
        if (i.test_point_inside(point)) return true;
    }
    return false;
}

bool base_line_intersection(std::vector<TileBoundingBox>& boxes,
                            glm::vec3 point, glm::vec3 dir, float& dis) {
    float res = 10000;
    for (auto& i : boxes) {
        float d = -1;
        if (i.test_line_intersection(point, dir, d)) {
            res = std::min(res, d);
        }
    }
    if (res < 10000) {
        dis = res;
        return true;
    } else {
        return false;
    }
}

bool ITerrain::base_line_intersection(glm::vec3 point, glm::vec3 dir,
                                      float& dis) {
    float len = glm::length(dir);
    auto boxes =
        get_bounding_boxes_range(std::minmax(point.x, point.x + dir.x),
                                 std::minmax(point.y, point.y + dir.y),
                                 std::minmax(point.z, point.z + dir.z));
    float d = -1;
    if (::base_line_intersection(boxes, point, glm::normalize(dir), d)) {
        if (d <= len) {
            dis = d;
            return true;
        }
    }
    return false;
}

bool ITerrain::base_box_movement_intersection(TileBoundingBox box,
                                              glm::vec3 dir, float& dis) {
    float len = glm::length(dir);
    auto boxes = get_bounding_boxes_range(
        std::minmax({box.a.x, box.a.x + box.s.x, box.a.x + dir.x,
                     box.a.x + dir.x + box.s.x}),
        std::minmax({box.a.y, box.a.y + box.s.y, box.a.y + dir.y,
                     box.a.y + dir.y + box.s.y}),
        std::minmax({box.a.z, box.a.z + box.s.z, box.a.z + dir.z,
                     box.a.z + dir.z + box.s.z}));
    for (auto& i : boxes) {
        i.a -= box.s;
        i.s += box.s;
    }
    float d = -1;
    if (::base_line_intersection(boxes, box.a, glm::normalize(dir), d)) {
        if (d <= len) {
            dis = d;
            return true;
        }
    }
    return false;
}

const float LINE_INTERSECTION_STEP = 1.2f;
const float LINE_INTERSECTION_FINAL = 1.5f;
const float CLIPPING_TEST_DELTA = 1e-3;

bool ITerrain::test_line_intersection(glm::vec3 point, glm::vec3 dir,
                                      float& dis) {
    auto end = point + dir;
    float remain = glm::length(dir);
    if (remain < CLIPPING_TEST_DELTA) {
        if (test_point_intersection(point)) {
            dis = CLIPPING_TEST_DELTA / 2;
            return true;
        } else {
            return false;
        }
    }
    auto step = glm::normalize(dir) * LINE_INTERSECTION_STEP;
    while (remain > LINE_INTERSECTION_FINAL) {
        float d = -1;
        if (base_line_intersection(point, step, d)) {
            dis = d;
            return true;
        }
        remain -= LINE_INTERSECTION_STEP;
        point += step;
    }
    return base_line_intersection(point, end - point, dis);
}

bool ITerrain::test_box_intersection(TileBoundingBox box) {
    auto boxes = get_bounding_boxes_range({box.a.x, box.a.x + box.s.x},
                                          {box.a.y, box.a.y + box.s.y},
                                          {box.a.z, box.a.z + box.s.z});
    for (auto& i : boxes) {
        i.a -= box.s;
        i.s += box.s;
        if (i.test_point_inside(box.a)) return true;
    }
    return false;
}

bool ITerrain::test_box_movement_intersection(TileBoundingBox box,
                                              glm::vec3 dir, float& dis) {
    auto end = box.a + dir;
    float remain = glm::length(dir);
    if (remain < CLIPPING_TEST_DELTA) {
        if (test_box_intersection(box)) {
            dis = CLIPPING_TEST_DELTA / 2;
            return true;
        } else {
            return false;
        }
    }
    auto step = glm::normalize(dir) * LINE_INTERSECTION_STEP;
    while (remain > LINE_INTERSECTION_FINAL) {
        float d = -1;
        if (base_box_movement_intersection(box, step, d)) {
            dis = d;
            return true;
        }
        remain -= LINE_INTERSECTION_STEP;
        box.a += step;
    }
    return base_box_movement_intersection(box, end - box.a, dis);
}

BoxClipping ITerrain::clip_point(glm::vec3& point) {
    BoxClipping result = BoxClipping::None;
    const float d = CLIPPING_TEST_DELTA;
    auto boxes = get_bounding_boxes_range({point.x - d, point.x + d},
                                          {point.y - d, point.y + d},
                                          {point.z - d, point.z + d});
    BoxClipping _;
    for (auto& i : boxes) {
        point = i.clip_point(point, _);
    }
    for (int i = 0; i < 6; i++) {
        auto p = point + d * boxClippingDelta[i];
        for (auto& box : boxes) {
            if (box.test_point_inside(p)) {
                result |= (BoxClipping)(1 << i);
                break;
            }
        }
    }
    return result;
}

BoxClipping ITerrain::clip_box(TileBoundingBox& box) {
    BoxClipping result = BoxClipping::None;
    const float d = CLIPPING_TEST_DELTA;
    auto boxes = get_bounding_boxes_range({box.a.x - d, box.a.x + box.s.x + d},
                                          {box.a.y - d, box.a.y + box.s.y + d},
                                          {box.a.z - d, box.a.z + box.s.z + d});
    BoxClipping _;
    for (auto& i : boxes) {
        i.a -= box.s;
        i.s += box.s;
        box.a = i.clip_point(box.a, _);
    }
    for (int i = 0; i < 6; i++) {
        auto p = box.a + d * boxClippingDelta[i];
        for (auto& t : boxes) {
            if (t.test_point_inside(p)) {
                result |= (BoxClipping)(1 << i);
                break;
            }
        }
    }
    return result;
}

bool ITerrain::test_connectivity(glm::vec3 a, glm::vec3 b) {
    auto dir = b - a;
    auto dis = glm::length(dir);
    if (dis < 0.01) {
        return !test_point_intersection(a);
    }
    float d = -1;
    return !test_line_intersection(a, dir, d);
}

std::vector<TileBoundingBox> ITerrain::get_bounding_boxes_range(
    std::pair<float, float> x, std::pair<float, float> y,
    std::pair<float, float> z) {
    std::vector<TileBoundingBox> boxes;
    int xmin = floor(x.first);
    int xmax = ceil(x.second);
    int ymin = floor(y.first);
    int ymax = ceil(y.second);
    int zmin = floor(z.first);
    int zmax = ceil(z.second);
    for (int x = xmin; x < xmax; x++) {
        for (int y = ymin; y < ymax; y++) {
            for (int z = zmin; z < zmax; z++) {
                get_bounding_boxes({x, y, z}, boxes);
            }
        }
    }
    return boxes;
}
