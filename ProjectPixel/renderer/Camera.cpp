#include "pch.h"
#include "Camera.h"

#include "../utils/FrameTimer.h"
#include "../utils/Window.h"
#include "../utils/Geomentry.h"
#include "Uniform.h"

glm::mat4 Camera::get_view() { return glm::lookAt(pos, pos + front, up); }

glm::mat4 Camera::get_projection() {
    return glm::perspective(
        glm::radians(fov), (float)Window::width / Window::height, 0.1f, 100.0f);
}

void Camera::apply_uniform() {
    Uniform::set_data("Camera", "view", get_view());
    Uniform::set_data("Camera", "projection", get_projection());
    Uniform::set_data("Camera", "viewPos", pos);
}

void FreeCamera::apply_to_window() {
    glfwSetInputMode(Window::handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    Window::register_command("move-front",
                             [this](float _) { move_pos(Direction::Front); });
    Window::register_command("move-back",
                             [this](float _) { move_pos(Direction::Back); });
    Window::register_command("move-left",
                             [this](float _) { move_pos(Direction::Left); });
    Window::register_command("move-right",
                             [this](float _) { move_pos(Direction::Right); });
    Window::register_command("move-up",
                             [this](float _) { move_pos(Direction::Up); });
    Window::register_command("move-down",
                             [this](float _) { move_pos(Direction::Down); });
    Window::register_command("mouse-enter",
                             [this](float p) { on_cursor_enter(p > 0); });
    Window::register_command("mouse-x",
                             [this](float t) { on_cursor_move(t, lastY); });
    Window::register_command("mouse-y",
                             [this](float t) { on_cursor_move(lastX, t); });
    Window::register_command(
        "scroll-x", [this](float t) { set_yaw_pitch(yaw + t * 10, pitch); });
    Window::register_command("scroll-y", [this](float t) { on_scroll(t); });
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

    front.x = cos(glm::radians(pitch)) * sin(-glm::radians(yaw));
    front.y = sin(glm::radians(pitch));
    front.z = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    front = glm::normalize(front);

    movementFront.x = sin(-glm::radians(yaw));
    movementFront.y = 0;
    movementFront.z = cos(glm::radians(yaw));
    movementFront = glm::normalize(movementFront);
}

void ThirdPersonCamera::set_entity_position(glm::vec3 epos) {
    groundPos = epos;
    pos = epos - groundFront * backwardDistance + glm::vec3(0, liftHeight, 0);
    front = epos - pos;
}

void ThirdPersonCamera::set_yaw(float yaw) {
    groundFront = {sin(-glm::radians(yaw)), 0, cos(glm::radians(yaw))};
}

glm::vec3 ThirdPersonCamera::resolve_cursor_pos() {
    double cursorScreenX = 0, cursorScreenY = 0;
    glfwGetCursorPos(Window::handle, &cursorScreenX, &cursorScreenY);
    auto cursorClipX = cursorScreenX / Window::width * 2 - 1;
    auto cursorClipY =
        (Window::height - cursorScreenY) / Window::height * 2 - 1;
    glm::vec3 clipPos{cursorClipX, cursorClipY, 0.0};
    glm::vec4 cursorPosWorld4 = glm::inverse(get_view()) *
                                glm::inverse(get_projection()) *
                                glm::vec4(clipPos, 1.0);
    glm::vec3 cursorPosWorld{cursorPosWorld4.x / cursorPosWorld4.w,
                             cursorPosWorld4.y / cursorPosWorld4.w,
                             cursorPosWorld4.z / cursorPosWorld4.w};
    glm::vec3 cursorDirWorld = glm::normalize(cursorPosWorld - pos);
    float d = -1;
    if (test_line_plane_intersection(pos, cursorDirWorld, {0, 1, 0}, {0, 0, 0},
                                     d)) {
        auto res = pos + d * cursorDirWorld;
        return {res.x, 0, res.z};
    } else {
        return {0, 0, 0};
    }
}