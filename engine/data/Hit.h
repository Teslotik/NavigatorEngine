#ifndef HIT_H
#define HIT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace engine::data {

struct Hit {
    glm::vec3 origin{0.0};
    glm::vec3 direction{0.0};
    float distance = 0;
    glm::vec3 position{0.0};

    bool operator<(const Hit &o) {
        return distance < o.distance;
    }

    bool operator>(const Hit &o) {
        return distance > o.distance;
    }
};

} // namespace engine::data

#endif