# Skip List Implementation in C++

## Project Overview & Architecture

The following is a **C++** based implementation of the Randomized Data Structure known as **"Skip List"**. The following **Default Seed** is provided:
```c++
// Seed
using seed_t = u_int64_t;
inline constexpr seed_t SEED_DEFAULT{17};
```

The implementation makes use of **distinct signed 64-bit integer Keys**, with the following representation of the **Sentinel Key**:
```c++
// Key
using key_t = int64_t;
inline constexpr key_t SENTINEL_KEY{std::numeric_limits<key_t>::min()};
```

**Node Heights are independently sampled**, with a **Default Height Promotion Probability** of:
```c++
// Probability
using prob_t = double;
inline constexpr prob_t PROB_DEFAULT{0.5};
```

**Node Heights are capped**, with a **Default Maximum Height** of:
```c++
// Height
using height_t = u_int8_t;
// NOTE: Under ideal conditions, height = log(n) <= log(2 ** 64) = 64
inline constexpr height_t MAX_HEIGHT_DEFAULT{64};
```

In addition to having default values, **both the Seed, the Height Promotion Probability and the Maximum Height can be configured manually when initializing the Skip List**.

The following is the representation that has been chosen for the **Nodes**:
```c++
// Skip List Node
struct Node {
    key_t key;
    height_t height;
    std::vector<Node*> next;

    // ...
};
```

Lastly, here follows the representation that has instead been chosen for the **Skip List** itself, including its **Public Interface**:
```c++
// Skip List
class SkipList {
public:
    // ...
    
    // Ordered Union
    static SkipList ordered_union(SkipList&& A, SkipList&& B);

    // Rebuilding Ordered Union
    static SkipList rebuilding_ordered_union(SkipList&& A, SkipList&& B);

    // Constructor
    explicit SkipList(height_t max_height = MAX_HEIGHT_DEFAULT, prob_t p = PROB_DEFAULT, seed_t seed = SEED_DEFAULT);

    // Copy Constructor
    SkipList(const SkipList&) = delete;
    SkipList& operator=(const SkipList&) = delete;

    // Move Constructor
    SkipList(SkipList&& other) noexcept;
    SkipList& operator=(SkipList&& other) noexcept;

    // Destructor
    ~SkipList();

    // Seed
    void seed(const seed_t seed) const;

    // Get head
    const Node* get_head() const;

    // Check if empty
    bool is_empty() const;

    // Get height
    height_t get_height() const;

    // Clear
    void clear();

    // Search
    bool search(key_t key) const;

    // Insert
    bool insert(key_t key);

    // Erase
    bool erase(key_t key);

    // Split
    std::pair<SkipList, SkipList> split(key_t pivot);

    // Rebuilding Split
    std::pair<SkipList, SkipList> rebuilding_split(key_t pivot);

    // Check invariants
    bool check_invariants() const;

private:
    Node* sentinel;
    height_t max_height;
    prob_t p;
    mutable rng_t rng;
    
    // ...
} 
```

Optionally, **when the library is compiled in Debug Mode and the Instrumentation is enabled**, the following **Logical Work Metrics can be collected** during execution:
```c++
// Metric
using metric_t = uint64_t;

// Instrumentation Metrics
static inline bool instrument_active{false};
static inline metric_t op_comparisons{0};
static inline metric_t op_traversals{0};
static inline metric_t op_allocations{0};
```

This is achieved through the use of **Macros**, which turn the corresponding operations into NOPs, ensuring that **the Performance of the library isn't affected in Release Mode**.

## Dependencies & Requirements

The project is built using **CMake**, therefore the following Dependencies are required:
- **CMake 4.3** (or above)
- **C++20** (or above) and **relevant compiler** (gcc, clang, ...)

In order to set up the **Python Environment** required to plot the results, the following Dependencies are also required:
- **uv 0.13.4** (or above) 
- **Python 3.12** (or above)

Please note that **the relevant required C++ (benchmark) and Python (numpy, pandas, matplotlib) libraries will automatically be downloaded by the respective Build Systems**.

**Valgrind and other Sanitizers must be installed separately**, if one wishes to repeat that part of the Testing process, **but are not required to build and run the library**.

## Build & Run

In order to correctly build the Source Code and run the Code properly, please follow the following instructions.

Clone the Repository and set up your Working Directory:
```bash
git clone git@github.com:sky-guard/skip_list.git
cd skip_list
```

In order to run the Tests, generate a Debug Build Directory, build the following Target and then run it:
```bash
cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug --target skip_list_test -j $(nproc)
./cmake-build-debug/skip_list_test
```

In order to run the Benchmarks, generate a Release Build Directory, build the following Target and then run it:
```bash
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --target skip_list_benchmark -j $(nproc)
./cmake-build-release/skip_list_benchmark --benchmark_out=results.json --benchmark_out_format=json
```

In order to generate the Plots, synchronize your Virtual Environment to download all the necessary Dependencies, move over the necessary JSON Outputs, change your Working Directory and then run the following script:
```bash
uv sync
mv results.json src/plots
mv cmake-build-debug/logical_work.json src/plots
cd src/plots
uv run plot.py
```

## Operation Contracts & Ownership Rules

**All four of the Structural Operations (search, insert, ordered_union, split) have been implemented with an efficient O(log(n)) Time Complexity and O(1) Node Allocations**. These are indeed the targeted theoretical results, which have been achieved and put into practice.

This means: **no rebuilding, no reinsertion, no copying**. 

**All Node Addresses and Heights are preserved**.

In the case of **consuming Operations (ordered_union, split)**, **the consumed Inputs remain valid, empty and reusable: Outputs acquire ownership over their Nodes**.

**Each allocated Node is freed exactly once**, preventing **Memory Leaks**.

## Structural VS Rebuilding Operations

In addition to the Structural Operations, **a baseline Rebuilding version of the ordered_union and split Operations has also been provided and used during testing to verify the expected results**. This was done to confirm that indeed **the baseline Rebuilding versions of the previously mentioned Operations are much slower than the Structural ones, demonstrating a O(nlog(n)) Time Complexity and O(n) Node Allocations**.

The following Plot proves the previously made claims:

![Empirical Runtime Scaling vs N](plots/runtime_scaling.png)

Indeed, we can see a **clear Logarithmic trend for both the ordered_union (in red) and split (in green) Operations: they appear as a flat horizontal line in the log-log Plot**. 

**The search (in blue) and insert (in orange) Operations follow closely after: while their trend isn't a perfectly straight line like in the previous case, it is still very far from being Linear**. This minor discrepancy can probably be attributed to effect of **Caches**: **as these Operations rely on pointer chasing much more than the previous two, the increasing cost of Cache Misses (which naturally start to occur more and more often as the Input Size keeps growing and the entire Data Structure can no longer be fit into the faster L1 - L2 Caches, but rather has to be spilled to the slower L3 Cache) very likely begins to take over**.

**The Rebuilding ordered_union (in purple) and Rebuilding split (in brown) show a completely different trend: they shoot up with a straight line with unary coefficient, which in a log-log Plot is a clear indicator of a O(nlog(n)) Complexity**, thus proving the expected theoretical results.

## Logical Work Comparison

**In addition to the Runtime Scaling Tests, additional Tests focusing on Logical Work were also conducted**. 

The aim of these Tests was to **collect additional Instrumentation Metrics as a second form of proof of the Logarithmic Complexity of the Structural Operations, as opposed to the Linear or Log-Linear Complexity of the Rebuilding ones**. 

The following Instrumentation Metrics were collected:
- **Comparisons**
- **Traversals**
- **Allocations**

Let's analyze the plotted results:

![Logical Work Comparison](plots/logical_work_comparison.png)

**On the left, we can see the three Instrumentation Metrics plotted for both the Structural ordered_union (in green) and the Rebuilding ordered_union (in purple)**. **For the Structural version, we can see a very clear Logarithmic trend (horizontal line) for the Traversals, while both Comparisons and Allocations are zero**, as expected. On the other hand, **for the Rebuilding version, we can identify a clear Linear trend (straight line, unary coefficient) for the Allocations and a Log-Linear trend (straight line, coefficient > 1) for the Comparisons and Traversals, caused by the repeated Insertions**.

**On the right, we can instead see the three Instrumentation Metrics plotted for both the Structural split (in red) and the Rebuilding split (in brown)**. Following the same reasoning as before, **we can identify Logarithmic Comparisons and Traversals and zero Allocations for the Structural version**. On the other hand, **the Rebuilding version shows Linear Allocations and Log-Linear Comparisons and Traversals**, just as we expected.

## Correctness Verification & Testing

In order to ensure functional correctness, **extensive testing has been performed during the development process**. 

The Source Code indeed includes a **dedicated executable for running such Tests, which additionally also generates a JSON Output File for collecting the Instrumentation Metrics used to generate the previous Plots when ran in Debug Mode**. 

The provided executable contains and executes the following Tests:
- **Generic Operations Test**
- **Clear Method Test**
- **Boundary Keys Test**
- **Empty and Singleton Test**
- **Split Edge Cases Test**
- **Ordered Union Edge Cases Test**
- **Rebuilding Operations Test**
- **Reusability of Consumed Inputs Test**
- **Address and Height Preservation Test**
- **Randomized Seeds Test (1500+ Operations)**
- **Complexity Instrumentation Test and generate JSON Output (Debug Mode only)**

When needed, **the following Reference Container is used to compare the output of any given Operation**:
```c++
std::set<key_t> ref;
```

In order to facilitate Testing, **the Skip List Public Interface also includes the following method to check that all the Invariants are satisfied**:
```c++
// Check invariants
bool check_invariants() const;
```

The Invariants for the Skip List that are checked are the following:
- **Absence of Cycles**
- **Strictly increasing Keys**
- **All Nodes present at the bottom level**
- **Node Height Consistency**

Additionally, **the previously mentioned executable has also been run and checked against Valgrind, AddressSanitizer and UndefinedBehaviorSanitizer**. Thus, **confirming the absence of Memory Leaks, Memory Corruptions and Undefined Behavior**.

## Benchmarks & Performance

TODO

## Declaration of Individual Work 

All the work hereby presented has been **conducted independently and exclusively by myself, in compliance with the Academic Integrity Guidelines**.

**AI Assistance was used during the development of the project**, in particular to:
- **identify and resolve bugs**
- **generate boilerplate code for the Tests and Benchmarks**
- **generate the script for plotting the results**

Regardless of this, **all the Source Code has been verified, authored and extensively modified by myself during the development process as if it were my own**.