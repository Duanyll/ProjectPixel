#include "pch.h"
#include "EmbbedAssets.h"

#include "ShaderProgram.h"

std::unordered_map<std::string, std::string> EmbbedAssets::texturePath{
    {"no-specular", "assets/all-black.png"},
    {"no-emission", "assets/all-black.png"},
    {"player", "assets/player-diffuse.png"},
    {"player-specular", "assets/player-specular.png"},
    {"player-emission", "assets/player-emission.png"},
    {"zombie", "assets/zombie-diffuse.png"},
    {"zombie-specular", "assets/zombie-specular.png"},
    {"skeleton", "assets/skeleton-diffuse.png"},
    {"skeleton-specular", "assets/skeleton-specular.png"},
    {"skeleton-emission", "assets/skeleton-emission.png"},
    {"stone", "assets/terrain/smooth_stone.png"},
    {"stone-specular", "assets/terrain/smooth_stone-specular.png"},
    {"planks", "assets/terrain/oak_planks.png"},
    {"planks-specular", "assets/terrain/oak_planks-specular.png"},
    {"diamond-sword", "assets/items/diamond_sword.png"},
    {"diamond-sword-specular", "assets/items/diamond_sword-specular.png"},
    {"diamond-axe", "assets/items/diamond_axe.png"},
    {"diamond-axe-specular", "assets/items/diamond_axe-specular.png"},
    {"bow", "assets/items/bow.png"},
    {"bow1", "assets/items/bow_pulling_0.png"},
    {"bow2", "assets/items/bow_pulling_1.png"},
    {"bow3", "assets/items/bow_pulling_2.png"},
    {"arrow", "assets/items/arrow.png"},
    {"arrow-specular", "assets/items/arrow-specular.png"},
    {"life-potion", "assets/items/life-potion.png"},
    {"heart-empty", "assets/ui/heart-empty.png"},
    {"heart-flashing", "assets/ui/heart-flashing.png"},
    {"heart-half", "assets/ui/heart-half.png"},
    {"heart-full", "assets/ui/heart-full.png"},
    {"crit0", "assets/particles/crit0.png"},
    {"crit1", "assets/particles/crit1.png"},
    {"crit2", "assets/particles/crit2.png"},
    {"crit3", "assets/particles/crit3.png"},
    {"crit4", "assets/particles/crit4.png"},
    {"heal", "assets/particles/heal.png"},
    {"fall0", "assets/particles/fall0.png"},
    {"fall1", "assets/particles/fall1.png"},
    {"death0", "assets/particles/death0.png"},
    {"death1", "assets/particles/death1.png"},
    {"death2", "assets/particles/death2.png"},
    {"death3", "assets/particles/death3.png"},
};
std::vector<std::string> itemVAOToLoad{"diamond-sword", "diamond-axe", "bow",
                                       "bow1",          "bow2",        "bow3",
                                       "arrow",         "life-potion"};
std::vector<std::string> particleVAOToLoad{
    "crit0", "crit1", "crit2",  "crit3",  "crit4",  "heal",
    "fall0", "fall1", "death0", "death1", "death2", "death3"};
std::unordered_map<std::string, std::vector<std::string>>
    EmbbedAssets::skyboxPath{
        {"default",
         {"assets/skybox/right.jpg", "assets/skybox/left.jpg",
          "assets/skybox/top.jpg", "assets/skybox/bottom.jpg",
          "assets/skybox/front.jpg", "assets/skybox/back.jpg"}}};
std::vector<std::string> EmbbedAssets::materialPresets{
    "player",        "zombie",      "skeleton",   "stone", "planks",
    "diamond-sword", "diamond-axe", "bow",        "bow1",  "bow2",
    "bow3",          "arrow",       "life-potion"};

pVAO EmbbedAssets::load_quad_vao() {
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

pVAO EmbbedAssets::load_box_vao() {
    auto res = std::make_shared<VAO>();
    // clang-format off
    float boxVertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };
    // clang-format on
    res->load_interleave_vbo(boxVertices, sizeof boxVertices, {3, 3, 2});
    return res;
}

pVAO EmbbedAssets::load_skybox_vao() {
    auto res = std::make_shared<VAO>();
    // clang-format off
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    // clang-format on
    res->load_interleave_vbo(skyboxVertices, sizeof skyboxVertices, {3});
    return res;
}

using glm::vec2;
using glm::vec3;
using std::vector;

void face(vector<EntityShader::Vertex>& res, vec3 a, vec3 b, vec3 c, vec3 d,
          vec3 normal, vec2 ta, vec2 td) {
    vec2 tb(td.x, ta.y);
    vec2 tc(ta.x, td.y);

    res.insert(res.end(), {{a, normal, ta},
                           {b, normal, tb},
                           {d, normal, td},
                           {a, normal, ta},
                           {d, normal, td},
                           {c, normal, tc}});
}

void generate_cube(vector<EntityShader::Vertex>& r, int w, int d, int h,
                   float resize, int x0, int y0) {
    vec3 A(-w / resize, h / resize, d / resize);
    vec3 B(w / resize, h / resize, d / resize);
    vec3 C(-w / resize, -h / resize, d / resize);
    vec3 D(w / resize, -h / resize, d / resize);
    vec3 E(-w / resize, h / resize, -d / resize);
    vec3 F(w / resize, h / resize, -d / resize);
    vec3 G(-w / resize, -h / resize, -d / resize);
    vec3 H(w / resize, -h / resize, -d / resize);

    face(r, A, B, C, D, vec3(0, 0, 1), vec2(x0 + d, y0 + d),
         vec2(x0 + d + w, y0 + d + h));
    face(r, E, A, G, C, vec3(-1, 0, 0), vec2(x0, y0 + d),
         vec2(x0 + d, y0 + d + h));
    face(r, B, F, D, H, vec3(1, 0, 0), vec2(x0 + d + w, y0 + d),
         vec2(x0 + d + w + d, y0 + d + h));
    face(r, F, E, H, G, vec3(0, 0, -1), vec2(x0 + d + w + d, y0 + d),
         vec2(x0 + d + w + d + w, y0 + d + h));
    face(r, E, F, A, B, vec3(0, 1, 0), vec2(x0 + d, y0),
         vec2(x0 + d + w, y0 + d));
    face(r, C, D, G, H, vec3(0, -1, 0), vec2(x0 + d + w, y0),
         vec2(x0 + d + w + w, y0 + d));
}

pVAO paperman_bodypart(glm::vec3 offset, int w, int d, int h, int x0, int y0,
                       int x1, int y1) {
    pVAO vao = std::make_shared<VAO>();
    vector<EntityShader::Vertex> res;
    generate_cube(res, w, d, h, 2, x0, y0);
    generate_cube(res, w, d, h, 1.8, x1, y1);
    for (auto& i : res) {
        i.pos += offset;
        i.tpos = vec2(i.tpos.x / 64, i.tpos.y / 64);
    }
    vao->load_interleave_vbo(reinterpret_cast<const float*>(res.data()),
                             res.size() * sizeof(EntityShader::Vertex),
                             {3, 3, 2});
    return vao;
}

void EmbbedAssets::load_paperman_vaos(
    std::unordered_map<std::string, pVAO>& data) {
    // Head 8 8 8 0 0 32 0
    // Body 8 4 12 16 16 16 32
    // LArm 4 4 12 32 48 48 48
    // RArm 4 4 12 40 16 40 32
    // LLeg 4 4 12 16 48 0 48
    // RLeg 4 4 12 0 16 0 32
    data["paperman-head"] =
        paperman_bodypart(vec3(0, 0, 0), 8, 8, 8, 0, 0, 32, 0);
    data["paperman-body"] =
        paperman_bodypart(vec3(0, 18, 0), 8, 4, 12, 16, 16, 16, 32);
    data["paperman-larm-slim"] =
        paperman_bodypart(vec3(-0.5, -4, 0), 3, 4, 12, 32, 48, 48, 48);
    data["paperman-rarm-slim"] =
        paperman_bodypart(vec3(0.5, -4, 0), 3, 4, 12, 40, 16, 40, 32);
    data["paperman-larm"] =
        paperman_bodypart(vec3(0, -4, 0), 4, 4, 12, 32, 48, 48, 48);
    data["paperman-rarm"] =
        paperman_bodypart(vec3(0, -4, 0), 4, 4, 12, 40, 16, 40, 32);
    data["paperman-lleg"] =
        paperman_bodypart(vec3(2, -6, 0), 4, 4, 12, 16, 48, 0, 48);
    data["paperman-rleg"] =
        paperman_bodypart(vec3(-2, -6, 0), 4, 4, 12, 0, 16, 0, 32);
}

struct Pixel {
    unsigned char r, g, b, a;
};

pVAO generate_item_vao(const std::string& texturePath) {
    pVAO vao;
    int width, height, channels;
    Pixel* data = reinterpret_cast<Pixel*>(
        stbi_load(texturePath.c_str(), &width, &height, &channels, 4));
    if (data) {
        auto pixel = [&](int x, int y) -> Pixel {
            if (x < 0 || x >= width || y < 0 || y >= height)
                return {0, 0, 0, 0};
            return data[x + y * width];
        };
        vector<EntityShader::Vertex> res;
        float p = 1.0f / width;
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                auto cur = pixel(x, y);
                if (cur.a == 0) continue;
                float x0 = p * x;
                float y0 = p * y;
                vec3 A{x0 + 0, p, y0 + p};
                vec3 B{x0 + p, p, y0 + p};
                vec3 C{x0 + 0, 0, y0 + p};
                vec3 D{x0 + p, 0, y0 + p};
                vec3 E{x0 + 0, p, y0 + 0};
                vec3 F{x0 + p, p, y0 + 0};
                vec3 G{x0 + 0, 0, y0 + 0};
                vec3 H{x0 + p, 0, y0 + 0};
                face(res, E, F, A, B, {0, 1, 0}, {x0, y0}, {x0 + p, y0 + p});
                face(res, C, D, G, H, {0, -1, 0}, {x0, y0}, {x0 + p, y0 + p});
                if (pixel(x, y - 1).a == 0) {
                    face(res, F, E, H, G, {0, 0, -1}, {x0, y0},
                         {x0 + p, y0 + p});
                }
                if (pixel(x, y + 1).a == 0) {
                    face(res, A, B, C, D, {0, 0, 1}, {x0, y0},
                         {x0 + p, y0 + p});
                }
                if (pixel(x + 1, y).a == 0) {
                    face(res, B, F, D, H, {1, 0, 0}, {x0, y0},
                         {x0 + p, y0 + p});
                }
                if (pixel(x - 1, y).a == 0) {
                    face(res, E, A, G, C, {-1, 0, 0}, {x0, y0},
                         {x0 + p, y0 + p});
                }
            }
        }
        vao = std::make_shared<VAO>();
        vao->load_interleave_vbo(reinterpret_cast<const float*>(res.data()),
                                 res.size() * sizeof(EntityShader::Vertex),
                                 {3, 3, 2});
    } else {
        std::cout << "Cannot create item vao from " << texturePath << std::endl;
    }
    stbi_image_free(data);
    return vao;
}

pVAO generate_particle_vao(const std::string& texturePath) {
    pVAO vao;
    int width, height, channels;
    Pixel* data = reinterpret_cast<Pixel*>(
        stbi_load(texturePath.c_str(), &width, &height, &channels, 4));
    if (data) {
        auto pixel = [&](int x, int y) -> Pixel {
            if (x < 0 || x >= width || y < 0 || y >= height)
                return {0, 0, 0, 0};
            return data[x + y * width];
        };
        vector<EntityShader::Vertex> res;
        float p = 1.0f / 16;
        float o = p * width * 0.5;
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                auto cur = pixel(x, y);
                if (cur.a == 0) continue;
                float x0 = p * x;
                float y0 = p * y;
                vec3 A{x0 + 0 - o, p - p / 2, y0 + p - o};
                vec3 B{x0 + p - o, p - p / 2, y0 + p - o};
                vec3 C{x0 + 0 - o, 0 - p / 2, y0 + p - o};
                vec3 D{x0 + p - o, 0 - p / 2, y0 + p - o};
                vec3 E{x0 + 0 - o, p - p / 2, y0 + 0 - o};
                vec3 F{x0 + p - o, p - p / 2, y0 + 0 - o};
                vec3 G{x0 + 0 - o, 0 - p / 2, y0 + 0 - o};
                vec3 H{x0 + p - o, 0 - p / 2, y0 + 0 - o};
                face(res, E, F, A, B, {0, 1, 0}, {x0, y0}, {x0 + p, y0 + p});
                face(res, C, D, G, H, {0, -1, 0}, {x0, y0}, {x0 + p, y0 + p});
                if (pixel(x, y - 1).a == 0) {
                    face(res, F, E, H, G, {0, 0, -1}, {x0, y0},
                         {x0 + p, y0 + p});
                }
                if (pixel(x, y + 1).a == 0) {
                    face(res, A, B, C, D, {0, 0, 1}, {x0, y0},
                         {x0 + p, y0 + p});
                }
                if (pixel(x + 1, y).a == 0) {
                    face(res, B, F, D, H, {1, 0, 0}, {x0, y0},
                         {x0 + p, y0 + p});
                }
                if (pixel(x - 1, y).a == 0) {
                    face(res, E, A, G, C, {-1, 0, 0}, {x0, y0},
                         {x0 + p, y0 + p});
                }
            }
        }
        vao = std::make_shared<VAO>();
        vao->load_interleave_vbo(reinterpret_cast<const float*>(res.data()),
                                 res.size() * sizeof(EntityShader::Vertex),
                                 {3, 3, 2});
    } else {
        std::cout << "Cannot create particle vao from " << texturePath
                  << std::endl;
    }
    stbi_image_free(data);
    return vao;
}

void EmbbedAssets::load_item_vaos(std::unordered_map<std::string, pVAO>& data) {
    for (auto& i : itemVAOToLoad) {
        data[i] = generate_item_vao(texturePath[i]);
    }
}

void EmbbedAssets::load_particle_vaos(
    std::unordered_map<std::string, pVAO>& data) {
    for (auto& i : particleVAOToLoad) {
        data[i] = generate_particle_vao(texturePath[i]);
    }
}
