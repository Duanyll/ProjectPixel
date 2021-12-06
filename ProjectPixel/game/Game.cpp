#include "pch.h"
#include "Game.h"

#include "../utils/Utils.h"
#include "../utils/Text.h"
#include "../utils/Geomentry.h"

Game::Game(LevelConfig& config)
    : config(config),
      terrainRenderer(config.get_terrain()),
      processor(config) {}

void Game::apply_to_window() {
    Window::register_command("move-front", [this](float _) {
        auto vecdir = glm::normalize(camera.groundFront) *
                      FrameTimer::get_last_frame_time();
        processor.input.add("speed-x", vecdir.x);
        processor.input.add("speed-z", vecdir.z);
    });
    Window::register_command("move-back", [this](float _) {
        auto vecdir = glm::normalize(-camera.groundFront) *
                      FrameTimer::get_last_frame_time();
        processor.input.add("speed-x", vecdir.x);
        processor.input.add("speed-z", vecdir.z);
    });
    Window::register_command("move-left", [this](float _) {
        auto vecdir =
            glm::normalize(-glm::cross(camera.groundFront, camera.up)) *
            FrameTimer::get_last_frame_time();
        processor.input.add("speed-x", vecdir.x);
        processor.input.add("speed-z", vecdir.z);
    });
    Window::register_command("move-right", [this](float _) {
        auto vecdir =
            glm::normalize(glm::cross(camera.groundFront, camera.up)) *
            FrameTimer::get_last_frame_time();
        processor.input.add("speed-x", vecdir.x);
        processor.input.add("speed-z", vecdir.z);
    });
    Window::register_command(
        "move-up", [this](float _) { processor.input.add_event("jump"); });
    Window::register_command("run", [this](float isDown) {
        if (isDown != 0) {
            processor.input.set_flag("run");
        } else {
            processor.input.clear_flag("run");
        }
    });
    Window::register_command("mouse", [this](float _) {
        auto it = entities.find("player1");
        if (it != entities.end()) {
            auto pos = it->second->position;
            auto facing = it->second->facing;
            auto control =
                camera.resolve_cursor_pos() - glm::vec3(pos.x, 0, pos.z);
            if (glm::length(control) >= 1) {
                processor.input.set(
                    "rotation", horizonal_angle({sin(-glm::radians(facing)), 0,
                                                 cos(glm::radians(facing))},
                                                control));
            }
        }
    });
    Window::register_command("mouse-button", [this](float _) {
        if (glfwGetMouseButton(Window::handle, GLFW_MOUSE_BUTTON_RIGHT) ==
            GLFW_PRESS) {
            processor.input.set_flag("aim");
        } else {
            processor.input.clear_flag("aim");
        }
    });
    Window::register_command("diagnostics", [this](float _) {
        auto it = entities.find("player1");
        if (it != entities.end()) {
            auto pos = it->second->position;
            Logger::info(
                std::format("X{:.2f} Y{:.2f} Z{:.2f}", pos.x, pos.y, pos.z));
        } else {
            Logger::error(std::format("NO PLAYER!"));
        }
        auto cursor = camera.resolve_cursor_pos();
        Logger::info(std::format("Cursor X{:.2f} Z{:.2f}", cursor.x, cursor.z));
    });
    camera.apply_to_window();
}

void Game::start() {
    dirLight.apply();
    DirLight::set_ambient({0.3, 0.3, 0.3});
    PointLight::set_active_count(0);
    updateTime = std::chrono::steady_clock::now();
    processor.start();
}

void Game::render() {
    update();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    auto it = entities.find("player1");
    if (it != entities.end()) {
        camera.set_entity_position(it->second->position);
    }
    camera.apply_uniform();
    spotLight.position = camera.pos;
    spotLight.direction = camera.front;
    spotLight.apply();

    for (auto& i : entities) {
        i.second->render();
    }

    terrainRenderer.render();
    skybox.render();
}

void Game::stop() { processor.stop(); }

void Game::update() {
    auto instructions = processor.output.try_get();
    if (instructions) {
        updateTime = instructions->creationTime;
        for (auto& i : instructions->entities) {
            auto it = entities.find(i.id);
            if (it == entities.end()) {
                entities[i.id] = get_entity_renderer(i);
            } else {
                it->second->update(i);
            }
        }
        for (auto& i : instructions->deletedEntities) {
            entities.erase(i);
        }
    }
    TimeStamp now = std::chrono::steady_clock::now();
    float delta = to_float_duration(now - updateTime);
    updateTime = now;
    for (auto& i : entities) {
        i.second->step(delta);
    }
}
