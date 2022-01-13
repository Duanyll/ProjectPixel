#include "pch.h"
#include "Game.h"

#include "../utils/Utils.h"
#include "../utils/Geomentry.h"
#include "../driver/UI.h"
#include "../driver/Flags.h"

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
    Window::register_command("heal", [this](float isDown) {
        if (isDown != 0) {
            processor.input.set_flag("heal");
        } else {
            processor.input.clear_flag("heal");
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
                    "rotation",
                    horizonal_angle(angle_to_front(facing), control));
            }
        }
    });
    Window::register_command("mouse-button", [this](float _) {
        if (glfwGetMouseButton(Window::handle, GLFW_MOUSE_BUTTON_LEFT) ==
            GLFW_PRESS) {
            processor.input.set_flag("attack");
        } else {
            processor.input.clear_flag("attack");
        }
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
            UI::log_info(
                std::format("X{:.2f} Y{:.2f} Z{:.2f}", pos.x, pos.y, pos.z));
        } else {
            UI::log_error(std::format("NO PLAYER!"));
        }
        auto cursor = camera.resolve_cursor_pos();
        UI::log_info(std::format("Cursor X{:.2f} Z{:.2f}", cursor.x, cursor.z));
    });
    Window::register_command(
        "slot-1", [this](float _) { processor.input.add_event("slot-1"); });
    Window::register_command(
        "slot-2", [this](float _) { processor.input.add_event("slot-2"); });
    Window::register_command(
        "slot-3", [this](float _) { processor.input.add_event("slot-3"); });
    camera.apply_to_window();
}

void Game::start() {
    Lights.dirLight.isActive = true;
    Lights.dirLight.direction = {0.3f, -0.6f, 1.0f};
    Lights.dirLight.diffuse = {0.6, 0.6, 0.5};
    Lights.dirLight.init_depth_map();
    Lights.pointLights[0].diffuse = {0.3, 0.3, 0.3};
    Lights.pointLights[0].specular = {0.1, 0.1, 0.1};
    Lights.spotLight.cutOff = 45;
    Lights.spotLight.outerCutOff = 50;
    Lights.spotLight.init_depth_map();
    updateTime = std::chrono::steady_clock::now();
    processor.start();
}

void Game::render() {
    update();
    DepthTest d(true);
    auto it = entities.find("player1");
    if (it != entities.end()) {
        auto player = it->second;
        camera.set_entity_position(it->second->position);

        Lights.dirLight.focus = player->position;

        Lights.spotLight.isActive = true;
        Lights.spotLight.position = player->position + glm::vec3{0, 1, 0};
        Lights.spotLight.direction = angle_to_front(player->facing);

        Lights.pointLightCount = 1;
        Lights.pointLights[0].position = player->position + glm::vec3{0, 1.5, 0};
    } else {
        Lights.spotLight.isActive = false;
        Lights.pointLightCount = 0;
    }

    Lights.dirLight.render_depth([&]() -> void {
        glClear(GL_DEPTH_BUFFER_BIT);
        for (auto& i : entities) {
            i.second->render_depth();
        }

        terrainRenderer.render_depth();
    });

    Lights.spotLight.render_depth([&]() -> void {
        glClear(GL_DEPTH_BUFFER_BIT);
        for (auto& i : entities) {
            if (i.first == "player1") continue;
            i.second->render_depth();
        }

        terrainRenderer.render_depth();
    });

    camera.apply();
    Lights.apply();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto& i : entities) {
        i.second->render();
    }

    terrainRenderer.render();
    skybox.render();

    hud.render();
}

void Game::stop() { processor.stop(); }

void Game::update() {
    if (!processor.isRunning) return;
    auto ins = processor.output.try_get();
    if (ins) {
        updateTime = ins->creationTime;
        for (auto& i : ins->entities) {
            auto it = entities.find(i.id);
            if (it == entities.end()) {
                entities[i.id] = get_entity_renderer(i);
            } else {
                it->second->update(i);
            }
        }
        for (auto& i : ins->deletedEntities) {
            entities.erase(i);
        }
        hud.update(*ins);

        for (auto& i : ins->messages) {
            UI::log_info(i);
        }
    }
    TimeStamp now = std::chrono::steady_clock::now();
    float delta = to_float_duration(now - updateTime);
    updateTime = now;
    for (auto& i : entities) {
        i.second->step(delta);
    }
}
