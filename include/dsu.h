#pragma once
#include <vector>
#include <numeric>   // std::iota

struct DSU {
    std::vector<int> parent;
    std::vector<int> rank_;

    explicit DSU(int n) : parent(n), rank_(n, 0) {
        std::iota(parent.begin(), parent.end(), 0);
    }

    // Path-compressed find
    int find(int x) {
        if (parent[x] != x)
            parent[x] = find(parent[x]);
        return parent[x];
    }

    // Union by rank — returns false if already in the same component
    bool unite(int x, int y) {
        x = find(x);
        y = find(y);
        if (x == y) return false;
        if (rank_[x] < rank_[y]) std::swap(x, y);
        parent[y] = x;
        if (rank_[x] == rank_[y]) rank_[x]++;
        return true;
    }

    bool same(int x, int y) { return find(x) == find(y); }
};
