#pragma once

#include "pch.h"

class Camera {
   public:
    inline Camera(int width, int height)
        : screenWidth(width), screenHeight(height) {}

    glm::vec3 pos = glm::vec3(0.0, 0.0, 0.0);
    glm::vec3 front = glm::vec3(1.0, 0.0, 0.0);
    float fov = 45.0f;

    glm::mat4 get_view();
    glm::mat4 get_projection();

    void apply_uniform();

   protected:
    glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
    int screenWidth, screenHeight;
};

class FreeCamera : public Camera {
   public:
    inline FreeCamera(int width, int height) : Camera(width, height) {}

    float cursorSensitivity = 0.05f;
    float moveSpeed = 2.5f;

    void change_screen_size(int new_w, int new_h);
    void on_cursor_move(double xpos, double ypos);
    void on_cursor_enter(int isEnter);
    void on_scroll(double offset);

    enum class Direction { Front, Back, Left, Right, Up, Down };

    void move_pos(Direction direction);
    void set_yaw_pitch(float yaw, float pitch);

   private:
    glm::vec3 movementFront = glm::vec3(1.0f, 0.0f, 0.0f);

    bool resetCursorFlag = true;
    float lastX = 0, lastY = 0;
    float yaw = 0, pitch = 0;
};