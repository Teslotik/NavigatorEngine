#ifndef RENDER_STACK_H
#define RENDER_STACK_H

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "surface.h"
#include "entity/Shader.h"
#include "primitive.h"

namespace engine {

/// @note I made such class in my other engine and ported it,
/// but I'am not sure is it working corretly
/// @todo Need to test
template<class T, int S = 25>
class RenderStack {
protected:
    unsigned x = 0, y = 0, w = 100, h = 100;
    unsigned iBuffer = 0;
    T buffers[S];
    T backbuffer;
    T backbuffer2;
    primitive::Quad quad;
    vector<glm::mat4> transformation;
    // vector<unisigned[4]> scissor;
    // unsigned view[4];

public:
    Shader *mixer = nullptr;

    RenderStack(unsigned maxW, unsigned maxH) {
        for (int i = 0; i < S; i++) buffers[i].init(maxW, maxH);
    };

    glm::mat4 getTransform() {
        return transformation.empty() ? glm::mat4(1.0) : transformation.back();
    }

    void pushTransform(glm::mat4 t) {
        transformation.push_back(getTransform() * t);
    }

    // void viewport(unsigned x, unsigned y, unsigned w, unsigned h) {
    //     view = {x, y, w, h};
    // }

    unsigned getWidth() {
        return w;
    }

    unsigned getHeight() {
        return h;
    }

    T *getBuffer() {
        return &buffers[iBuffer];
    }

    T *getBackbuffer() {
        return &backbuffer;
    }

    T *getBackbuffer2() {
        return &backbuffer2;
    }

    T *push() {
        if (iBuffer + 1 >= S) return nullptr;
        buffers[iBuffer].end();

        backbuffer.begin();
        backbuffer.clear(0, 0, 0, 0);
        backbuffer.end();
        
        backbuffer2.begin();
        backbuffer2.clear(0, 0, 0, 0);
        backbuffer2.end();
        
        T &buffer = buffers[++iBuffer];
        buffer.begin();
        buffer.clear(0, 0, 0, 0);

        return &buffer;
    }

    T *pop(int blend = 1, float factor = 1.0) {
        if (iBuffer == 0) return nullptr;
        
        T *top = &buffers[iBuffer];

        if (blend == 0) {
            // backbuffer.clear(Transparent);
            top->clear(0, 0, 0, 0);
            top->end();
            top = &buffers[--iBuffer];
            top->begin();
            return top;
        }

        top.end();
        
        T *current = &buffers[--iBuffer];
        
        mixer->setMat4("mvp", getTransform());
        mixer->setTex("src", current->col, 0);
        mixer->setTex("dst", top->col, 1);
        mixer->setInt("blend", blend);
        mixer->setFloat("factor", factor);
        mixer->setInt("alpha", 0);

        /// @todo not fullscreen
        backbuffer.begin();
        quad.begin(-100, -100, 100, 100);
        mixer->use();
        quad.end();
        backbuffer.end();
        
        top.begin();
        swap();
    }

    T *swap() {
        T &buffer = buffers[iBuffer];
        buffer.clear();
        buffer.end();
        std::swap(buffer, backbuffer);
        buffer.begin();
        return &buffer;
    }

    T *swap2() {
        T &buffer = buffers[iBuffer];
        buffer.clear();
        buffer.end();
        std::swap(buffer, backbuffer2);
        buffer.begin();
        return &buffer;
    }

    void clear() {
        iBuffer = 0;
        transformation.clear();
    }

    // void clearTransform() {
    // }

};

} // namespace engine

#endif