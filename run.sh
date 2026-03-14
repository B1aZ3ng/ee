source .venv/bin/activate

# Customize node counts and densities
python3 benchmark_plot.py --densities 0.0005 0.001 0.002 0.003 0.004 0.005 0.006 0.007 0.008 0.009 0.01 0.012 0.014 0.016 0.018 0.02 --nodes 500 1000 5000 --outdir results/


