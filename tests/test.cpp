#include "skip_list.hpp"

#include <iostream>
#include <set>
#include <cassert>
#include <algorithm>
#include <fstream>
#include <unordered_map>

namespace skip_list {

    // Generic Operations Test
    static void test_generic_operations() {
        std::cout << "[+] Running Generic Operations Test...\n";

        // Initialize Skip List and Set for reference
        SkipList sl;
        std::set<key_t> ref;

        // Perform Tests on duplicate and non-duplicate Insertions
        {
            // Perform Tests on non-duplicate Insertions
            for (key_t key{0}; key < 500; ++key) {
                const auto inserted_sl{sl.insert(key)};
                const auto inserted_ref{ref.insert(key).second};
                assert(inserted_sl == inserted_ref);
            }
            const auto inv_1{sl.check_invariants()};
            assert(inv_1);

            // Perform Tests on duplicate Insertions
            for (key_t key{100}; key < 300; ++key) {
                const auto ins_dup{sl.insert(key)};
                assert(ins_dup == false);
            }
            const auto inv_2{sl.check_invariants()};
            assert(inv_2);
        }

        // Perform Tests on Search (both present and absent Keys)
        {
            for (key_t key{0}; key < 600; ++key) {
                const auto found{sl.search(key)};
                const auto ref_found{(ref.contains(key))};
                assert(found == ref_found);
            }
            const auto inv_3{sl.check_invariants()};
            assert(inv_3);
        }

        // Perform Tests on Erase and Search after Erase
        {
            // Perform Tests on Erase
            for (key_t key{50}; key < 250; ++key) {
                const auto erased_sl{sl.erase(key)};
                const auto erased_ref{ref.erase(key) > 0};
                assert(erased_sl == erased_ref);
                const bool inv_loop{sl.check_invariants()};
                assert(inv_loop);
            }
            const auto inv_4{sl.check_invariants()};
            assert(inv_4);

            // Perform Tests on Search after Erase
            for (key_t key{0}; key < 600; ++key) {
                const auto found{sl.search(key)};
                const auto ref_found{(ref.contains(key))};
                assert(found == ref_found);
            }
            const auto inv_5{sl.check_invariants()};
            assert(inv_5);
        }

        std::cout << "[+] Generic Operations Test completed successfully!\n";
    }

    // Clear Method Test
    static void test_clear_method() {
        std::cout << "[+] Running Clear Method Test...\n";

        // Initialize Skip List
        SkipList sl;
        for (key_t key{0}; key < 100; ++key) {
            const auto ins{sl.insert(key)};
            assert(ins);
        }

        // Perform Tests on Clear
        {
            const auto empty_check{!sl.is_empty()};
            assert(empty_check);
            sl.clear();
            const auto empty_check2{sl.is_empty()};
            assert(empty_check2);
            const auto inv_clear{sl.check_invariants()};
            assert(inv_clear);
            const auto search_50{sl.search(50)};
            assert(!search_50);
            const auto ins_500{sl.insert(500)};
            assert(ins_500);
            const auto search_500{sl.search(500)};
            assert(search_500);
            const auto inv_clear2{sl.check_invariants()};
            assert(inv_clear2);
        }

        std::cout << "[+] Clear Method Test completed successfully!\n";
    }

    // Boundary Keys Test
    static void test_boundary_keys() {
        std::cout << "[+] Running Boundary Keys Test...\n";

        // Initialize Skip List
        SkipList sl;
        constexpr auto min_key{std::numeric_limits<key_t>::min()};
        constexpr auto max_key{std::numeric_limits<key_t>::max()};
        constexpr auto neg_key{-42000};
        assert(sl.insert(min_key));
        assert(sl.insert(max_key));
        assert(sl.insert(neg_key));

        // Perform Tests on non-duplicate Insertions
        {
            assert(sl.search(min_key));
            assert(sl.search(max_key));
            assert(sl.search(neg_key));
            assert(sl.check_invariants());
        }

        std::cout << "[+] Boundary Keys Test completed successfully!\n";
    }

    // Empty and Singleton Test
    static void test_empty_and_singleton() {
        std::cout << "[+] Running Empty and Singleton Test...\n";

        // Perform Tests on Empty Skip List
        {
            // Initialize Empty Skip List
            SkipList empty_list;

            // Perform Tests on Empty Skip List
            const auto is_empty_res{empty_list.is_empty()};
            assert(is_empty_res);
            const auto search_42{empty_list.search(42)};
            assert(!search_42);
            const auto erase_42{empty_list.erase(42)};
            assert(!erase_42);
            const auto inv_empty{empty_list.check_invariants()};
            assert(inv_empty);
        }

        // Perform Tests on Singleton Skip List
        {
            // Initialize Singleton Skip List
            SkipList singleton_list;
            const auto ins_100{singleton_list.insert(100)};
            assert(ins_100);

            // Perform Tests on Singleton Skip List
            const auto not_empty{!singleton_list.is_empty()};
            assert(not_empty);
            const auto search_100{singleton_list.search(100)};
            assert(search_100);
            const auto search_50{singleton_list.search(50)};
            assert(!search_50);
            const auto inv_singleton{singleton_list.check_invariants()};
            assert(inv_singleton);
        }

        std::cout << "[+] Empty and Singleton Test completed successfully!\n";
    }

    // Split Edge Cases Test
    static void test_split_edge_cases() {
        std::cout << "[+] Running Split Edge Cases Test...\n";

        // Initialize Skip List
        SkipList sl;
        for (key_t key{10}; key <= 50; key += 10) {
            const auto ins{sl.insert(key)};
            assert(ins);
        }

        // Perform Tests on Split before all keys (pivot < min)
        {
            const auto [L, R]{sl.split(5)};

            const auto empty_L{L.is_empty()};
            assert(empty_L);
            const auto not_empty_R{!R.is_empty()};
            assert(not_empty_R);
            const auto search_10{R.search(10)};
            const auto search_50{R.search(50)};
            assert(search_10 && search_50);
            const auto inv_L{L.check_invariants()};
            const auto inv_R{R.check_invariants()};
            assert(inv_L && inv_R);
        }

        // Rebuild Skip List for next split test
        for (key_t key{10}; key <= 50; key += 10) {
            const auto ins{sl.insert(key)};
            assert(ins);
        }

        // Perform Tests on Split after all keys (pivot > max)
        {
            const auto [L, R]{sl.split(100)};

            const auto not_empty_L{!L.is_empty()};
            assert(not_empty_L);
            const auto empty_R{R.is_empty()};
            assert(empty_R);
            const auto search_10{L.search(10)};
            const auto search_50{L.search(50)};
            assert(search_10 && search_50);
            const auto inv_L{L.check_invariants()};
            const auto inv_R{R.check_invariants()};
            assert(inv_L && inv_R);
        }

        // Rebuild Skip List for next split test
        for (key_t key{10}; key <= 50; key += 10) {
            const auto ins{sl.insert(key)};
            assert(ins);
        }

        // Perform Tests on Split exactly at a stored key
        {
            const auto [L, R]{sl.split(30)};

            const auto search_10{L.search(10)};
            const auto search_30{L.search(30)};
            assert(search_10 && search_30);
            const auto search_40{L.search(40)};
            assert(!search_40);
            const auto search_30_R{R.search(30)};
            assert(!search_30_R);
            const auto search_40_R{R.search(40)};
            const auto search_50_R{R.search(50)};
            assert(search_40_R && search_50_R);
            const auto is_empty_sl{sl.is_empty()};
            assert(is_empty_sl);
            const auto inv_L{L.check_invariants()};
            const auto inv_R{R.check_invariants()};
            assert(inv_L && inv_R);
        }

        std::cout << "[+] Split Edge Cases Test completed successfully!\n";
    }

    // Ordered Union Edge Cases Test
    static void test_ordered_union_edge_cases() {
        std::cout << "[+] Running Ordered Union Edge Cases Test...\n";

        // Perform Tests on Empty Union
        {
            // Initialize Empty Skip Lists and merge
            SkipList empty1, empty2;
            const auto empty_union{SkipList::ordered_union(std::move(empty1), std::move(empty2))};

            // Perform Tests on Empty Union
            const auto empty_union_chk{empty_union.is_empty()};
            assert(empty_union_chk);
            const auto inv_empty_union{empty_union.check_invariants()};
            assert(inv_empty_union);
        }

        // Perform Tests on Union
        {
            // Initialize Unequal Disjoint Skip Lists and merge
            SkipList A, B;
            for (key_t key{1}; key <= 5; ++key) {
                const auto ins{A.insert(key)};
                assert(ins);
            }
            for (key_t key{6}; key <= 1000; ++key) {
                const auto ins{B.insert(key)};
                assert(ins);
            }
            const auto union_unequal{SkipList::ordered_union(std::move(A), std::move(B))};

            // Perform Tests on Union
            const auto inv_union{union_unequal.check_invariants()};
            assert(inv_union);
            const auto empty_A{A.is_empty()};
            const auto empty_B{B.is_empty()};
            assert(empty_A && empty_B);
            const auto search_1{union_unequal.search(1)};
            const auto search_500{union_unequal.search(500)};
            const auto search_1000{union_unequal.search(1000)};
            assert(search_1 && search_500 && search_1000);
        }

        // Perform Tests on Inverse Property
        {
            // Initialize Skip List, split and merge
            SkipList original;
            for (key_t key{1}; key <= 100; ++key) {
                const auto ins{original.insert(key)};
                assert(ins);
            }
            auto [L, R]{original.split(50)};
            const auto restored{SkipList::ordered_union(std::move(L), std::move(R))};

            // Perform Tests on Inverse Property
            const auto inv_restored{restored.check_invariants()};
            assert(inv_restored);
            for (key_t key{1}; key <= 100; ++key) {
                const auto search_key{restored.search(key)};
                assert(search_key);
            }
        }

        // Perform Tests on Independence after Split
        {
            // Initialize Skip List, perform Split and then modify both outputs independently and verify neither affects the other
            SkipList original;
            for (key_t key{1}; key <= 20; ++key) {
                const auto ins{original.insert(key)};
                assert(ins);
            }
            auto [L, R]{original.split(10)};

            // Perform Tests on Independence after Split
            const auto ins_5{L.insert(5)};
            assert(!ins_5);
            const auto erase_15{R.erase(15)};
            assert(erase_15);
            const auto search_5{L.search(5)};
            assert(search_5);
            const auto search_15{R.search(15)};
            assert(!search_15);
            const auto inv_L{L.check_invariants()};
            const auto inv_R{R.check_invariants()};
            assert(inv_L && inv_R);
        }

        std::cout << "[+] Ordered Union Edge Cases Test completed successfully!\n";
    }

    // Rebuilding Operations Test
    static void test_rebuilding_operations() {
        std::cout << "[+] Running Rebuilding Operations Test...\n";

        // Initialize Skip Lists
        SkipList A, B;
        for (key_t key = 1; key <= 50; ++key) {
            const auto insert{A.insert(key)};
            assert(insert);
        }
        for (key_t k = 51; k <= 100; ++k) {
            const auto insert{B.insert(k)};
            assert(insert);
        }

        // Perform Tests on Rebuilding Split and Ordered Union
        {
            auto res_union{SkipList::rebuilding_ordered_union(std::move(A), std::move(B))};
            assert(res_union.check_invariants());
            assert(A.is_empty() && B.is_empty());

            const auto [L, R]{res_union.rebuilding_split(50)};
            assert(L.check_invariants() && R.check_invariants());
            assert(res_union.is_empty());
        }

        std::cout << "[+] Rebuilding Operations Test completed successfully!\n";
    }

    // Reusability of Consumed Inputs Test
    static void test_reuse_consumed_inputs() {
        std::cout << "[+] Running Reusability of Consumed Inputs Test...\n";

        // Initialize Skip Lists and consume them through Union
        SkipList X, Y;
        const auto ins_X1{X.insert(10)};
        assert(ins_X1);
        const auto ins_Y1{Y.insert(20)};
        assert(ins_Y1);
        const auto combined{SkipList::ordered_union(std::move(X), std::move(Y))};

        // Perform Tests on Reusability
        {
            const auto empty_X{X.is_empty()};
            assert(empty_X);
            const auto empty_Y{Y.is_empty()};
            assert(empty_Y);
            const auto ins_999{X.insert(999)};
            assert(ins_999);
            const auto search_999{X.search(999)};
            assert(search_999);
            const auto inv_X{X.check_invariants()};
            assert(inv_X);
        }

        std::cout << "[+] Reusability of Consumed Inputs Test completed successfully!\n";
    }

    // Address and Height Preservation Test
    static void test_address_and_height_preservation() {
        std::cout << "[+] Running Address and Height Preservation Test...\n";

        // Initialize Skip List and Node Map with Addresses and Heights
        SkipList sl;
        std::unordered_map<key_t, std::pair<const Node*, height_t>> node_map;
        for (key_t key{1}; key <= 100; ++key) {
            sl.insert(key);
        }
        for (const auto* curr_node{sl.get_head()}; curr_node != nullptr; curr_node = curr_node->next[0]) {
            node_map[curr_node->key] = {curr_node, curr_node->height};
        }

        // Perform Tests on Split and Ordered Union
        {
            // Perform Split
            auto [L, R]{sl.split(50)};

            // Verify L nodes
            for (const auto* curr_node{L.get_head()}; curr_node != nullptr; curr_node = curr_node->next[0]) {
                const auto [orig_node, orig_height]{node_map[curr_node->key]};
                assert(curr_node == orig_node);
                assert(curr_node->height == orig_height);
            }

            // Verify R nodes
            for (const auto* curr_node{R.get_head()}; curr_node != nullptr; curr_node = curr_node->next[0]) {
                const auto [orig_node, orig_height]{node_map[curr_node->key]};
                assert(curr_node == orig_node);
                assert(curr_node->height == orig_height);
            }

            // Perform Ordered Union
            const auto merged{SkipList::ordered_union(std::move(L), std::move(R))};

            // Verify merged Nodes
            for (const auto* curr_node{merged.get_head()}; curr_node != nullptr; curr_node = curr_node->next[0]) {
                const auto [orig_node, orig_height]{node_map[curr_node->key]};
                assert(curr_node == orig_node);
                assert(curr_node->height == orig_height);
            }
        }

        std::cout << "[+] Address and Height Preservation Test completed successfully!\n";
    }

    // Randomized Seeds Test
    static void test_randomized_seeds() {
        std::cout << "[+] Running Randomized Seeds Test...\n";

        // Test different seeds
        for (const seed_t seed : {17, 32, 1337}) {
            SkipList sl{MAX_HEIGHT_DEFAULT, PROB_DEFAULT, seed};
            std::set<key_t> ref;

            // Run multiple mixed Operations
            for (size_t step{0}; step < 1500; ++step) {
                // Generate pseudo-random Key
                const auto val{step * 37 % 2000};

                // Generate pseudo-random Operation Type and execute it
                if (const auto op_type{step % 5}; op_type == 0 || op_type == 1) { // Insert (40%)
                    const auto res_sl{sl.insert(static_cast<key_t>(val))};
                    const auto res_ref{ref.insert(static_cast<key_t>(val)).second};
                    assert(res_sl == res_ref);
                } else if (op_type == 2) { // Erase (20%)
                    const auto res_sl{sl.erase(static_cast<key_t>(val))};
                    const auto res_ref{ref.erase(static_cast<key_t>(val)) > 0};
                    assert(res_sl == res_ref);
                } else { // Search (40%)
                    const auto search_sl{sl.search(static_cast<key_t>(val))};
                    const auto search_ref{(ref.contains(static_cast<key_t>(val)))};
                    assert(search_sl == search_ref);
                }

                // Periodically check Invariants
                if (step % 300 == 0) {
                    const auto inv_step{sl.check_invariants()};
                    assert(inv_step);
                }
            }

            // Final Invariants check
            const auto inv_final{sl.check_invariants()};
            assert(inv_final);
        }

        std::cout << "[+] Randomized Seeds Test completed successfully!\n";
    }

    // Complexity Instrumentation Test
    static void test_complexity_instrumentation() {
        std::cout << "[+] Running Complexity Instrumentation Test...\n";

#ifndef NDEBUG
        // Define Input Sizes and Seeds (matching with Google Benchmark)
        std::vector<size_t> sizes{1024, 4096, 16384, 65536, 262144, 1048576};
        std::vector<seed_t> seeds{17, 42, 1337};

        // Data Structures to store the Instrumentation Metrics
        std::vector<metric_t> sh_struct_comps, sh_struct_travs, sh_struct_allocs;
        std::vector<metric_t> i_struct_comps, i_struct_travs, i_struct_allocs;
        std::vector<metric_t> u_struct_comps, u_struct_travs, u_struct_allocs;
        std::vector<metric_t> u_rebuild_comps, u_rebuild_travs, u_rebuild_allocs;
        std::vector<metric_t> s_struct_comps, s_struct_travs, s_struct_allocs;
        std::vector<metric_t> s_rebuild_comps, s_rebuild_travs, s_rebuild_allocs;

        // Run Instrumentation Tests over multiple Input Sizes
        for (auto N : sizes) {
            // Accumulators for averaging across seeds
            metric_t shs_c{0}, shs_t{0}, shs_a{0};
            metric_t is_c{0}, is_t{0}, is_a{0};
            metric_t us_c{0}, us_t{0}, us_a{0};
            metric_t ur_c{0}, ur_t{0}, ur_a{0};
            metric_t ss_c{0}, ss_t{0}, ss_a{0};
            metric_t sr_c{0}, sr_t{0}, sr_a{0};

            // Run Instrumentation Tests over multiple Seeds per Input Size
            for (const auto seed: seeds) {
                // Structural Search
                {
                    // Initialize Skip List
                    SkipList sl{MAX_HEIGHT_DEFAULT, PROB_DEFAULT, seed};
                    for (size_t key_idx{0}; key_idx < N; ++key_idx) {
                        sl.insert(static_cast<key_t>(key_idx * 2));
                    }

                    // Generate absent query Key
                    auto query_key{static_cast<key_t>(N)};

                    // Collect Instrumentation Metrics
                    SKIP_LIST_ENABLE_INSTRUMENTATION();
                    assert(sl.search(query_key));
                    SKIP_LIST_DISABLE_INSTRUMENTATION();

                    // Accumulate Instrumentation Metrics
                    shs_c += SkipList::op_comparisons;
                    shs_t += SkipList::op_traversals;
                    shs_a += SkipList::op_allocations;

                    // Check Invariants
                    assert(sl.check_invariants());
                }

                // Structural Insert
                {
                    // Initialize Skip List
                    SkipList sl{MAX_HEIGHT_DEFAULT, PROB_DEFAULT, seed};
                    for (size_t key_idx{0}; key_idx < N; ++key_idx) {
                        sl.insert(static_cast<key_t>(key_idx * 2));
                    }

                    // Generate absent Key
                    auto new_key{static_cast<key_t>(N + 1)};

                    // Collect Instrumentation Metrics
                    SKIP_LIST_ENABLE_INSTRUMENTATION();
                    sl.insert(new_key);
                    SKIP_LIST_DISABLE_INSTRUMENTATION();

                    // Accumulate Instrumentation Metrics
                    is_c += SkipList::op_comparisons;
                    is_t += SkipList::op_traversals;
                    is_a += SkipList::op_allocations;

                    // Check Invariants
                    assert(sl.check_invariants());
                }

                // Structural Ordered Union
                {
                    // Initialize Skip Lists
                    SkipList A{MAX_HEIGHT_DEFAULT, PROB_DEFAULT, seed};
                    SkipList B{MAX_HEIGHT_DEFAULT, PROB_DEFAULT, seed + 1};
                    for (size_t key_idx{0}; key_idx < N / 2; ++key_idx) {
                        A.insert(static_cast<key_t>(key_idx));
                        B.insert(static_cast<key_t>(N + key_idx));
                    }

                    // Collect Instrumentation Metrics
                    SKIP_LIST_ENABLE_INSTRUMENTATION();
                    const auto res{SkipList::ordered_union(std::move(A), std::move(B))};
                    SKIP_LIST_DISABLE_INSTRUMENTATION();

                    // Accumulate Instrumentation Metrics
                    us_c += SkipList::op_comparisons;
                    us_t += SkipList::op_traversals;
                    us_a += SkipList::op_allocations;

                    // Check Invariants
                    assert(res.check_invariants());
                }

                // Rebuilding Ordered Union
                {
                    // Initialize Skip Lists
                    SkipList A{MAX_HEIGHT_DEFAULT, PROB_DEFAULT, seed};
                    SkipList B{MAX_HEIGHT_DEFAULT, PROB_DEFAULT, seed + 1};
                    for (size_t key_idx{0}; key_idx < N / 2; ++key_idx) {
                        A.insert(static_cast<key_t>(key_idx));
                        B.insert(static_cast<key_t>(N + key_idx));
                    }

                    // Collect Instrumentation Metrics
                    SKIP_LIST_ENABLE_INSTRUMENTATION();
                    const auto res{SkipList::rebuilding_ordered_union(std::move(A), std::move(B))};
                    SKIP_LIST_DISABLE_INSTRUMENTATION();

                    // Accumulate Instrumentation Metrics
                    ur_c += SkipList::op_comparisons;
                    ur_t += SkipList::op_traversals;
                    ur_a += SkipList::op_allocations;

                    // Check Invariants
                    assert(res.check_invariants());
                }

                // Structural Split
                {
                    // Initialize Skip List
                    SkipList sl{MAX_HEIGHT_DEFAULT, PROB_DEFAULT, seed};
                    for (size_t key_idx{0}; key_idx < N; ++key_idx) {
                        sl.insert(static_cast<key_t>(key_idx * 2));
                    }

                    // Collect Instrumentation Metrics
                    SKIP_LIST_ENABLE_INSTRUMENTATION();
                    const auto [L, R]{sl.split(static_cast<key_t>(N))};
                    SKIP_LIST_DISABLE_INSTRUMENTATION();

                    // Accumulate Instrumentation Metrics
                    ss_c += SkipList::op_comparisons;
                    ss_t += SkipList::op_traversals;
                    ss_a += SkipList::op_allocations;

                    // Check Invariants
                    assert(L.check_invariants() && R.check_invariants());
                }

                // Rebuilding Split
                {
                    // Initialize Skip List
                    SkipList sl{MAX_HEIGHT_DEFAULT, PROB_DEFAULT, seed};
                    for (size_t key_idx{0}; key_idx < N; ++key_idx) {
                        sl.insert(static_cast<key_t>(key_idx * 2));
                    }

                    // Collect Instrumentation Metrics
                    SKIP_LIST_ENABLE_INSTRUMENTATION();
                    const auto [L, R]{sl.rebuilding_split(static_cast<key_t>(N))};
                    SKIP_LIST_DISABLE_INSTRUMENTATION();

                    // Accumulate Instrumentation Metrics
                    sr_c += SkipList::op_comparisons;
                    sr_t += SkipList::op_traversals;
                    sr_a += SkipList::op_allocations;

                    // Check Invariants
                    assert(L.check_invariants() && R.check_invariants());
                }
            }

            // Compute Means across Seeds and store
            const auto num_seeds = seeds.size();

            // Search Structural
            sh_struct_comps.push_back(shs_c / num_seeds);
            sh_struct_travs.push_back(shs_t / num_seeds);
            sh_struct_allocs.push_back(shs_a / num_seeds);

            // Insert Structural
            i_struct_comps.push_back(is_c / num_seeds);
            i_struct_travs.push_back(is_t / num_seeds);
            i_struct_allocs.push_back(is_a / num_seeds);

            // Unordered Union Structural
            u_struct_comps.push_back(us_c / num_seeds);
            u_struct_travs.push_back(us_t / num_seeds);
            u_struct_allocs.push_back(us_a / num_seeds);

            // Unordered Union Rebuild
            u_rebuild_comps.push_back(ur_c / num_seeds);
            u_rebuild_travs.push_back(ur_t / num_seeds);
            u_rebuild_allocs.push_back(ur_a / num_seeds);

            // Split Structural
            s_struct_comps.push_back(ss_c / num_seeds);
            s_struct_travs.push_back(ss_t / num_seeds);
            s_struct_allocs.push_back(ss_a / num_seeds);

            // Split Rebuild
            s_rebuild_comps.push_back(sr_c / num_seeds);
            s_rebuild_travs.push_back(sr_t / num_seeds);
            s_rebuild_allocs.push_back(sr_a / num_seeds);
        }

        std::cout << "[+] Generating 'logical_work.json'...\n";

        // Write out to logical_work.json
        std::ofstream json_out{"logical_work.json"};

        // Write Input Sizes
        json_out << "{\n  \"N\": [";
        for (size_t sizes_idx{0}; sizes_idx < sizes.size(); ++sizes_idx) {
            json_out << sizes[sizes_idx] << (sizes_idx + 1 == sizes.size() ? "" : ", ");
        }
        json_out << "],\n";

        // Helper function to write Instrumentation Metrics
        const auto write_metric_block = [&](const std::string& name, const std::vector<uint64_t>& c, const std::vector<uint64_t>& t, const std::vector<uint64_t>& a, const bool last) {
            // Write Operation name
            json_out << "  \"" << name << "\": {\n";

            // Write Comparisons
            json_out << "    \"comparisons\": [";
            for (size_t idx{0}; idx < c.size(); ++idx)
                json_out << c[idx] << (idx + 1 == c.size() ? "" : ", ");

            // Write Traversals
            json_out << "],\n    \"traversals\": [";
            for (size_t idx{0}; idx < t.size(); ++idx)
                json_out << t[idx] << (idx + 1 == t.size() ? "" : ", ");

            // Write Allocations
            json_out << "],\n    \"allocations\": [";
            for (size_t idx{0}; idx < a.size(); ++idx)
                json_out << a[idx] << (idx + 1 == a.size() ? "" : ", ");

            // Close JSON object
            json_out << "]\n  }" << (last ? "\n" : ",\n");
        };

        // Write Instrumentation Metrics
        write_metric_block("search_structural", sh_struct_comps, sh_struct_travs, sh_struct_allocs, false);
        write_metric_block("insert_structural", i_struct_comps, i_struct_travs, i_struct_allocs, false);
        write_metric_block("ordered_union_structural", u_struct_comps, u_struct_travs, u_struct_allocs, false);
        write_metric_block("ordered_union_rebuilding", u_rebuild_comps, u_rebuild_travs, u_rebuild_allocs, false);
        write_metric_block("split_structural", s_struct_comps, s_struct_travs, s_struct_allocs, false);
        write_metric_block("split_rebuilding", s_rebuild_comps, s_rebuild_travs, s_rebuild_allocs, true);

        // Close JSON object and file
        json_out << "}\n";
        json_out.close();

        std::cout << "[+] Successfully wrote 'logical_work.json'!\n";
#endif

        std::cout << "[+] Complexity Instrumentation Test completed successfully!\n";
    }

    // Run all Tests sequentially, one after the other
    static void run_tests() {
        std::cout << "[+] Running Skip List Tests...\n";

        // Run Generic Operations Test
        test_generic_operations();

        // Run Clear Method Test
        test_clear_method();

        // Run Boundary Keys Test
        test_boundary_keys();

        // Run Empty and Singleton Test
        test_empty_and_singleton();

        // Run Split Edge Cases Test
        test_split_edge_cases();

        // Run Ordered Union Edge Cases Test
        test_ordered_union_edge_cases();

        // Run Rebuilding Operations Test
        test_rebuilding_operations();

        // Run Reusability of Consumed Inputs Test
        test_reuse_consumed_inputs();

        // Run Address and Height Preservation Test
        test_address_and_height_preservation();

        // Run Randomized Seeds Test (1500+ Operations)
        test_randomized_seeds();

        // Run Complexity Instrumentation Test and generate JSON Output
        test_complexity_instrumentation();

        std::cout << "[+] Skip List Tests completed successfully!\n";
    }

} // skip_list

// Test entry point
int main() {
    // Run Tests
    skip_list::run_tests();

    return 0;
}