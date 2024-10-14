#ifndef SUN_SHADER
#define SUN_SHADER

namespace engine::shader::sun {

static const char *vert = R"(#version 330 core
    layout (location = 0) in int aIdx;
    layout (location = 1) in vec3 aPos;
    layout (location = 2) in vec3 aNormal;
    layout (location = 3) in vec4 aCol;

    out vec2 uv;

    uniform mat4 mvp;

    void main() {
        gl_Position = mvp * vec4(aPos, 1.0);
    }
)";

static const char *frag = R"(#version 330 core
    void main() {
        
    }
)";

}

#endif