#ifndef IHIERARCHY_H
#define IHIERARCHY_H

#include <unordered_map>
#include <functional>
#include "interface/IListenable.h"
#include "utils.inl"

namespace engine {

/// @brief Collection of hierarchy algorithms
template<typename T>
struct IHierarchyBase {
    std::unordered_map<int, std::pair<std::function<bool(IListenable*)>, std::function<bool(IListenable*)>>> events;

    template<typename F>
    IHierarchyBase *apply(F func) {
        T *base = static_cast<T*>(this);
        for (auto child: base->children) {
            func(child);
        }
        return this;
    }

    template<typename F>
    IHierarchyBase *forEach(F pre = nullptr, F post = nullptr) {
        T *base = static_cast<T*>(this);
        if (pre) pre(base);
        for (auto child: base->children) {
            child->forEach(pre, post);
        }
        if (post) post(base);
        return this;
    }

    void subscribe(const char *label, std::function<bool(IListenable*)> callback) {
        events.emplace(utils::hash(label), std::pair{callback, nullptr});
    }

    void subscribe(const char *label, std::function<bool(IListenable*)> onForward, std::function<bool(IListenable*)> onBackward) {
        events.emplace(utils::hash(label), std::pair{onForward, onBackward});
    }

    void dispatch(int label, IListenable *event) {
        T *base = static_cast<T*>(this);
        auto it = events.find(label);
        if (it == events.end()) {
            for (auto child: base->children) child->dispatch(label, event);
        } else {
            auto [onForward, onBackward] = it->second;
            if (onForward != nullptr) onForward(event);
            if (onBackward == nullptr) {
                for (auto child: base->children) {
                    child->dispatch(label, event);
                }
            } else {
                for (auto child: base->children) {
                    child->dispatch(label, event);
                    onBackward(event);
                }
            }
        }
    }

    void dispatch(const char *label, IListenable *event) {
        dispatch(utils::hash(label), event);
    }

    void bubble(int label, IListenable *event, bool self) {
        T *base = static_cast<T*>(this);
        if (!self) {
            if (base->parent) base->parent->bubble(label, event, true);
            return;
        }
        auto it = events.find(label);
        if (it == events.end()) {
            if (base->parent) base->parent->bubble(label, event, true);
        } else {
            auto [onForward, onBackward] = it->second;
            if (onForward != nullptr) {
                if (onForward(event)) {
                    if (base->parent) base->parent->bubble(label, event, true);
                }
            } else {
                if (base->parent) base->parent->bubble(label, event, true);
            }
            if (onBackward != nullptr) onBackward(event);
        }
    }

    void bubble(const char *label, IListenable *event, bool self) {
        bubble(utils::hash(label), event, self);
    }
};

template<typename T>
struct IStrongHierarchy: IHierarchyBase<T> {
    template<typename C>
    void addChild(C *child, int pos = -1) {
        T *base = static_cast<T*>(this);
        if (auto it = std::find(base->children.begin(), base->children.end(), child); it != base->children.end()) {
            base->children.erase(it);
        }
        child->parent = base;
        base->children.insert(std::next(base->children.begin(), pos < 0 ? base->children.size() - pos - 1 : pos), child);
        child->onParent();
    }

    template<typename C>
    void removeChild(C *child) {
        T *base = static_cast<T*>(this);
        if (auto it = std::find(base->children.begin(), base->children.end(), child); it != base->children.end()) {
            base->children.erase(it);
            child->parent = nullptr;
            child->onRemove();
        }
    }
};

template<typename T>
struct IWeakHierarchy: IHierarchyBase<T> {
    template<typename C>
    void addChild(C *child, int pos = -1) {
        T *base = static_cast<T*>(this);
        if (auto it = std::find(base->children.begin(), base->children.end(), child); it != base->children.end()) {
            base->children.erase(it);
        }
        base->children.insert(std::next(base->children.begin(), pos < 0 ? base->children.size() - pos - 1 : pos), child);
        child->onParent();
    }

    template<typename C>
    void removeChild(C *child) {
        T *base = static_cast<T*>(this);
        if (auto it = std::find(base->children.begin(), base->children.end(), child); it != base->children.end()) {
            base->children.erase(it);
            child->onRemove();
        }
    }
};

} // namespace engine


#endif