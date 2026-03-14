#include "graph.h"
#include <vector>
#include <queue>
#include <climits>

long long prim_mst(const Graph& G) {
    if (G.node_count == 0) return -1;

    int n = G.node_count;
    std::vector<bool> in_mst(n, false);
    std::vector<int>  key(n, INT_MAX);   // cheapest edge weight into each node

    // Min-heap stores (edge_weight, node_index)
    using P = std::pair<int, int>;
    std::priority_queue<P, std::vector<P>, std::greater<P>> pq;

    // Start from node 0 with cost 0 (no edge needed to enter the first node)
    key[0] = 0;
    pq.push({ 0, 0 });

    long long total = 0;

    while (!pq.empty()) {
        auto [w, u] = pq.top();
        pq.pop();

        if (in_mst[u]) continue;   // already processed via a cheaper path
        in_mst[u] = true;
        total += w;

        // Relax neighbours
        for (int i = G.heads[u]; i != -1; i = G.edge_list[i].next) {
            int v  = G.edge_list[i].to;
            int ew = G.edge_list[i].weight;
            if (!in_mst[v] && ew < key[v]) {
                key[v] = ew;
                pq.push({ ew, v });
            }
        }
    }

    return total;
}
