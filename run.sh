#!/bin/bash
source .venv/bin/activate

#node counts and densities
python3 benchmark_plot.py --nodes 50 100 500 1000 --outdir results/

python3 benchmark_plot.py --nodes 50 100 500 1000  --outdir log_results/ --logscale


