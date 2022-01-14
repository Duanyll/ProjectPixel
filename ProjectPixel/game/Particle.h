#pragma once

#include "pch.h"
#include "Objects.h"

class Particle : public RenderObject {
   public:
    int id;
    glm::vec3 pos, speed;
    glm::vec3 rotationAxis{0, -1, 0};
    float rotationSpeed = 0, rotation = 0;
    glm::vec3 basePos, baseDir;
    glm::vec4 color{1.0, 1.0, 1.0, 1.0};
    float scale = 1.0f;
    float age = 0;

    virtual bool step(float time) = 0;
    virtual void render() = 0;

   protected:
    void step_motion(float time);
    void render(pVAO vao, pTexture diffuse, glm::mat4 baseModel = glm::mat4(),
                pTexture emission = AssetsHub::get_texture_2d("no-emission"));
};
typedef std::shared_ptr<Particle> pParticle;

class CritParticle : public Particle {
    int variant;
   public:
    CritParticle(glm::vec3 basePos, glm::vec3 baseDir, std::mt19937& random);
    bool step(float time);
    void render();

    const static int minCluster = 2;
    const static int maxCluster = 4;
};

class FallParticle : public Particle {
    int variant;
   public:
    FallParticle(glm::vec3 basePos, glm::vec3 baseDir, std::mt19937& random);
    bool step(float time);
    void render();

    const static int minCluster = 6;
    const static int maxCluster = 8;
};

class HealParticle : public Particle {
   public:
    HealParticle(glm::vec3 basePos, glm::vec3 baseDir, std::mt19937& random);
    bool step(float time);
    void render();

    const static int minCluster = 3;
    const static int maxCluster = 5;
};

class DeathParticle : public Particle {
    int variant;
   public:
    DeathParticle(glm::vec3 basePos, glm::vec3 baseDir, std::mt19937& random);
    bool step(float time);
    void render();

    const static int minCluster = 4;
    const static int maxCluster = 6;
};

class ParticleManager {
   public:
    int particleLimit = 32;
    void add_cluster(ParticleInstruction i);
    void add_one(pParticle p);
    void step(float time);
    void render();

   protected:
    int newParticleId = 0;
    std::map<int, pParticle> particles;

    std::mt19937 random{
        std::chrono::steady_clock::now().time_since_epoch().count()};

    template<typename T>
    void base_add_cluster(glm::vec3 pos, glm::vec3 dir) {
        std::uniform_int_distribution<int> countDis(T::minCluster, T::maxCluster);
        int count = countDis(random);
        for (int i = 0; i < count; i++) {
            add_one(std::make_shared<T>(pos, dir, random));
        }
    }
};