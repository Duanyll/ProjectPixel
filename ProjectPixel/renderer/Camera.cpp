#include "pch.h"
#include "Camera.h"

#include "../utils/FrameTimer.h"
#include "Uniform.h"

glm::mat4 Camera::get_view() { return glm::lookAt(pos, pos + front, up); }

glm::mat4 Camera::get_projection() {
    return glm::perspective(glm::radians(fov),
                            (float)screenWidth / screenHeight, 0.1f, 100.0f);
}

void Camera::apply_uniform() {
    Uniform::set_data("Camera", "view", get_view());
    Uniform::set_data("Camera", "projection", get_projection());
    Uniform::set_data("Camera", "viewPos", pos);
}

void FreeCamera::change_screen_size(int new_w, int new_h) {
    screenWidth = new_w;
    screenHeight = new_h;
    resetCursorFlag = true;
}

void FreeCamera::on_cursor_move(double xpos, double ypos) {
    if (resetCursorFlag) {
        resetCursorFlag = false;
        lastX = xpos;
        lastY = ypos;
    }
    float xoffset = xpos - lastX;
    float yoffset =
        lastY - ypos;  // 注意这里是相反的，因为y坐标是从底部往顶部依次增大的
    lastX = xpos;
    lastY = ypos;

    xoffset *= cursorSensitivity;
    yoffset *= cursorSensitivity;

    set_yaw_pitch(yaw + xoffset, pitch + yoffset);
}

void FreeCamera::on_cursor_enter(int isEnter) {
    if (!isEnter) resetCursorFlag = true;
}

void FreeCamera::on_scroll(double offset) {
    fov += offset;
    fov = glm::clamp(fov, 1.0f, 45.0f);
}

void FreeCamera::move_pos(Direction direction) {
    float dis = moveSpeed * FrameTimer::get_last_frame_time();
    switch (direction) {
        case Direction::Front:
            pos += dis * movementFront;
            break;
        case Direction::Back:
            pos -= dis * movementFront;
            break;
        case Direction::Left:
            pos -= glm::normalize(glm::cross(front, up)) * dis;
            break;
        case Direction::Right:
            pos += glm::normalize(glm::cross(front, up)) * dis;
            break;
        case Direction::Up:
            pos += up * dis;
            break;
        case Direction::Down:
            pos -= up * dis;
            break;
    }
}

void FreeCamera::set_yaw_pitch(float yaw, float pitch) {
    if (yaw < -180) yaw += 360;
    if (yaw >= 180) yaw -= 360;
    pitch = glm::clamp(pitch, -89.0f, 89.0f);
    this->yaw = yaw;
    this->pitch = pitch;

    front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    front.y = sin(glm::radians(pitch));
    front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    front = glm::normalize(front);

    movementFront.x = cos(glm::radians(yaw));
    movementFront.y = 0;
    movementFront.z = sin(glm::radians(yaw));
    movementFront = glm::normalize(movementFront);
}
