#ifndef DS_SIGNAL_H
#define DS_SIGNAL_H

#include <vector>
#include <algorithm>
#include <utility>
#include <functional>
#include "priority.h"

namespace engine::ds {

using std::vector;
using std::pair;
using std::size_t;
using std::function;

template<typename ...T>
class Signal {
    struct Slot {
        int priority;
        function<void(T...)> callback;
        void *mount;

        Slot(void *mount, function<void(T...)> callback, int priority):
            mount(mount),
            callback(callback),
            priority(priority) {

        }
    };
    vector<Slot> slots;

public:
    Signal() = default;

    Signal(const Signal &other) {
        slots = other.slots;
    }

    Signal(const Signal &&other) {
        slots = std::move(other.slots);
    }

    Signal &operator=(const Signal &other) {
        slots = other.slots;
        return *this;
    }

    Signal &operator=(Signal &&other) {
        slots = std::move(other.slots);
        return *this;
    }

    Signal &operator=(std::initializer_list<Slot> ilist) {
        slots = ilist;
        return *this;
    }

    Slot &operator[](int priority) {
        return *find_if(slots.begin(), slots.end(), [priority](const Slot &slot) {
            return slot.priority == priority;
        });
    }

    Slot &operator[](function<void(T...)> callback) {
        return *find_if(slots.begin(), slots.end(), [callback](const Slot &slot) {
            return slot.callback == callback;
        });
    }

    Slot &operator[](void *mount) {
        return *find_if(slots.begin(), slots.end(), [mount](const Slot &slot) {
            return slot.mount == mount;
        });
    }

    void operator()(T... args) {
        emit(args...);
    }

    typename vector<Slot>::const_iterator begin()  {
        return slots.begin();
    }

    typename vector<Slot>::const_iterator end() {
        return slots.end();
    }

    bool empty() const {
        return slots.empty();
    }

    size_t size() const {
        return slots.size();
    }

    void clear() {
        slots.clear();
    }

    void subscribe(void *mount, int priority, function<void(T...)> callback) {
        slots.insert(std::find_if(slots.begin(), slots.end(), [priority](const Slot &slot) {
            return slot.priority < priority;
        }), Slot(mount, callback, priority));
    }

    void subscribe(void *mount, function<void(T...)> callback, int priority = engine::enums::Priority::Normal) {
        subscribe(mount, priority, callback);
    }

    void subscribe(int priority, function<void(T...)> callback) {
        slots.insert(std::find_if(slots.begin(), slots.end(), [priority](const Slot &slot) {
            return slot.priority < priority;
        }), Slot(nullptr, callback, priority));
    }

    void subscribe(function<void(T...)> callback) {
        slots.emplace_back(nullptr, callback, engine::enums::Priority::Normal);
    }

    void unsubscribe(int priority) {
        slots.erase(slots.begin(), remove_if(slots.begin(), slots.end(), [priority](const Slot &slot) -> bool {
            return slot.priority == priority;
        }));
    }

    /// @todo change, because euals operator doesn't exist for std::function
    // void unsubscribe(function<void(T...)> callback) {
    //     slots.erase(remove_if(slots.begin(), slots.end(), [callback](const Slot &slot) -> bool {
    //         return slot.callback == callback;
    //     }), slots.end());
    // }

    void unsubscribe(void *mount) {
        auto r = slots.erase(remove_if(slots.begin(), slots.end(), [mount](const Slot &slot) -> bool {
            return slot.mount == mount;
        }), slots.end());
    }

    void emit(T... args) {
        for (auto slot: slots) {
            slot.callback(args...);
        }
    }
};

} // namespace engine::ds

#endif