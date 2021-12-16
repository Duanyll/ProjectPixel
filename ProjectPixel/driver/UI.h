#pragma once

#include "pch.h"
#include "AssetsHub.h"

class TextPrinter {
   public:
    TextPrinter(const std::string& font_path);
    void print(const std::string& text, GLfloat x, GLfloat y, GLfloat scale,
               glm::vec3 color);
    float get_text_width(const std::string& text, float scale = 1.0f);
   private:
    glm::mat4 projection;

    /// Holds all state information relevant to a character as loaded using
    /// FreeType
    struct Character {
        GLuint TextureID;    // ID handle of the glyph texture
        glm::ivec2 Size;     // Size of glyph
        glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
        GLuint Advance;      // Horizontal offset to advance to next glyph
    };

    std::unordered_map<GLchar, Character> Characters;
    pVAO vao;
};

typedef std::shared_ptr<TextPrinter> pTextPrinter;

namespace UI {
void init(const std::string& font_path = "assets/CaskaydiaCoveNF.ttf");
void log_info(const std::string& str);
void log_error(const std::string& str);
void print_text(const std::string& text, GLfloat x, GLfloat y, GLfloat scale = 1.0f,
                glm::vec3 color = glm::vec3(1.0, 1.0, 1.0));
float get_text_width(const std::string& text, float scale = 1.0f);
void print_image2d(pTexture texture, GLfloat x, GLfloat y, GLfloat w,
                   GLfloat h);
void print_logs();
}  // namespace logger