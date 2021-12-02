#pragma once

#include "pch.h"
#include "../renderer/VAO.h"
#include "../renderer/Texture.h"
#include "../renderer/Objects.h"

enum class Tile : unsigned char { Air = 0, Stone = 1, Box = 2 };

class ITerrain {
   public:
    // 测试一个点是否与地形碰撞：
    // 大于 0 的值在地形内部，等于 0 在地形表面，小于 0 在地形之外
    virtual int test_point(glm::vec3 point) = 0;
    virtual bool test_line_intersection(glm::vec3 from, glm::vec3 to,
                                        glm::vec3& point) = 0;
    virtual Tile get_tile(glm::vec3 point) = 0;
    virtual Material create_material() = 0;
    virtual pVAO create_vao(Material material) = 0;

    virtual int get_xsize() = 0;
    virtual int get_ysize() = 0;
    virtual int get_zsize() = 0;
};

typedef std::shared_ptr<ITerrain> pTerrain;

class BoxStackTerrain : public ITerrain {
   public:
    // data: xxxxxxxxx
    //       z
    //       z
    //       z
    BoxStackTerrain(int xSize, int zSize, const std::string& data);
    int test_point(glm::vec3 point);
    bool test_line_intersection(glm::vec3 from, glm::vec3 to, glm::vec3& point);
    Tile get_tile(glm::vec3 point);
    Material create_material();
    pVAO create_vao(Material material);

    inline int get_xsize() { return xSize; }
    inline int get_ysize() { return 32; }
    inline int get_zsize() { return zSize; }

   protected:
    int xSize, zSize;
    std::vector<int> heightMap;
};

class TerrainRenderer : public RenderObject {
   public:
    TerrainRenderer(pTerrain terrain);

    void render();

    Material material;
   protected:
    pVAO vao;
};