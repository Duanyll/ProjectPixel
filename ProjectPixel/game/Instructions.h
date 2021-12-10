#pragma once

#include "pch.h"
#include "../utils/Utils.h"

enum class LegAction : char {
    Standing,
    Walking,
    Running,
    Lying
};
enum class HandAction : char {
    None,
    ZombieHanging,
    Holding,
    Attacking,
    ZombieAttacking,
    Sweeping
};
enum class Item : char { None, DiamondSword, DiamondAxe, Bow };

struct EntityInstruction {
    std::string id, type;
    // ��ʵ���Զ��� state ��Ϣ����;
    char state[8] = {0};
    glm::vec3 pos, speed;
    float facing, rotationSpeed;
};

struct SceneInstruction {
    TimeStamp creationTime;
    std::vector<EntityInstruction> entities;
    std::vector<std::string> deletedEntities;
};