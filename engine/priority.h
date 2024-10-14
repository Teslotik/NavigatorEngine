#ifndef PRIORITY_H
#define PRIORITY_H

namespace engine::enums::Priority {

enum {
    Lowest = 0,
    Low = 25,
    Normal = 50,
    High = 75,
    Highest = 100
};


namespace ProcessPriority {
    enum {
        Gui = 0,
        Gizmo = 25,
        Compositor = 50,
        Render = 75,
        Animation = 100,
        Input = 125
    };
}

} // namespace engine::enums::Priority


#endif