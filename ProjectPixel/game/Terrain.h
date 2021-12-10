#pragma once

#include "pch.h"
#include "../driver/VAO.h"
#include "../driver/Texture.h"
#include "Objects.h"
#include "../utils/Geomentry.h"

enum class Tile : unsigned char { Air = 0, Stone = 1, Box = 2 };

class ITerrain {
   public:
    virtual void get_bounding_boxes(glm::vec3 point,
                                    std::vector<TileBoundingBox>& res) = 0;
    virtual bool test_point_intersection(glm::vec3 point);
    virtual bool test_line_intersection(glm::vec3 point, glm::vec3 dir,
                                        float& dis);
    virtual bool test_box_intersection(TileBoundingBox box);
    virtual bool test_box_movement_intersection(TileBoundingBox box,
                                                glm::vec3 dir, float& dis);
    virtual BoxClipping clip_point(glm::vec3& point);
    virtual BoxClipping clip_box(TileBoundingBox& box);
    virtual bool test_connectivity(glm::vec3 a, glm::vec3 b);

    virtual Tile get_tile(glm::vec3 point) = 0;
    virtual Material create_material() = 0;
    virtual pVAO create_vao(Material material) = 0;

    virtual int get_xsize() = 0;
    virtual int get_ysize() = 0;
    virtual int get_zsize() = 0;

   private:
    std::vector<TileBoundingBox> get_bounding_boxes_range(
        std::pair<float, float> x, std::pair<float, float> y,
        std::pair<float, float> z);

    bool base_line_intersection(glm::vec3 point, glm::vec3 dir, float& dis);

    bool base_box_movement_intersection(TileBoundingBox box, glm::vec3 dir,
                                        float& dis);
};

typedef std::shared_ptr<ITerrain> pTerrain;

class BoxStackTerrain : public ITerrain {
   public:
    // data: xxxxxxxxx
    //       z
    //       z
    //       z
    BoxStackTerrain(int xSize, int zSize, const std::string& data);
    void get_bounding_boxes(glm::vec3 point, std::vector<TileBoundingBox>& res);
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