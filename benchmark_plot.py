#!/usr/bin/env python3
"""
benchmark_plot.py
Runs ./mst_benchmark across a range of densities and node counts,
collects the averaged timing data, and produces one plot per node count.

Usage:
    python3 benchmark_plot.py                          # defaults
    python3 benchmark_plot.py --nodes 500              # fixed node count
    python3 benchmark_plot.py --nodes 200 500 1000     # multiple node counts
    python3 benchmark_plot.py --densities 0.1 0.3 0.5 0.7 0.9
    python3 benchmark_plot.py --logscale               # log x-axis (good for exponential densities)
    python3 benchmark_plot.py --binary ./path/to/mst_benchmark
"""

import subprocess
import argparse
import sys
import os
from collections import defaultdict

try:
    import matplotlib.pyplot as plt
    import matplotlib.ticker as ticker
except ImportError:
    print("matplotlib not found. Install it with:  pip install matplotlib")
    sys.exit(1)

# ── defaults ──────────────────────────────────────────────────────────────────

DEFAULT_NODES     = [200, 500, 1000]
DEFAULT_DENSITIES = [0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9]

# Suggested densities for log-scale mode (exponentially spaced)
LOG_DENSITIES     = [0.005, 0.01, 0.02, 0.04, 0.08, 0.16, 0.32, 0.64]

BINARY            = "./mst_benchmark"

ALGO_STYLE = {
    "Kruskal": dict(color="#e05c2a", marker="o", linestyle="-"),
    "Prim":    dict(color="#2a7ae0", marker="s", linestyle="--"),
    "Boruvka": dict(color="#27a85f", marker="^", linestyle="-."),
}

# ── runner ────────────────────────────────────────────────────────────────────

def run_benchmark(binary, nodes, density):
    cmd = [binary, str(nodes), f"{density:.4f}", "--csv"]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
        line = result.stdout.strip()
        if not line:
            print(f"  [!] No output for nodes={nodes} density={density:.4f}", flush=True)
            print(f"      stderr: {result.stderr.strip()[:200]}")
            return None
        parts = line.split(",")
        if len(parts) != 6:
            print(f"  [!] Unexpected CSV format: {line}")
            return None
        _, actual_density, edges, kruskal, prim, boruvka = parts
        return float(actual_density), int(edges), float(kruskal), float(prim), float(boruvka)
    except subprocess.TimeoutExpired:
        print(f"  [!] Timeout for nodes={nodes} density={density:.4f}")
        return None
    except FileNotFoundError:
        print(f"[ERROR] Binary not found: {binary}")
        print("Build it first:  g++ -std=c++17 -O2 -Iinclude src/main.cpp src/graph_utils.cpp src/kruskal.cpp src/prim.cpp src/boruvka.cpp -o mst_benchmark")
        sys.exit(1)

# ── plotting ──────────────────────────────────────────────────────────────────

def plot_density_vs_time(data, nodes_list, out_dir, logscale=False):
    """Save one PNG per node count: x = density, y = avg time (µs)."""
    for nodes in nodes_list:
        rows = data[nodes]
        if not rows:
            print(f"  [!] No data for nodes={nodes}, skipping.")
            continue

        densities = [r[0] for r in rows]
        times = {
            "Kruskal": [r[2] for r in rows],
            "Prim":    [r[3] for r in rows],
            "Boruvka": [r[4] for r in rows],
        }

        fig, ax = plt.subplots(figsize=(8, 5))
        scale_label = " (log scale)" if logscale else ""
        fig.suptitle(f"MST Runtime vs Graph Density  ({nodes} nodes){scale_label}",
                     fontsize=13, fontweight="bold")

        for algo, ys in times.items():
            s = ALGO_STYLE[algo]
            ax.plot(densities, ys, label=algo,
                    color=s["color"], marker=s["marker"],
                    linestyle=s["linestyle"], linewidth=2, markersize=5)

        if logscale:
            ax.set_xscale("log")
            ax.set_yscale("log")
            # Show the actual decimal values at each data point as tick labels
            ax.set_xticks(densities)
            ax.xaxis.set_major_formatter(ticker.FuncFormatter(
                lambda x, _: f"{x:.4g}"   # e.g. 0.005, 0.01, 0.02 ...
            ))
            plt.setp(ax.get_xticklabels(), rotation=45, ha="right")
        else:
            # Plain decimal labels: 0.1, 0.2, ... (not percentages)
            ax.xaxis.set_major_formatter(ticker.FuncFormatter(
                lambda x, _: f"{x:.2g}"   # e.g. 0.1, 0.2, 0.5, 1
            ))

        ax.set_xlabel("Graph Density")
        ax.set_ylabel("Avg Time (µs)")
        ax.legend(fontsize=10)
        ax.grid(True, linestyle="--", alpha=0.4)

        plt.tight_layout()
        suffix = "_log" if logscale else ""
        path = os.path.join(out_dir, f"density_vs_time_{nodes}nodes{suffix}.png")
        plt.savefig(path, dpi=150)
        print(f"Saved: {path}")
        plt.close()

# ── main ──────────────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(description="MST benchmark plotter")
    parser.add_argument("--binary",    default=BINARY,
                        help="path to mst_benchmark binary")
    parser.add_argument("--nodes",     nargs="+", type=int, default=DEFAULT_NODES,
                        help="node counts to test (space-separated)")
    parser.add_argument("--densities", nargs="+", type=float, default=None,
                        help="densities to sweep [0.0-1.0] (default depends on --logscale)")
    parser.add_argument("--logscale",  action="store_true",
                        help="use log x-axis; defaults to exponential density spacing")
    parser.add_argument("--outdir",    default="results",
                        help="directory to save plots (default: results/)")
    args = parser.parse_args()

    # Pick default densities based on scale mode if not explicitly provided
    if args.densities is None:
        args.densities = LOG_DENSITIES if args.logscale else DEFAULT_DENSITIES

    os.makedirs(args.outdir, exist_ok=True)

    print(f"Binary   : {args.binary}")
    print(f"Nodes    : {args.nodes}")
    print(f"Densities: {[f'{d:.4g}' for d in args.densities]}")
    print(f"Log scale: {args.logscale}")
    print(f"Output   : {args.outdir}\n")

    # data[nodes] = list of (actual_density, edges, kruskal_us, prim_us, boruvka_us)
    data = defaultdict(list)

    total = len(args.nodes) * len(args.densities)
    done  = 0

    for nodes in args.nodes:
        for density in args.densities:
            done += 1
            print(f"[{done:3d}/{total}] nodes={nodes:5d}  density={density:.4g}  ... ",
                  end="", flush=True)
            row = run_benchmark(args.binary, nodes, density)
            if row:
                data[nodes].append(row)
                print(f"Kruskal={row[2]:8.1f}us  Prim={row[3]:8.1f}us  Boruvka={row[4]:8.1f}us")
            else:
                print("FAILED")

    print("\nGenerating plots...")
    plot_density_vs_time(data, args.nodes, args.outdir, logscale=args.logscale)

    print("\nDone! Plots saved to:", os.path.abspath(args.outdir))


if __name__ == "__main__":
    main()