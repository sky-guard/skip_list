#include "skip_list.hpp"

#include <iostream>
#include <set>
#include <cassert>
#include <algorithm>

namespace skip_list {

    static void run_tests() {
        std::cout << "[+] Running Skip List Tests...\n";

        // Generic Tests for Insertion, Search, Erase and Duplicates
        {
            // Initialize Skip List and Set for reference
            SkipList sl;
            std::set<key_t> ref;

            {
                // Perform Tests on non-duplicate Insertions
                for (key_t key{0}; key < 500; ++key) {
                    auto inserted_sl{sl.insert(key)};
                    auto inserted_ref{ref.insert(key).second};
                    assert(inserted_sl == inserted_ref);
                }
                assert(sl.check_invariants());

                // Perform Tests on duplicate Insertions
                for (key_t key{100}; key < 300; ++key) {
                    assert(sl.insert(key) == false);
                }
                assert(sl.check_invariants());
            }

            {
                // Perform Tests on Search (both present and absent Keys)
                for (key_t key{0}; key < 600; ++key) {
                    assert(sl.search(key) == (ref.find(key) != ref.end()));
                }
                assert(sl.check_invariants());
            }

            {
                // Perform Tests on Erase
                for (key_t key{50}; key < 250; ++key) {
                    auto erased_sl{sl.erase(key)};
                    auto erased_ref{ref.erase(key) > 0};
                    assert(erased_sl == erased_ref);
                    assert(sl.check_invariants());
                }
                assert(sl.check_invariants());

                // Perform Tests on Search after Erase
                for (key_t key{0}; key < 600; ++key) {
                    assert(sl.search(key) == (ref.find(key) != ref.end()));
                }
                assert(sl.check_invariants());
            }
        }

        // Clear Method Tests
        {
            // Initialize Skip List
            SkipList sl;
            for (key_t key{0}; key < 100; ++key) {
                assert(sl.insert(key));
            }

            // Perform Tests on Clear
            {
                assert(!sl.is_empty());
                sl.clear();
                assert(sl.is_empty());
                assert(sl.check_invariants());
                assert(!sl.search(50));
                assert(sl.insert(500));
                assert(sl.search(500));
                assert(sl.check_invariants());
            }
        }

        // Empty and Singleton Edge Cases
        {
            {
                // Initialize Empty Skip List
                SkipList empty_list;

                // Perform Tests on Empty Skip List
                assert(empty_list.is_empty());
                assert(!empty_list.search(42));
                assert(!empty_list.erase(42));
                assert(empty_list.check_invariants());

            }

            {
                // Initialize Singleton Skip List
                SkipList singleton_list;
                assert(singleton_list.insert(100));

                // Perform Tests on Singleton Skip List
                assert(!singleton_list.is_empty());
                assert(singleton_list.search(100));
                assert(!singleton_list.search(50));
                assert(singleton_list.check_invariants());
            }
        }

        // Split Edge Cases
        {
            // Initialize Skip List
            SkipList sl;
            for (key_t key{10}; key <= 50; key += 10)
                assert(sl.insert(key));

            {
                // Perform Tests on Split before all keys (pivot < min)
                auto [L, R]{sl.split(5)};
                assert(L.is_empty());
                assert(!R.is_empty());
                assert(R.search(10) && R.search(50));
                assert(L.check_invariants() && R.check_invariants());
            }

            // Rebuild Skip List for next split test
            for (key_t key{10}; key <= 50; key += 10)
                assert(sl.insert(key));

            {
                // Perform Tests on Split after all keys (pivot > max)
                auto [L, R]{sl.split(100)};
                assert(!L.is_empty());
                assert(R.is_empty());
                assert(L.search(10) && L.search(50));
                assert(L.check_invariants() && R.check_invariants());
            }

            // Rebuild Skip List for next split test
            for (key_t key{10}; key <= 50; key += 10)
                assert(sl.insert(key));

            {
                // Perform Tests on Split exactly at a stored key
                auto [L, R]{sl.split(30)};
                assert(L.search(10) && L.search(30));
                assert(!L.search(40));
                assert(!R.search(30));
                assert(R.search(40) && R.search(50));
                assert(sl.is_empty());
                assert(L.check_invariants() && R.check_invariants());
            }
        }

        // Ordered Union Edge Cases & Inverse Property
        {
            {
                // Initialize Empty Skip Lists and merge
                SkipList empty1, empty2;
                SkipList empty_union{SkipList::ordered_union(std::move(empty1), std::move(empty2))};

                // Perform Tests on Empty Union
                assert(empty_union.is_empty());
                assert(empty_union.check_invariants());
            }

            {
                // Initialize Unequal Disjoint Skip Lists and merge
                SkipList A, B;
                for (key_t key{1}; key <= 5; ++key)
                    assert(A.insert(key));
                for (key_t key{6}; key <= 1000; ++key)
                    assert(B.insert(key));
                SkipList union_unequal{SkipList::ordered_union(std::move(A), std::move(B))};

                // Perform Tests on Union
                assert(union_unequal.check_invariants());
                assert(A.is_empty() && B.is_empty());
                assert(union_unequal.search(1) && union_unequal.search(500) && union_unequal.search(1000));
            }

            {
                // Initialize Skip List, split and merge
                SkipList original;
                for (key_t key{1}; key <= 100; ++key)
                    assert(original.insert(key));
                auto [L, R]{original.split(50)};
                SkipList restored{SkipList::ordered_union(std::move(L), std::move(R))};

                // Perform Tests on Inverse Property
                assert(restored.check_invariants());
                for (key_t key{1}; key <= 100; ++key) {
                    assert(restored.search(key));
                }
            }

            {
                // Modify both split outputs independently and verify neither affects the other[cite: 2]
                SkipList original;
                for (key_t key{1}; key <= 20; ++key)
                    assert(original.insert(key));
                auto [L, R]{original.split(10)};
                assert(!L.insert(5));
                assert(R.erase(15));
                assert(L.search(5));
                assert(!R.search(15));
                assert(L.check_invariants() && R.check_invariants());
            }
        }

        // Reuse of Consumed Inputs
        {
            // Initialize Skip Lists and consume them through Union
            SkipList X, Y;
            assert(X.insert(10));
            assert(Y.insert(20));
            SkipList combined{SkipList::ordered_union(std::move(X), std::move(Y))};

            // Perform Tests on Reusability
            {
                assert(X.is_empty());
                assert(Y.is_empty());
                assert(X.insert(999));
                assert(X.search(999));
                assert(X.check_invariants());
            }
        }

        // Randomized Testing Across Multiple Seeds
        {
            // Test different seeds
            for (seed_t seed : {17, 32, 1337}) {
                SkipList sl{MAX_HEIGHT_DEFAULT, PROB_DEFAULT, seed};
                std::set<key_t> ref;

                // Run multiple mixed Operations
                for (size_t step{0}; step < 1500; ++step) {
                    // Generate pseudo-random Key and Operation Type
                    const key_t val{step * 37 % 2000};
                    if (const auto op_type{step % 5}; op_type == 0 || op_type == 1) { // Insert (40%)
                        auto res_sl{sl.insert(val)};
                        auto res_ref{ref.insert(val).second};
                        assert(res_sl == res_ref);
                    } else if (op_type == 2) { // Erase (20%)
                        auto res_sl{sl.erase(val)};
                        auto res_ref{ref.erase(val) > 0};
                        assert(res_sl == res_ref);
                    } else { // Search (40%)
                        assert(sl.search(val) == (ref.find(val) != ref.end()));
                    }

                    // Periodically check Invariants
                    if (step % 300 == 0) {
                        assert(sl.check_invariants());
                    }
                }

                // Final Invariants check
                assert(sl.check_invariants());
            }
        }

        // Complexity Instrumentation Tests for Ordered Union & Split
        {
            // Ordered Union Scaling Test
            std::cout << "\n--- Ordered Union Operations Scaling ---\n";
            std::cout << "N\t\tComparisons\t\tTraversals\n";
            std::cout << "--------------------------------------------------\n";
            for (size_t N : {1000, 2000, 4000, 8000, 16000, 32000, 64000}) {
                // Initialize Skip Lists with equal sizes
                SkipList A;
                SkipList B;
                for (size_t i{0}; i < N / 2; ++i) {
                    assert(A.insert(i));
                    assert(B.insert(N + i));
                }

                // Enable Instrumentation
                SkipList::op_comparisons = 0;
                SkipList::op_traversals = 0;
                SkipList::instrument_active = true;

                // Perform Ordered Union
                auto res{SkipList::ordered_union(std::move(A), std::move(B))};

                // Disable Instrumentation
                SkipList::instrument_active = false;

                // Print Instrumentation results
                std::cout << N << "\t\t" << SkipList::op_comparisons << "\t\t" << SkipList::op_traversals << "\n";
                assert(res.check_invariants());
            }

            // Split Scaling Test
            std::cout << "\n--- Split Operations Scaling ---\n";
            std::cout << "N\t\tComparisons\t\tTraversals\n";
            std::cout << "--------------------------------------------------\n";
            for (size_t N : {1000, 2000, 4000, 8000, 16000, 32000, 64000}) {
                // Initialize Skip List
                SkipList sl;
                for (size_t i{0}; i < N; ++i) {
                    assert(sl.insert(i * 2));
                }

                // Enable Instrumentation
                SkipList::op_comparisons = 0;
                SkipList::op_traversals = 0;
                SkipList::instrument_active = true;

                // Perform Split
                auto [L, R]{sl.split(N)};

                // Disable Instrumentation
                SkipList::instrument_active = false;

                // Print Instrumentation results
                std::cout << N << "\t\t" << SkipList::op_comparisons << "\t\t" << SkipList::op_traversals << "\n";
                assert(L.check_invariants() && R.check_invariants());
            }
        }

        std::cout << "[+] Skip List Tests completed successfully!\n";
    }

} // skip_list

// Test entry point
int main() {
    // Run Tests
    skip_list::run_tests();

    return 0;
}
