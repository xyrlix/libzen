/**
 * @file test_graph_string.cpp
 * @brief 图算法和字符串算法测试
 */

#include <iostream>
#include <cassert>
#include "algorithms/graph.h"
#include "algorithms/string.h"

using namespace zen;

// ============================================================================
// 图算法测试
// ============================================================================

void test_graph_basic() {
    std::cout << "测试图的基本操作...\n";

    Graph<int> g;
    size_t n0 = g.add_node(0);
    size_t n1 = g.add_node(1);
    size_t n2 = g.add_node(2);
    size_t n3 = g.add_node(3);

    g.add_undirected_edge(n0, n1, 1.0);
    g.add_undirected_edge(n1, n2, 2.0);
    g.add_undirected_edge(n2, n3, 3.0);
    g.add_undirected_edge(n3, n0, 4.0);

    assert(g.node_count() == 4);
    assert(g.get_node(0) == 0);

    std::cout << "  ✓ 图的基本操作测试通过\n";
}

void test_bfs() {
    std::cout << "测试 BFS 算法...\n";

    Graph<int> g;
    auto n0 = g.add_node(0);
    auto n1 = g.add_node(1);
    auto n2 = g.add_node(2);
    auto n3 = g.add_node(3);

    g.add_undirected_edge(n0, n1);
    g.add_undirected_edge(n1, n2);
    g.add_undirected_edge(n2, n3);

    auto order = bfs_order(g, n0);
    assert(order.size() == 4);
    assert(order[0] == 0);

    auto path = bfs_shortest_path(g, n0, n3);
    assert(path.size() == 4);
    assert(path[0] == 0);
    assert(path[3] == 3);

    std::cout << "  ✓ BFS 算法测试通过\n";
}

void test_dfs() {
    std::cout << "测试 DFS 算法...\n";

    Graph<int> g;
    auto n0 = g.add_node(0);
    auto n1 = g.add_node(1);
    auto n2 = g.add_node(2);

    g.add_directed_edge(n0, n1);
    g.add_directed_edge(n1, n2);

    auto order = dfs_order(g, n0);
    assert(order.size() == 3);

    std::cout << "  ✓ DFS 算法测试通过\n";
}

void test_dijkstra() {
    std::cout << "测试 Dijkstra 最短路径算法...\n";

    Graph<int> g;
    auto n0 = g.add_node(0);
    auto n1 = g.add_node(1);
    auto n2 = g.add_node(2);
    auto n3 = g.add_node(3);

    g.add_directed_edge(n0, n1, 4);
    g.add_directed_edge(n0, n2, 2);
    g.add_directed_edge(n1, n2, 1);
    g.add_directed_edge(n1, n3, 5);
    g.add_directed_edge(n2, n3, 8);

    auto dist = dijkstra(g, n0);
    assert(dist[0] == 0);
    assert(dist[1] == 3);  // 0 -> 2 -> 1 = 2 + 1
    assert(dist[2] == 2);  // 0 -> 2 = 2

    auto path = dijkstra_path(g, n0, n3);
    assert(path.size() == 4);  // 0 -> 2 -> 1 -> 3 = 2 + 1 + 5 = 8

    std::cout << "  ✓ Dijkstra 算法测试通过\n";
}

void test_topological_sort() {
    std::cout << "测试拓扑排序...\n";

    Graph<int> g;
    auto n0 = g.add_node(0);  // 依赖关系: 3 -> 2 -> 1 -> 0
    auto n1 = g.add_node(1);
    auto n2 = g.add_node(2);
    auto n3 = g.add_node(3);

    g.add_directed_edge(n3, n2);
    g.add_directed_edge(n2, n1);
    g.add_directed_edge(n1, n0);

    auto order = topological_sort(g);
    assert(order.size() == 4);
    assert(order[0] == 3);
    assert(order[3] == 0);

    std::cout << "  ✓ 拓扑排序测试通过\n";
}

void test_connected_components() {
    std::cout << "测试连通分量检测...\n";

    Graph<int> g;
    auto n0 = g.add_node(0);
    auto n1 = g.add_node(1);
    auto n2 = g.add_node(2);

    g.add_undirected_edge(n0, n1);
    // n2 是孤立的

    auto components = find_connected_components(g);
    assert(components[0] == components[1]);
    assert(components[2] != components[0]);

    assert(is_connected(g) == false);

    g.add_undirected_edge(n1, n2);
    assert(is_connected(g) == true);

    std::cout << "  ✓ 连通分量检测测试通过\n";
}

// ============================================================================
// 字符串算法测试
// ============================================================================

void test_kmp() {
    std::cout << "测试 KMP 算法...\n";

    std::string text = "ABABDABACDABABCABAB";
    std::string pattern = "ABABCABAB";

    auto positions = kmp_search(text, pattern);
    assert(positions.size() == 1);
    assert(positions[0] == 10);

    assert(kmp_find_first(text, pattern) == 10);

    std::cout << "  ✓ KMP 算法测试通过\n";
}

void test_boyer_moore() {
    std::cout << "测试 Boyer-Moore 算法...\n";

    std::string text = "ABAAABCDEF";
    std::string pattern = "ABCD";

    auto positions = boyer_moore_search(text, pattern);
    assert(positions.size() == 1);
    assert(positions[0] == 4);

    assert(boyer_moore_find_first(text, pattern) == 4);

    std::cout << "  ✓ Boyer-Moore 算法测试通过\n";
}

void test_rabin_karp() {
    std::cout << "测试 Rabin-Karp 算法...\n";

    std::string text = "GEEKS FOR GEEKS";
    std::string pattern = "GEEK";

    auto positions = rabin_karp_search(text, pattern);
    assert(positions.size() == 2);

    std::cout << "  ✓ Rabin-Karp 算法测试通过\n";
}

void test_string_utils() {
    std::cout << "测试字符串工具...\n";

    std::string s = "Hello World";

    assert(split(s, ' ').size() == 2);
    assert(split(s, ' ')[0] == "Hello");

    assert(trim("  hello  ") == "hello");
    assert(ltrim("  hello") == "hello");
    assert(rtrim("hello  ") == "hello");

    assert(to_lower("HELLO") == "hello");
    assert(to_upper("hello") == "HELLO");

    assert(starts_with("hello world", "hello") == true);
    assert(ends_with("hello world", "world") == true);

    assert(replace_all("hello world", "world", "there") == "hello there");

    assert(reverse("hello") == "olleh");
    assert(is_palindrome("racecar") == true);

    std::cout << "  ✓ 字符串工具测试通过\n";
}

void test_edit_distance() {
    std::cout << "测试编辑距离...\n";

    assert(levenshtein_distance("kitten", "sitting") == 3);
    assert(levenshtein_distance("saturday", "sunday") == 3);
    assert(levenshtein_distance("", "abc") == 3);
    assert(levenshtein_distance("abc", "") == 3);

    std::cout << "  ✓ 编辑距离测试通过\n";
}

void test_string_hash() {
    std::cout << "测试字符串哈希...\n";

    std::string s1 = "hello";
    std::string s2 = "hello";

    assert(hash_djb2(s1) == hash_djb2(s2));
    assert(hash_sdbm(s1) == hash_sdbm(s2));
    assert(hash_fnv1a(s1) == hash_fnv1a(s2));

    assert(hash_djb2("hello") != hash_djb2("world"));

    std::cout << "  ✓ 字符串哈希测试通过\n";
}

void test_wildcard_match() {
    std::cout << "测试通配符匹配...\n";

    assert(wildcard_match("hello", "h*") == true);
    assert(wildcard_match("hello", "h?llo") == true);
    assert(wildcard_match("hello", "*o") == true);
    assert(wildcard_match("hello", "a*") == false);

    std::cout << "  ✓ 通配符匹配测试通过\n";
}

// ============================================================================
// 主测试函数
// ============================================================================

int main() {
    std::cout << "========================================\n";
    std::cout << "    图算法和字符串算法测试\n";
    std::cout << "========================================\n\n";

    // 图算法测试
    std::cout << "--- 图算法测试 ---\n";
    test_graph_basic();
    test_bfs();
    test_dfs();
    test_dijkstra();
    test_topological_sort();
    test_connected_components();

    // 字符串算法测试
    std::cout << "\n--- 字符串算法测试 ---\n";
    test_kmp();
    test_boyer_moore();
    test_rabin_karp();
    test_string_utils();
    test_edit_distance();
    test_string_hash();
    test_wildcard_match();

    std::cout << "\n========================================\n";
    std::cout << "    所有测试通过！\n";
    std::cout << "========================================\n";

    return 0;
}
