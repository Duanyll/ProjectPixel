#pragma once

#include "pch.h"

class Camera {
   public:
    inline Camera() {}

    inline virtual void apply_to_window() {}

    glm::vec3 pos = glm::vec3(0.0, 1.5, 0.0);
    glm::vec3 front = glm::vec3(0.0, 0.0, 1.0);
    float fov = 45.0f;

    glm::mat4 get_view();
    glm::mat4 get_projection();

    void apply();

    glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
   protected:
};

class FreeCamera : public Camera {
   public:
    inline FreeCamera() {}

    void apply_to_window();

    float cursorSensitivity = 0.05f;
    float moveSpeed = 2.5f;

    void on_cursor_move(double xpos, double ypos);
    void on_cursor_enter(int isEnter);
    void on_scroll(double offset);

    enum class Direction { Front, Back, Left, Right, Up, Down };

    void move_pos(Direction direction);
    float yaw = 0, pitch = 0;
    void set_yaw_pitch(float yaw, float pitch);

   private:
    glm::vec3 movementFront = glm::vec3(0.0f, 0.0f, 1.0f);

    bool resetCursorFlag = true;
    float lastX = 0, lastY = 0;
};

class ThirdPersonCamera : public Camera {
   public:
    inline ThirdPersonCamera() {}

    void set_entity_position(glm::vec3 pos);
    void set_yaw(float yaw);
    glm::vec3 groundPos = {0, 0, 0};
    glm::vec3 groundFront = glm::normalize(glm::vec3(-1.0, 0.0, -1.0));
    float backwardDistance = 6.0;
    float liftHeight = 10.0;

    glm::vec3 resolve_cursor_dir();
};