#include "PathFinder.hpp"
#include <queue>
#include <stack>
#include <limits>
#include <chrono>
#include <sstream>
#include <algorithm>

namespace campus {

RouteResult PathFinder::dijkstra(const CampusGraph& graph, int startId, int targetId) {
    RouteResult result;
    result.algorithmName = "Dijkstra (Min-Priority Queue)";
    result.startId = startId;
    result.targetId = targetId;

    if (!graph.hasLocation(startId) || !graph.hasLocation(targetId)) {
        result.found = false;
        result.notes = "Error: Invalid source or destination vertex ID.";
        return result;
    }

    if (startId == targetId) {
        result.found = true;
        result.path = {startId};
        result.totalDistance = 0.0;
        result.hops = 0;
        result.nodesVisited = 1;
        result.notes = "Origin and destination are identical.";
        return result;
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    // Min-priority queue storing pairs of (distance, vertexId)
    using Element = std::pair<double, int>;
    std::priority_queue<Element, std::vector<Element>, std::greater<Element>> pq;

    std::unordered_map<int, double> dist;
    std::unordered_map<int, int> parent;
    std::unordered_map<int, bool> visited;

    const double INF = std::numeric_limits<double>::infinity();
    for (int id : graph.getAllLocationIds()) {
        dist[id] = INF;
        visited[id] = false;
    }

    dist[startId] = 0.0;
    pq.push({0.0, startId});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        // Lazy deletion: if we already found a shorter path to u, skip
        if (visited[u] || d > dist[u]) {
            continue;
        }

        visited[u] = true;
        result.nodesVisited++;

        // Early termination: greedy choice property ensures shortest path settled
        if (u == targetId) {
            break;
        }

        for (const auto& edge : graph.getNeighbors(u)) {
            result.edgeEvaluations++;
            int v = edge.to;
            double weight = edge.weight;

            if (!visited[v] && dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                parent[v] = u;
                pq.push({dist[v], v});
            }
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    result.executionTimeUs = std::chrono::duration<double, std::micro>(endTime - startTime).count();

    if (dist[targetId] == INF || parent.find(targetId) == parent.end()) {
        result.found = false;
        result.totalDistance = INF;
        result.notes = "Disconnected Graph: Target is unreachable from origin.";
        return result;
    }

    // Path reconstruction via parent backpointers
    std::vector<int> path;
    int curr = targetId;
    while (curr != startId) {
        path.push_back(curr);
        curr = parent[curr];
    }
    path.push_back(startId);
    std::reverse(path.begin(), path.end());

    result.found = true;
    result.path = std::move(path);
    result.totalDistance = dist[targetId];
    result.hops = static_cast<int>(result.path.size() - 1);
    result.notes = "Optimal minimum walking distance found.";

    return result;
}

RouteResult PathFinder::bfs(const CampusGraph& graph, int startId, int targetId) {
    RouteResult result;
    result.algorithmName = "Breadth-First Search (Fewest Hops)";
    result.startId = startId;
    result.targetId = targetId;

    if (!graph.hasLocation(startId) || !graph.hasLocation(targetId)) {
        result.found = false;
        result.notes = "Error: Invalid source or destination vertex ID.";
        return result;
    }

    if (startId == targetId) {
        result.found = true;
        result.path = {startId};
        result.totalDistance = 0.0;
        result.hops = 0;
        result.nodesVisited = 1;
        result.notes = "Origin and destination are identical.";
        return result;
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    std::queue<int> q;
    std::unordered_map<int, bool> visited;
    std::unordered_map<int, int> parent;

    visited[startId] = true;
    q.push(startId);

    bool reached = false;
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        result.nodesVisited++;

        if (u == targetId) {
            reached = true;
            break;
        }

        for (const auto& edge : graph.getNeighbors(u)) {
            result.edgeEvaluations++;
            int v = edge.to;
            if (!visited[v]) {
                visited[v] = true;
                parent[v] = u;
                q.push(v);
            }
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    result.executionTimeUs = std::chrono::duration<double, std::micro>(endTime - startTime).count();

    if (!reached) {
        result.found = false;
        result.totalDistance = std::numeric_limits<double>::infinity();
        result.notes = "Disconnected Graph: Target is unreachable from origin.";
        return result;
    }

    // Reconstruct path
    std::vector<int> path;
    int curr = targetId;
    while (curr != startId) {
        path.push_back(curr);
        curr = parent[curr];
    }
    path.push_back(startId);
    std::reverse(path.begin(), path.end());

    // Compute actual physical distance along BFS hop path
    double distSum = 0.0;
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        Edge edge(0, 0);
        if (graph.findEdge(path[i], path[i + 1], edge)) {
            distSum += edge.weight;
        }
    }

    result.found = true;
    result.path = std::move(path);
    result.totalDistance = distSum;
    result.hops = static_cast<int>(result.path.size() - 1);
    result.notes = "Path with minimum intersections / hops.";

    return result;
}

RouteResult PathFinder::dfs(const CampusGraph& graph, int startId, int targetId) {
    RouteResult result;
    result.algorithmName = "Depth-First Search (Exploratory)";
    result.startId = startId;
    result.targetId = targetId;

    if (!graph.hasLocation(startId) || !graph.hasLocation(targetId)) {
        result.found = false;
        result.notes = "Error: Invalid source or destination vertex ID.";
        return result;
    }

    if (startId == targetId) {
        result.found = true;
        result.path = {startId};
        result.totalDistance = 0.0;
        result.hops = 0;
        result.nodesVisited = 1;
        result.notes = "Origin and destination are identical.";
        return result;
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    // Standard iterative DFS using explicit stack
    std::stack<int> s;
    std::unordered_map<int, bool> visited;
    std::unordered_map<int, int> parent;

    s.push(startId);
    visited[startId] = true;

    bool reached = false;
    while (!s.empty()) {
        int u = s.top();
        s.pop();
        result.nodesVisited++;

        if (u == targetId) {
            reached = true;
            break;
        }

        // Iterate neighbors
        for (const auto& edge : graph.getNeighbors(u)) {
            result.edgeEvaluations++;
            int v = edge.to;
            if (!visited[v]) {
                visited[v] = true;
                parent[v] = u;
                s.push(v);
            }
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    result.executionTimeUs = std::chrono::duration<double, std::micro>(endTime - startTime).count();

    if (!reached) {
        result.found = false;
        result.totalDistance = std::numeric_limits<double>::infinity();
        result.notes = "Disconnected Graph: Target is unreachable from origin.";
        return result;
    }

    // Path reconstruction
    std::vector<int> path;
    int curr = targetId;
    while (curr != startId) {
        path.push_back(curr);
        curr = parent[curr];
    }
    path.push_back(startId);
    std::reverse(path.begin(), path.end());

    double distSum = 0.0;
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        Edge edge(0, 0);
        if (graph.findEdge(path[i], path[i + 1], edge)) {
            distSum += edge.weight;
        }
    }

    result.found = true;
    result.path = std::move(path);
    result.totalDistance = distSum;
    result.hops = static_cast<int>(result.path.size() - 1);
    result.notes = "Exploratory path via deep branch traversal.";

    return result;
}

AlgorithmComparison PathFinder::compareAlgorithms(const CampusGraph& graph, int startId, int targetId) {
    AlgorithmComparison comp;
    comp.dijkstra = dijkstra(graph, startId, targetId);
    comp.bfs = bfs(graph, startId, targetId);
    comp.dfs = dfs(graph, startId, targetId);
    comp.isConnected = comp.dijkstra.found;
    return comp;
}

bool PathFinder::isReachable(const CampusGraph& graph, int startId, int targetId) {
    if (!graph.hasLocation(startId) || !graph.hasLocation(targetId)) return false;
    if (startId == targetId) return true;

    std::queue<int> q;
    std::unordered_map<int, bool> visited;
    visited[startId] = true;
    q.push(startId);

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        if (u == targetId) return true;

        for (const auto& edge : graph.getNeighbors(u)) {
            if (!visited[edge.to]) {
                visited[edge.to] = true;
                q.push(edge.to);
            }
        }
    }
    return false;
}

std::vector<std::vector<int>> PathFinder::getConnectedComponents(const CampusGraph& graph) {
    std::vector<std::vector<int>> components;
    std::unordered_map<int, bool> visited;

    for (int id : graph.getAllLocationIds()) {
        visited[id] = false;
    }

    for (int id : graph.getAllLocationIds()) {
        if (!visited[id]) {
            std::vector<int> component;
            std::queue<int> q;

            visited[id] = true;
            q.push(id);

            while (!q.empty()) {
                int u = q.front();
                q.pop();
                component.push_back(u);

                for (const auto& edge : graph.getNeighbors(u)) {
                    if (!visited[edge.to]) {
                        visited[edge.to] = true;
                        q.push(edge.to);
                    }
                }
            }

            std::sort(component.begin(), component.end());
            components.push_back(std::move(component));
        }
    }

    return components;
}

std::string PathFinder::getComplexityReport() {
    std::ostringstream oss;
    oss << "================================================================================\n"
        << "           ADSA THEORETICAL COMPLEXITY ANALYSIS & ALGORITHM COMPARISON           \n"
        << "================================================================================\n\n"
        << "1. GRAPH REPRESENTATION: ADJACENCY LIST\n"
        << "   - Space Complexity: O(|V| + |E|)\n"
        << "     Why: Stores an entry per vertex and exactly one Edge node per directed arc.\n"
        << "     Comparison: An Adjacency Matrix requires O(|V|^2) space regardless of density,\n"
        << "     wasting excessive memory on sparse campus road networks (|E| << |V|^2).\n"
        << "   - Vertex Lookup: O(1) average via hash map or direct indexing.\n"
        << "   - Out-Degree Traversal: O(deg(u)), optimal for graph search.\n\n"
        << "2. DIJKSTRA'S ALGORITHM WITH MIN-PRIORITY QUEUE\n"
        << "   - Time Complexity: O((|V| + |E|) log |V|)\n"
        << "     Breakdown:\n"
        << "       * |V| extract-min operations from binary heap: O(|V| log |V|)\n"
        << "       * |E| edge relaxation attempts pushing into heap: O(|E| log |V|)\n"
        << "     Why Min-Heap over Array O(|V|^2)?\n"
        << "       On sparse graphs where |E| = O(|V|), binary heap runs in O(|V| log |V|),\n"
        << "       which vastly outperforms linear scanning O(|V|^2).\n"
        << "   - Space Complexity: O(|V|)\n"
        << "     Stores distance array, parent array, visited flags, and up to |E| heap elements.\n"
        << "   - Precondition: All edge weights must be strictly non-negative (w >= 0).\n"
        << "     Failure mode with negative weights: Greedy choice assumption is violated,\n"
        << "     requiring Bellman-Ford (O(|V|*|E|)) or Johnson's algorithm instead.\n\n"
        << "3. BREADTH-FIRST SEARCH (BFS)\n"
        << "   - Time Complexity: O(|V| + |E|)\n"
        << "     Each vertex enqueued once; each edge traversed once.\n"
        << "   - Space Complexity: O(|V|)\n"
        << "     FIFO queue holds vertices at frontier; visited set tracks explored nodes.\n"
        << "   - ADSA Guarantee: BFS is mathematically proven to find the SHORTEST PATH in terms\n"
        << "     of hop count (minimum intersections / edges), but NOT physical distance on\n"
        << "     weighted graphs.\n\n"
        << "4. DEPTH-FIRST SEARCH (DFS)\n"
        << "   - Time Complexity: O(|V| + |E|)\n"
        << "   - Space Complexity: O(|V|)\n"
        << "     Call stack / LIFO stack depth up to |V| in degenerate or tree topologies.\n"
        << "   - ADSA Purpose: Connectivity testing, cycle detection, topological sorting, and\n"
        << "     connected component decomposition.\n\n"
        << "5. SUMMARY MATRIX\n"
        << "   Algorithm    | Time Complexity       | Space | Optimality Criterion\n"
        << "   -------------|-----------------------|-------|------------------------------------\n"
        << "   Dijkstra     | O((|V| + |E|) log |V|) | O(|V|)| Shortest total physical distance\n"
        << "   BFS          | O(|V| + |E|)          | O(|V|)| Fewest turns / minimum hop count\n"
        << "   DFS          | O(|V| + |E|)          | O(|V|)| First reachable path (exploratory)\n"
        << "================================================================================\n";
    return oss.str();
}

} // namespace campus
