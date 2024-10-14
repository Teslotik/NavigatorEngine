#ifndef ENTITY_H
#define ENTITY_H

#include <string>

namespace engine::entity {

/// @note also the resource
class Entity {
protected:
    std::string name;
    virtual ~Entity() = default;
    // virtual void enable() {}
    // virtual void disable() {}
public:
    virtual void onParent() {}
    virtual void onRemove() {}
};

} // namespace engine::entity

#endif