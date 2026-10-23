#ifndef SKIP_LIST_SKIP_LIST_HPP
#define SKIP_LIST_SKIP_LIST_HPP

#include <cstdint>
#include <vector>
#include <random>
#include <utility>

namespace skip_list {

    // Key
    using key_t = uint64_t;
    inline constexpr key_t SENTINEL_KEY{std::numeric_limits<key_t>::min()};

    // Height
    using height_t = u_int8_t;
    // NOTE: Under ideal conditions, height = log(n) <= log(2 ** 64) = 64
    inline constexpr height_t MAX_HEIGHT_DEFAULT{64};

    // Probability
    using prob_t = double;
    inline constexpr prob_t PROB_MIN{0};
    inline constexpr prob_t PROB_MAX{1};
    inline constexpr prob_t PROB_DEFAULT{0.5};

    // Random Number Generator
    using rng_t = std::mt19937;

    // Seed
    using seed_t = u_int64_t;
    inline constexpr seed_t SEED_DEFAULT{17};

    // Metric
    using metric_t = uint64_t;

    // Skip List Node
    struct Node {
        key_t key;
        height_t height;
        std::vector<Node*> next;

        // Constructor
        Node(const key_t key, const height_t height) : key{key}, height{height}, next(height, nullptr) {}
    };

    // Skip List
    class SkipList {
    public:
        // Instrumentation Metrics
        static inline bool instrument_active{false};
        static inline metric_t op_comparisons{0};
        static inline metric_t op_traversals{0};

        // Ordered Union
        static SkipList ordered_union(SkipList&& A, SkipList&& B);

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
        void seed(const seed_t seed) const {
            rng.seed(seed);
        }

        // Check if empty
        bool is_empty() const {
            return sentinel->next[0] == nullptr;
        }

        // Get head
        const Node* get_head() const {
            return sentinel->next[0];
        }

        // Get height
        height_t get_height() const {
            // Traverse Sentinel downwards
            for (height_t curr_height{max_height}; curr_height > 0;) {
                // Adjust current height
                --curr_height;

                // Check if there exists a Node at the current height level
                if (sentinel->next[curr_height] != nullptr) {
                    return curr_height;
                }
            }

            return 0;
        }

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

        // Check invariants
        bool check_invariants() const;

    private:
        Node* sentinel;
        height_t max_height;
        prob_t p;
        mutable rng_t rng;

        // Generate random height
        height_t random_height() const {
            height_t height{1};
            std::uniform_real_distribution dist{PROB_MIN, PROB_MAX};

            // Keep sampling from Uniform Distribution until Failure or Max Height is reached
            while (dist(rng) < p && height < max_height) {
                height++;
            }

            return height;
        }
    };

} // skip_list

#endif //SKIP_LIST_SKIP_LIST_HPP
