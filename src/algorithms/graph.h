/**
 * @file graph.h
 * @brief 图算法模块
 * @details 提供图数据结构和图算法实现
 *          包括 BFS、DFS、最短路径、最小生成树、拓扑排序等
 */

#ifndef ZEN_ALGORITHMS_GRAPH_H
#define ZEN_ALGORITHMS_GRAPH_H

#include <vector>
#include <queue>
#include <stack>
#include <limits>
#include <unordered_map>
#include <functional>
#include <type_traits>
#include "../base/type_traits.h"

namespace zen {

// ============================================================================
// 图数据结构
// ============================================================================

/**
 * @brief 邻接表表示的图
 * @tparam T 节点值类型
 * @tparam WeightType 边权重类型
 */
template <typename T, typename WeightType = double>
class Graph {
public:
    using Node = T;
    using Weight = WeightType;
    using Edge = std::pair<size_t, Weight>;  // 目标节点索引和权重

    Graph() = default;

    /**
     * @brief 添加节点
     * @param value 节点值
     * @return 节点索引
     */
    size_t add_node(const Node& value) {
        nodes_.push_back(value);
        adjacency_.emplace_back();
        return nodes_.size() - 1;
    }

    /**
     * @brief 添加无向边
     * @param from 起始节点索引
     * @param to 目标节点索引
     * @param weight 边权重
     */
    void add_undirected_edge(size_t from, size_t to, Weight weight = 1.0) {
        add_directed_edge(from, to, weight);
        add_directed_edge(to, from, weight);
    }

    /**
     * @brief 添加有向边
     * @param from 起始节点索引
     * @param to 目标节点索引
     * @param weight 边权重
     */
    void add_directed_edge(size_t from, size_t to, Weight weight = 1.0) {
        if (from >= nodes_.size() || to >= nodes_.size()) {
            return;
        }
        adjacency_[from].emplace_back(to, weight);
    }

    /**
     * @brief 获取节点数量
     */
    size_t node_count() const { return nodes_.size(); }

    /**
     * @brief 获取节点值
     */
    const Node& get_node(size_t index) const { return nodes_[index]; }

    /**
     * @brief 获取邻接表
     */
    const std::vector<std::vector<Edge>>& adjacency() const { return adjacency_; }

    /**
     * @brief 获取邻居节点
     */
    const std::vector<Edge>& neighbors(size_t index) const {
        return adjacency_[index];
    }

private:
    std::vector<Node> nodes_;
    std::vector<std::vector<Edge>> adjacency_;
};

// ============================================================================
// 广度优先搜索 (BFS)
// ============================================================================

/**
 * @brief 广度优先搜索
 * @tparam T 节点类型
 * @tparam WeightType 权重类型
 * @param graph 图
 * @param start 起始节点索引
 * @param visitor 访问回调函数 (bool visitor(size_t current_index, size_t parent_index))
 * @return 是否成功完成所有访问
 */
template <typename T, typename WeightType = double>
bool bfs(
    const Graph<T, WeightType>& graph,
    size_t start,
    std::function<bool(size_t, size_t)> visitor = nullptr
) {
    if (start >= graph.node_count()) {
        return false;
    }

    std::vector<bool> visited(graph.node_count(), false);
    std::queue<size_t> queue;

    visited[start] = true;
    queue.push(start);

    while (!queue.empty()) {
        size_t current = queue.front();
        queue.pop();

        if (visitor) {
            if (!visitor(current, static_cast<size_t>(-1))) {
                return false;
            }
        }

        for (const auto& edge : graph.neighbors(current)) {
            size_t neighbor = edge.first;
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                queue.push(neighbor);
            }
        }
    }

    return true;
}

/**
 * @brief 广度优先搜索（返回访问顺序）
 */
template <typename T, typename WeightType = double>
std::vector<size_t> bfs_order(const Graph<T, WeightType>& graph, size_t start) {
    std::vector<size_t> order;
    bfs(graph, start, [&order](size_t current, size_t) {
        order.push_back(current);
        return true;
    });
    return order;
}

/**
 * @brief 广度优先搜索（计算最短路径）
 */
template <typename T, typename WeightType = double>
std::vector<size_t> bfs_shortest_path(
    const Graph<T, WeightType>& graph,
    size_t start,
    size_t end
) {
    if (start >= graph.node_count() || end >= graph.node_count()) {
        return {};
    }

    std::vector<size_t> parent(graph.node_count(), static_cast<size_t>(-1));
    std::vector<bool> visited(graph.node_count(), false);
    std::queue<size_t> queue;

    visited[start] = true;
    queue.push(start);

    while (!queue.empty()) {
        size_t current = queue.front();
        queue.pop();

        if (current == end) {
            break;
        }

        for (const auto& edge : graph.neighbors(current)) {
            size_t neighbor = edge.first;
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                parent[neighbor] = current;
                queue.push(neighbor);
            }
        }
    }

    // 构建路径
    if (parent[end] == static_cast<size_t>(-1) && start != end) {
        return {};  // 没有找到路径
    }

    std::vector<size_t> path;
    for (size_t v = end; v != static_cast<size_t>(-1); v = parent[v]) {
        path.push_back(v);
    }
    std::reverse(path.begin(), path.end());
    return path;
}

// ============================================================================
// 深度优先搜索 (DFS)
// ============================================================================

/**
 * @brief 深度优先搜索（递归）
 */
template <typename T, typename WeightType = double>
bool dfs(
    const Graph<T, WeightType>& graph,
    size_t start,
    std::function<bool(size_t)> visitor = nullptr
) {
    if (start >= graph.node_count()) {
        return false;
    }

    std::vector<bool> visited(graph.node_count(), false);

    std::function<bool(size_t)> dfs_helper = [&](size_t current) -> bool {
        visited[current] = true;

        if (visitor) {
            if (!visitor(current)) {
                return false;
            }
        }

        for (const auto& edge : graph.neighbors(current)) {
            size_t neighbor = edge.first;
            if (!visited[neighbor]) {
                if (!dfs_helper(neighbor)) {
                    return false;
                }
            }
        }
        return true;
    };

    return dfs_helper(start);
}

/**
 * @brief 深度优先搜索（迭代）
 */
template <typename T, typename WeightType = double>
bool dfs_iterative(
    const Graph<T, WeightType>& graph,
    size_t start,
    std::function<bool(size_t)> visitor = nullptr
) {
    if (start >= graph.node_count()) {
        return false;
    }

    std::vector<bool> visited(graph.node_count(), false);
    std::stack<size_t> stack;

    stack.push(start);

    while (!stack.empty()) {
        size_t current = stack.top();
        stack.pop();

        if (visited[current]) {
            continue;
        }

        visited[current] = true;

        if (visitor) {
            if (!visitor(current)) {
                return false;
            }
        }

        // 反向压栈以保证正确顺序
        for (auto it = graph.neighbors(current).rbegin(); it != graph.neighbors(current).rend(); ++it) {
            size_t neighbor = it->first;
            if (!visited[neighbor]) {
                stack.push(neighbor);
            }
        }
    }

    return true;
}

/**
 * @brief 深度优先搜索（返回访问顺序）
 */
template <typename T, typename WeightType = double>
std::vector<size_t> dfs_order(const Graph<T, WeightType>& graph, size_t start) {
    std::vector<size_t> order;
    dfs(graph, start, [&order](size_t current) {
        order.push_back(current);
        return true;
    });
    return order;
}

// ============================================================================
// 最短路径算法 (Dijkstra)
// ============================================================================

/**
 * @brief Dijkstra 最短路径算法
 */
template <typename T, typename WeightType = double>
std::vector<typename Graph<T, WeightType>::Weight> dijkstra(
    const Graph<T, WeightType>& graph,
    size_t start
) {
    using Weight = typename Graph<T, WeightType>::Weight;
    const Weight INF = std::numeric_limits<Weight>::infinity();

    size_t n = graph.node_count();
    std::vector<Weight> dist(n, INF);
    std::vector<bool> visited(n, false);

    dist[start] = 0;

    for (size_t i = 0; i < n; ++i) {
        // 找到未访问的最小距离节点
        size_t u = n;
        Weight min_dist = INF;

        for (size_t j = 0; j < n; ++j) {
            if (!visited[j] && dist[j] < min_dist) {
                min_dist = dist[j];
                u = j;
            }
        }

        if (u == n || dist[u] == INF) {
            break;
        }

        visited[u] = true;

        // 松弛操作
        for (const auto& edge : graph.neighbors(u)) {
            size_t v = edge.first;
            Weight weight = edge.second;
            if (dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
            }
        }
    }

    return dist;
}

/**
 * @brief Dijkstra 最短路径（返回路径）
 */
template <typename T, typename WeightType = double>
std::vector<size_t> dijkstra_path(
    const Graph<T, WeightType>& graph,
    size_t start,
    size_t end
) {
    using Weight = typename Graph<T, WeightType>::Weight;
    const Weight INF = std::numeric_limits<Weight>::infinity();

    size_t n = graph.node_count();
    std::vector<Weight> dist(n, INF);
    std::vector<size_t> parent(n, static_cast<size_t>(-1));
    std::vector<bool> visited(n, false);

    dist[start] = 0;

    for (size_t i = 0; i < n; ++i) {
        size_t u = n;
        Weight min_dist = INF;

        for (size_t j = 0; j < n; ++j) {
            if (!visited[j] && dist[j] < min_dist) {
                min_dist = dist[j];
                u = j;
            }
        }

        if (u == n || dist[u] == INF) {
            break;
        }

        visited[u] = true;

        for (const auto& edge : graph.neighbors(u)) {
            size_t v = edge.first;
            Weight weight = edge.second;
            if (dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                parent[v] = u;
            }
        }
    }

    if (dist[end] == INF) {
        return {};
    }

    std::vector<size_t> path;
    for (size_t v = end; v != static_cast<size_t>(-1); v = parent[v]) {
        path.push_back(v);
    }
    std::reverse(path.begin(), path.end());
    return path;
}

// ============================================================================
// 最小生成树算法 (Prim 和 Kruskal)
// ============================================================================

/**
 * @brief Prim 最小生成树算法
 */
template <typename T, typename WeightType = double>
typename Graph<T, WeightType>::Weight prim_mst(
    const Graph<T, WeightType>& graph
) {
    using Weight = typename Graph<T, WeightType>::Weight;
    const Weight INF = std::numeric_limits<Weight>::infinity();

    size_t n = graph.node_count();
    if (n == 0) return 0;

    std::vector<Weight> key(n, INF);
    std::vector<bool> in_mst(n, false);

    key[0] = 0;
    Weight total_weight = 0;

    for (size_t count = 0; count < n; ++count) {
        // 找到不在 MST 中且 key 最小的节点
        size_t u = n;
        Weight min_key = INF;

        for (size_t v = 0; v < n; ++v) {
            if (!in_mst[v] && key[v] < min_key) {
                min_key = key[v];
                u = v;
            }
        }

        if (u == n || min_key == INF) {
            break;
        }

        in_mst[u] = true;
        total_weight += key[u];

        // 更新邻居的 key
        for (const auto& edge : graph.neighbors(u)) {
            size_t v = edge.first;
            Weight weight = edge.second;
            if (!in_mst[v] && weight < key[v]) {
                key[v] = weight;
            }
        }
    }

    return total_weight;
}

// ============================================================================
// 拓扑排序
// ============================================================================

/**
 * @brief 拓扑排序（Kahn 算法）
 */
template <typename T, typename WeightType = double>
std::vector<size_t> topological_sort(const Graph<T, WeightType>& graph) {
    size_t n = graph.node_count();

    // 计算入度
    std::vector<size_t> in_degree(n, 0);
    for (size_t u = 0; u < n; ++u) {
        for (const auto& edge : graph.neighbors(u)) {
            in_degree[edge.first]++;
        }
    }

    // 入度为 0 的节点队列
    std::queue<size_t> queue;
    for (size_t u = 0; u < n; ++u) {
        if (in_degree[u] == 0) {
            queue.push(u);
        }
    }

    std::vector<size_t> result;
    while (!queue.empty()) {
        size_t u = queue.front();
        queue.pop();
        result.push_back(u);

        for (const auto& edge : graph.neighbors(u)) {
            size_t v = edge.first;
            if (--in_degree[v] == 0) {
                queue.push(v);
            }
        }
    }

    // 如果结果数不等于节点数，说明有环
    if (result.size() != n) {
        return {};
    }

    return result;
}

// ============================================================================
// 连通分量检测
// ============================================================================

/**
 * @brief 检测连通分量
 * @return 每个节点所属的连通分量编号
 */
template <typename T, typename WeightType = double>
std::vector<size_t> find_connected_components(const Graph<T, WeightType>& graph) {
    size_t n = graph.node_count();
    std::vector<size_t> component_id(n, static_cast<size_t>(-1));
    size_t current_component = 0;

    for (size_t start = 0; start < n; ++start) {
        if (component_id[start] != static_cast<size_t>(-1)) {
            continue;
        }

        // BFS 标记所有连通节点
        std::queue<size_t> queue;
        queue.push(start);
        component_id[start] = current_component;

        while (!queue.empty()) {
            size_t u = queue.front();
            queue.pop();

            for (const auto& edge : graph.neighbors(u)) {
                size_t v = edge.first;
                if (component_id[v] == static_cast<size_t>(-1)) {
                    component_id[v] = current_component;
                    queue.push(v);
                }
            }
        }

        current_component++;
    }

    return component_id;
}

/**
 * @brief 判断图是否连通
 */
template <typename T, typename WeightType = double>
bool is_connected(const Graph<T, WeightType>& graph) {
    size_t n = graph.node_count();
    if (n == 0) return true;

    auto components = find_connected_components(graph);
    size_t first_component = components[0];

    for (size_t i = 1; i < n; ++i) {
        if (components[i] != first_component) {
            return false;
        }
    }

    return true;
}

// ============================================================================
// 环检测
// ============================================================================

/**
 * @brief 检测无向图是否有环
 */
template <typename T, typename WeightType = double>
bool has_cycle_undirected(const Graph<T, WeightType>& graph) {
    size_t n = graph.node_count();
    std::vector<bool> visited(n, false);

    for (size_t start = 0; start < n; ++start) {
        if (visited[start]) {
            continue;
        }

        std::function<bool(size_t, size_t)> dfs_cycle = [&](size_t u, size_t parent) -> bool {
            visited[u] = true;

            for (const auto& edge : graph.neighbors(u)) {
                size_t v = edge.first;
                if (!visited[v]) {
                    if (dfs_cycle(v, u)) {
                        return true;
                    }
                } else if (v != parent) {
                    return true;  // 访问了非父节点，说明有环
                }
            }
            return false;
        };

        if (dfs_cycle(start, static_cast<size_t>(-1))) {
            return true;
        }
    }

    return false;
}

/**
 * @brief 检测有向图是否有环（DFS）
 */
template <typename T, typename WeightType = double>
bool has_cycle_directed(const Graph<T, WeightType>& graph) {
    size_t n = graph.node_count();
    std::vector<bool> visited(n, false);
    std::vector<bool> on_stack(n, false);

    std::function<bool(size_t)> dfs_cycle = [&](size_t u) -> bool {
        visited[u] = true;
        on_stack[u] = true;

        for (const auto& edge : graph.neighbors(u)) {
            size_t v = edge.first;
            if (!visited[v]) {
                if (dfs_cycle(v)) {
                    return true;
                }
            } else if (on_stack[v]) {
                return true;  // 在递归栈中，说明有环
            }
        }

        on_stack[u] = false;
        return false;
    };

    for (size_t u = 0; u < n; ++u) {
        if (!visited[u]) {
            if (dfs_cycle(u)) {
                return true;
            }
        }
    }

    return false;
}

}  // namespace zen

#endif  // ZEN_ALGORITHMS_GRAPH_H
