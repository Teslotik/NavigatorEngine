#ifndef SURFACE_H
#define SURFACE_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <glad/glad.h>
// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include <stb_image_write.h>

using std::cout;
using std::endl;
using std::swap;
using std::vector;

namespace engine {

/// @todo refactor hierarchy and add destructors
struct Surface {
    unsigned fbo = 0;
    unsigned col = 0;
    unsigned depth = 0;

    int maxW, maxH;
    // int w, h;

    // Surface() = delete;
    Surface() = default;

    Surface(unsigned w, unsigned h) {
        init(w, h);
    }

    virtual ~Surface() {
        // delete[] buffer;
    }

    /// @note this method it is not a good idea at all,
    /// because user can forgot to initialize Surface.
    /// but it is required for RenderStack class
    /// @todo make something with it
    void init(int w, int h) {
        maxW = w;
        maxH = h;
        // this->w = w;
        // this->h = h;

        glGenFramebuffers(1, &fbo);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &col);
        glBindTexture(GL_TEXTURE_2D, col);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI, w, h, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, nullptr);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, col, 0);

        glGenRenderbuffers(1, &depth);
        glBindRenderbuffer(GL_RENDERBUFFER, depth);
        
        // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        
        // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            cout << "[Error] Failed to create fbo" << endl;
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    virtual void begin() {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, maxW, maxH);
    }

    virtual void end() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void viewport(unsigned w, unsigned h) {
        glViewport(0, 0, w, h);
    }

    virtual void clear(float r = 1.0f, float g = 0.3f, float b = 0.3f, float a = 1.0f) {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // void resize(unsigned w, unsigned h) {
    //     this->w = w;
    //     this->h = h;
    // }

    // int* buffer = new int[800 * 600 * 4];   // @todo должен состоять из unsigned byte?
    // void screenshot(const char *pathname) {
    //     glBindTexture(GL_TEXTURE_2D, col);
    //     glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    //     // unsigned* buffer = new unsigned[800 * 600 * 4];
    //     // glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, buffer);
    //     stbi_write_png(pathname, 800, 600, 4, buffer, 800 * 4);
    // }

protected:
    
};

struct ScreenSurface : public Surface {
    // static ScreenSurface *Load() {
    //     ScreenSurface *surface = new ScreenSurface();
    //     return surface;
    // }

    ScreenSurface(unsigned w, unsigned h) {
        Surface::maxW = w;
        Surface::maxH = h;
    }
};

struct IntSurface : public Surface {
    IntSurface(int w, int h) {
        Surface::maxW = w;
        Surface::maxH = h;
        // this->w = w;
        // this->h = h;

        glGenFramebuffers(1, &fbo);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &col);
        glBindTexture(GL_TEXTURE_2D, col);
        
        // Настройка текстуры для буфера цвета...
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI, w, h, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, nullptr);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, w, h, 0, GL_RED_INTEGER, GL_INT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, col, 0);

        glGenRenderbuffers(1, &depth);
        glBindRenderbuffer(GL_RENDERBUFFER, depth);
        
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            cout << "[Error] Failed to create fbo" << endl;
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    virtual ~IntSurface() {
        // delete[] bb;
    }

    /// @todo
    // int* bb = new int[800 * 600];
    // int pick(int x, int y) {
    //     /// @todo unbind?
    //     glBindTexture(GL_TEXTURE_2D, col);
    //     glGetTexImage(GL_TEXTURE_2D, 0, GL_RED_INTEGER, GL_INT, bb);

    //     // glReadPixels(0, 0, 800, 600, GL_RED_INTEGER, GL_INT, bb);

    //     // cout << "pick" << endl;
    //     // // for (int i = 0; i < 800 * 600 * 4; i++) {
    //     // //     if (buffer[i] != 0 && buffer[i] != -1)
    //     // //     cout << (buffer[i] & 0xFF) << " " << ((buffer[i] >> 8) & 0xFF) << " " << ((buffer[i] >> 16) & 0xFF) << " " << ((buffer[i] >> 24) & 0xFF) << endl;
    //     // // }
    //     // // @todo combine rgba into single (unsigned?) integer
    //     // constexpr int channel = 0;
    //     // return buffer[x + y * 800 + channel * 600 * 800];
    //     // cout << x << ", " << y << ", " << x + y * w << endl;
    //     // cout << "x:" << x << ", y:" << y << ", pos:" << x + y * w << endl;
    //     // return bb[x + (h - y) * w];  //
    //     return bb[x + (maxH - y) * maxW];
    // }
};

struct DepthSurface : public Surface {
    DepthSurface(int w, int h) {
        Surface::maxW = w;
        Surface::maxH = h;
        // this->w = w;
        // this->h = h;

        glGenFramebuffers(1, &fbo);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &col);
        glBindTexture(GL_TEXTURE_2D, col);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, col, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            cout << "[Error] Failed to create depth fbo" << endl;
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    virtual void begin() override {
        glEnable(GL_DEPTH_TEST);
        Surface::begin();
    }

    virtual void end() override {
        Surface::end();
        glDisable(GL_DEPTH_TEST);
    }

    virtual void clear(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f) override {
        glClearColor(r, g, b, a);
        glClear(GL_DEPTH_BUFFER_BIT);
    }
};

struct GBuffer : public Surface {
    // unsigned col = 0;
    unsigned normal = 0;
    unsigned albedo = 0;
    unsigned pos = 0;
    unsigned shadow = 0;

    /// @note Некоторые буферы можно объединить
    GBuffer(unsigned w, unsigned h) {
        Surface::maxW = w;
        Surface::maxH = h;
        // this->w = w;
        // this->h = h;

        glGenFramebuffers(1, &fbo);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // Position buffer
        glGenTextures(1, &pos);
        glBindTexture(GL_TEXTURE_2D, pos);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pos, 0);
        
        // Normal buffer
        glGenTextures(1, &normal);
        glBindTexture(GL_TEXTURE_2D, normal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal, 0);
        
        // Color
        glGenTextures(1, &albedo);
        glBindTexture(GL_TEXTURE_2D, albedo);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, albedo, 0);

        // Shadow
        glGenTextures(1, &shadow);
        glBindTexture(GL_TEXTURE_2D, shadow);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, shadow, 0);

        unsigned attachments[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
        glDrawBuffers(4, attachments);

        //
        // glGenRenderbuffers(1, &depth);
        // glBindRenderbuffer(GL_RENDERBUFFER, depth);
        // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
        // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

        glGenTextures(1, &depth);
        glBindTexture(GL_TEXTURE_2D, depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            cout << "[Error] Failed to create gbuffer" << endl;
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

} // namespace engine

#endif