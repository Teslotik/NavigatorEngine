#ifndef UI_STATE_H
#define UI_STATE_H

#include <map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp>

#include <array>
#include <string>

#include "typedefs.h"
#include "shader/ui.inl"
#include "entity/Shader.h"
#include "utils.inl"

namespace engine::ui {

using std::array;
using std::string;

class Ui {
public:
    class Handle {
        friend class Ui;
        
        glm::mat4 local{1.0};
        glm::mat4 world{1.0};
        glm::mat4 inverse{1.0};
        glm::mat4 tr{1.0};

        float color[4] = {1.0, 1.0, 1.0, 1.0};  /// @todo stack of colors (tint)
        float z = 0.0f;

        Handle *parent = nullptr;
        unsigned index = 0;

        Handle() = default;

        void use(Handle *parent, unsigned index, float x, float y, float w, float h) {
            this->parent = parent;
            this->index = index;
            local = glm::mat4(1.0);
            tr = glm::mat4(1.0);
            this->x = x;
            this->y = y;
            z = -0.01 * (ui->iHandle + 1);
            this->w = w;
            this->h = h;
            texX = 0;
            texY = 0;
            texW = 1;
            texH = 1;
            color[0] = 1;
            color[1] = 1;
            color[2] = 1;
            color[3] = 1;
            im = false;
            buildTransform();
            isClipped = false;
        }

        inline void buildTransform() {
            local = glm::translate(glm::mat4(1.0), glm::vec3(w / 2.0f + x, h / 2.0f + y, 0.0f));
            if (parent) {
                world = parent->world * (parent->inverse * local) * tr;
            } else {
                world = local * tr;
            }
            inverse = glm::inverse(local);
        }

    public:
        Handle *transform(glm::mat4 m) {
            tr *= m;
            buildTransform();
            return this;
        }

        Handle *pop(unsigned c = 1) {
            return ui->popHandle(c);
        }

        Handle *activate() {
            assert(ui->iHandle >= index);
            return ui->popHandle(ui->iHandle - index);
        }

        Handle *setColor(float r, float g, float b, float a) {
            color[0] = r;
            color[1] = g;
            color[2] = b;
            color[3] = a;
            return this;
        }

        Handle *setColor(glm::vec4 col) {
            color[0] = col.r;
            color[1] = col.g;
            color[2] = col.b;
            color[3] = col.a;
            return this;
        }

        Handle *setImage(float x, float y, float w, float h) {
            texX = x;
            texY = y;
            texW = w;
            texH = h;
            im = true;
            return this;
        }

        Handle *setIcon(unsigned label) {
            assert(label < ui->tiles);
            texX = static_cast<float>(label) / static_cast<float>(ui->tiles);
            texY = 0;
            texW = 1.0f / static_cast<float>(ui->tiles);
            texH = 1.0f;
            im = true;
            return this;
        }

        [[nodiscard]] Handle *div(float ratio, bool vertical = false) {
            if (vertical) {
                float size = ratio * h;
                // Handle *handle = ui->pushHandle(x, y, w, size);
                // y += size;
                Handle *handle = ui->pushHandle(this, x, y + size, w, size);
                h -= size;
                return handle;
            }

            float size = ratio * w;
            // Handle *handle = ui->pushHandle(x, y, size, h);
            // x += size;
            Handle *handle = ui->pushHandle(this, x + size, y, size, h);
            w -= size;
            return handle;
        }

        [[nodiscard]] Handle *grid(unsigned rows, unsigned cols) {
            assert(rows > 0);
            assert(cols > 0);
            float width = (w - ui->spacing * (cols - 1)) / cols;
            float height = (h - ui->spacing * (rows - 1)) / rows;
            for (int y = 0; y < rows; y++) {
                for (int x = 0; x < cols; x++) {
                    // We push handles in the reverse order so user can fill grid like this:
                    // handle->pop()->rect(); handle->pop()->rect(); ...
                    ui->pushHandle(this,
                        // this->x + w - (x + 1) * width - ui->spacing * x,
                        // this->y + y * height + ui->spacing * y,

                        this->x + w - (x + 1) * width - ui->spacing * x,
                        this->y + h - (y + 1) * height - ui->spacing * y,

                        width, height
                    );
                }
            }
            return ui->getHandle(); // returning the first cell
        }

        [[nodiscard]] Handle *row(float w, float h) {
            return ui->pushHandle(this, x + this->w + ui->spacing, y, w, h);
        }

        [[nodiscard]] Handle *col(float w, float h) {
            return ui->pushHandle(this, x, y + this->h + ui->spacing, w, h);
        }

        /// @todo сделать anchor через литералы: 1.0r (relative) 1.0 (absolute)

        [[nodiscard]] Handle *anchorAbs(float left = 0, float top = 0, float right = 0, float bottom = 0) {
            return ui->pushHandle(this,
                x + left,
                y + top,
                w - (left + right),
                h - (top + bottom)
            );
        }

        [[nodiscard]] Handle *anchorRel(float left = 1, float top = 1, float right = 1, float bottom = 1) {
            float l = w * left;
            float t = h * top;
            float r = w * right;
            float b = h * bottom;
            return ui->pushHandle(this,
                x + l,
                y + t,
                w - (l + r),
                h - (t + b)
            );
        }

        [[nodiscard]] Handle *anchorCenter(float left, float top, float right, float bottom) {
            float l = w / 2.0f - left;
            float t = h / 2.0f - top;
            float r = w / 2.0f + right;
            float b = h / 2.0f + bottom;
            return ui->pushHandle(this,
                x + l,
                y + t,
                r - l,
                b - t
            );
        }

        [[nodiscard]] Handle *anchorRelSize(float horizontal, float vertical, float w, float h) {
            float x = this->x + utils::lerp(horizontal, -1.0f, 0.0f, 1.0f, this->w - w);
            float y = this->y + utils::lerp(vertical, -1.0f, 0.0f, 1.0f, this->h - h);
            return ui->pushHandle(this, x, y, w, h);
        }

        [[nodiscard]] Handle *floating(float x, float y, float z, float w, float h) {
            Handle *handle = ui->pushHandle(this, this->x + x, this->y + y, w, h);
            handle->z = z;
            return handle;
        }

        [[nodiscard]] Handle *floating(float x, float y, float w, float h) {
            return ui->pushHandle(this, this->x + x, this->y + y, w, h);
        }

        // widgets
        Handle *rect() {
            if (im) {
                drawImageRect(-w / 2.0f, -h / 2.0f, w, h, texX, texY, texW, texH, color[0], color[1], color[2], color[3]);
            } else {
                drawColoredRect(-w / 2.0f, -h / 2.0f, w, h, color[0], color[1], color[2], color[3]);
            }
            return this;
        }

        void text();
        void input();
        void slider();

        // area
        bool over(bool eat = true) {
            glm::vec4 ca = ui->camera * world * glm::vec4(-w / 2.0f, -h / 2.0f, z, 1.0);
            glm::vec4 cb = ui->camera * world * glm::vec4(-w / 2.0f, h / 2.0f, z, 1.0);
            glm::vec4 cc = ui->camera * world * glm::vec4(w / 2.0f, h / 2.0f, z, 1.0);
            glm::vec4 cd = ui->camera * world * glm::vec4(w / 2.0f, -h / 2.0f, z, 1.0);
            
            ca /= ca.w;
            cb /= cb.w;
            cc /= cc.w;
            cd /= cd.w;
            
            // Normalized device coordinates
            glm::vec2 mouse = glm::vec2(ui->mouse.x / ui->width * 2.0f - 1.0f, -(ui->mouse.y / ui->height * 2.0f - 1.0f));
            
            bool i1 = utils::isInside(ca.x, ca.y, cb.x, cb.y, cc.x, cc.y, mouse.x, mouse.y);
            bool i2 = utils::isInside(ca.x, ca.y, cc.x, cc.y, cd.x, cd.y, mouse.x, mouse.y);

            // bool test = !ui->event && (i1 || i2);
            // if (eat) ui->event |= test;
            bool test = (!ui->event || !eat) && (i1 || i2);
            ui->over |= i1 || i2;
            return test;
        }

        bool down(bool eat = true) {
            // return ui->lmb && over(eat);
            return over(eat) && ui->lmb;
        }

        bool clicked(bool eat = true) {
            // return ui->lmb && !ui->wasLmb && over(eat);
            return over(eat) && ui->lmb && !ui->wasLmb;
        }

        bool released(bool eat = true) {
            // return !ui->lmb && ui->wasLmb && over(eat);
            return over(eat) && !ui->lmb && ui->wasLmb;
        }

        void scrolled();

        Handle *clip() {
            isClipped = true;
            return this;
        }

        // Params

        Handle *setSpacing(float s) {
            ui->spacing = s;
            return this;
        }

        Handle *setPadding(float l, float t, float r, float b) {
            ui->padding[0] = l;
            ui->padding[1] = t;
            ui->padding[2] = r;
            ui->padding[3] = b;
            return this;
        }

        void getW();
        void getH();
        void getX();
        void getY();
        
    protected:
    public:
        float x = 0, y = 0;
        float w = 100, h = 100;
        float texX = 0, texY = 0;
        float texW = 0, texH = 0;
        bool im = false;
        Ui *ui;
        bool isClipped = false;

        void drawColoredRect(float x, float y, float w, float h, float r, float g, float b, float a) {
            if (ui->vCount >= 512) ui->draw();

            ui->setShader(&ui->colored);

            glm::vec4 ca = world * glm::vec4(x, y, z, 1.0);
            glm::vec4 cb = world * glm::vec4(x, y + h, z, 1.0);
            glm::vec4 cc = world * glm::vec4(x + w, y + h, z, 1.0);
            glm::vec4 cd = world * glm::vec4(x + w, y, z, 1.0);

            ca /= ca.w;
            cb /= cb.w;
            cc /= cc.w;
            cd /= cd.w;

            // Position
            ui->setPos(0, ca.x, ca.y, ca.z);
            ui->setPos(1, cb.x, cb.y, cb.z);
            ui->setPos(2, cc.x, cc.y, cc.z);
            ui->setPos(3, cd.x, cd.y, cd.z);

            // UV
            ui->setUv(0, 0, 1);
            ui->setUv(1, 0, 0);
            ui->setUv(2, 1, 0);
            ui->setUv(3, 1, 1);

            // Color
            ui->setCol(0, r, g, b, a);
            ui->setCol(1, r, g, b, a);
            ui->setCol(2, r, g, b, a);
            ui->setCol(3, r, g, b, a);

            ui->update(4);
        }

        void drawImageRect(float x, float y, float w, float h, float texX, float texY, float texW, float texH, float r, float g, float b, float a) {
            if (ui->vCount >= 512) ui->draw();

            ui->setShader(&ui->image);

            glm::vec4 ca = world * glm::vec4(x, y, z, 1.0);
            glm::vec4 cb = world * glm::vec4(x, y + h, z, 1.0);
            glm::vec4 cc = world * glm::vec4(x + w, y + h, z, 1.0);
            glm::vec4 cd = world * glm::vec4(x + w, y, z, 1.0);

            ca /= ca.w;
            cb /= cb.w;
            cc /= cc.w;
            cd /= cd.w;

            ui->setPos(0, ca.x, ca.y, ca.z);
            ui->setPos(1, cb.x, cb.y, cb.z);
            ui->setPos(2, cc.x, cc.y, cc.z);
            ui->setPos(3, cd.x, cd.y, cd.z);

            // UV
            ui->setUv(0, texX, texY + texH);
            ui->setUv(1, texX, texY);
            ui->setUv(2, texX + texW, texY);
            ui->setUv(3, texX + texW, texY + texH);

            // Color
            ui->setCol(0, r, g, b, a);
            ui->setCol(1, r, g, b, a);
            ui->setCol(2, r, g, b, a);
            ui->setCol(3, r, g, b, a);

            ui->update(4);
        }

        /// @todo Make it work
        /*
        void drawSprite(float x, float y, float w, float h, float r, float g, float b, float a) {
            if (ui->vCount >= 512) ui->draw();

            glm::mat4 orthoCamera(1.0f);
            glm::vec4 ca = glm::inverse(ui->view) * orthoCamera * world * glm::vec4(x, y, z, 1.0);
            glm::vec4 cb = glm::inverse(ui->view) * orthoCamera * world * glm::vec4(x, y + h, z, 1.0);
            glm::vec4 cc = glm::inverse(ui->view) * orthoCamera * world * glm::vec4(x + w, y + h, z, 1.0);
            glm::vec4 cd = glm::inverse(ui->view) * orthoCamera * world * glm::vec4(x + w, y, z, 1.0);

            ca /= ca.w;
            cb /= cb.w;
            cc /= cc.w;
            cd /= cd.w;

            ui->setPos(0, ca.x, ca.y, ca.z);
            ui->setPos(1, cb.x, cb.y, cb.z);
            ui->setPos(2, cc.x, cc.y, cc.z);
            ui->setPos(3, cd.x, cd.y, cd.z);

            // UV
            ui->setUv(0, 0, 1);
            ui->setUv(1, 0, 0);
            ui->setUv(2, 1, 0);
            ui->setUv(3, 1, 1);

            // Color
            ui->setCol(0, r, g, b, a);
            ui->setCol(1, r, g, b, a);
            ui->setCol(2, r, g, b, a);
            ui->setCol(3, r, g, b, a);

            ui->update(4);
        }
        */

        void drawLine(float x1, float y1, float x2, float y2, float w, float r, float g, float b, float a) {
            if (ui->vCount >= 512) ui->draw();

            ui->setShader(&ui->colored);

            glm::vec3 perp = glm::normalize(glm::vec3(y1 - y2, 0, x2 - x1)) * w / 2.0f;

            glm::vec4 ca = world * glm::vec4(glm::vec3(x1, 0, y1) - perp, 1.0f);
            glm::vec4 cb = world * glm::vec4(glm::vec3(x1, 0, y1) + perp, 1.0f);
            glm::vec4 cc = world * glm::vec4(glm::vec3(x2, 0, y2) + perp, 1.0f);
            glm::vec4 cd = world * glm::vec4(glm::vec3(x2, 0, y2) - perp, 1.0f);

            ca /= ca.w;
            cb /= cb.w;
            cc /= cc.w;
            cd /= cd.w;

            // Position
            ui->setPos(0, ca.x, ca.y, ca.z);
            ui->setPos(1, cb.x, cb.y, cb.z);
            ui->setPos(2, cc.x, cc.y, cc.z);
            ui->setPos(3, cd.x, cd.y, cd.z);

            // UV
            ui->setUv(0, 0, 1);
            ui->setUv(1, 0, 0);
            ui->setUv(2, 1, 0);
            ui->setUv(3, 1, 1);

            // Color
            ui->setCol(0, r, g, b, a);
            ui->setCol(1, r, g, b, a);
            ui->setCol(2, r, g, b, a);
            ui->setCol(3, r, g, b, a);

            ui->update(4);
        }
    };

    // handle
    Handle *getHandle() {
        return &handles[iHandle];
    }

    Handle *getParent();

    // Persistent

    Ui *setProp(size_t label, Property property) {
        properties[label] = property;
        return this;
    }
    
    Ui *setProp(string label, Property property) {
        setProp(std::hash<string>{}(label), property);
        return this;
    }

    Property getProp(size_t label) {
        return properties[label];
    }

    Property getProp(string label) {
        return getProp(std::hash<string>{}(label));
    }

    Ui *removeProp(size_t label) {
        properties.erase(label);
        return this;
    }

    Ui *removeProp(string label) {
        removeProp(std::hash<string>{}(label));
        return this;
    }

private:
    unsigned int vbo;
    unsigned int ibo;
    unsigned int vao;
    unsigned structureLength = 3 + 2 + 4;   // pos, uv, col
    float vertices[512 * (3 + 2 + 4) * 4];  // multiplication by 4, because 4 vertices per widget (handle)
    unsigned indices[512 * 6];
    int iCount = 0;
    int vCount = 0;

    std::map<size_t, Property> properties;
    Handle handles[512];
    Handle *handle = nullptr;
    unsigned iHandle = 0;   // pointer to the top of the stack

    engine::entity::Shader colored;
    engine::entity::Shader image;
    engine::entity::Shader *shader = nullptr;

public:
    glm::mat4 camera{1.0};
    glm::mat4 view{1.0};
    glm::mat4 proj{1.0};
    
    float spacing = 0;
    float padding[4] = {0, 0, 0, 0};

    int width, height;
    int atlas = -1;
    unsigned tiles = 1;

    // input
    glm::vec2 mouse{0.0f, 0.0f};
    glm::vec2 prev{0.0f, 0.0f};
    bool event = false;
    bool lmb = false;
    bool wasLmb = false;
    bool over = false;
    bool drag = false;
    bool wasDragged = false;
    glm::vec2 delta{0.0, 0.0};

    void update(unsigned vCount) {
        unsigned iCount = (vCount - 2) * 3;

        // Fan triangulation
        for (int i = 0; i < vCount - 2; i++) {
            indices[this->iCount + i * 3 + 0] = this->vCount;
            indices[this->iCount + i * 3 + 1] = this->vCount + i + 1;
            indices[this->iCount + i * 3 + 2] = this->vCount + i + 2;
        }

        this->iCount += iCount;
        this->vCount += vCount;
    }

    void setShader(engine::entity::Shader *shader) {
        if (shader != this->shader) draw();
        this->shader = shader;
    }

    void setPos(unsigned i, float x, float y, float z) {
        int offset = vCount * structureLength;
        vertices[(offset + structureLength * i + 0)] = x;
        vertices[(offset + structureLength * i + 1)] = y;
        vertices[(offset + structureLength * i + 2)] = z;
    }

    void setUv(unsigned i, float x, float y) {
        int offset = vCount * structureLength;
        vertices[(offset + structureLength * i + 3)] = x;
        vertices[(offset + structureLength * i + 4)] = y;
    }

    void setCol(unsigned i, float r, float g, float b, float a) {
        int offset = vCount * structureLength;
        vertices[(offset + structureLength * i + 5)] = r;
        vertices[(offset + structureLength * i + 6)] = g;
        vertices[(offset + structureLength * i + 7)] = b;
        vertices[(offset + structureLength * i + 8)] = a;
    }
    
    void drawPoint(float x, float y, float z, float w, float h, float r, float g, float b, float a) {
        if (vCount >= 512) draw();
        
        setShader(&colored);

        // Position
        setPos(0, x, y, z);
        setPos(1, x, y + h, z);
        setPos(2, x + w, y + h, z);
        setPos(3, x + w, y, z);

        // UV
        setUv(0, 0, 1);
        setUv(1, 0, 0);
        setUv(2, 1, 0);
        setUv(3, 1, 1);

        // Color
        setCol(0, r, g, b, a);
        setCol(1, r, g, b, a);
        setCol(2, r, g, b, a);
        setCol(3, r, g, b, a);

        update(4);
    }

    void setAtlas(int image, unsigned tiles = 1) {
        if (image == atlas) return;
        draw();
        atlas = image;
        this->tiles = tiles;
    }

    Handle *pushHandle(Handle *parent, float x, float y, float w, float h) {
        // cout << x << ", " << y << ", " << w << ", " << h << endl;

        if (handles[iHandle].isClipped) {
            Handle *handle = &handles[iHandle];
            draw();
            // glEnable(GL_SCISSOR_TEST);
            glScissor(handle->x, height - handle->y - handle->h, handle->w, handle->h);
        }

        handle = &handles[++iHandle];
        handle->use(
            parent,
            iHandle,
            x + padding[0],
            y + padding[1],
            w - (padding[2] + padding[0]),
            h - (padding[3] + padding[1])
        );
        return handle;
    }

    Handle *popHandle(unsigned c) {
        for (int i = 0; i < c && iHandle > 0; i++, iHandle--) {
            if (handles[iHandle].isClipped) {
                draw();
                glScissor(0, 0, width, height);
                // glDisable(GL_SCISSOR_TEST);
            }
        }

        spacing = 0;
        padding[0] =  0;
        padding[1] =  0;
        padding[2] =  0;
        padding[3] =  0;
        
        return &handles[iHandle];
    }

public:

    Ui(): colored(
        engine::shader::gui::vert, engine::shader::gui::coloredFrag
    ), image(
        engine::shader::gui::vert, engine::shader::gui::imageFrag
    ) {
        shader = &colored;
        for (auto &i: handles) i.ui = this;

        glGenVertexArrays(1, &vao); 
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ibo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, structureLength * sizeof(float), (void*)0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, structureLength * sizeof(float), (void*)(3 * sizeof(float)));
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, structureLength * sizeof(float), (void*)((3 + 2) * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void draw() {
        if (!iCount) return;
        Handle *handle = getHandle();
        glBindVertexArray(vao);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 512 * 4 * structureLength * sizeof(float), vertices, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 512 * 6 * sizeof(unsigned), indices, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glEnable(GL_BLEND);
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);

        shader->use();
        shader->setMat4("mvp", camera);
        if (shader == &image) {
            image.setTex("atlas", atlas, 0);
        }
        
        glDrawElements(GL_TRIANGLES, iCount, GL_UNSIGNED_INT, 0);
        glDisable(GL_BLEND);

        iCount = 0;
        vCount = 0;
    }

    Handle *begin(int w, int h, bool flip = true) {
        if (flip) {
            return begin(glm::mat4(1.0), glm::ortho(0.0f, (float)w, (float)h, 0.0f, 0.001f, 1.0f), w, h);
        }
        return begin(glm::mat4(1.0), glm::ortho(0.0f, (float)w, 0.0f, (float)h, 0.001f, 1.0f), w, h);
    }

    Handle *begin(glm::mat4 view, glm::mat4 proj, int w, int h) {
        width = w;
        height = h;
        iHandle = 0;
        iCount = 0;
        vCount = 0;
        this->view = view;
        this->proj = proj;
        this->camera = proj * view;
        glEnable(GL_SCISSOR_TEST);
        glScissor(0, 0, w, h);
        Handle *handle = getHandle();
        handle->use(nullptr, iHandle, 0, 0, w, h);
        return handle;
    }

    void end() {
        while (iHandle > 0) getHandle()->pop();
        draw();
        glDisable(GL_SCISSOR_TEST);
    }

    void onInput(float x, float y, bool down) {
        /// @todo иерархия кликов с съедание события
        /// @upd нормальная реализация
        prev = mouse;
        mouse = glm::vec2(x, y);
        delta = mouse - prev;
        wasLmb = lmb;
        lmb = down;
        wasDragged = drag;
        drag = down && (drag || glm::distance(mouse, prev) > 2.0f);
        event = false;
        over = false;
    }

    bool down() {
        return lmb;
    }

    bool clicked() {
        return !wasLmb && lmb;
    }

    bool released() {
        return wasLmb && !lmb;
    }
};

} // namespace engine::ui

#endif