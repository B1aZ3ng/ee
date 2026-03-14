#include "graph.h"
#include "dsu.h"
#include <vector>
#include <climits>

long long boruvka_mst(const Graph& G) {
    if (G.node_count == 0) return -1;

    int n = G.node_count;
    DSU  dsu(n);
    long long total      = 0;
    int  components      = n;

    // Each round we halve the number of components (at worst),
    // so we need at most O(log n) rounds.
    while (components > 1) {

        // cheapest_w[r]  = weight of the cheapest outgoing edge from component r
        // cheapest_src/dst[r] = endpoints of that edge
        std::vector<int> cheapest_w(n, INT_MAX);
        std::vector<int> cheapest_u(n, -1);
        std::vector<int> cheapest_v(n, -1);

        // Scan every directed edge; for each component find its cheapest exit.
        for (int u = 0; u < n; u++) {
            int pu = dsu.find(u);
            for (int i = G.heads[u]; i != -1; i = G.edge_list[i].next) {
                int v  = G.edge_list[i].to;
                int w  = G.edge_list[i].weight;
                int pv = dsu.find(v);

                if (pu != pv && w < cheapest_w[pu]) {
                    cheapest_w[pu] = w;
                    cheapest_u[pu] = u;
                    cheapest_v[pu] = v;
                }
            }
        }

        // Merge components along the cheapest edges found.
        bool any_merged = false;
        for (int r = 0; r < n; r++) {
            if (cheapest_u[r] == -1) continue;   // component has no exit (isolated? shouldn't happen)

            int u = cheapest_u[r];
            int v = cheapest_v[r];
            int w = cheapest_w[r];

            if (dsu.unite(u, v)) {
                total += w;
                components--;
                any_merged = true;
            }
        }

        if (!any_merged) break;   // graph is disconnected — stop early
    }

    return total;
}
