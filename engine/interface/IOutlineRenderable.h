#ifndef IOUTLINE_REDNDERABLE_H
#define IOUTLINE_REDNDERABLE_H

#include "IListenable.h"

namespace engine {

struct IOutlineRenderable: virtual IListenable {
    virtual void renderInOutline() = 0;
};

} // namespace engine

#endif