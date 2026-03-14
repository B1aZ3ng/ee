#include "graph.h"
#include <random>
#include <numeric>    // std::iota
#include <algorithm>  // std::shuffle, std::min, std::max
#include <set>
#include <stdexcept>

// ── internal helpers ──────────────────────────────────────────────────────────

// Add a single directed edge u → v.
static void add_directed_edge(Graph& G, int u, int v, int w) {
    Edge e;
    e.to     = v;
    e.weight = w;
    e.next   = G.heads[u];   // prepend to u's adjacency list
    G.heads[u] = static_cast<int>(G.edge_list.size());
    G.edge_list.push_back(e);
    G.edge_count++;
}

// Add an undirected edge as two directed edges.
static void add_undirected_edge(Graph& G, int u, int v, int w) {
    add_directed_edge(G, u, v, w);
    add_directed_edge(G, v, u, w);
}

// ── public API ────────────────────────────────────────────────────────────────

Graph create_graph(int nodes, double density, int min_weight, int max_weight) {
    if (nodes <= 0)
        throw std::invalid_argument("nodes must be > 0");
    if (density < 0.0 || density > 1.0)
        throw std::invalid_argument("density must be in [0.0, 1.0]");
    if (min_weight > max_weight)
        throw std::invalid_argument("min_weight must be <= max_weight");

    Graph G;
    G.node_count = nodes;
    G.edge_count = 0;
    G.heads.assign(nodes, -1);

    if (nodes == 1) return G;   // single node — no edges possible

    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> wdist(min_weight, max_weight);

    // Maximum undirected edges in a simple graph: n*(n-1)/2
    const long long max_edges = static_cast<long long>(nodes) * (nodes - 1) / 2;
    // Ensure the graph is at least a spanning tree; respect density ceiling.
    long long target = std::max(
        static_cast<long long>(nodes - 1),
        static_cast<long long>(density * max_edges + 0.5)
    );
    target = std::min(target, max_edges);

    // Track existing undirected edges as (min, max) pairs to avoid duplicates.
    std::set<std::pair<int,int>> existing;

    // ── Step 1: build a random spanning tree (guarantees connectivity) ─────────
    // Shuffle node order, then attach each new node to a random earlier node.
    std::vector<int> order(nodes);
    std::iota(order.begin(), order.end(), 0);
    std::shuffle(order.begin(), order.end(), rng);

    for (int i = 1; i < nodes; i++) {
        std::uniform_int_distribution<int> pick(0, i - 1);
        int u = order[i];
        int v = order[pick(rng)];
        int w = wdist(rng);
        add_undirected_edge(G, u, v, w);
        existing.insert({ std::min(u, v), std::max(u, v) });
    }

    // ── Step 2: add random edges until the target density is reached ──────────
    // To avoid an infinite loop when nearly all edges exist, we limit retries.
    const int max_retries = static_cast<int>(std::min(max_edges * 10LL, 1'000'000LL));
    int retries = 0;

    std::uniform_int_distribution<int> ndist(0, nodes - 1);

    while (static_cast<long long>(existing.size()) < target && retries < max_retries) {
        int u = ndist(rng);
        int v = ndist(rng);
        if (u == v) { retries++; continue; }

        auto key = std::make_pair(std::min(u, v), std::max(u, v));
        if (existing.count(key)) { retries++; continue; }

        add_undirected_edge(G, u, v, wdist(rng));
        existing.insert(key);
        retries = 0;   // reset on success
    }

    return G;
}
