#ifndef MATERIAL_H
#define MATERIAL_H

namespace engine::data {

struct Material {
    float albedo[4];
    float roughness;
    float metallic;
    float emission;
    float ior;
};

} // namespace engine::data

#endif