#include "pch.h"
#include "UI.h"
#include "../utils/Window.h"
#include "Uniform.h"
#include "Flags.h"

TextPrinter::TextPrinter(const std::string& font_path) {
    // FreeType
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library"
                  << std::endl;

    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, font_path.c_str(), 0, &face))
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load first 128 characters of ASCII set
    for (GLubyte c = 0; c < 128; c++) {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width,
                     face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
                     face->glyph->bitmap.buffer);
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x};
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    vao = std::make_shared<VAO>();
    vao->load_interleave_vbo(nullptr, sizeof(float) * 6 * 4, {4});
}

void TextPrinter::print(const std::string& text, GLfloat x, GLfloat y,
                        GLfloat scale, glm::vec3 color) {
    DepthTest d(false);
    Blend b(true);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto shader = AssetsHub::get_shader<TextShader>();
    shader->configure(color);
    glActiveTexture(GL_TEXTURE0);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;
        // Update VBO for each character
        // clang-format off
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }
        };
        // clang-format on
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // Render glyph texture over quad
        vao->update_vbo(reinterpret_cast<float*>(vertices), 0,
                        sizeof(vertices));
        // Render quad
        vao->draw();
        // Now advance cursors for next glyph (note that advance is number of
        // 1/64 pixels)
        x += (ch.Advance >> 6) *
             scale;  // Bitshift by 6 to get value in pixels (2^6 = 64 (divide
                     // amount of 1/64th pixels by 64 to get amount of pixels))
    }
}

float TextPrinter::get_text_width(const std::string& text, float scale) {
    float res = 0;
    for (const auto& i : text) {
        Character ch = Characters[i];
        res += (ch.Advance >> 6) * scale;
    }
    return res;
}

pTextPrinter printer;
struct TextLog {
    glm::vec3 color;
    std::string str;
    double timeToDecay;
};
std::list<TextLog> logs;
const int MAX_LOGS = 10;
const int LOG_DECAY = 5;

void reset_projection2d() {
    auto projection = glm::ortho(0.0f, static_cast<GLfloat>(Window::width),
                                 0.0f, static_cast<GLfloat>(Window::height));
    Uniform::set_data("Screen", "projection2d", projection);
}

pVAO imageVAO;
void UI::init(const std::string& font_path) {
    printer = std::make_shared<TextPrinter>(font_path);
    reset_projection2d();
    class UIResizeWatcher : public Window::ResizeWatcher {
       public:
        void on_size_changed(int width, int height) { reset_projection2d(); }
    };
    Window::add_watcher(std::make_shared<UIResizeWatcher>());
    imageVAO = std::make_shared<VAO>();
    imageVAO->load_interleave_vbo(nullptr, sizeof(float) * 6 * 4, {4});
}

void base_log(const std::string& str, const std::string& level,
              glm::vec3 color) {
    std::cout << "[" << level << "]: " << str << std::endl;
    TextLog log;
    log.str = str;
    log.color = color;
    log.timeToDecay = glfwGetTime() + LOG_DECAY;
    logs.push_front(log);
    if (logs.size() > MAX_LOGS) {
        logs.pop_back();
    }
}

void UI::log_info(const std::string& str) {
    base_log(str, "INFO", glm::vec3(0.8, 0.8, 0.8));
}

void UI::log_error(const std::string& str) {
    base_log(str, "ERROR", glm::vec3(1.0, 0.0, 0.0));
}

void UI::print_text(const std::string& text, GLfloat x, GLfloat y,
                    GLfloat scale, glm::vec3 color) {
    printer->print(text, x, y, scale, color);
}

float UI::get_text_width(const std::string& text, float scale) {
    return printer->get_text_width(text, scale);
}

void UI::print_image2d(pTexture texture, GLfloat xpos, GLfloat ypos, GLfloat w,
                       GLfloat h) {
    DepthTest d(false);
    Blend b(true);
    auto shader = AssetsHub::get_shader<HUDShader>();
    shader->configure(texture);
    // clang-format off
    GLfloat vertices[6][4] = {
        { xpos,     ypos + h,   0.0, 0.0 },
        { xpos,     ypos,       0.0, 1.0 },
        { xpos + w, ypos,       1.0, 1.0 },

        { xpos,     ypos + h,   0.0, 0.0 },
        { xpos + w, ypos,       1.0, 1.0 },
        { xpos + w, ypos + h,   1.0, 0.0 }
    };
    // clang-format on
    imageVAO->update_vbo(reinterpret_cast<float*>(vertices), 0,
                         sizeof(vertices));
    // Render quad
    imageVAO->draw();
}

void UI::print_logs() {
    int ypos = 100;
    for (auto i : logs) {
        printer->print(i.str, 25, ypos, 0.6f, i.color);
        ypos += 30;
    }
    if (!logs.empty() && logs.back().timeToDecay < glfwGetTime()) {
        logs.pop_back();
    }
}
