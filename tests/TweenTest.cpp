#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "tween.h"

TEST(Tween, PropertyAnimationCallbacks) {
    engine::tween::PropertyAnimation animation;

    bool isStarted = false;
    bool isEnded = false;
    bool wasPlayed = false;
    animation.onStart = [&isStarted]() {
        isStarted = true;
    };
    animation.onEnd = [&isEnded]() {
        isEnded = true;
    };
    animation.onFrameChanged = [&wasPlayed](float progress) {
        wasPlayed = true;
    };
    animation.duration = 1.0f;
    animation.play();
    animation.update(100.0f);

    ASSERT_TRUE(isStarted);
    ASSERT_TRUE(isEnded);
    ASSERT_TRUE(wasPlayed);
}

TEST(Tween, PropertyAnimationPlay) {
    engine::tween::PropertyAnimation animation;

    float value = 0;
    animation.onFrameChanged = [&value](float progress) {
        value = progress;
    };
    animation.play();
    animation.update(100.0f);

    ASSERT_EQ(value, 1.0f);
}

TEST(Tween, PropertyAnimationStop) {
    engine::tween::PropertyAnimation animation;

    bool isEnded = false;
    animation.onEnd = [&isEnded]() {
        isEnded = true;
    };
    animation.onFrameChanged = [](float progress) {
        
    };
    animation.play();
    animation.stop();

    ASSERT_TRUE(isEnded);
    ASSERT_FALSE(animation.isPlaying);
}