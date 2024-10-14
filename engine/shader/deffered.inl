#ifndef DEFFERED_SHADER
#define DEFFERED_SHADER

namespace engine::shader::deffered {

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
    struct Lamp {
        vec3 pos;
        vec3 dir;
        vec4 col;
    };
    
    in vec2 uv;

    out vec4 frag;

    // uniform sampler2D rendered;
    uniform sampler2D albedo;
    uniform sampler2D normal;
    // uniform sampler2D depth;
    uniform sampler2D shadow;
    uniform sampler2D ambientOcclusion;
    
    uniform vec4 ambient;
    uniform Lamp sun;

    void main() {
        vec4 s = texture(shadow, uv);
        vec4 ao = texture(ambientOcclusion, uv);
        vec4 a = texture(albedo, uv);
        vec3 n = texture(normal, uv).xyz;
        
        vec4 diffuse = vec4(vec3(max(dot(n, sun.dir), 0.0)), 1.0) * sun.col;
        frag = (ambient + diffuse) * a * s * ao;
    }
)";

}

#endif