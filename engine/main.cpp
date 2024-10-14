#include "platform.h"

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Application.h"
#include "script/StreetMap.h"

using engine::Application;

/*
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
//   fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
//            ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
//             type, severity, message );
    if (type == GL_DEBUG_TYPE_ERROR)
    cout << message << endl;
}
*/

int main(int argc, char *argv[]) {
    cout << "Hello, world!" << endl;

    Application *app = new Application("StreetMap");

// #if defined(LINUX)
//     glEnable(GL_DEBUG_OUTPUT);
//     glDebugMessageCallback(MessageCallback, 0);
// #endif

    script::street::StreetMap script(app);
    script.enable();

    app->run();

    delete app;

    return 0;
}
