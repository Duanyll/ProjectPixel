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
