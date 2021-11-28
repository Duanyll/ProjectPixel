#include "pch.h"
#include "VAO.h"

VAO::VAO() { glGenVertexArrays(1, &id); }

VAO::~VAO() {
    glDeleteVertexArrays(1, &id);
    if (vboSize != 0) glDeleteBuffers(1, &vbo);
    if (eboSize != 0) glDeleteBuffers(1, &ebo);
}

void VAO::load_interleave_vbo(const float* vertices, size_t size,
                          std::initializer_list<size_t> memberSize) {
    assert(vbo == 0);
    glBindVertexArray(id);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    vboSize = size;

    size_t structSize = 0;
    for (const auto& i : memberSize) structSize += i;
    size_t idx = 0;
    size_t offset = 0;
    for (const auto& i : memberSize) {
        glVertexAttribPointer(idx, i, GL_FLOAT, GL_FALSE,
                              structSize * sizeof(float),
                              (void*)(offset * sizeof(float)));
        glEnableVertexAttribArray(idx);
        idx++;
        offset += i;
    }
    triangleCount = vboSize / sizeof(float) / structSize;
}

void VAO::load_ebo(const int* indices, size_t size) {
    assert(ebo == 0);
    glBindVertexArray(id);
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
    eboSize = size;
    elementCount = size / sizeof(float) / 3;
}

void VAO::draw() {
    glBindVertexArray(id);
    if (eboSize) {
        glDrawElements(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, triangleCount);
    }
}

void VAO::update_vbo(const float* vertices, size_t offset, size_t size) {
    assert(vbo != 0);
    glBindVertexArray(id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, vertices);
}
