#include "utils.inl"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// #include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <iostream>

// using namespace engine::utils;
using glm::mat4;
using glm::vec3;
using glm::vec4;

float engine::utils::area(float x1, float y1, float x2, float y2, float x3, float y3) {
    return fabs((x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) / 2.0f);
}

bool engine::utils::isInside(float x1, float y1, float x2, float y2, float x3, float y3, float x, float y) {
    float A = area(x1, y1, x2, y2, x3, y3);
    float A1 = area(x, y, x2, y2, x3, y3);
    float A2 = area(x1, y1, x, y, x3, y3);
    float A3 = area(x1, y1, x2, y2, x, y);
    return fabs(A1 + A2 + A3 - A) < 1e-3f;
}

float engine::utils::rayTriangleIntersection(const glm::vec3& orig, const glm::vec3& dir, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
    glm::vec3 e1 = v1 - v0;
    glm::vec3 e2 = v2 - v0;
    // Вычисление вектора нормали к плоскости
    glm::vec3 pvec = cross(dir, e2);
    float det = dot(e1, pvec);

    // Луч параллелен плоскости
    if (det < 1e-8 && det > -1e-8) {
        return 0;
    }

    float inv_det = 1.0f / det;
    glm::vec3 tvec = orig - v0;
    float u = dot(tvec, pvec) * inv_det;
    if (u < 0 || u > 1) {
        return 0;
    }

    glm::vec3 qvec = cross(tvec, e1);
    float v = dot(dir, qvec) * inv_det;
    if (v < 0 || u + v > 1) {
        return 0;
    }
    
    return glm::dot(e2, qvec) * inv_det;
}

// float engine::utils::rayShphereIntersection(vec3 orig, vec3 dir, vec3 center, float r) {
//     vec3 oc = orig - center;
//     float b = dot(oc, dir);
//     float c = dot(oc, oc) - r * r;
//     float h = b * b - c;
//     if (h < 0.0) return 0; // no intersection
//     h = sqrt(h);
//     // return -b - h;
//     return -b + h;
// }

// bool intersect_sphere(const struct Sphere* sphere, const struct Ray* ray, float* t) {
float engine::utils::rayShphereIntersection(vec3 orig, vec3 dir, vec3 center, float r) {
    vec3 rayToCenter = center - orig;

    /* calculate coefficients a, b, c from quadratic equation */

    /* float a = dot(ray->dir, ray->dir); // ray direction is normalised, dotproduct simplifies to 1 */ 
    float b = dot(rayToCenter, dir);
    float c = dot(rayToCenter, rayToCenter) - r * r;
    float disc = b * b - c; /* discriminant of quadratic formula */

    /* solve for t (distance to hitpoint along ray) */

    if (disc < 0.0f) return 0;
    float t = b - sqrt(disc);

    if (t < 0.0f) {
        t = b + sqrt(disc);
        if (t < 0.0f) return 0; 
    }
    return t;
}

glm::vec4 engine::utils::getPosition(glm::mat4 m) {
    mat4 inv = glm::inverse(m);
    return glm::column(inv, 3);
}

glm::vec3 engine::utils::getUp(glm::mat4 m) {
    glm::vec3 pp = glm::row(m, 2);
    return glm::normalize(glm::vec3(pp));
}

glm::vec3 engine::utils::getRight(glm::mat4 m) {
    glm::vec3 pp = glm::row(m, 0);
    return glm::normalize(glm::vec3(pp));
}

/// @todo
glm::quat engine::utils::getRotation(glm::mat4 m) {
    return glm::conjugate(glm::normalize(glm::quat_cast(m)));   /// @todo not sure about `conjugate`, need to test
}

glm::mat4 engine::utils::lerp(glm::mat4 m, glm::vec3 pos, glm::quat rot, glm::vec3 scale, float v) {
    // Interpolation
    glm::vec3 p = glm::mix(glm::vec3(engine::utils::getPosition(m)), pos, v);
    auto r = glm::slerp(engine::utils::getRotation(m), rot, v);
    /// @todo scale

    // Compositing
    return glm::mat4_cast(glm::conjugate(r)) * glm::translate(glm::mat4(1.0f), -p);
}

glm::mat4 engine::utils::lerp(glm::mat4 m, glm::vec3 pos, float v) {
    return lerp(m, pos, glm::quat{}, vec3(1.0f), v);
}