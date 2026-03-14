#include "graph.h"
#include "dsu.h"
#include <vector>
#include <algorithm>

long long kruskal_mst(const Graph& G) {
    if (G.node_count == 0) return -1;

    // Collect unique undirected edges by iterating each node's adjacency list
    // and keeping only edges where u < v (each undirected edge appears twice).
    struct UEdge { int u, v, w; };
    std::vector<UEdge> edges;
    edges.reserve(G.edge_count / 2);

    for (int u = 0; u < G.node_count; u++) {
        for (int i = G.heads[u]; i != -1; i = G.edge_list[i].next) {
            int v = G.edge_list[i].to;
            if (u < v)
                edges.push_back({ u, v, G.edge_list[i].weight });
        }
    }

    // Sort edges by ascending weight (the Kruskal greedy step).
    std::sort(edges.begin(), edges.end(),
              [](const UEdge& a, const UEdge& b) { return a.w < b.w; });

    DSU dsu(G.node_count);
    long long total = 0;
    int added = 0;

    for (const auto& e : edges) {
        if (dsu.unite(e.u, e.v)) {
            total += e.w;
            if (++added == G.node_count - 1) break;  // MST is complete
        }
    }

    return total;
}
