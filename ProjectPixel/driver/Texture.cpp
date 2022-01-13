#include "pch.h"
#include "Texture.h"

#include "AssetsHub.h"

#include "Flags.h"

Texture::Texture(const std::string& filePath, bool flipped) {
    stbi_set_flip_vertically_on_load(flipped);
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    unsigned char* data =
        stbi_load(filePath.c_str(), &width, &height, &channels, 0);
    if (data) {
        switch (channels) {
            case 1:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED,
                             GL_UNSIGNED_BYTE, data);
                break;
            case 3:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                             GL_UNSIGNED_BYTE, data);
                break;
            case 4:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                             GL_RGBA, GL_UNSIGNED_BYTE, data);
                break;
            default:
                std::clog << "Failed to load texture from " << filePath
                          << ", unknown channel" << channels << std::endl;
                break;
        }
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::clog << "Failed to load texture from " << filePath << std::endl;
    }
    stbi_image_free(data);
}

Texture::~Texture() { glDeleteTextures(1, &id); }

CubeTexture::CubeTexture(const std::vector<std::string>& facesPath) {
    stbi_set_flip_vertically_on_load(false);

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    for (unsigned int i = 0; i < facesPath.size(); i++) {
        unsigned char* data =
            stbi_load(facesPath[i].c_str(), &width, &height, &channels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width,
                         height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap texture failed to load at path: "
                      << facesPath[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

FrameBufferTexture::FrameBufferTexture(int width, int height,
                                       bool isPixelized) {
    this->width = width;
    this->height = height;
    this->channels = 4;
    // framebuffer configuration
    // -------------------------
    glGenFramebuffers(1, &fbo);
    FrameBuffer b(fbo);
    // create a color attachment texture
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    isPixelized ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    isPixelized ? GL_NEAREST : GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           id, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't
    // be sampling these)
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width,
                          height);  // use a single renderbuffer object for both
                                    // a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, rbo);  // now actually attach it
    // now that we actually created the framebuffer and added all attachments we
    // want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!"
                  << std::endl;
}

FrameBufferTexture::~FrameBufferTexture() {
    if (rbo != 0) glDeleteRenderbuffers(1, &rbo);
    if (fbo != 0) glDeleteFramebuffers(1, &fbo);
}

void FrameBufferTexture::draw_inside(std::function<void()> draw) {
    FrameBuffer b(fbo);
    Viewport v(width, height);
    draw();
}

TextureMatrix::TextureMatrix(int subWidth, int subHeight, int column, int row,
                             bool isPixelized)
    : subWidth(subWidth),
      subHeight(subHeight),
      column(column),
      row(row),
      FrameBufferTexture(subWidth * column, subHeight * row, isPixelized) {}

void TextureMatrix::load(const std::vector<pTexture>& subTextures) {
    if (subTextures.size() > row * column) {
        std::cout << "WARNING: Loading too many textures into a TextureMatrix!"
                  << std::endl;
    }

    auto shader = AssetsHub::get_shader<QuadShader>();
    auto vao = std::make_shared<VAO>();
    vao->load_interleave_vbo(nullptr, 24 * sizeof(float), {2, 2});
    draw_inside([&]() -> void {
        DepthTest _(false);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        for (int i = 0; i < subTextures.size(); i++) {
            auto p = query_position(i);
            for (auto& k : p) {
                k.x = k.x * 2 - 1;
                k.y = k.y * 2 - 1;
            }
            GLfloat vertices[] = {
                p[0].x, p[0].y, 0.0, 1.0, p[3].x, p[3].y, 0.0, 0.0,
                p[2].x, p[2].y, 1.0, 0.0, p[0].x, p[0].y, 0.0, 1.0,
                p[2].x, p[2].y, 1.0, 0.0, p[1].x, p[1].y, 1.0, 1.0};
            shader->configure(subTextures[i]);
            vao->update_vbo(reinterpret_cast<float*>(vertices), 0,
                            sizeof vertices);
            vao->draw();
        }
        // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    });
}

std::vector<glm::vec2> TextureMatrix::query_position(int subId) {
    int r = subId / column;
    int c = subId % column;
    return {
        {float(c) / column, float(r) / row},
        {float(c + 1) / column, float(r) / row},
        {float(c + 1) / column, float(r + 1) / row},
        {float(c) / column, float(r + 1) / row},
    };
}

DepthMap::DepthMap(int width, int height) {
    this->width = width;
    this->height = height;
    channels = 1;

    glGenFramebuffers(1, &fbo);
    FrameBuffer b(fbo);

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           id, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
}