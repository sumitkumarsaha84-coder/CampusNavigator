#ifndef CAMPUS_NAVIGATOR_PATHFINDER_HPP
#define CAMPUS_NAVIGATOR_PATHFINDER_HPP

#include "CampusGraph.hpp"
#include "RouteResult.hpp"
#include <vector>
#include <string>

namespace campus {

/**
 * @brief Side-by-side comparison matrix for Dijkstra, BFS, and DFS on the same query.
 */
struct AlgorithmComparison {
    RouteResult dijkstra;
    RouteResult bfs;
    RouteResult dfs;
    bool isConnected{false};
};

/**
 * @brief Implements core ADSA graph algorithms: Dijkstra with Min-Priority Queue, BFS, DFS,
 *        and Connected Component analysis.
 */
class PathFinder {
public:
    /**
     * @brief Finds the shortest weighted path using Dijkstra's Algorithm with a min-priority queue.
     * 
     * Time Complexity: O((|V| + |E|) log |V|) using binary heap (std::priority_queue)
     * Space Complexity: O(|V|) for distance map, parent map, and priority queue
     * 
     * @param graph Campus graph with positive edge weights.
     * @param startId Origin vertex ID.
     * @param targetId Destination vertex ID.
     * @return RouteResult containing optimal path, total meters, hops, and metrics.
     */
    static RouteResult dijkstra(const CampusGraph& graph, int startId, int targetId);

    /**
     * @brief Finds the path with minimum number of intersections / hops using Breadth-First Search.
     * 
     * Note: In an unweighted graph, BFS finds the shortest path. In a weighted graph, it finds
     * the path with fewest edges, which may not be the path with lowest physical distance.
     * 
     * Time Complexity: O(|V| + |E|)
     * Space Complexity: O(|V|)
     */
    static RouteResult bfs(const CampusGraph& graph, int startId, int targetId);

    /**
     * @brief Explores a path from start to target using Depth-First Search.
     * 
     * Explores as far as possible along each branch before backtracking. Does not guarantee
     * shortest distance or fewest hops, serving as an educational contrast.
     * 
     * Time Complexity: O(|V| + |E|)
     * Space Complexity: O(|V|) for recursion stack / explicit stack and visited set
     */
    static RouteResult dfs(const CampusGraph& graph, int startId, int targetId);

    /**
     * @brief Evaluates all three algorithms on identical source and target vertices.
     */
    static AlgorithmComparison compareAlgorithms(const CampusGraph& graph, int startId, int targetId);

    /**
     * @brief Checks if a path exists between start and target vertices.
     */
    static bool isReachable(const CampusGraph& graph, int startId, int targetId);

    /**
     * @brief Identifies all connected components in the campus graph.
     *        Essential for diagnosing disconnected campus zones or isolated buildings.
     * @return List of components, where each component is a list of location IDs.
     */
    static std::vector<std::vector<int>> getConnectedComponents(const CampusGraph& graph);

    /**
     * @brief Returns comprehensive theoretical complexity breakdown and design rationales.
     */
    static std::string getComplexityReport();
};

} // namespace campus

#endif // CAMPUS_NAVIGATOR_PATHFINDER_HPP
