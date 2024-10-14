#ifndef CAMERA_ACTOR
#define CAMERA_ACTOR

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_access.hpp>

#include "ActorEntity.h"

namespace engine::entity {

class CameraActor: public ActorEntity {
public:
    glm::mat4 view{1.0};
    glm::mat4 projection{1.0};

    CameraActor() {
        glm::vec3 pos{-3, 0, 0};
        glm::vec3 front{1, 0, 0};
        view = glm::lookAt(pos, pos + glm::normalize(front), glm::vec3(0, 1, 0));
    }

    void localTransform(float dx, float dy, float dz, float rx, float ry) {
        glm::vec4 pos = utils::getPosition(view);
        pos /= pos.w;

        // rotation
        glm::mat4 horizontal = glm::rotate(glm::mat4(glm::mat3(view)), glm::radians(rx), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 right = glm::normalize(glm::row(horizontal, 0));
        glm::mat4 vertical = glm::rotate(glm::mat4(glm::mat3(horizontal)), glm::radians(ry), glm::vec3(right));
        glm::vec3 look = glm::normalize(glm::row(vertical, 2));

        // position
        view = glm::translate(vertical, -glm::vec3(pos) + look * dx - right * dz);
        view = glm::translate(view, glm::vec3(0.0f, -dy, 0.0f));
    }
};

} // namespace engine::entity

#endif