import json
import matplotlib.pyplot as plt
import numpy as np
import collections

# --- 1. GENERATE FIGURE 1: RUNTIME SCALING PLOT ---
with open('results.json', 'r') as f:
    bench_json = json.load(f)

bench_data = collections.defaultdict(lambda: collections.defaultdict(list))
for benchmark in bench_json['benchmarks']:
    name = benchmark['name']
    if '_mean' in name or '_median' in name:
        continue
    parts = name.split('/')
    if len(parts) >= 3:
        family = parts[0]
        try:
            n_val = int(parts[1])
            bench_data[family][n_val].append(benchmark['real_time'])
        except ValueError:
            continue

processed_bench = {}
for family, n_dict in bench_data.items():
    sorted_ns = sorted(n_dict.keys())
    avg_times = [np.mean(n_dict[n]) for n in sorted_ns]
    processed_bench[family] = (np.array(sorted_ns), np.array(avg_times))

plt.style.use('seaborn-v0_8-paper' if 'seaborn-v0_8-paper' in plt.style.available else 'default')

# Figure 1 Setup
fig1, ax1 = plt.subplots(figsize=(6, 4.5), dpi=300)

styles = {
    'BM_SkipListSearch': {'label': 'Search (Structural)', 'color': '#1f77b4', 'marker': 'o', 'ls': '-'},
    'BM_SkipListInsert': {'label': 'Insert (Structural)', 'color': '#ff7f0e', 'marker': 's', 'ls': '-'},
    'BM_SkipListOrderedUnion': {'label': 'Union (Structural)', 'color': '#2ca02c', 'marker': '^', 'ls': '--'},
    'BM_SkipListSplit': {'label': 'Split (Structural)', 'color': '#d62728', 'marker': 'd', 'ls': '--'},
    'BM_SkipListOrderedUnionRebuilding': {'label': 'Union (Rebuilding)', 'color': '#9467bd', 'marker': 'x', 'ls': ':'},
    'BM_SkipListSplitRebuilding': {'label': 'Split (Rebuilding)', 'color': '#8c564b', 'marker': '+', 'ls': ':'}
}

for family, (ns, times) in processed_bench.items():
    if family in styles:
        cfg = styles[family]
        ax1.plot(ns, times, marker=cfg['marker'], linestyle=cfg['ls'], color=cfg['color'], label=cfg['label'],
                 linewidth=1.5, markersize=4)

ax1.set_xscale('log', base=2)
ax1.set_yscale('log', base=10)
ax1.set_xlabel('Input Size $N$ (keys)', fontsize=10, fontweight='bold')
ax1.set_ylabel('Execution Time per Operation (ns)', fontsize=10, fontweight='bold')
ax1.set_title('Empirical Runtime Scaling vs. $N$', fontsize=11, fontweight='bold')
ax1.grid(True, which="both", ls="--", alpha=0.4)
ax1.legend(fontsize=7, loc='upper left', frameon=True)

fig1.tight_layout()
fig1.savefig('runtime_scaling.pdf', bbox_inches='tight')
fig1.savefig('runtime_scaling.png', bbox_inches='tight')
print("[+] Generated standalone 'runtime_scaling.pdf'")

# --- 2. GENERATE FIGURE 2: LOGICAL WORK COMPARISON (UNION vs SPLIT) ---
with open('logical_work.json', 'r') as f:
    lw = json.load(f)

ns = np.array(lw['N'])

fig2, (ax_union, ax_split) = plt.subplots(1, 2, figsize=(11, 4.5), dpi=300)

# Helper function to plot structural vs rebuilding metrics for an operation
def plot_op_metrics(ax, struct_data, rebuild_data, title, struct_color, rebuild_color):
    # Structural metrics (Solid = Comp, Dashed = Trav, Dotted = Alloc)
    ax.plot(ns, struct_data['comparisons'], marker='o', linestyle='-', color=struct_color, label='Struct: Comparisons')
    ax.plot(ns, struct_data['traversals'], marker='s', linestyle='--', color=struct_color, label='Struct: Traversals')
    ax.plot(ns, struct_data['allocations'], marker='^', linestyle=':', color=struct_color, label='Struct: Allocations')

    # Rebuilding metrics
    ax.plot(ns, rebuild_data['comparisons'], marker='x', linestyle='-', color=rebuild_color, alpha=0.8,
            label='Rebuild: Comparisons')
    ax.plot(ns, rebuild_data['traversals'], marker='+', linestyle='--', color=rebuild_color, alpha=0.8,
            label='Rebuild: Traversals')
    ax.plot(ns, rebuild_data['allocations'], marker='d', linestyle=':', color=rebuild_color, alpha=0.8,
            label='Rebuild: Allocations')

    ax.set_xscale('log', base=2)
    ax.set_yscale('symlog', base=10)  # symlog handles 0 and large linear numbers nicely
    ax.set_xlabel('Input Size $N$ (keys)', fontsize=10, fontweight='bold')
    ax.set_ylabel('Logical Work Count', fontsize=10, fontweight='bold')
    ax.set_title(title, fontsize=11, fontweight='bold')
    ax.grid(True, which="both", ls="--", alpha=0.4)
    ax.legend(fontsize=6.5, loc='upper left', frameon=True)


# Plot Union (Structural: #2ca02c, Rebuilding: #9467bd)
plot_op_metrics(ax_union, lw['ordered_union_structural'], lw['ordered_union_rebuilding'],
                '(a) Ordered Union: Structural vs. Rebuilding', '#2ca02c', '#9467bd')

# Plot Split (Structural: #d62728, Rebuilding: #8c564b)
plot_op_metrics(ax_split, lw['split_structural'], lw['split_rebuilding'],
                '(b) Split: Structural vs. Rebuilding', '#d62728', '#8c564b')

fig2.tight_layout()
fig2.savefig('logical_work_comparison.pdf', bbox_inches='tight')
fig2.savefig('logical_work_comparison.png', bbox_inches='tight')
print("[+] Generated standalone 'logical_work_comparison.pdf'")

# --- 3. GENERATE FIGURE 3: LOGICAL WORK COMPARISON (INSERT & SEARCH) ---
fig3, (ax_search, ax_insert) = plt.subplots(1, 2, figsize=(11, 4.5), dpi=300)

def plot_single_kernel_metrics(ax, struct_data, title, color):
    # Structural metrics for single kernels (Search / Insert)
    ax.plot(ns, struct_data['comparisons'], marker='o', linestyle='-', color=color, label='Comparisons')
    ax.plot(ns, struct_data['traversals'], marker='s', linestyle='--', color=color, label='Traversals')
    ax.plot(ns, struct_data['allocations'], marker='^', linestyle=':', color=color, label='Allocations')

    ax.set_xscale('log', base=2)
    ax.set_yscale('symlog', base=10)
    ax.set_xlabel('Input Size $N$ (keys)', fontsize=10, fontweight='bold')
    ax.set_ylabel('Logical Work Count', fontsize=10, fontweight='bold')
    ax.set_title(title, fontsize=11, fontweight='bold')
    ax.grid(True, which="both", ls="--", alpha=0.4)
    ax.legend(fontsize=6.5, loc='upper left', frameon=True)

# Plot Search (Color: #1f77b4) on the left
plot_single_kernel_metrics(ax_search, lw['search_structural'], '(a) Search: Structural', '#1f77b4')

# Plot Insert (Color: #ff7f0e) on the right
plot_single_kernel_metrics(ax_insert, lw['insert_structural'], '(b) Insert: Structural', '#ff7f0e')

fig3.tight_layout()
fig3.savefig('logical_work_search_insert.pdf', bbox_inches='tight')
fig3.savefig('logical_work_search_insert.png', bbox_inches='tight')
print("[+] Generated standalone 'logical_work_search_insert.pdf'")