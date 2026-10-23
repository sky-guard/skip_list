#include "skip_list.hpp"

#include <algorithm>
#include <unordered_set>

namespace skip_list {

    SkipList SkipList::ordered_union(SkipList&& A, SkipList&& B) {
        // Initialize the resulting Skip List from the Ordered Union
        const auto res_max_height{std::max(A.max_height, B.max_height)};
        SkipList res{res_max_height, A.p, 0};
        res.rng = A.rng;

        // Traverse the Skip List, finding A's last Node and stitch it to B's first Node at each height level
        auto* last_A{A.sentinel};
        for (auto curr_height{res_max_height}; curr_height > 0;) {
            // Adjust current height
            --curr_height;

            // Traverse to the end of A at this specific height level
            if (curr_height < A.max_height) {
                while (last_A->next[curr_height] != nullptr) {
                    // Update Traversals Metric
                    if (instrument_active)
                        ++op_traversals;

                    // Move to the next Node
                    last_A = last_A->next[curr_height];
                }
            }

            // Check if A is empty at this height level
            if (last_A == A.sentinel) {
                // Wire result's Sentinel to B's list directly
                res.sentinel->next[curr_height] = (curr_height < B.max_height) ? B.sentinel->next[curr_height] : nullptr;
            } else {
                // Wire A's tail to B's head
                last_A->next[curr_height] = (curr_height < B.max_height) ? B.sentinel->next[curr_height] : nullptr;

                // Wire result's Sentinel to the resulting list
                res.sentinel->next[curr_height] = A.sentinel->next[curr_height];
            }
        }

        // Reset Sentinels of the original Skip Lists
        std::ranges::fill(A.sentinel->next, nullptr);
        std::ranges::fill(B.sentinel->next, nullptr);

        return res;
    }

    SkipList::SkipList(const height_t max_height, const prob_t p, const seed_t seed) : max_height{max_height}, p{p}, rng{seed} {
        // Initialize Sentinel
        sentinel = new Node{SENTINEL_KEY, max_height};
    }

    SkipList::~SkipList() {
        // Clear
        clear();

        // Delete Sentinel
        delete sentinel;
    }

    SkipList::SkipList(SkipList&& other) noexcept : sentinel{other.sentinel}, max_height{other.max_height}, p{other.p}, rng{other.rng} {
        // Move values from the other Skip List by taking ownership of its Sentinel
        other.sentinel = new Node{SENTINEL_KEY, other.max_height};
    }

    SkipList& SkipList::operator=(SkipList&& other) noexcept {
        // Check if assigning to the same object, in which case do nothing
        if (this != &other) {
            // Clear
            clear();

            // Delete Sentinel
            delete sentinel;

            // Copy over parameters
            sentinel = other.sentinel;
            max_height = other.max_height;
            p = other.p;
            rng = other.rng;

            // Move values from the other Skip List by taking ownership of its Sentinel
            other.sentinel = new Node{SENTINEL_KEY, other.max_height};
        }

        return *this;
    }

    void SkipList::clear() {
        // Delete Nodes
        const auto* curr_node{sentinel->next[0]};
        const Node* next_node{nullptr};
        while (curr_node != nullptr) {
            next_node = curr_node->next[0];
            delete curr_node;
            curr_node = next_node;
        }

        // Reset Sentinel
        std::ranges::fill(sentinel->next, nullptr);
    }

    bool SkipList::search(const key_t key) const {
        // Traverse the Skip List, moving across and then down each height level
        const auto* curr_node{sentinel};
        for (auto curr_height{max_height}; curr_height > 0;) {
            // Adjust current height
            --curr_height;

            // Move across the current height level
            while (curr_node->next[curr_height] != nullptr) {
                // Update Comparisons Metric
                if (instrument_active)
                    ++op_comparisons;

                // Check if we found the Key
                if (curr_node->next[curr_height]->key == key) {
                    return true;
                }

                // Update Comparisons Metric
                if (instrument_active)
                    ++op_comparisons;

                // Check if the Key could still be found at this height level or if we need to move down
                if (curr_node->next[curr_height]->key < key) {
                    // Update Traversals Metric
                    if (instrument_active)
                        ++op_traversals;

                    // Keep moving across
                    curr_node = curr_node->next[curr_height];
                } else {
                    // Stop moving across and move down instead
                    break;
                }
            }
        }

        // Key not found
        return false;
    }

    bool SkipList::insert(const key_t key) {
        // Traverse the Skip List, moving across and then down each height level and storing the predecessor Nodes on the way
        auto* curr_node{sentinel};
        std::vector<Node*> pred_nodes(max_height, nullptr);
        for (auto curr_height{max_height}; curr_height > 0;) {
            // Adjust current height
            --curr_height;

            // Move across the current height level
            while (curr_node->next[curr_height] != nullptr) {
                // Update Comparisons Metric
                if (instrument_active)
                    ++op_comparisons;

                // Check if the Key already exists
                if (curr_node->next[curr_height]->key == key) {
                    // Insertion unsuccessful
                    return false;
                }

                // Update Comparisons Metric
                if (instrument_active)
                    ++op_comparisons;

                // Check if the Key could still be found at this height level or if we need to move down
                if (curr_node->next[curr_height]->key < key) {
                    // Update Traversals Metric
                    if (instrument_active)
                        ++op_traversals;

                    // Keep moving across
                    curr_node = curr_node->next[curr_height];
                } else {
                    // Stop moving across and move down instead
                    break;
                }
            }

            // Save the predecessor of the new Node for the current height level
            pred_nodes[curr_height] = curr_node;
        }

        // Create and insert the new Node
        const auto new_height{random_height()};
        auto* const new_node{new Node{key, new_height}};
        for (height_t curr_height{0}; curr_height < new_height; ++curr_height) {
            new_node->next[curr_height] = pred_nodes[curr_height]->next[curr_height];
            pred_nodes[curr_height]->next[curr_height] = new_node;
        }

        // Insertion successful
        return true;
    }

    bool SkipList::erase(const key_t key) {
        // Traverse the Skip List, moving across and then down each height level and storing the predecessor Nodes on the way
        auto* curr{sentinel};
        std::vector<Node*> pred_nodes(max_height, nullptr);
        for (auto curr_height{max_height}; curr_height > 0;) {
            // Adjust current height
            --curr_height;

            // Move across the current height level as long as the next Key is less than the Key to delete
            while (curr->next[curr_height] != nullptr && curr->next[curr_height]->key < key) {
                curr = curr->next[curr_height];
            }

            // Save the predecessor of the Node to delete for the current height level
            pred_nodes[curr_height] = curr;
        }

        // Check if Node was found
        curr = curr->next[0];
        if (curr == nullptr || curr->key != key) {
            // Deletion unsuccessful
            return false;
        }

        // Unlink Node
        for (height_t curr_height{0}; curr_height < curr->height; ++curr_height) {
            pred_nodes[curr_height]->next[curr_height] = curr->next[curr_height];
        }

        // Delete Node
        delete curr;

        // Deletion successful
        return true;
    }

    std::pair<SkipList, SkipList> SkipList::split(const key_t pivot) {
        // Initialize resulting Skip Lists from the Split
        SkipList L{max_height, p, 0};
        SkipList R{max_height, p, 0};
        L.rng = rng;
        R.rng = rng;

        // Traverse the Skip List, moving across and then down each height level and rewiring links
        auto* curr_node{sentinel};
        for (auto curr_height{max_height}; curr_height > 0;) {
            // Adjust current height
            --curr_height;

            // Update Comparisons Metric
            if (instrument_active)
                ++op_comparisons;

            // Move across the current height level as long as the next Key is less than or equal to the Pivot
            while (curr_node->next[curr_height] != nullptr && curr_node->next[curr_height]->key <= pivot) {
                // Update Traversals Metric
                if (instrument_active)
                    ++op_traversals;

                // Keep moving across
                curr_node = curr_node->next[curr_height];

                // Update Comparisons Metric
                if (instrument_active)
                    ++op_comparisons;
            }

            // Wire R's Sentinel at this height level
            R.sentinel->next[curr_height] = curr_node->next[curr_height];

            // Check if L will be empty at this height level
            if (curr_node == sentinel) {
                // Terminate L's list at this height level
                L.sentinel->next[curr_height] = nullptr;
            } else {
                // Wire L's Sentinel at this height level
                L.sentinel->next[curr_height] = sentinel->next[curr_height];

                // Terminate L's list at this height level
                curr_node->next[curr_height] = nullptr;
            }
        }

        // Reset Sentinel of the original Skip List
        std::ranges::fill(sentinel->next, nullptr);

        return {std::move(L), std::move(R)};
    }

    bool SkipList::check_invariants() const {
        // If Skip List is empty, Invariants are satisfied
        auto* curr_node{sentinel->next[0]};
        if (curr_node == nullptr)
            return true;

        // Check Invariants for the first height level
        std::unordered_set<Node*> visited;
        visited.insert(curr_node);
        auto prev_key = curr_node->key;
        curr_node = curr_node->next[0];
        while (curr_node != nullptr) {
            // Check if Cycle is detected
            if (visited.contains(curr_node))
                return false;

            // Check if not strictly increasing Keys
            if (curr_node->key <= prev_key)
                return false;

            // Collect Node
            visited.insert(curr_node);

            // Move to the next Node
            prev_key = curr_node->key;
            curr_node = curr_node->next[0];
        }

        // Check Invariants for the other height levels
        for (height_t curr_height{1}; curr_height < max_height; ++curr_height) {
            curr_node = sentinel->next[curr_height];
            while (curr_node != nullptr) {
                // Check if Node is part of the bottom height level list
                if (!visited.contains(curr_node))
                    return false;

                // Check if Node height is correct
                if (curr_node->height <= curr_height)
                    return false;

                // Check if not strictly increasing Keys
                if (curr_node->next[curr_height] != nullptr && curr_node->key >= curr_node->next[curr_height]->key) {
                    return false;
                }

                // Move to the next Node
                curr_node = curr_node->next[curr_height];
            }
        }

        // Invariants are satisfied
        return true;
    }

} // skip_list