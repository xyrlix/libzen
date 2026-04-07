/**
 * @file graph_example.cpp
 * @brief 图算法使用示例
 */

#include <iostream>
#include "algorithms/graph.h"

using namespace zen;

void example_bfs() {
    std::cout << "=== BFS 示例 ===\n";

    Graph<int> g;
    auto n0 = g.add_node(0);
    auto n1 = g.add_node(1);
    auto n2 = g.add_node(2);
    auto n3 = g.add_node(3);

    // 创建图: 0-1-2-3
    g.add_undirected_edge(n0, n1);
    g.add_undirected_edge(n1, n2);
    g.add_undirected_edge(n2, n3);

    // BFS 遍历
    auto order = bfs_order(g, n0);
    std::cout << "BFS 访问顺序: ";
    for (auto idx : order) {
        std::cout << g.get_node(idx) << " ";
    }
    std::cout << "\n";

    // 查找最短路径
    auto path = bfs_shortest_path(g, n0, n3);
    std::cout << "从 0 到 3 的最短路径: ";
    for (auto idx : path) {
        std::cout << g.get_node(idx) << " ";
    }
    std::cout << "\n\n";
}

void example_dijkstra() {
    std::cout << "=== Dijkstra 示例 ===\n";

    Graph<int> g;
    auto n0 = g.add_node(0);  // 起点
    auto n1 = g.add_node(1);
    auto n2 = g.add_node(2);
    auto n3 = g.add_node(3);  // 终点

    // 创建带权有向图
    g.add_directed_edge(n0, n1, 4);
    g.add_directed_edge(n0, n2, 2);
    g.add_directed_edge(n1, n2, 1);
    g.add_directed_edge(n1, n3, 5);
    g.add_directed_edge(n2, n3, 8);

    // 计算从起点到所有点的最短距离
    auto dist = dijkstra(g, n0);
    std::cout << "从 0 到各节点的最短距离:\n";
    for (size_t i = 0; i < dist.size(); ++i) {
        std::cout << "  0 -> " << i << ": " << dist[i] << "\n";
    }

    // 查找特定路径
    auto path = dijkstra_path(g, n0, n3);
    std::cout << "从 0 到 3 的最短路径: ";
    for (auto idx : path) {
        std::cout << g.get_node(idx);
        if (idx != path.back()) std::cout << " -> ";
    }
    std::cout << "\n\n";
}

void example_topological_sort() {
    std::cout << "=== 拓扑排序示例 ===\n";

    Graph<std::string> g;
    auto a = g.add_node("A");
    auto b = g.add_node("B");
    auto c = g.add_node("C");
    auto d = g.add_node("D");

    // 课程依赖关系: A -> B -> C -> D
    g.add_directed_edge(a, b);
    g.add_directed_edge(b, c);
    g.add_directed_edge(c, d);

    // 拓扑排序
    auto order = topological_sort(g);
    if (!order.empty()) {
        std::cout << "拓扑排序结果: ";
        for (auto idx : order) {
            std::cout << g.get_node(idx) << " ";
        }
        std::cout << "\n";
    } else {
        std::cout << "图中存在环！\n";
    }
    std::cout << "\n";
}

void example_connected_components() {
    std::cout << "=== 连通分量示例 ===\n";

    Graph<int> g;
    auto n0 = g.add_node(0);
    auto n1 = g.add_node(1);
    auto n2 = g.add_node(2);
    auto n3 = g.add_node(3);
    auto n4 = g.add_node(4);

    // 创建两个连通分量: 0-1-2 和 3-4
    g.add_undirected_edge(n0, n1);
    g.add_undirected_edge(n1, n2);
    g.add_undirected_edge(n3, n4);

    // 检测连通分量
    auto components = find_connected_components(g);
    std::cout << "连通分量:\n";
    for (size_t i = 0; i < components.size(); ++i) {
        std::cout << "  节点 " << i << " 属于分量 " << components[i] << "\n";
    }

    std::cout << "图是否连通: " << (is_connected(g) ? "是" : "否") << "\n\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "    图算法示例\n";
    std::cout << "========================================\n\n";

    example_bfs();
    example_dijkstra();
    example_topological_sort();
    example_connected_components();

    return 0;
}
