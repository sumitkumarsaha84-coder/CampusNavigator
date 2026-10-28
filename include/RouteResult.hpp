#ifndef CAMPUS_NAVIGATOR_ROUTERESULT_HPP
#define CAMPUS_NAVIGATOR_ROUTERESULT_HPP

#include <vector>
#include <string>

namespace campus {

/**
 * @brief Encapsulates the complete result of running a pathfinding algorithm.
 */
struct RouteResult {
    bool found{false};                  // True if a valid path connects source and target
    int startId{-1};                    // Origin location ID
    int targetId{-1};                   // Target location ID
    std::vector<int> path;              // In-order sequence of location IDs from start to target
    double totalDistance{0.0};          // Total path distance in meters (0 for disconnected)
    int hops{0};                        // Number of edges traversed
    size_t nodesVisited{0};             // Total vertices settled / expanded during traversal
    size_t edgeEvaluations{0};          // Total edge relaxations / checks performed
    double executionTimeUs{0.0};        // Wall-clock search duration in microseconds
    std::string algorithmName;          // e.g., "Dijkstra (Min-Heap)", "BFS", "DFS"
    std::string notes;                  // Additional execution remarks or edge case flags
};

} // namespace campus

#endif // CAMPUS_NAVIGATOR_ROUTERESULT_HPP
