#pragma once

#include "pch.h"
#include "../utils/Utils.h"

class VAO {
   public:
    NO_COPY_CLASS(VAO)
    VAO();
    ~VAO();
    GLuint id = 0, vbo = 0, ebo = 0;

    size_t vboSize = 0;
    size_t triangleCount = 0;
    size_t eboSize = 0;
    size_t elementCount = 0;

    void load_interleave_vbo(const float* vertices, size_t size, std::initializer_list<size_t> memberSize);
    void load_ebo(const int* indices, size_t size);
    void draw();

    void update_vbo(const float* vertices, size_t offset, size_t size);
};

typedef std::shared_ptr<VAO> pVAO;