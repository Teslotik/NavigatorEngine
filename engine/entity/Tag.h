#ifndef TAG_H
#define TAG_H

#include <list>
#include <vector>
#include <unordered_map>
#include "Resource.h"
#include "Entity.h"
#include "Script.h"
#include "interface/IHierarchy.h"
#include "tween.h"
#include "Pipeline.h"
#include "interface/IProcessable.h"

namespace engine::entity {

class Tag : public Entity, public IWeakHierarchy<Tag> {
public:
    Tag *parent = nullptr;
    std::vector<Entity*> children;    /// @note can also act like a resources holder
    Script *script = nullptr;
    tween::Tween tween;

    // Tag();
    void input(Pipeline *pipeline);
    void update(Pipeline *pipeline);
    void fixedUpdate(Pipeline *pipeline);
};

} // namespace engine::entity

#endif