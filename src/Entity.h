#pragma once

class Entity {
public:
    Entity();
    virtual ~Entity() = default;
    Entity(const Entity& rhs) = delete;
    Entity(Entity&& rhs) noexcept = default;
    Entity& operator=(const Entity& rhs) = delete;
    Entity& operator=(Entity&& rhs) noexcept = default;


};
