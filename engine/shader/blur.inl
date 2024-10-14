#ifndef BLUR_SHADER
#define BLUR_SHADER

namespace engine::shader::blur::box {

static const char *vert = R"(#version 330 core
    in vec3 aPos;
    in vec2 aUv;

    out vec2 uv;

    uniform mat4 mvp;

    void main() {
        gl_Position = mvp * vec4(aPos, 1.0);
        uv = aUv;
    }
)";

static const char *frag = R"(#version 330 core
    in vec2 uv;

    out vec4 frag;

    uniform sampler2D image;
    uniform int radius = 2;

    void main() {
        vec4 res = vec4(0.0);
        vec2 size = 1.0 / textureSize(image, 0);
        for (int x = -radius; x <= radius; ++x) {
            for (int y = -radius; y <= radius; ++y) {
                res += texture(image, uv + vec2(x * size.x, y * size.y));
            }
        }
        frag = vec4(res / ((radius * 2 + 1) * (radius * 2 + 1)));
    }
)";

}

#endif