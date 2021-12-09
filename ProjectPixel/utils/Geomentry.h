#pragma once

#include "pch.h"
#include "Utils.h"

// 射线与平面相交：l0 射线起点, l 射线方向向量, n 平面法向量, p0 平面上一点
// 若相交, 返回 true 和参数 d
inline bool test_line_plane_intersection(glm::vec3 l0, glm::vec3 l, glm::vec3 n,
                                         glm::vec3 p0, float& d) {
    float deno = glm::dot(l, n);
    if (dcmp(deno, 0.0f) == 0) {
        return false;
    } else {
        d = glm::dot(p0 - l0, n) / deno;
        return true;
    }
}

inline bool test_point_inside_triangle(glm::vec3 p, glm::vec3 a, glm::vec3 b,
                                       glm::vec3 c) {
    glm::vec3 v0 = c - a;
    glm::vec3 v1 = b - a;
    glm::vec3 v2 = p - a;

    float dot00 = glm::dot(v0, v0);
    float dot01 = glm::dot(v0, v1);
    float dot02 = glm::dot(v0, v2);
    float dot11 = glm::dot(v1, v1);
    float dot12 = glm::dot(v1, v2);

    // NOTE: 保证分母不为0
    float deno = (dot00 * dot11 - dot01 * dot01);

    float u = (dot11 * dot02 - dot01 * dot12) / deno;
    if (u < 0 || u > 1) {
        return false;
    }

    float v = (dot00 * dot12 - dot01 * dot02) / deno;
    if (v < 0 || v > 1) {
        return false;
    }

    return u + v <= 1;
}

inline bool test_point_inside_quad(glm::vec3 p, glm::vec3 a, glm::vec3 b,
                                   glm::vec3 c) {
    glm::vec3 v0 = c - a;
    glm::vec3 v1 = b - a;
    glm::vec3 v2 = p - a;

    float dot00 = glm::dot(v0, v0);
    float dot01 = glm::dot(v0, v1);
    float dot02 = glm::dot(v0, v2);
    float dot11 = glm::dot(v1, v1);
    float dot12 = glm::dot(v1, v2);

    // NOTE: 保证分母不为0
    float deno = (dot00 * dot11 - dot01 * dot01);

    float u = (dot11 * dot02 - dot01 * dot12) / deno;
    if (u <= 0 || u >= 1) {
        return false;
    }

    float v = (dot00 * dot12 - dot01 * dot02) / deno;
    if (v <= 0 || v >= 1) {
        return false;
    }

    return true;
}

enum class BoxClipping {
    None = 0,
    PosX = 1 << 0,
    PosY = 1 << 1,
    PosZ = 1 << 2,
    NegX = 1 << 3,
    NegY = 1 << 4,
    NegZ = 1 << 5,
    Full = 0x3f
};

inline const glm::vec3 boxClippingDelta[] = {
    {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {-1, 0, 0}, {0, -1, 0}, {0, 0, -1}};

struct TileBoundingBox {
    glm::vec3 a;
    glm::vec3 s;

    inline bool test_line_intersection(glm::vec3 l0, glm::vec3 l, float& d) {
        glm::vec3 A{a.x, a.y, a.z};
        glm::vec3 B{a.x + s.x, a.y, a.z};
        glm::vec3 C{a.x + s.z, a.y, a.z + s.z};
        glm::vec3 D{a.x, a.y, a.z + s.z};
        glm::vec3 E{a.x, a.y + s.y, a.z};
        glm::vec3 F{a.x + s.x, a.y + s.y, a.z};
        glm::vec3 G{a.x + s.z, a.y + s.y, a.z + s.z};
        glm::vec3 H{a.x, a.y + s.y, a.z + s.z};

        auto face = [&](glm::vec3 A, glm::vec3 B, glm::vec3 C,
                        glm::vec3 norm) -> auto {
            float d = -1;
            if (test_line_plane_intersection(l0, l, norm, A, d)) {
                if (dcmp(d, 0.0f) > 0 && test_point_inside_quad(d * l + l0, A, B, C)) {
                    return d;
                }
            }
            return -1.0f;
        };

        float dis[6] = {face(C, G, B, {1, 0, 0}),  face(A, D, E, {-1, 0, 0}),
                        face(B, C, A, {0, -1, 0}), face(H, E, G, {0, 1, 0}),
                        face(D, H, C, {0, 0, 1}),  face(B, F, A, {0, 0, -1})};
        float result = 1000;
        for (int i = 0; i < 6; i++) {
            if (dis[i] < 0) continue;
            result = std::min(result, dis[i]);
        }
        if (result < 1000) {
            d = result;
            return true;
        } else {
            return false;
        }
    }

    inline bool test_point_inside(glm::vec3 p) {
        return p.x > a.x && p.x < a.x + s.x && p.y > a.y && p.y < a.y + s.y &&
               p.z > a.z && p.z < a.z + s.z;
    }

    inline glm::vec3 clip_point(glm::vec3 p, BoxClipping& clipping) {
        if (!test_point_inside(p)) return p;
        struct ClipMethod {
            BoxClipping dir;
            glm::vec3 point;
            float dis;
        };
        ClipMethod d[] = {
            {BoxClipping::PosX, {a.x, p.y, p.z}, p.x - a.x},
            {BoxClipping::PosY, {p.x, a.y, p.z}, p.y - a.y},
            {BoxClipping::PosZ, {p.x, p.y, a.z}, p.z - a.z},
            {BoxClipping::NegX, {a.x + s.x, p.y, p.z}, a.x + s.x - p.x},
            {BoxClipping::NegY, {p.x, a.y + s.y, p.z}, a.y + s.y - p.y},
            {BoxClipping::NegZ, {p.x, p.y, a.z + s.z}, a.z + s.z - p.z}};
        auto m = std::min_element(
            d, d + 6, [](auto a, auto b) -> auto { return a.dis < b.dis; });
        clipping |= m->dir;
        return m->point;
    }
};

inline float horizonal_angle(glm::vec3 base, glm::vec3 dir) {
    base = glm::normalize(glm::vec3(base.x, 0, base.z));
    dir = glm::normalize(glm::vec3(dir.x, 0, dir.z));
    auto dot = glm::dot(base, dir);
    auto cross = glm::cross(base, dir);
    if (cross.y > 0) {
        return -glm::degrees(std::acosf(dot));
    } else {
        return glm::degrees(std::acosf(dot));
    }
}

inline glm::vec3 angle_to_front(float facing) {
    return {sin(-glm::radians(facing)), 0, cos(glm::radians(facing))};
}