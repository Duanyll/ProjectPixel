#pragma once

#include "pch.h"
#include "../renderer/AssetsHub.h"

class TextPrinter {
   public:
    TextPrinter(const std::string& font_path, int screen_width,
                int screen_height);
    void on_screen_size_changed(int new_w, int new_h);
    void print(const std::string& text, GLfloat x, GLfloat y, GLfloat scale,
               glm::vec3 color);

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
    GLuint VAO, VBO;
};

typedef std::shared_ptr<TextPrinter> pTextPrinter;

namespace Logger {
void init(const std::string& font_path = "assets/CaskaydiaCoveNF.ttf",
          int screen_width = 1920, int screen_height = 1080);
void info(const std::string& str);
void error(const std::string& str);
void flush();
}  // namespace logger