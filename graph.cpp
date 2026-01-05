#include <iostream>
#include <vector>
using namespace std;

struct node {
    int edg;  // next edge index
    int to;   // destination
    int wgt;  // weight
};

struct Graph {
    int n, m;
    vector<int> head;
    vector<node> edges;
};

Graph create_graph(int num_nodes, int num_edges) {
    Graph G;
    G.n = num_nodes;
    G.m = num_edges;
    G.head.assign(num_nodes, -1);
    G.edges.reserve(num_edges * 2);

    int cnt = 0;
    auto add_edge = [&](int u, int v, int w) {
        G.edges.push_back({G.head[u], v, w});
        G.head[u] = cnt++;
    };

    srand(time(nullptr));

    for (int i = 0; i < num_edges; i++) {
        int u = rand() % num_nodes;
        int v = rand() % num_nodes;
        while (v == u) v = rand() % num_nodes;

        int w = rand() % 100 + 1;

        add_edge(u, v, w);
        add_edge(v, u, w);  // undirected
    }

    return G;
}


