#ifndef SCRIPT_H
#define SCRIPT_H

#include "entity/Entity.h"
#include "Pipeline.h"

namespace engine {

/// @todo или подписка на события
class Script {
public:
    virtual void input(Pipeline *pipeline) {};
    virtual void update(Pipeline *pipeline) {};
    virtual void fixedUpdate(Pipeline *pipeline) {};
    virtual void enable() {};
    virtual void dsiable() {};
};

} // namespace engine::script

#endif