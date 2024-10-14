#ifndef BLEND_SHADER
#define BLEND_SHADER

namespace engine::shader::blend {

static const char *frag = R"(#version 330 core
    const int Normal        = 0;
    const int AlphaOver     = 1;
    const int Dissolve      = 5;
    const int Add           = 10;
    const int Subtract      = 15;
    const int Multiply      = 20;
    const int Divide        = 25;
    const int Screen        = 30;
    const int Exclusion     = 35;
    const int Difference    = 40;
    const int Power         = 45;
    const int Root          = 50;
    const int Overlay       = 55;
    const int HardLight     = 60;
    const int SoftLight     = 65;
    const int VividLight    = 70;
    const int LinearLight   = 75;
    const int Lighten       = 80;
    const int Darken        = 85;
    const int ColorBurn     = 90;
    const int LinearBurn    = 95;
    const int ColorDodge    = 100;
    const int LinearDodge   = 105;
    const int Hue           = 110;
    const int Saturation    = 115;
    const int Value         = 120;
    const int Color         = 125;
    // const int Offset        = 130;
    // const int Rotation      = 135;
    
    in vec3 pos;
    in vec4 col;
    in vec2 uv;

    uniform sampler2D src;
    uniform sampler2D dst;
    uniform int blend;
    uniform float factor;
    uniform int alpha;

    out vec4 frag;

    void main() {
        vec4 s = texture(src, uv);
        vec4 d = texture(dst, uv);
        float a = (alpha == 1) ? 1.0 : s.a;
        // float a = (alpha == 1) ? 1.0 : d.a;

        vec4 m = vec4(0);
        // // m = mix(s, d, d.w);
        // m = d;
        // if (true) {

        // }
        if (blend == Normal) {
            m = d;
        } else if (blend == AlphaOver) {
            // https://github.com/blender/blender/blob/fc8341538a9cd5e0e4c049497277507b82237c9a/source/blender/compositor/realtime_compositor/shaders/library/gpu_shader_compositor_alpha_over.glsl#L23
            // m = mix(s, vec4(d.rgb, 1.0), d.a);
            m = (1.0 - d.a) * s + d;
        } else if (blend == Dissolve) {

        } else if (blend == Add) {
            m = s + d;
            m.a = a;
        } else if (blend == Subtract) {
            m = s - d;
            m.a = a;
        } else if (blend == Multiply) {
            m = s * d;
            m.a = a;
        } else if (blend == Divide) {
            m = s / d;
            m.a = a;
        } else if (blend == Screen) {
            
        } else if (blend == Exclusion) {
            
        } else if (blend == Difference) {
            
        } else if (blend == Power) {
            
        } else if (blend == Root) {
            
        } else if (blend == Overlay) {
            
        } else if (blend == HardLight) {
            
        } else if (blend == SoftLight) {
            
        } else if (blend == VividLight) {
            
        } else if (blend == LinearLight) {
            
        } else if (blend == Lighten) {
            
        } else if (blend == Darken) {
            
        } else if (blend == ColorBurn) {
            
        } else if (blend == LinearBurn) {
            
        } else if (blend == ColorDodge) {
            
        } else if (blend == LinearDodge) {
            
        } else if (blend == Hue) {
            
        } else if (blend == Saturation) {
            
        } else if (blend == Value) {
            
        } else if (blend == Color) {
            
        }
        frag = mix(s, m, factor);
    }
)";

}

#endif