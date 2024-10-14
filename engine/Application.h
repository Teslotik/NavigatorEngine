#ifndef APPLICATION_H
#define APPLICATION_H

#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <glad/glad.h>
#include <functional>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Pipeline.h"
#include "RenderStack.h"
#include "surface.h"
#include "entity/Tag.h"
#include "ds/Signal.h"
#include "primitive.h"
#include "Loader.h"

namespace engine {

class Application {
public:
    std::string label;

    // input
    glm::vec2 mouse{0, 0};
    glm::vec2 mouseDelta{0, 0};
    bool mouseDown = false; /// @todo replace with input class
    double time = 0;
    double deltaTime = 0;
    unsigned w, h;
    unsigned displayWidth, displayHeight;

    // primitives
    GLFWmonitor *monitor;
    GLFWwindow *window;
    RenderStack<Surface> *renderer;
    Pipeline pipeline;
    Surface *screen;
    entity::Tag root;

    /// @todo move to the Context or to the Pipeline?
    /// @todo RenderStack?
    ds::Signal<Pipeline*> scriptablePipeline;

    // storage
    Loader appStorage;
    Loader userStorage;
    Loader persistentStorage;

    std::list<std::function<void()>> delayed;

    Application(std::string label);
    void run();
    
    void delay(std::function<void()> f) {
        delayed.push_back(f);
    }
};

} // namespace engine

#endif