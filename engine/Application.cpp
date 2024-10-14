#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Application.h"
#include "platform.h"

using namespace engine;
using engine::ds::Signal;

static double mouseX = 0;
static double mouseY = 0;
void mouseInput(GLFWwindow* window, double xpos, double ypos) {
    mouseX = xpos;
    mouseY = ypos;
}

bool mouseDown = false;
void mouseButton(GLFWwindow* window, int button, int action, int mods) {
    mouseDown = button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS;
}

static float WIDTH = 800;
static float HEIGHT = 600;

Application::Application(std::string label): label(label) {
    if (!glfwInit()) throw "[Error] Failed to initialize GLFW ";

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(800, 600, label.c_str(), nullptr, nullptr);
    WIDTH = 800;
    HEIGHT = 600;
    if (!window) {
        glfwTerminate();
        throw "[Error] Failed to create window";
    }

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int w, int h) {
        WIDTH = w;
        HEIGHT = h;
    });

    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouseInput);
    glfwSetMouseButtonCallback(window, mouseButton);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw "[Error] Failed to initialize GLAD";
    }

    monitor = glfwGetPrimaryMonitor();
    displayWidth = glfwGetVideoMode(monitor)->width;
    displayHeight = glfwGetVideoMode(monitor)->height;
    w = displayWidth;
    h = displayHeight;

#if defined(LINUX)
    appStorage = Loader("app");
    userStorage = Loader("user");
    persistentStorage = Loader(std::filesystem::path("/usr") / "share" / label);
#elif defined(WINDOWS)
    appStorage = Loader("app");
    userStorage = Loader("user");
    /// @note needs administration privileges?
    persistentStorage = Loader(std::filesystem::path("C:\\ProgramData") / label);
#endif

    renderer = new RenderStack<Surface>(displayWidth, displayHeight);
    pipeline = {
        .deltaTime = 0.0,
        .renderStack = renderer
    };
    screen = new ScreenSurface(displayWidth, displayHeight);
}

void Application::run() {
    while (!glfwWindowShouldClose(window)) {
        w = WIDTH;
        h = HEIGHT;
        mouseDelta = mouse - glm::vec2(mouseX, mouseY);
        mouse = glm::vec2(mouseX, mouseY);
        mouseDown = ::mouseDown;

        for (auto &item: delayed) item();
        delayed.clear();

        double time = glfwGetTime();
        deltaTime = time - this->time;
        this->time = time;

        pipeline.deltaTime = deltaTime;

        root.update(&pipeline);
        scriptablePipeline.emit(&pipeline);

        mouseDelta = glm::vec2(0, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
}