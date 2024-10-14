#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ds/Signal.h"

TEST(SignalTest, Subscribe1) {
    engine::ds::Signal<> signal;
    char mount = 'a';

    signal.subscribe([]() {});
    signal.subscribe([]() {});

    ASSERT_EQ(signal.size(), 2);
}

TEST(SignalTest, Subscribe2) {
    engine::ds::Signal<> signal;
    char mount = 'a';

    signal.subscribe(&mount, []() {});
    signal.subscribe(&mount, []() {});

    ASSERT_EQ(signal.size(), 2);
}

TEST(SignalTest, Mount) {
    engine::ds::Signal<> signal;
    char mount = 'a';

    signal.subscribe(&mount, []() {});
    signal.subscribe(&mount, []() {});
    signal.unsubscribe(&mount);

    ASSERT_EQ(signal.size(), 0);
}