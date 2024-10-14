#ifndef TWEEN_H
#define TWEEN_H

#include <functional>
#include <algorithm>
#include <numbers>
#include <list>
#include "Easing.h"

namespace engine::tween {

struct Animation {
    bool isFreezed = false;
    bool isPlaying = false;
    bool isLooped = false;
    float duration = 10.0;
    float speed = 1.0;
    float offset = 0.0;
    float elapsed = 0.0;
    enums::Easing::Easing easing = enums::Easing::EaseInOutLinear;

    // Callbacks
    std::function<void(float)> onFrameChanged = nullptr;
    std::function<void()> onStart = nullptr;
    std::function<void()> onEnd = nullptr;

    float getProgress() {
        return enums::Easing::ease(easing, elapsed / duration);
    }

    virtual bool setFrame(float frame) = 0;
    virtual void setProgress(float progress) = 0;
    virtual void play() = 0;
    virtual void stop(bool end = true) = 0;
    virtual void update(float deltaTime) = 0;
};

// template<typename T>
class PropertyAnimation : public Animation {
public:
    // T *data;

    // PropertyAnimation(T *data): data(data) {

    // }

    virtual bool setFrame(float frame) override {
        elapsed = frame;
        if (isLooped) elapsed = std::max(std::fmod(elapsed, duration), offset);
        if (elapsed > duration) return false;
        if (onFrameChanged) onFrameChanged(getProgress());
        return true;
    }

    virtual void setProgress(float progress) override {
        setFrame(duration * progress);
    }

    virtual void play() override {
        stop();
        isPlaying = true;
        if (onStart) onStart();
        elapsed = offset;
    }

    virtual void stop(bool end = true) override {
        if (!isPlaying) return;
        isPlaying = false;
        if (end && onEnd) onEnd();
    }

    virtual void update(float deltaTime) override {
        if (!isPlaying || isFreezed) return;
        if (!setFrame(elapsed + deltaTime * speed)) {
            elapsed = duration;
            if (onFrameChanged) onFrameChanged(getProgress());
            stop();
        }
    }
};

class SequenceAnimation : public Animation {
public:
    std::list<Animation*> children;
    Animation *active = nullptr;

    virtual bool setFrame(float frame) override {
        elapsed = frame;
        if (isLooped) {
            for (auto child: children) child->elapsed = offset;
            elapsed = std::max(std::fmod(elapsed, duration), offset);
        }
        if (elapsed > duration) return false;
        if (onFrameChanged) onFrameChanged(getProgress());

        float total = 0.0;
        for (auto child: children) {
            active = child;
            if (total + child->duration < elapsed) {
                total += child->duration;
                child->elapsed = child->duration;
            } else {
                break;
            }
        }
        active->setFrame(elapsed - total);

        return true;
    }

    virtual void setProgress(float progress) override {
        setFrame(duration * progress);
    }

    virtual void play() override {
        stop();
        updateDuration();
        isPlaying = true;
        if (onStart) onStart();
        
        for (auto child: children) child->elapsed = child->offset;

        if (children.empty()) return;
        
        active = children.front();

        elapsed = offset;
    }

    virtual void update(float deltaTime) override {
        if (!isPlaying || isFreezed || !active) return;
        if (!setFrame(elapsed + deltaTime * speed * active->speed)) {
            elapsed = duration;
            active->setFrame(active->duration);
            if (onFrameChanged) onFrameChanged(getProgress());
            stop();
        }
    }

    virtual void stop(bool end = true) override {
        if (!isPlaying) return;
        isPlaying = false;
        if (active) active->stop();
        active = nullptr;
        if (end && onEnd) onEnd();
    }

    void updateDuration() {
        duration = 0;
        for (auto child: children) duration += child->duration;
    }
};


class Tween {
public:
    std::list<Animation*> children;

    void update(float deltaTime) {
        for (auto child: children) child->update(deltaTime);
    }
};

} // namespace engine::tween

#endif