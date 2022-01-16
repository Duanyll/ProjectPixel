#include "pch.h"
#include "Particle.h"
#include "../driver/Flags.h"
#include "../utils/Geomentry.h"

void Particle::step_motion(float time) {
    age += time;
    pos += speed * time;
    rotation += rotationSpeed * time;
}

void Particle::render(pVAO vao, pTexture diffuse, glm::mat4 baseModel,
                      pTexture emission) {
    Blend b(true);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    auto shader = AssetsHub::get_shader<ParticleShader>();
    glm::mat4 model;
    model = glm::translate(model, pos);
    model = glm::rotate(model, glm::radians(rotation), rotationAxis);
    model = glm::scale(model, {scale, scale, scale});
    shader->configure(diffuse, emission, model * baseModel, color);
    vao->draw();
}

void ParticleManager::add_cluster(ParticleInstruction i) {
    switch (i.type) {
        case ParticleType::Crit:
            base_add_cluster<CritParticle>(i.pos, i.dir);
            break;
        case ParticleType::Fall:
            base_add_cluster<FallParticle>(i.pos, i.dir);
            break;
        case ParticleType::Heal:
            base_add_cluster<HealParticle>(i.pos, i.dir);
            break;
        case ParticleType::Death:
            base_add_cluster<DeathParticle>(i.pos, i.dir);
            break;
        default:
            break;
    }
}

void ParticleManager::add_one(pParticle p) {
    p->id = newParticleId++;
    if (particles.size() >= particleLimit) {
        particles.erase(particles.begin());
    }
    particles.insert({p->id, p});
}

void ParticleManager::step(float time) {
    std::vector<int> eraseList;
    for (auto &[id, p] : particles) {
        if (!p->step(time)) eraseList.push_back(id);
    }
    for (auto &id : eraseList) {
        particles.erase(id);
    }
}

void ParticleManager::render() {
    for (auto &[_, p] : particles) {
        p->render();
    }
}

CritParticle::CritParticle(glm::vec3 basePos, glm::vec3 baseDir,
                           std::mt19937 &random) {
    this->basePos = basePos;
    this->baseDir = baseDir;

    this->pos = basePos;
    this->pos.y += 1;
    float baseAngle = horizonal_angle({0, 0, 1}, baseDir);
    std::uniform_real_distribution<float> dir(baseAngle - 20, baseAngle + 20);
    this->speed = angle_to_front(dir(random)) * 10.0f;

    std::uniform_real_distribution<float> rotation(-360, 360);
    this->rotationSpeed = rotation(random);
    this->rotation = rotation(random);

    std::uniform_int_distribution<int> var(0, 4);
    this->variant = var(random);

    std::uniform_real_distribution<float> col(0.8, 1);
    this->color = {1.5 - col(random), 1.5 - col(random), col(random), 1};

    this->scale = 0.4;
}

bool CritParticle::step(float time) {
    step_motion(time);
    if (age >= 0.5) return false;
    speed -= glm::normalize(speed) * 20.0f * time;
    color.a = 1 - age * 2;
    return true;
}

void CritParticle::render() {
    auto asset = "crit" + std::to_string(variant);
    Particle::render(AssetsHub::get_vao(asset),
                     AssetsHub::get_texture_2d(asset));
}

FallParticle::FallParticle(glm::vec3 basePos, glm::vec3 baseDir,
                           std::mt19937 &random) {
    this->basePos = basePos;
    this->baseDir = baseDir;

    std::uniform_real_distribution<float> rotation(-360, 360);
    this->speed = angle_to_front(rotation(random)) * 5.0f;
    this->pos = basePos + this->speed * 0.1f;
    this->pos.y += 0.1;
    this->rotation = rotation(random);

    std::uniform_int_distribution<int> var(0, 1);
    this->variant = var(random);
    this->scale = 0.8;
}

bool FallParticle::step(float time) {
    step_motion(time);
    if (age >= 0.5) return false;
    speed -= glm::normalize(speed) * 10.0f * time;
    color.a = 1 - age * 2;
    return true;
}

void FallParticle::render() {
    auto asset = "fall" + std::to_string(variant);
    Particle::render(AssetsHub::get_vao(asset),
                     AssetsHub::get_texture_2d(asset));
}

HealParticle::HealParticle(glm::vec3 basePos, glm::vec3 baseDir,
                           std::mt19937 &random) {
    this->basePos = basePos;
    this->baseDir = baseDir;

    std::uniform_real_distribution<float> rotation(-360, 360);
    this->speed = angle_to_front(rotation(random)) * 0.5f;
    this->pos = basePos + this->speed;
    this->pos.y += 1;
    this->rotation = rotation(random);

    this->scale = 0.5;
}

bool HealParticle::step(float time) {
    step_motion(time);
    if (age >= 1) return false;
    speed -= glm::normalize(glm::vec3(speed.x, 0, speed.z)) * 0.5f * time;
    speed.y += 3.0 * time;
    color.a = 1 - age * age;
    return true;
}

void HealParticle::render() {
    Particle::render(
        AssetsHub::get_vao("heal"), AssetsHub::get_texture_2d("heal"),
        glm::rotate(glm::mat4(), glm::radians(90.0f), {1.0f, 0.0f, 0.0f}));
}

DeathParticle::DeathParticle(glm::vec3 basePos, glm::vec3 baseDir,
                             std::mt19937 &random) {
    this->basePos = basePos;
    this->baseDir = baseDir;

    std::uniform_real_distribution<float> rotation(-360, 360);
    this->speed = angle_to_front(rotation(random)) * 0.5f;
    this->pos = basePos;
    this->pos.y += 0.2;
    this->rotation = rotation(random);

    std::uniform_int_distribution<int> var(0, 3);
    this->variant = var(random);
}

bool DeathParticle::step(float time) {
    step_motion(time);
    if (age >= 1) return false;
    speed -= glm::normalize(glm::vec3(speed.x, 0, speed.z)) * 0.5f * time;
    speed.y += 3.0 * time;
    color.a = 1 - age;
    return true;
}

void DeathParticle::render() {
    auto asset = "death" + std::to_string(variant);
    Particle::render(
        AssetsHub::get_vao(asset), AssetsHub::get_texture_2d(asset),
        glm::rotate(glm::mat4(), glm::radians(90.0f), {1.0f, 0.0f, 0.0f}));
}
