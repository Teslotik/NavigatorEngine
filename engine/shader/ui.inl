#ifndef GUI_SHADER
#define GUI_SHADER

namespace engine::shader::gui {

static const char *vert = R"(#version 330 core
    in vec3 aPos;
    in vec2 aUv;
    in vec4 aCol;

    out vec3 pos;
    out vec2 uv;
    out vec4 col;

    uniform mat4 mvp;

    void main() {
        gl_Position = mvp * vec4(aPos, 1.0);
        pos = aPos;
        uv = aUv;
        col = aCol;
    }
)";

static const char *coloredFrag = R"(#version 330 core
    in vec3 pos;
    in vec2 uv;
    in vec4 col;

    out vec4 frag;

    uniform mat4 mvp;

    void main() {
        // frag = vec4(0.0, 1.0, 1.0, 1.0);
        // frag = vec4(uv, 0.0, 1.0);
        frag = col;
    }
)";

static const char *imageFrag = R"(#version 330 core
    in vec3 pos;
    in vec2 uv;
    in vec4 col;

    out vec4 frag;

    uniform mat4 mvp;
    uniform sampler2D atlas;

    void main() {
        // frag = vec4(0.0, 1.0, 1.0, 1.0);
        // frag = vec4(uv, 0.0, 1.0);
        // frag = col;
        frag = texture(atlas, uv) * col;
    }
)";

}

#endif