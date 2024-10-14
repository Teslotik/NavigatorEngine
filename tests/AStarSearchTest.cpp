#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "alg/search.h"

TEST(AStarTest, Nodes) {
    engine::algorithm::astar::AStar<engine::algorithm::astar::Node3D> astar(engine::algorithm::euclideanDistance);

    astar.node(0, 0, 0, 0);
    astar.node(0, 1, 0, 0);
    astar.node(1, 0, 0, 0);

    ASSERT_EQ(astar.getNodes()->size(), 3);
}

TEST(AStarTest, Connections) {
    engine::algorithm::astar::AStar<engine::algorithm::astar::Node3D> astar(engine::algorithm::euclideanDistance);

    astar.node(0, 0, 0, 0);
    astar.node(1, 1, 0, 0);
    astar.node(2, 2, 0, 0);
    astar.connection(0, 1, 1);
    astar.connection(1, 2, 1);

    ASSERT_EQ(astar(0, 2).size(), 3);
}

TEST(AStarTest, Search1) {
    engine::algorithm::astar::AStar<engine::algorithm::astar::Node3D> astar(engine::algorithm::euclideanDistance);

    astar.node(0, 0, 0, 0);
    astar.node(1, 1, 0, 0);
    astar.node(2, 1, 1, 0);
    astar.node(3, 2, 1, 0);
    astar.connection(0, 1, 1);
    astar.connection(0, 2, 1);
    astar.connection(2, 3, 1);
    astar.connection(0, 3, 5);
    auto r = astar(0, 3);

    ASSERT_EQ(r.size(), 3);
    ASSERT_EQ(r[0]->label, 0);
    ASSERT_EQ(r[1]->label, 2);
    ASSERT_EQ(r[2]->label, 3);
}

TEST(AStarTest, Search2) {
    engine::algorithm::astar::AStar<engine::algorithm::astar::Node3D> astar(engine::algorithm::euclideanDistance);

    astar.node(0, 0, 0, 0);
    astar.node(1, 1, 0, 0);
    astar.node(2, 1, 1, 0);
    astar.node(3, 2, 1, 0);
    astar.connection(0, 1, 1);
    astar.connection(0, 2, 1);
    astar.connection(2, 3, 1);
    astar.connection(0, 3, 1);
    auto r = astar(0, 3);

    ASSERT_EQ(r.size(), 2);
    ASSERT_EQ(r[0]->label, 0);
    ASSERT_EQ(r[1]->label, 3);
}