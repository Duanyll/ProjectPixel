#include "pch.h"
#include "RenderMethods.h"

void RenderMethods::fullscreen_quad(pTexture texture) {
    glDisable(GL_DEPTH_TEST);
    auto shader = AssetsHub::get_shader<QuadShader>();
    auto vao = AssetsHub::get_vao("quad");
    shader->configure(texture);
    vao->draw();
    glEnable(GL_DEPTH_TEST);
}

void RenderMethods::skybox(pCubeTexture texture) {
    glDepthFunc(GL_LEQUAL);
    auto shader = AssetsHub::get_shader<SkyboxShader>();
    auto vao = AssetsHub::get_vao("skybox");
    shader->configure(texture);
    vao->draw();
    glDepthFunc(GL_LESS);
}

void RenderMethods::paperman_standing(glm::vec3 position, float facing,
                                      float head_yaw, float head_pitch,
                                      EntityMaterial material, bool is_slim) {
    auto headVAO = AssetsHub::get_vao("paperman-head");
    auto bodyVAO = AssetsHub::get_vao("paperman-body");
    auto larmVAO =
        AssetsHub::get_vao(is_slim ? "paperman-larm-slim" : "paperman-larm");
    auto rarmVAO =
        AssetsHub::get_vao(is_slim ? "paperman-rarm-slim" : "paperman-rarm");
    auto llegVAO = AssetsHub::get_vao("paperman-lleg");
    auto rlegVAO = AssetsHub::get_vao("paperman-rleg");

    auto baseModel =
        glm::scale(glm::rotate(glm::translate(glm::mat4(), position),
                               glm::radians(facing), {0.0, 1.0, 0.0}),
                   {0.05, 0.05, 0.05});
    auto shader = AssetsHub::get_shader<EntityShader>();
    if (!material.emission)
        material.emission = AssetsHub::get_texture_2d("no-emission");
    if (!material.specular)
        material.specular = AssetsHub::get_texture_2d("no-specular");

    head_yaw = glm::clamp(head_yaw, -90.0f, 90.0f);
    head_pitch = glm::clamp(head_pitch, -75.0f, 75.0f);
    shader->configure(
        material,
        baseModel * glm::rotate(glm::rotate(glm::mat4(), glm::radians(head_yaw),
                                            {0.0, 1.0, 0.0}),
                                glm::radians(head_pitch), {1.0, 0.0, 0.0}));
    headVAO->draw();

    shader->configure(material, baseModel);
    bodyVAO->draw();
    larmVAO->draw();
    rarmVAO->draw();
    llegVAO->draw();
    rlegVAO->draw();
}