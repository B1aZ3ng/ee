
"""
crossover.py
Finds the graph density at which Prim's runtime overtakes Kruskal's
for each node count from 100 to 1600 (step 100), using binary search.

Binary search stops when Kruskal and Prim are within THRESHOLD of each other
(default 1%), taking the midpoint as the crossover density.

Usage:
    python3 crossover.py
    python3 crossover.py --binary ./mst_benchmark
    python3 crossover.py --threshold 0.02   # 2% tolerance
    python3 crossover.py --outdir results
"""

import subprocess
import argparse
import sys
import os
import csv

try:
    import matplotlib.pyplot as plt
    import matplotlib.ticker as ticker
except ImportError:
    print("matplotlib not found.  pip install matplotlib")
    sys.exit(1)

# ── config ────────────────────────────────────────────────────────────────────

BINARY        = "./mst_benchmark"
NODE_COUNTS   = [100,200,300,400,500,600,700,800,900,1000,1100,1200]   # 100, 200, ..., 1600
THRESHOLD     = 0.005                           # stop when times within 0.5%
MAX_ITERS     = 300                             # max binary search iterations
MIN_DENSITY   = 0.0001
MAX_DENSITY   = 0.35

# ── runner ────────────────────────────────────────────────────────────────────

def run_benchmark(binary, nodes, density):
    """Returns (kruskal_us, prim_us) or None on failure."""
    cmd = [binary, str(nodes), f"{density:.6f}", "--csv"]
    result = subprocess.run(cmd, capture_output=True, text=True)
    line = result.stdout.strip()
    if not line:
        return None
    parts = line.split(",")
    if len(parts) != 6:
        return None
    _, _, _, kruskal, prim, _ = parts
    return float(kruskal), float(prim)


# ── binary search ─────────────────────────────────────────────────────────────

def find_crossover(binary, nodes, threshold):
    """
    Binary search for the density where prim_us ≈ kruskal_us.

    At low density  → Kruskal faster (kruskal < prim  → ratio < 1)
    At high density → Prim faster    (kruskal > prim  → ratio > 1)

    Returns (crossover_density, low, high, iters) or None if no crossover found.
    """

    def ratio(density):
        """kruskal / prim — >1 means Kruskal is slower (Prim has taken over)."""
        r = run_benchmark(binary, nodes, density)
        if r is None:
            return None
        k, p = r
        if p == 0:
            return None
        return k / p

    # ── verify crossover exists in [MIN_DENSITY, MAX_DENSITY] ────────────────
    r_low  = ratio(MIN_DENSITY)
    r_high = ratio(MAX_DENSITY)

    if r_low is None or r_high is None:
        return None

    # At low density Kruskal should be faster (ratio < 1).
    # If not, Prim is always faster — crossover is below MIN_DENSITY.
    if r_low >= 1.0:
        print(f"  [!] Prim already faster at min density {MIN_DENSITY} (ratio={r_low:.3f})")
        return None

    # At high density Prim should be faster (ratio > 1).
    if r_high <= 1.0:
        print(f"  [!] Kruskal still faster at max density {MAX_DENSITY} (ratio={r_high:.3f})")
        return None

    # ── binary search ─────────────────────────────────────────────────────────
    lo, hi = MIN_DENSITY, MAX_DENSITY
    iters  = 0

    while iters < MAX_ITERS:
        mid = (lo + hi) / 2.0
        r   = ratio(mid)
        if r is None:
            break

        # How close are the two algorithms?  |r - 1| / 1  = |r - 1|
        closeness = abs(r - 1.0)
        iters += 1

        if closeness <= threshold:
            # Within threshold — mid is our crossover estimate
            return mid, lo, hi, iters

        if r < 1.0:
            # Kruskal still faster → crossover is at higher density
            lo = mid
        else:
            # Prim has taken over → crossover is at lower density
            hi = mid

    # Ran out of iterations — return midpoint anyway
    return (lo + hi) / 2.0, lo, hi, iters

# ── plotting ──────────────────────────────────────────────────────────────────

def plot_crossover(results, out_dir):
    nodes_list   = [r[0] for r in results]
    crossovers   = [1/r[1] for r in results]
    low_bounds   = [1/r[2] for r in results]
    high_bounds  = [1/r[3] for r in results]

    fig, ax = plt.subplots(figsize=(10, 6))
    fig.suptitle("Density at which Prim overtakes Kruskal vs Node Count",
                 fontsize=14, fontweight="bold")

    # Shaded confidence band between lo/hi bounds
    ax.fill_between(nodes_list, low_bounds, high_bounds,
                    alpha=0.2, color="#2a7ae0", label="Search bounds")

    # Crossover line
    ax.plot(nodes_list, crossovers,
            color="#e05c2a", marker="o", linewidth=2.5,
            markersize=6, label="Crossover density")

    ax.set_xlabel("1 / Number of Nodes")
    ax.set_ylabel("Crossover Density")
    ax.yaxis.set_major_formatter(ticker.FuncFormatter(lambda x, _: f"{x:.3f}"))
    ax.legend(fontsize=10)
    ax.grid(True, linestyle="--", alpha=0.4)

    plt.tight_layout()
    path = os.path.join(out_dir, "prim_kruskal_crossover.png")
    plt.savefig(path, dpi=150)

    plt.close()

# ── main ──────────────────────────────────────────────────────────────────────

parser = argparse.ArgumentParser(description="Find Prim/Kruskal crossover density")
parser.add_argument("--binary",    default=BINARY)
parser.add_argument("--threshold", default=THRESHOLD, type=float,
                    help="stop when runtimes are within this fraction of each other (default 0.01 = 1%%)")
parser.add_argument("--outdir",    default="results")
args = parser.parse_args()

os.makedirs(args.outdir, exist_ok=True)



results = []   # (nodes, crossover, lo, hi, iters)

for nodes in NODE_COUNTS:
    print(nodes)
    found = find_crossover(args.binary, nodes, args.threshold)


    crossover, lo, hi, iters = found
    results.append((nodes, crossover, lo, hi))



    

# ── Save raw CSV ──────────────────────────────────────────────────────────
csv_path = os.path.join(args.outdir, "crossover_data.csv")
with open(csv_path, "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["nodes", "crossover_density", "low_bound", "high_bound"])
    for row in results:
        writer.writerow(row)


# ── Plot ──────────────────────────────────────────────────────────────────

plot_crossover(results, args.outdir)



