#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "entity/Shader.h"

using std::cout;
using std::endl;
using std::swap;
using std::vector;
using engine::entity::Shader;

namespace engine::primitive {

struct Quad {
    unsigned int vbo;
    unsigned int ibo;
    unsigned int vao;
    unsigned iCount;

    Quad() {
        unsigned int indices[] = {
            0, 1, 3,
            1, 2, 3
        };

        glGenVertexArrays(1, &vao); 
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ibo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned), indices, GL_STATIC_DRAW);    // sizeof(indices)

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        this->iCount = 6;
    }

    ~Quad() {
        // glDeleteBuffers(1, &vbo);
        // glDeleteBuffers(1, &ibo);
        // glDeleteBuffers(1, &vao);
    }

    void begin(float x, float y, float w, float h) {
        // xyz, uv
        float vertices[] = {
            x,      y,      0.0f, 0.0f, 1.0f,
            x + w,  y,      0.0f, 1.0f, 1.0f,
            x + w,  y + h,  0.0f, 1.0f, 0.0f,
            x,      y + h,  0.0f, 0.0f, 0.0f,
        };

        glBindVertexArray(vao);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 4 * 5 * sizeof(float), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void end() {
        glDrawElements(GL_TRIANGLES, iCount, GL_UNSIGNED_INT, 0);
    }
};

}

#endif