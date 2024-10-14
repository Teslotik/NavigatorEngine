#ifndef GBUFFER_SHADER
#define GBUFFER_SHADER

namespace engine::shader::gbuffer {

static const char *vert = R"(#version 330 core
    layout (location = 0) in int aIdx;
    layout (location = 1) in vec3 aPos;
    layout (location = 2) in vec3 aNorm;
    layout (location = 3) in vec4 aCol;

    flat out int idx;
    out vec3 pos;
    out vec3 norm;
    out vec4 col;

    uniform mat4 mvp;

    void main() {
        vec4 p = mvp * vec4(aPos, 1.0);
        gl_Position = p;
        idx = aIdx;
        // pos = p.xyz / p.w;
        pos = aPos;
        norm = aNorm;
        col = aCol;
    }
)";

static const char *frag = R"(#version 330 core
    flat in int idx;
    in vec3 pos;
    in vec3 norm;
    in vec4 col;
    
    layout (location = 0) out vec3 gPos;
    layout (location = 1) out vec3 gNorm;
    layout (location = 2) out vec4 gCol;
    layout (location = 3) out vec4 gShad;
    
    uniform mat4 lightSpaceMatrix;
    uniform sampler2D shadow;

    void main() {
        gPos = pos;
        gNorm = normalize(norm);
        gCol = col;

        vec4 fragPos = lightSpaceMatrix * vec4(pos, 1.0);
        vec3 proj = fragPos.xyz / fragPos.w;
        proj = proj * 0.5 + 0.5;
        float bias = 0.002;
        float sh = 0.0;
        vec2 texelSize = 1.0 / textureSize(shadow, 0);
        for (int x = -1; x <= 1; ++x) {
            for (int y = -1; y <= 1; ++y) {
                float dep = texture(shadow, proj.xy + vec2(x, y) * texelSize).r; 
                sh += dep > proj.z - bias ? 1.0 : 0.35;
            }
        }
        sh /= 9.0;
        gShad = vec4(vec3(sh), 1.0);
    }
)";

}

#endif