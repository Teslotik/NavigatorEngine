#ifndef MESH_ACTOR_H
#define MESH_ACTOR_H

#include "ActorEntity.h"
#include "interface/IGameRenderable.h"

namespace engine::entity {

class MeshActor : public ActorEntity, virtual public IGameRenderable {
public:
    unsigned int vbo;
    unsigned int ibo;
    unsigned int vao;

    unsigned iCount;
    unsigned vCount;
    unsigned structureLength = 10;

    /// @note Do not free this after glBufferData! It could be used for batch rendering
    unsigned *indices;
    float *vertices;

    MeshActor(float *vertices, int verticesCount, unsigned int *indices, int indicesCount) {
        this->indices = indices;
        this->vertices = vertices;
        this->iCount = indicesCount;
        this->vCount = verticesCount;

        unsigned idx;
        int *ib = new int[verticesCount];
        for (int i = 0; i < verticesCount; i++) ib[i] = i;

        glGenVertexArrays(1, &vao); 
        glGenBuffers(1, &idx);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ibo);

        glBindVertexArray(vao);
        
        glBindBuffer(GL_ARRAY_BUFFER, idx);
        glBufferData(GL_ARRAY_BUFFER, verticesCount * sizeof(int), ib, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        // glBufferData(GL_ARRAY_BUFFER, verticesCount * sizeof(float) * structureLength, vertices, GL_STATIC_DRAW);
        glBufferData(GL_ARRAY_BUFFER, verticesCount * sizeof(float), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesCount * sizeof(unsigned), indices, GL_STATIC_DRAW);

        // index attribute
        // glVertexAttribPointer(0, 1, GL_INT, GL_FALSE, sizeof(int), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, idx);
        glVertexAttribIPointer(0, 1, GL_INT, sizeof(int), (void*)0);
        glEnableVertexAttribArray(0);
        // position attribute
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, structureLength * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        // normal attribute
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, structureLength * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        // color
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, structureLength * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    ~MeshActor() {
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ibo);
        glDeleteBuffers(1, &vao);
        delete[] indices;
        delete[] vertices;
    }

    void beginRenderInGame(RenderStack<Surface> *renderer) override {
        glBindVertexArray(vao);
    }

    void endRenderInGame(RenderStack<Surface> *renderer) override {
        glDrawElements(GL_TRIANGLES, iCount, GL_UNSIGNED_INT, 0);
    };
};

} // namespace engine::entity

#endif