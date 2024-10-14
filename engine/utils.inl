/// @todo rename file to utils.h, because it is no longer inline
/// @note Not sure about what methods must be presented here and how
/// they named, so I will make tests a little bit later /// @todo

#ifndef UTILS_H
#define UTILS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace engine::utils {

#define STRINGIFY(x) #x

// https://cp-algorithms.com/string/string-hashing.html
/// @todo replace with std functiion
constexpr int hash(const char *string) {
    constexpr int p = 31;
    constexpr int m = 1000000009;
    int hash = 0;
    int pow = 1;
    for (int i = 0; string[i] != '\0'; ++i) {
        hash = (hash + (string[i]) * pow) % m;
        pow = (pow * p) % m;
    }
    return hash;
}

/// @brief Perfect hash function
static int makeHash(long x, long max) {
    constexpr long k = 1234;
    constexpr long primal = 541439;
    return (k * x % primal) % max;
}

template<typename T>
T lerp(T t, T x0, T y0, T x1, T y1) {
    return y0 + (t - x0) * (y1 - y0) / (x1 - x0);
}

template<typename T>
T mix(T t, T x, T y) {
    return x + t * (y - x);
}

float area(float x1, float y1, float x2, float y2, float x3, float y3);
bool isInside(float x1, float y1, float x2, float y2, float x3, float y3, float x, float y);

// Möller–Trumbore intersection algorithm
// https://ru.wikipedia.org/wiki/%D0%90%D0%BB%D0%B3%D0%BE%D1%80%D0%B8%D1%82%D0%BC_%D0%9C%D0%BE%D0%BB%D0%BB%D0%B5%D1%80%D0%B0_%E2%80%94_%D0%A2%D1%80%D1%83%D0%BC%D0%B1%D0%BE%D1%80%D0%B0
float rayTriangleIntersection(const glm::vec3& orig, const glm::vec3& dir, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2);

float rayShphereIntersection(glm::vec3 orig, glm::vec3 dir, glm::vec3 center, float r);

glm::vec4 getPosition(glm::mat4 m);
glm::vec3 getLook(glm::mat4 m);
glm::vec3 getUp(glm::mat4 m);
glm::vec3 getRight(glm::mat4 m);
glm::quat getRotation(glm::mat4 m);
glm::vec3 getScale(glm::mat4 m);    /// @todo

glm::mat4 lerp(glm::mat4 m, glm::vec3 pos, glm::quat rot, glm::vec3 scale, float v);
glm::mat4 lerp(glm::mat4 m, glm::vec3 pos, float v);

} // namespace engine::utils

#endif