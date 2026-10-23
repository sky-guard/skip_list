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
                auto inv_1{sl.check_invariants()};
                assert(inv_1);

                // Perform Tests on duplicate Insertions
                for (key_t key{100}; key < 300; ++key) {
                    auto ins_dup{sl.insert(key)};
                    assert(ins_dup == false);
                }
                auto inv_2{sl.check_invariants()};
                assert(inv_2);
            }

            {
                // Perform Tests on Search (both present and absent Keys)
                for (key_t key{0}; key < 600; ++key) {
                    auto found{sl.search(key)};
                    auto ref_found{(ref.find(key) != ref.end())};
                    assert(found == ref_found);
                }
                auto inv_3{sl.check_invariants()};
                assert(inv_3);
            }

            {
                // Perform Tests on Erase
                for (key_t key{50}; key < 250; ++key) {
                    auto erased_sl{sl.erase(key)};
                    auto erased_ref{ref.erase(key) > 0};
                    assert(erased_sl == erased_ref);
                    bool inv_loop{sl.check_invariants()};
                    assert(inv_loop);
                }
                auto inv_4{sl.check_invariants()};
                assert(inv_4);

                // Perform Tests on Search after Erase
                for (key_t key{0}; key < 600; ++key) {
                    auto found{sl.search(key)};
                    auto ref_found{(ref.find(key) != ref.end())};
                    assert(found == ref_found);
                }
                auto inv_5{sl.check_invariants()};
                assert(inv_5);
            }
        }

        // Clear Method Tests
        {
            // Initialize Skip List
            SkipList sl;
            for (key_t key{0}; key < 100; ++key) {
                auto ins{sl.insert(key)};
                assert(ins);
            }

            // Perform Tests on Clear
            {
                auto empty_check{!sl.is_empty()};
                assert(empty_check);
                sl.clear();
                auto empty_check2{sl.is_empty()};
                assert(empty_check2);
                auto inv_clear{sl.check_invariants()};
                assert(inv_clear);
                auto search_50{sl.search(50)};
                assert(!search_50);
                auto ins_500{sl.insert(500)};
                assert(ins_500);
                auto search_500{sl.search(500)};
                assert(search_500);
                auto inv_clear2{sl.check_invariants()};
                assert(inv_clear2);
            }
        }

        // Empty and Singleton Edge Cases
        {
            {
                // Initialize Empty Skip List
                SkipList empty_list;

                // Perform Tests on Empty Skip List
                auto is_empty_res{empty_list.is_empty()};
                assert(is_empty_res);
                auto search_42{empty_list.search(42)};
                assert(!search_42);
                auto erase_42{empty_list.erase(42)};
                assert(!erase_42);
                auto inv_empty{empty_list.check_invariants()};
                assert(inv_empty);

            }

            {
                // Initialize Singleton Skip List
                SkipList singleton_list;
                auto ins_100{singleton_list.insert(100)};
                assert(ins_100);

                // Perform Tests on Singleton Skip List
                auto not_empty{!singleton_list.is_empty()};
                assert(not_empty);
                auto search_100{singleton_list.search(100)};
                assert(search_100);
                auto search_50{singleton_list.search(50)};
                assert(!search_50);
                auto inv_singleton{singleton_list.check_invariants()};
                assert(inv_singleton);
            }
        }

        // Split Edge Cases
        {
            // Initialize Skip List
            SkipList sl;
            for (key_t key{10}; key <= 50; key += 10) {
                auto ins{sl.insert(key)};
                assert(ins);
            }

            {
                // Perform Tests on Split before all keys (pivot < min)
                auto [L, R]{sl.split(5)};
                auto empty_L{L.is_empty()};
                assert(empty_L);
                auto not_empty_R{!R.is_empty()};
                assert(not_empty_R);
                auto search_10{R.search(10)};
                auto search_50{R.search(50)};
                assert(search_10 && search_50);
                auto inv_L{L.check_invariants()};
                auto inv_R{R.check_invariants()};
                assert(inv_L && inv_R);
            }

            // Rebuild Skip List for next split test
            for (key_t key{10}; key <= 50; key += 10) {
                auto ins{sl.insert(key)};
                assert(ins);
            }

            {
                // Perform Tests on Split after all keys (pivot > max)
                auto [L, R]{sl.split(100)};
                auto not_empty_L{!L.is_empty()};
                assert(not_empty_L);
                auto empty_R{R.is_empty()};
                assert(empty_R);
                auto search_10{L.search(10)};
                auto search_50{L.search(50)};
                assert(search_10 && search_50);
                auto inv_L{L.check_invariants()};
                auto inv_R{R.check_invariants()};
                assert(inv_L && inv_R);
            }

            // Rebuild Skip List for next split test
            for (key_t key{10}; key <= 50; key += 10) {
                auto ins{sl.insert(key)};
                assert(ins);
            }

            {
                // Perform Tests on Split exactly at a stored key
                auto [L, R]{sl.split(30)};
                auto search_10{L.search(10)};
                auto search_30{L.search(30)};
                assert(search_10 && search_30);
                auto search_40{L.search(40)};
                assert(!search_40);
                auto search_30_R{R.search(30)};
                assert(!search_30_R);
                auto search_40_R{R.search(40)};
                auto search_50_R{R.search(50)};
                assert(search_40_R && search_50_R);
                auto is_empty_sl{sl.is_empty()};
                assert(is_empty_sl);
                auto inv_L{L.check_invariants()};
                auto inv_R{R.check_invariants()};
                assert(inv_L && inv_R);
            }
        }

        // Ordered Union Edge Cases & Inverse Property
        {
            {
                // Initialize Empty Skip Lists and merge
                SkipList empty1, empty2;
                auto empty_union{SkipList::ordered_union(std::move(empty1), std::move(empty2))};

                // Perform Tests on Empty Union
                auto empty_union_chk{empty_union.is_empty()};
                assert(empty_union_chk);
                auto inv_empty_union{empty_union.check_invariants()};
                assert(inv_empty_union);
            }

            {
                // Initialize Unequal Disjoint Skip Lists and merge
                SkipList A, B;
                for (key_t key{1}; key <= 5; ++key) {
                    auto ins{A.insert(key)};
                    assert(ins);
                }
                for (key_t key{6}; key <= 1000; ++key) {
                    auto ins{B.insert(key)};
                    assert(ins);
                }
                auto union_unequal{SkipList::ordered_union(std::move(A), std::move(B))};

                // Perform Tests on Union
                auto inv_union{union_unequal.check_invariants()};
                assert(inv_union);
                auto empty_A{A.is_empty()};
                auto empty_B{B.is_empty()};
                assert(empty_A && empty_B);
                auto search_1{union_unequal.search(1)};
                auto search_500{union_unequal.search(500)};
                auto search_1000{union_unequal.search(1000)};
                assert(search_1 && search_500 && search_1000);
            }

            {
                // Initialize Skip List, split and merge
                SkipList original;
                for (key_t key{1}; key <= 100; ++key) {
                    auto ins{original.insert(key)};
                    assert(ins);
                }
                auto [L, R]{original.split(50)};
                auto restored{SkipList::ordered_union(std::move(L), std::move(R))};

                // Perform Tests on Inverse Property
                auto inv_restored{restored.check_invariants()};
                assert(inv_restored);
                for (key_t key{1}; key <= 100; ++key) {
                    auto search_key{restored.search(key)};
                    assert(search_key);
                }
            }

            {
                // Modify both split outputs independently and verify neither affects the other[cite: 2]
                SkipList original;
                for (key_t key{1}; key <= 20; ++key) {
                    auto ins{original.insert(key)};
                    assert(ins);
                }
                auto [L, R]{original.split(10)};
                auto ins_5{L.insert(5)};
                assert(!ins_5);
                auto erase_15{R.erase(15)};
                assert(erase_15);
                auto search_5{L.search(5)};
                assert(search_5);
                auto search_15{R.search(15)};
                assert(!search_15);
                auto inv_L{L.check_invariants()};
                auto inv_R{R.check_invariants()};
                assert(inv_L && inv_R);
            }
        }

        // Reuse of Consumed Inputs
        {
            // Initialize Skip Lists and consume them through Union
            SkipList X, Y;
            auto ins_X1{X.insert(10)};
            assert(ins_X1);
            auto ins_Y1{Y.insert(20)};
            assert(ins_Y1);
            auto combined{SkipList::ordered_union(std::move(X), std::move(Y))};

            // Perform Tests on Reusability
            {
                auto empty_X{X.is_empty()};
                assert(empty_X);
                auto empty_Y{Y.is_empty()};
                assert(empty_Y);
                auto ins_999{X.insert(999)};
                assert(ins_999);
                auto search_999{X.search(999)};
                assert(search_999);
                auto inv_X{X.check_invariants()};
                assert(inv_X);
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
                        auto search_sl{sl.search(val)};
                        auto search_ref{(ref.find(val) != ref.end())};
                        assert(search_sl == search_ref);
                    }

                    // Periodically check Invariants
                    if (step % 300 == 0) {
                        auto inv_step{sl.check_invariants()};
                        assert(inv_step);
                    }
                }

                // Final Invariants check
                auto inv_final{sl.check_invariants()};
                assert(inv_final);
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
                    auto ins_A{A.insert(i)};
                    assert(ins_A);
                    auto ins_B{B.insert(N + i)};
                    assert(ins_B);
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
                auto inv_res{res.check_invariants()};
                assert(inv_res);
            }

            // Split Scaling Test
            std::cout << "\n--- Split Operations Scaling ---\n";
            std::cout << "N\t\tComparisons\t\tTraversals\n";
            std::cout << "--------------------------------------------------\n";
            for (size_t N : {1000, 2000, 4000, 8000, 16000, 32000, 64000}) {
                // Initialize Skip List
                SkipList sl;
                for (size_t i{0}; i < N; ++i) {
                    auto ins{sl.insert(i * 2)};
                    assert(ins);
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
                auto inv_LR{L.check_invariants() && R.check_invariants()};
                assert(inv_LR);
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