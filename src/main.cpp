#include "graph.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <string>
#include <vector>

using namespace std::chrono;

static constexpr int GRAPHS = 100;  // independent graphs generated per density
static constexpr int RUNS   = 10;   // times each algorithm is run per graph

// ── helpers ───────────────────────────────────────────────────────────────────

static void print_usage(const char* prog) {
    std::cerr
        << "Usage: " << prog << " <nodes> <density> [min_weight] [max_weight] [--csv]\n"
        << "  nodes      : number of graph vertices (>= 1)\n"
        << "  density    : fraction of edges present [0.0-1.0]\n"
        << "  min_weight : minimum edge weight (default 1)\n"
        << "  max_weight : maximum edge weight (default 100)\n"
        << "  --csv      : emit a single CSV line instead of human output\n"
        << "               format: nodes,density,edges,kruskal_us,prim_us,boruvka_us\n"
        << "\nExample: " << prog << " 1000 0.4\n";
}

// Time a single call of fn, return microseconds.
template<typename Fn>
static double time_once(Fn fn) {
    auto t0 = steady_clock::now();
    fn();
    auto t1 = steady_clock::now();
    return duration_cast<nanoseconds>(t1 - t0).count() / 1'000.0;
}

// Run fn RUNS times on the same graph, return average microseconds.
template<typename Fn>
static double time_avg(Fn fn) {
    double total = 0.0;
    for (int r = 0; r < RUNS; r++)
        total += time_once(fn);
    return total / RUNS;
}

// ── main ──────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    if (argc < 3) { print_usage(argv[0]); return 1; }

    int    nodes      = std::stoi(argv[1]);
    double density    = std::stod(argv[2]);
    int    min_weight = (argc >= 4 && std::string(argv[3]) != "--csv") ? std::stoi(argv[3]) : 1;
    int    max_weight = (argc >= 5 && std::string(argv[4]) != "--csv") ? std::stoi(argv[4]) : 100;

    bool csv_mode = false;
    for (int i = 1; i < argc; i++)
        if (std::string(argv[i]) == "--csv") csv_mode = true;

    if (nodes <= 0 || density < 0.0 || density > 1.0 ||
        min_weight <= 0 || max_weight < min_weight) {
        print_usage(argv[0]);
        return 1;
    }

    if (!csv_mode)
        std::cerr << "Generating " << GRAPHS << " graphs, "
                  << RUNS << " runs each (nodes=" << nodes
                  << ", density=" << density << ")...\n";

    // Outer loop: GRAPHS independent graphs
    // Inner loop (inside time_avg): RUNS runs of each algorithm on that graph
    // Final result: mean of the GRAPHS per-graph averages
    double total_k = 0, total_p = 0, total_b = 0;
    long long total_edges = 0;

    for (int g = 0; g < GRAPHS; g++) {
        Graph G = create_graph(nodes, density, min_weight, max_weight);
        total_edges += G.edge_count / 2;

        total_k += time_avg([&]{ kruskal_mst(G); });
        total_p += time_avg([&]{ prim_mst(G);    });
        total_b += time_avg([&]{ boruvka_mst(G); });

        if (!csv_mode)
            std::cerr << "  graph " << (g + 1) << "/" << GRAPHS << "\r" << std::flush;
    }
    if (!csv_mode) std::cerr << "\n";

    double avg_k     = total_k / GRAPHS;
    double avg_p     = total_p / GRAPHS;
    double avg_b     = total_b / GRAPHS;
    double avg_edges = static_cast<double>(total_edges) / GRAPHS;

    long long max_edges   = static_cast<long long>(nodes) * (nodes - 1) / 2;
    double actual_density = (max_edges > 0) ? avg_edges / max_edges : 0.0;

    // ── Output ────────────────────────────────────────────────────────────────
    if (csv_mode) {
        std::cout << std::fixed << std::setprecision(6)
                  << nodes          << ","
                  << actual_density << ","
                  << static_cast<long long>(avg_edges) << ","
                  << avg_k << ","
                  << avg_p << ","
                  << avg_b << "\n";
        return 0;
    }

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "\n=== Graph ===\n"
              << "  Nodes             : " << nodes                             << "\n"
              << "  Avg edges         : " << static_cast<long long>(avg_edges) << "\n"
              << "  Requested density : " << density                           << "\n"
              << "  Actual density    : " << actual_density                    << "\n"
              << "  Weight range      : [" << min_weight << ", " << max_weight << "]\n\n";

    std::cout << "=== MST Benchmarks ===\n"
              << "  (" << GRAPHS << " graphs x " << RUNS << " runs each = "
              << GRAPHS * RUNS << " total runs per algorithm)\n\n";
    std::cout << std::left
              << std::setw(12) << "Algorithm"
              << std::setw(20) << "Avg Time (us)"
              << "\n"
              << std::string(32, '-') << "\n";

    auto report = [&](const char* name, double us) {
        std::cout << std::setw(12) << name
                  << std::setprecision(2) << std::setw(20) << us << "\n";
    };

    report("Kruskal", avg_k);
    report("Prim",    avg_p);
    report("Boruvka", avg_b);
    std::cout << std::string(32, '-') << "\n";

    return 0;
}
