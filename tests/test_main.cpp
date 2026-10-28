#include "CampusGraph.hpp"
#include "CampusData.hpp"
#include "PathFinder.hpp"
#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include <vector>

// Lightweight assertions framework
static int g_testsRun = 0;
static int g_testsPassed = 0;

#define TEST_CASE(name) void name(); \
    struct Register_##name { \
        Register_##name() { \
            std::cout << "[RUNNING] " << #name << " ... "; \
            g_testsRun++; \
            try { \
                name(); \
                g_testsPassed++; \
                std::cout << "\033[32mPASSED\033[0m\n"; \
            } catch (const std::exception& e) { \
                std::cout << "\033[31mFAILED: " << e.what() << "\033[0m\n"; \
            } catch (...) { \
                std::cout << "\033[31mFAILED: Unknown exception\033[0m\n"; \
            } \
        } \
    } instance_##name; \
    void name()

#define ASSERT_TRUE(cond) \
    if (!(cond)) { \
        throw std::runtime_error("Assertion failed: " #cond " at line " + std::to_string(__LINE__)); \
    }

#define ASSERT_FALSE(cond) \
    if (cond) { \
        throw std::runtime_error("Assertion failed: NOT(" #cond ") at line " + std::to_string(__LINE__)); \
    }

#define ASSERT_EQ(val1, val2) \
    if ((val1) != (val2)) { \
        throw std::runtime_error("Equality assertion failed at line " + std::to_string(__LINE__)); \
    }

#define ASSERT_NEAR(val1, val2, eps) \
    if (std::abs((val1) - (val2)) > (eps)) { \
        throw std::runtime_error("Near equality assertion failed: |" + std::to_string(val1) + " - " + \
                                 std::to_string(val2) + "| > " + std::to_string(eps) + " at line " + std::to_string(__LINE__)); \
    }

using namespace campus;

// -----------------------------------------------------------------------------
// TEST SUITES
// -----------------------------------------------------------------------------

TEST_CASE(Test_Graph_Vertex_And_Edge_Validation) {
    CampusGraph g;
    // Valid location addition
    ASSERT_TRUE(g.addLocation(Location(1, "Gate 1", "G1", "Entrance")));
    ASSERT_TRUE(g.addLocation(Location(2, "Hall A", "HA", "Academic")));
    ASSERT_EQ(g.getVertexCount(), 2);

    // Reject duplicate ID
    ASSERT_FALSE(g.addLocation(Location(1, "Different Name", "G1-B", "Entrance")));

    // Reject duplicate Name (case-insensitive)
    ASSERT_FALSE(g.addLocation(Location(3, "gate 1", "G1-C", "Entrance")));

    // Reject negative or empty
    ASSERT_FALSE(g.addLocation(Location(-5, "Negative ID", "N1", "Entrance")));
    ASSERT_FALSE(g.addLocation(Location(4, "", "EMPTY", "Entrance")));

    // Edge validations: valid bidirectional
    ASSERT_TRUE(g.addEdge(1, 2, 150.0, "Paved Walkway", true));
    ASSERT_EQ(g.getDirectedEdgeCount(), 2); // 1->2 and 2->1

    // Check neighbors
    const auto& n1 = g.getNeighbors(1);
    ASSERT_EQ(n1.size(), 1);
    ASSERT_EQ(n1[0].to, 2);
    ASSERT_NEAR(n1[0].weight, 150.0, 0.001);

    const auto& n2 = g.getNeighbors(2);
    ASSERT_EQ(n2.size(), 1);
    ASSERT_EQ(n2[0].to, 1);

    // Reject edge with invalid weight (<= 0)
    ASSERT_FALSE(g.addEdge(1, 2, 0.0));
    ASSERT_FALSE(g.addEdge(1, 2, -10.0));

    // Reject self-loop
    ASSERT_FALSE(g.addEdge(1, 1, 50.0));

    // Reject edge referencing non-existent vertices
    ASSERT_FALSE(g.addEdge(1, 999, 100.0));
    ASSERT_FALSE(g.addEdge(888, 2, 100.0));
}

TEST_CASE(Test_Dijkstra_Shortest_Path_Accuracy) {
    CampusGraph g;
    CampusData::loadDefaultCampus(g);

    // Test route from Main North Gate (1) to Student Centre (8)
    // Note:
    // Direct path: 1 -> 8 (Perimeter Express Road) is 600m (1 hop)
    // Path via Admin and Auditorium: 1 -> 2 (120m) + 2 -> 7 (180m) + 7 -> 8 (120m) = 420m (3 hops)!
    // Dijkstra must select the 420m path!
    RouteResult res = PathFinder::dijkstra(g, 1, 8);
    ASSERT_TRUE(res.found);
    ASSERT_NEAR(res.totalDistance, 420.0, 0.01);
    ASSERT_EQ(res.hops, 3);
    ASSERT_EQ(res.path.size(), 4);
    ASSERT_EQ(res.path[0], 1);
    ASSERT_EQ(res.path[1], 2);
    ASSERT_EQ(res.path[2], 7);
    ASSERT_EQ(res.path[3], 8);

    // Test route from North Gate (1) to Physics Labs (6)
    // 1 -> 4 (280) -> 6 (130) = 410m (2 hops)
    // 1 -> 2 (120) -> 4 (160) -> 5 (90) -> 6 (80) = 450m (4 hops)
    // 1 -> 4 (280) -> 5 (90) -> 6 (80) = 450m (3 hops)
    // 1 -> 2 (120) -> 4 (160) -> 6 (130) = 410m (3 hops)
    // Optimal distance is 410m!
    RouteResult res2 = PathFinder::dijkstra(g, 1, 6);
    ASSERT_TRUE(res2.found);
    ASSERT_NEAR(res2.totalDistance, 410.0, 0.01);

    // Test identical start and end
    RouteResult same = PathFinder::dijkstra(g, 4, 4);
    ASSERT_TRUE(same.found);
    ASSERT_NEAR(same.totalDistance, 0.0, 0.001);
    ASSERT_EQ(same.hops, 0);
    ASSERT_EQ(same.path.size(), 1);
}

TEST_CASE(Test_BFS_Minimum_Hop_Count_Contrasted_With_Dijkstra) {
    CampusGraph g;
    CampusData::loadDefaultCampus(g);

    // On query 1 -> 8:
    // Dijkstra chose 3 hops (420m).
    // BFS MUST choose the 1-hop path 1 -> 8 (600m)!
    RouteResult bfsRes = PathFinder::bfs(g, 1, 8);
    ASSERT_TRUE(bfsRes.found);
    ASSERT_EQ(bfsRes.hops, 1);
    ASSERT_EQ(bfsRes.path.size(), 2);
    ASSERT_EQ(bfsRes.path[0], 1);
    ASSERT_EQ(bfsRes.path[1], 8);
    ASSERT_NEAR(bfsRes.totalDistance, 600.0, 0.01);

    // Verify BFS hop count is strictly less than Dijkstra's hop count here
    RouteResult dijRes = PathFinder::dijkstra(g, 1, 8);
    ASSERT_TRUE(bfsRes.hops < dijRes.hops);
    ASSERT_TRUE(dijRes.totalDistance < bfsRes.totalDistance);
}

TEST_CASE(Test_DFS_Path_Validity) {
    CampusGraph g;
    CampusData::loadDefaultCampus(g);

    RouteResult dfsRes = PathFinder::dfs(g, 1, 13); // From North Gate (1) to South Gate (13)
    ASSERT_TRUE(dfsRes.found);
    ASSERT_TRUE(dfsRes.path.size() >= 2);
    ASSERT_EQ(dfsRes.path.front(), 1);
    ASSERT_EQ(dfsRes.path.back(), 13);

    // Ensure every hop in the path is a valid physical edge in graph
    for (size_t i = 0; i + 1 < dfsRes.path.size(); ++i) {
        Edge edge(0, 0);
        ASSERT_TRUE(g.findEdge(dfsRes.path[i], dfsRes.path[i + 1], edge));
    }
}

TEST_CASE(Test_Disconnected_Graph_Handling) {
    CampusGraph g;
    CampusData::loadDefaultCampus(g);

    // Location 16 (High-Altitude Space Observatory) is completely disconnected!
    ASSERT_FALSE(PathFinder::isReachable(g, 1, 16));
    ASSERT_FALSE(PathFinder::isReachable(g, 8, 16));

    // Dijkstra to disconnected node
    RouteResult dijRes = PathFinder::dijkstra(g, 1, 16);
    ASSERT_FALSE(dijRes.found);
    ASSERT_TRUE(std::isinf(dijRes.totalDistance));
    ASSERT_TRUE(dijRes.path.empty());

    // BFS to disconnected node
    RouteResult bfsRes = PathFinder::bfs(g, 1, 16);
    ASSERT_FALSE(bfsRes.found);
    ASSERT_TRUE(std::isinf(bfsRes.totalDistance));

    // DFS to disconnected node
    RouteResult dfsRes = PathFinder::dfs(g, 1, 16);
    ASSERT_FALSE(dfsRes.found);
    ASSERT_TRUE(std::isinf(dfsRes.totalDistance));
}

TEST_CASE(Test_Connected_Components_Analysis) {
    CampusGraph g;
    CampusData::loadDefaultCampus(g);

    auto components = PathFinder::getConnectedComponents(g);
    // Should have exactly 2 components: Main Campus (15 nodes) and Observatory (1 node)
    ASSERT_EQ(components.size(), 2);

    size_t mainSize = components[0].size();
    size_t isolatedSize = components[1].size();
    if (mainSize == 1) std::swap(mainSize, isolatedSize);

    ASSERT_EQ(mainSize, 15);
    ASSERT_EQ(isolatedSize, 1);
}

TEST_CASE(Test_Dynamic_Topology_Add_Road_And_Reroute) {
    CampusGraph g;
    CampusData::loadDefaultCampus(g);

    // Initially 16 is disconnected
    ASSERT_FALSE(PathFinder::isReachable(g, 15, 16));

    // Add a new mountain trail connecting Lakeside Pavillion (15) to Observatory (16)
    ASSERT_TRUE(g.addEdge(15, 16, 450.0, "Observatory Hill Trail", true));

    // Now components count must drop from 2 to 1!
    auto components = PathFinder::getConnectedComponents(g);
    ASSERT_EQ(components.size(), 1);
    ASSERT_EQ(components[0].size(), 16);

    // Now 16 is reachable from Main North Gate (1)!
    ASSERT_TRUE(PathFinder::isReachable(g, 1, 16));
    RouteResult res = PathFinder::dijkstra(g, 1, 16);
    ASSERT_TRUE(res.found);
    // Path: 1 -> 2 (120) -> 7 (180) -> 15 (230) -> 16 (450) = 980m
    ASSERT_NEAR(res.totalDistance, 980.0, 0.01);
    ASSERT_EQ(res.path.back(), 16);

    // Test edge removal
    ASSERT_TRUE(g.removeEdge(15, 16, true));
    ASSERT_FALSE(PathFinder::isReachable(g, 1, 16));
}

TEST_CASE(Test_Location_Search_And_Autocomplete) {
    CampusGraph g;
    CampusData::loadDefaultCampus(g);

    // Lookup by exact ID string
    ASSERT_EQ(g.findLocationId("4"), 4);

    // Lookup by code
    ASSERT_EQ(g.findLocationId("CS-01"), 4);
    ASSERT_EQ(g.findLocationId("lib-01"), 3);

    // Lookup by case-insensitive name
    ASSERT_EQ(g.findLocationId("turing computer science department"), 4);
    ASSERT_EQ(g.findLocationId("MAIN NORTH GATE"), 1);

    // Category search
    auto academicLocs = g.searchLocations("Academic");
    ASSERT_TRUE(academicLocs.size() >= 4);

    // Hostel search
    auto hostels = g.searchLocations("Hostel");
    ASSERT_EQ(hostels.size(), 2);

    // Non-existent search
    auto missing = g.searchLocations("QuantumTeleportationStation");
    ASSERT_TRUE(missing.empty());
}

// -----------------------------------------------------------------------------
// MAIN TEST DRIVER
// -----------------------------------------------------------------------------
int main() {
    std::cout << "================================================================================\n"
              << "       CAMPUS NAVIGATOR - ADSA GRAPH ALGORITHMS VERIFICATION SUITE              \n"
              << "================================================================================\n\n";

    std::cout << "\n================================================================================\n";
    std::cout << "TEST EXECUTION SUMMARY: " << g_testsPassed << " / " << g_testsRun << " SUITES PASSED.\n";
    if (g_testsPassed == g_testsRun) {
        std::cout << "\033[32mALL UNIT & INTEGRATION TESTS COMPLETED SUCCESSFULLY!\033[0m\n";
    } else {
        std::cout << "\033[31mSOME TESTS FAILED!\033[0m\n";
    }
    std::cout << "================================================================================\n";

    return (g_testsPassed == g_testsRun) ? 0 : 1;
}
