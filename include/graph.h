#pragma once
#include <vector>

struct Edge {
    int next;    // index of next edge from same source node (-1 = end)
    int to;      // destination node
    int weight;  // edge weight
};

struct Graph {
    int node_count;
    int edge_count;              // directed edges stored (each undirected edge = 2)
    std::vector<int>  heads;     // heads[u] = index of first edge from u (-1 = none)
    std::vector<Edge> edge_list; // all directed edges
};

Graph create_graph(int nodes, double density,
                   int min_weight = 1, int max_weight = 100);

long long kruskal_mst (const Graph& G);
long long prim_mst    (const Graph& G);
long long boruvka_mst (const Graph& G);