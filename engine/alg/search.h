#ifndef SEARCH_H
#define SEARCH_H

#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <cmath>
#include <vector>
#include <list>

namespace engine::algorithm {

template<typename T>
float euclideanDistance(T a, T b) {
    float dx = a->x - b->x;
    float dy = a->y - b->y;
    float dz = a->z - b->z;
    return std::powf(dx * dx + dy * dy + dz * dz, 0.5);
}

} // namespace engine::algorithm

namespace engine::algorithm::astar {

struct Node3D {
    int label;
    float x, y, z;

    Node3D(int label, float x, float y, float z): label(label), x(x), y(y), z(z) {

    }
};

template<typename N>
class AStar {
protected:
    using H = float(*)(N *a, N *b);
    H heuristic;

    std::vector<N> nodes;
    std::unordered_multimap<N*, std::pair<N*, float>> connections;

public:
    AStar(H heuristic): heuristic(heuristic) {

    }

    const std::vector<N> *getNodes() const {
        return &nodes;
    }

    int node(int label, float x, float y, float z) {
        nodes.emplace_back(label, x, y, z);
        return nodes.size() - 1;
    }

    void connection(int parent, int child, float weight) {
        connections.emplace(&nodes[parent], std::make_pair(&nodes[child], weight));
    }

    template<typename T, typename ... A>
    void connect(int parent, T item, A ... least) {
        connection(parent, item.first, item.second);
        if constexpr (sizeof...(least) > 0) {
            connect(parent, least...);
        }
    }

    std::vector<N*> operator()(int s, int e) {
        N *start = &nodes[s];
        N *end = &nodes[e];
        std::unordered_set<N*> opened;
        std::unordered_set<N*> visited;
        std::unordered_map<N*, N*> parents;
        std::unordered_map<N*, float> distances;

        opened.emplace(start);
        distances[start] = 0;

        while (!opened.empty()) {
            N *node = *std::min_element(opened.begin(), opened.end(), [this, end, &distances](N* a, N* b) {
                return distances[a] + this->heuristic(a, end) > distances[a] + this->heuristic(b, end);
            });
            float weight = distances[node];
            auto range = connections.equal_range(node);
            for (auto it = range.first; it != range.second; ++it) {
                auto [n, w] = it->second;
                if (std::find(visited.begin(), visited.end(), n) != visited.end()) {
                    if (weight + w < distances[n]) {
                        distances[n] = weight + w;
                        visited.erase(std::find(visited.begin(), visited.end(), n));
                        opened.emplace(n);
                        parents[n] = node;
                    }
                } else if (std::find(opened.begin(), opened.end(), n) != opened.end()) {
                    if (weight + w < distances[n]) {
                        distances[n] = weight + w;
                        parents[n] = node;
                    }
                } else {
                    opened.emplace(n);
                    distances[n] = weight + w;
                    parents[n] = node;
                }
            }
            opened.erase(std::find(opened.begin(), opened.end(), node));
            visited.emplace(node);
        }

        std::vector<N*> path;
        path.push_back(end);
        N *node = end;
        while (node != start) {
            auto it = parents.find(node);
            if (it == parents.end()) return {};
            node = it->second;
            path.emplace_back(node);
        }
        std::reverse(path.begin(), path.end());
        return path;
    }
};

}; // namespace engine::algorithm::astar

#endif