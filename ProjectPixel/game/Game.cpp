#include "pch.h"
#include "Game.h"

#include "../utils/Utils.h"
#include "../utils/Geomentry.h"
#include "../driver/UI.h"
#include "../driver/Flags.h"

Game::Game(LevelConfig& config)
    : config(config), terrainRenderer(config.get_terrain()), processor(config) {
    outlineBuffer = std::make_shared<OutlineFrameBufferTexture>(Window::width,
                                                                Window::height);
}

void Game::apply_to_window() { processor.input.active(); }

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
        Lights.spotLight.position = player->position + glm::vec3{0, 1.1, 0};
        Lights.spotLight.direction = angle_to_front(player->facing);

        Lights.pointLightCount = 1;
        Lights.pointLights[0].position =
            player->position + glm::vec3{0, 1.5, 0};
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

    outlineBuffer->clear_outline();
    outlineBuffer->draw_inside([&]() -> void {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    });
    for (auto& [id, e] : entities) {
        if (e->highlight) {
            outlineBuffer->draw_and_mark_outline(
                [&]() -> void { e->render(); });
        } else {
            outlineBuffer->draw_inside([&]() -> void { e->render(); });
        }
    }
    outlineBuffer->draw_and_mark_outline([&]() -> void {
        for (auto& i : entities) {
            i.second->render();
        }
    });

    outlineBuffer->draw_inside([&]() -> void {
        terrainRenderer.render();
        skybox.render();
        particles.render();
    });

    outlineBuffer->draw_outline({1, 0, 0});
    FullScreenQuad quad(outlineBuffer);
    quad.render();

    hud.render();
}

void Game::stop() { processor.stop(); }

void Game::update() {
    if (!processor.isRunning) return;
    processor.input.collect_state(camera);
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

        for (auto& i : ins->particles) {
            particles.add_cluster(i);
            // UI::log_info(std::format("Particle {} at ({}, {}, {})",
            // (int)i.type, i.pos.x, i.pos.y, i.pos.z));
        }
    }
    TimeStamp now = std::chrono::steady_clock::now();
    float delta = to_float_duration(now - updateTime);
    updateTime = now;
    for (auto& i : entities) {
        i.second->step(delta);
    }
    particles.step(delta);
}
