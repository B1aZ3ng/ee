#ifndef GRAPH_H
#define GRAPH_H
#include <iostream>
#include <vector>

struct Edge {
    int next;  // index of next edge
    int to;    // destination
    int w;     // weight
};

struct Graph {
    int n;                  // number of nodes
    int m;                  // number of edges (undirected count)
    std::vector<int> head;  // adjacency list heads
    std::vector<Edge> edges;
};

// creates a random undirected weighted graph
Graph create_graph(int num_nodes, int num_edges);

#endif
