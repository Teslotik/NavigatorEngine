#ifndef IGAME_REDNDERABLE_H
#define IGAME_REDNDERABLE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ds/Signal.h"
#include <RenderStack.h>
#include <surface.h>
#include "IListenable.h"

namespace engine {

struct IGameRenderable: virtual IListenable {
    glm::mat4 transform{1.0};
    glm::mat4 world{1.0};

    ds::Signal<> onRenderInGame;
    virtual void beginRenderInGame(RenderStack<Surface> *renderer) = 0;
    virtual void endRenderInGame(RenderStack<Surface> *renderer) = 0;
};

} // namespace engine

#endif