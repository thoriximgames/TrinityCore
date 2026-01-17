#pragma once
#include <cstdint>
#include "../Shared/Protocol/MMO_generated.h"

namespace MMO::Engine {

/**
 * @brief Base class for all world objects (Players, NPCs, etc.)
 */
class Entity {
public:
    Entity(uint64_t id, Protocol::EntityType type) 
        : _id(id), _type(type), _position{0.0f, 0.0f, 0.0f}, _rotation(0.0f), _isDirty(false) {}

    virtual ~Entity() = default;

    uint64_t GetId() const { return _id; }
    Protocol::EntityType GetType() const { return _type; }

    void SetPosition(float x, float y, float z) {
        _position.x = x;
        _position.y = y;
        _position.z = z;
        _isDirty = true;
    }

    const Protocol::Vec3& GetPosition() const { 
        return *reinterpret_cast<const Protocol::Vec3*>(&_position); 
    }

    float GetRotation() const { return _rotation; }
    void SetRotation(float r) { _rotation = r; _isDirty = true; }

    bool IsDirty() const { return _isDirty; }
    void ClearDirty() { _isDirty = false; }

protected:
    uint64_t _id;
    Protocol::EntityType _type;
    
    // Internal position storage compatible with Protocol::Vec3 layout
    struct { float x, y, z; } _position;
    float _rotation;
    bool _isDirty;
};

/**
 * @brief Represents a player-controlled entity.
 */
class Player : public Entity {
public:
    Player(uint64_t id, const std::string& name) 
        : Entity(id, Protocol::EntityType_Player), _name(name) {}

    const std::string& GetName() const { return _name; }

private:
    std::string _name;
};

} // namespace MMO::Engine
