#pragma once

#include "pch.h"
#include "../renderer/Objects.h"

struct EntityInstruction {
    std::string id, type, state;
    glm::vec3 pos, speed;
    float facing, rotationSpeed;
};

struct SceneInstruction {
    TimeStamp creationTime;
    std::vector<EntityInstruction> entities;
};