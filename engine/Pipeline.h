#ifndef PIPELINE_H
#define PIPELINE_H

#include "RenderStack.h"
#include "surface.h"

namespace engine {

struct Pipeline {
    double deltaTime = 0;
    RenderStack<Surface> *renderStack = nullptr;
};

} // namespace engine

#endif