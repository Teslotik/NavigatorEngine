#ifndef ACTOR_ENTITY_H
#define ACTOR_ENTITY_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Entity.h"
#include "Shader.h"
#include "interface/IGameRenderable.h"
#include "interface/IHierarchy.h"

namespace engine::entity {

class ActorEntity : public Entity, public IStrongHierarchy<ActorEntity> {
    glm::mat4 transform{1.0};
    glm::mat4 world{1.0};
    glm::mat4 local{1.0};
    glm::mat4 inverse{1.0};
public:
    ActorEntity *parent = nullptr;
    std::vector<ActorEntity*> children;
    
    Shader *shader = nullptr;

    const glm::mat4 &getMatrixTransform() {
        return transform;
    }

    const glm::mat4 &getMatrixWorld() {
        return world;
    }

    const glm::mat4 &getMatrixLocal() {
        return local;
    }

    const glm::mat4 &getMatrixInverse() {
        return inverse;
    }

    void setMatrixTransform(glm::mat4 m) {
        transform = m;
        buildTransform();
    }

    void setMatrixWorld(glm::mat4 m) {
        world = m;
        // buildTransform();
    }

    void setMatrixLocal(glm::mat4 m) {
        local = m;
        buildTransform();
    }

    void buildTransform() {
        if (parent) {
            world = parent->world * (parent->inverse * local) * transform;
            inverse = glm::inverse(local);
        } else {
            world = transform;
            // local = glm::mat4{1.0};
            inverse = glm::mat4{1.0};
        }
        for (auto child: children) {
            child->buildTransform();
        }
    }
};

} // namespace engine::entiry

#endif