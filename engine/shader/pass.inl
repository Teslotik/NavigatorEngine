#ifndef PASS_SHADER
#define PASS_SHADER

namespace engine::shader::pass {

static const char *vert = R"(#version 330 core
    in vec3 aPos;
    in vec2 aUv;

    out vec2 uv;

    uniform mat4 mvp;

    void main() {
        gl_Position = mvp * vec4(aPos, 1);
        uv = aUv;
    }
)";

}

#endif