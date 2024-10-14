#ifndef SSAO_SHADER
#define SSAO_SHADER

namespace engine::shader::ssao {

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

    uniform sampler2D position;
    uniform sampler2D normal;
    uniform sampler2D depth;

    uniform mat4 proj;
    uniform mat4 view;
    uniform mat4 matrixNormal;
    uniform vec3 eye;

    // @todo seed
    vec3 hash(uvec3 x) {
        const uint k = 1664525U;    // Numerical Recipes

        x = ((x >> 8U) ^ x.yzx) * k;
        x = ((x >> 8U) ^ x.yzx) * k;
        x = ((x >> 8U) ^ x.yzx) * k;
        
        return vec3(x) * (1.0 / float(0xffffffffU));
    }

    float ease(float x) {
        float t = 1.0 - x;
        return (x * x);
    }

    const float near = 0.1;
    const float far = 100.0;

    float linear(float depth) {
        float z = depth * 2.0 - 1.0;
        return (2.0 * near * far) / (far + near - z * (far - near));
    }

    void main() {
        // float radius = 0.085;    //
        // float radius = 0.1;
        // float bias = 0.1;    //
        int samples = 20;

        float bias = 0.11;
        float radius = 0.1;
        float codirectional = 2.0;

        vec2 ndc = uv * 2.0 - 1.0;
        vec3 basePos = vec3(ndc.x, ndc.y, linear(texture(depth, uv).r));
        /// @todo world space position
        // vec3 basePos = eye + vec3(ndc.x, ndc.y, linear(texture(depth, uv).r));
        // vec3 basePos = texture(position, uv).xyz;
        vec3 baseNorm = normalize((matrixNormal * texture(normal, uv)).xyz);

        vec2 size = textureSize(depth, 0);
        float occ = 0.0;
        for (int i = 0; i < samples; ++i) {
            vec3 n = normalize(hash(uvec3(uint(uv.x * size.x), uint(uv.y * size.y), i)) * 2.0 - 1.0);
            // Orienting hemisphere to the baseNorm direction
            if (dot(baseNorm, n) < 0.0) n = -n;
            // We need to cast more rays towards surface normal to reduce noise on the plains
            n = normalize(baseNorm * codirectional + n);

            // used instead of randomization
            float rayLength = float(i) / float(samples);
            // reconstructing 3d position
            // squared rayLength is used to frequently test occlusion closer to the basePos
            vec3 posSample = basePos + n * radius * (rayLength * rayLength);
            float depthSample = linear(texture(depth, posSample.xy * 0.5 + 0.5).r);

            float rangeCheck = smoothstep(0.0, 1.0, radius / (posSample.z - depthSample));
            // float rangeCheck = clamp(0.0, 1.0, radius / (posSample.z - depthSample));

            // if (depthSample < posSample.z - bias) occ += rangeCheck;
            if (depthSample < posSample.z - bias) occ += pow(rangeCheck, 0.3);
        }

        frag = vec4(vec3(ease(1.0 - float(occ) / float(samples))), 1.0);
    }
)";

}

#endif