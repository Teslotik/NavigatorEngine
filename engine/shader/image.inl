#ifndef IMAGE_SHADER
#define IMAGE_SHADER

namespace engine::shader::image {

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

    uniform sampler2D image;

    out vec4 frag;

    void main() {
        frag = texture(image, uv);
        // Keep it for tests
        // frag = vec4(uv.x, uv.y, 1.0, 1.0);
        // frag = vec4(vec3(texture(image, uv).r), 1.0);
    }
)";

}

#endif