#include "TerminalUI.hpp"
#include "CampusData.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>
#include <cmath>

namespace campus {

TerminalUI::TerminalUI(CampusGraph& graph) : m_graph(graph) {}

std::string TerminalUI::formatMinutes(double meters) {
    // Average walking speed: 5.0 km/h = 83.33 m/min
    double minutes = meters / 83.33;
    if (minutes < 1.0) {
        int seconds = static_cast<int>(minutes * 60.0);
        return std::to_string(seconds) + " sec";
    }
    int mins = static_cast<int>(minutes);
    int secs = static_cast<int>((minutes - mins) * 60.0);
    if (secs == 0) return std::to_string(mins) + " min";
    return std::to_string(mins) + "m " + std::to_string(secs) + "s";
}

int TerminalUI::readInt(const std::string& prompt, int minVal, int maxVal) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) {
            return minVal;
        }
        if (line.empty()) continue;

        try {
            size_t idx = 0;
            int val = std::stoi(line, &idx);
            if (idx == line.length() && val >= minVal && val <= maxVal) {
                return val;
            }
        } catch (...) {
            // fallthrough to error message
        }
        std::cout << "\033[31m[!] Invalid input. Please enter an integer between " 
                  << minVal << " and " << maxVal << ".\033[0m\n";
    }
}

double TerminalUI::readDouble(const std::string& prompt, double minVal, double maxVal) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) {
            return minVal;
        }
        if (line.empty()) continue;

        try {
            size_t idx = 0;
            double val = std::stod(line, &idx);
            if (idx == line.length() && val >= minVal && val <= maxVal) {
                return val;
            }
        } catch (...) {
            // fallthrough
        }
        std::cout << "\033[31m[!] Invalid number. Please enter a value between " 
                  << minVal << " and " << maxVal << ".\033[0m\n";
    }
}

std::string TerminalUI::readLine(const std::string& prompt, bool allowEmpty) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) {
            return "";
        }
        // Trim leading and trailing whitespace
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            if (allowEmpty) return "";
            std::cout << "\033[31m[!] Input cannot be blank.\033[0m\n";
            continue;
        }
        size_t last = line.find_last_not_of(" \t\r\n");
        return line.substr(first, (last - first + 1));
    }
}

bool TerminalUI::readBool(const std::string& prompt, bool defaultVal) {
    std::string promptWithDefault = prompt + (defaultVal ? " [Y/n]: " : " [y/N]: ");
    std::string input = readLine(promptWithDefault, true);
    if (input.empty()) return defaultVal;
    char c = static_cast<char>(std::tolower(input[0]));
    return (c == 'y' || c == '1' || c == 't');
}

int TerminalUI::selectLocationPrompt(const std::string& rolePrompt) const {
    while (true) {
        std::cout << rolePrompt;
        std::string query;
        if (!std::getline(std::cin, query)) return -1;
        
        size_t first = query.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            std::cout << "\033[31m[!] Please enter a location name, building code, or ID.\033[0m\n";
            continue;
        }
        size_t last = query.find_last_not_of(" \t\r\n");
        query = query.substr(first, (last - first + 1));

        if (query == "?" || query == "list") {
            displayCampusMap();
            continue;
        }

        int id = m_graph.findLocationId(query);
        if (id != -1) {
            auto loc = m_graph.getLocation(id);
            if (loc) {
                std::cout << "\033[32m-> Selected: [" << loc->id << "] " << loc->name 
                          << " (" << loc->code << ")\033[0m\n";
            }
            return id;
        }

        // Try fuzzy search
        auto results = m_graph.searchLocations(query);
        if (results.empty()) {
            std::cout << "\033[31m[!] No matching campus location found for '" << query 
                      << "'. Type 'list' to view all landmarks.\033[0m\n";
        } else if (results.size() == 1) {
            std::cout << "\033[32m-> Auto-matched: [" << results[0].id << "] " 
                      << results[0].name << " (" << results[0].code << ")\033[0m\n";
            return results[0].id;
        } else {
            std::cout << "\033[33mMultiple matches found:\033[0m\n";
            for (const auto& r : results) {
                std::cout << "   [" << std::setw(2) << r.id << "] " << r.name 
                          << " (" << r.code << ", " << r.category << ")\n";
            }
            std::cout << "Please specify exact ID or full name.\n";
        }
    }
}

void TerminalUI::printBanner() const {
    std::cout << "\033[36m"
              << "================================================================================\n"
              << "       CAMPUS NAVIGATOR - SMART CAMPUS ROUTE FINDER & ADSA BENCHMARK            \n"
              << "       Advanced Data Structures & Algorithms | C++17 Graph Suite               \n"
              << "================================================================================\n"
              << "\033[0m";
}

void TerminalUI::printMenu() const {
    std::cout << "\n\033[1m=== ROUTE PLANNING & OPERATIONS MENU ===\033[0m\n"
              << "  \033[32m[1]\033[0m Find Shortest Route (Dijkstra - Minimum Walking Distance)\n"
              << "  \033[32m[2]\033[0m Find Fewest Turns Route (BFS - Minimum Intersections)\n"
              << "  \033[32m[3]\033[0m Explore Deep Alternative Route (DFS - Deep Traversal)\n"
              << "  \033[33m[4]\033[0m Multi-Algorithm Comparison Matrix (Dijkstra vs BFS vs DFS)\n"
              << "  \033[34m[5]\033[0m Search Campus Landmarks & Facilities\n"
              << "  \033[34m[6]\033[0m View Full Campus Map & Adjacency Directory\n"
              << "  \033[35m[7]\033[0m Add New Campus Location\n"
              << "  \033[35m[8]\033[0m Add New Road / Pathway Connection\n"
              << "  \033[31m[9]\033[0m Audit Campus Connectivity & Disconnected Subgraphs\n"
              << "  \033[36m[10]\033[0m ADSA Theoretical Complexity Analysis & Viva Reference\n"
              << "  \033[37m[0]\033[0m Exit Campus Navigator\n";
}

void TerminalUI::printRouteResult(const RouteResult& result, const std::string& title) const {
    std::cout << "\n\033[36m================================================================================\n"
              << "   " << title << "\n"
              << "   Algorithm: " << result.algorithmName << "\n"
              << "================================================================================\033[0m\n";

    if (!result.found) {
        std::cout << "\033[31m[X] NO ROUTE FOUND: " << result.notes << "\033[0m\n";
        auto srcLoc = m_graph.getLocation(result.startId);
        auto dstLoc = m_graph.getLocation(result.targetId);
        std::cout << "    From: " << (srcLoc ? srcLoc->name : "ID " + std::to_string(result.startId)) << "\n"
                  << "    To:   " << (dstLoc ? dstLoc->name : "ID " + std::to_string(result.targetId)) << "\n"
                  << "    Status: Destination is physically isolated / disconnected.\n";
        return;
    }

    auto startLoc = m_graph.getLocation(result.startId);
    auto endLoc   = m_graph.getLocation(result.targetId);

    std::cout << "Origin:      \033[1m" << (startLoc ? startLoc->name : "") << " (" 
              << (startLoc ? startLoc->code : "") << ")\033[0m\n"
              << "Destination: \033[1m" << (endLoc ? endLoc->name : "") << " (" 
              << (endLoc ? endLoc->code : "") << ")\033[0m\n"
              << "Total Walk:  \033[32m\033[1m" << std::fixed << std::setprecision(1) << result.totalDistance 
              << " meters (" << formatMinutes(result.totalDistance) << " walk)\033[0m\n"
              << "Segments:    " << result.hops << " pathways (" << result.path.size() << " waypoints)\n"
              << "Performance: Settled " << result.nodesVisited << " nodes, checked " 
              << result.edgeEvaluations << " edges in " << std::fixed << std::setprecision(1) 
              << result.executionTimeUs << " µs\n\n";

    // Turn-by-turn itinerary table
    std::cout << "\033[1m-----------------------------------------------------------------------------------------\n"
              << "Leg  From                  To                    Dist     Cumul    Pace     Pathway Type\n"
              << "-----------------------------------------------------------------------------------------\033[0m\n";

    double cumDist = 0.0;
    for (size_t i = 0; i + 1 < result.path.size(); ++i) {
        int u = result.path[i];
        int v = result.path[i + 1];
        auto uLoc = m_graph.getLocation(u);
        auto vLoc = m_graph.getLocation(v);
        Edge edge(0, 0);
        m_graph.findEdge(u, v, edge);

        cumDist += edge.weight;

        std::string uName = uLoc ? uLoc->name : ("ID " + std::to_string(u));
        std::string vName = vLoc ? vLoc->name : ("ID " + std::to_string(v));
        if (uName.length() > 21) uName = uName.substr(0, 18) + "...";
        if (vName.length() > 21) vName = vName.substr(0, 18) + "...";

        std::cout << std::left << std::setw(5) << (i + 1)
                  << std::setw(22) << uName
                  << std::setw(22) << vName
                  << std::right << std::setw(6) << std::fixed << std::setprecision(0) << edge.weight << "m "
                  << std::setw(7) << std::fixed << std::setprecision(0) << cumDist << "m  "
                  << std::setw(7) << formatMinutes(edge.weight) << "  "
                  << std::left << edge.roadType
                  << (edge.accessible ? " [Wheelchair OK]" : "")
                  << "\n";
    }
    std::cout << "-----------------------------------------------------------------------------------------\n"
              << "Summary: " << result.notes << "\n";
}

void TerminalUI::printComparison(const AlgorithmComparison& comp) const {
    std::cout << "\n\033[33m================================================================================\n"
              << "          MULTI-ALGORITHM BENCHMARK MATRIX (SAME SOURCE & DESTINATION)          \n"
              << "================================================================================\033[0m\n";

    auto srcLoc = m_graph.getLocation(comp.dijkstra.startId);
    auto dstLoc = m_graph.getLocation(comp.dijkstra.targetId);

    std::cout << "Origin:      " << (srcLoc ? srcLoc->name : "") << "\n"
              << "Destination: " << (dstLoc ? dstLoc->name : "") << "\n\n";

    std::cout << std::left << std::setw(24) << "Evaluation Metric"
              << std::setw(20) << "Dijkstra (Min-Heap)"
              << std::setw(18) << "BFS (Fewest Hops)"
              << std::setw(18) << "DFS (Exploratory)"
              << "\n--------------------------------------------------------------------------------\n";

    // Path status
    std::cout << std::left << std::setw(24) << "Path Status"
              << std::setw(20) << (comp.dijkstra.found ? "\033[32mOptimal\033[0m" : "\033[31mUnreachable\033[0m")
              << std::setw(20) << (comp.bfs.found ? "\033[32mMin Hops\033[0m" : "\033[31mUnreachable\033[0m")
              << std::setw(20) << (comp.dfs.found ? "\033[32mFound\033[0m" : "\033[31mUnreachable\033[0m")
              << "\n";

    if (!comp.isConnected) {
        std::cout << "\n\033[31mDestination is not connected to source in the campus graph.\033[0m\n";
        return;
    }

    // Total distance
    std::ostringstream dStr, bStr, fStr;
    dStr << std::fixed << std::setprecision(1) << comp.dijkstra.totalDistance << " m";
    bStr << std::fixed << std::setprecision(1) << comp.bfs.totalDistance << " m";
    fStr << std::fixed << std::setprecision(1) << comp.dfs.totalDistance << " m";

    std::cout << std::left << std::setw(24) << "Physical Distance"
              << std::setw(20) << dStr.str()
              << std::setw(18) << bStr.str()
              << std::setw(18) << fStr.str()
              << "\n";

    // Hop count
    std::cout << std::left << std::setw(24) << "Hop Count (Intersections)"
              << std::setw(20) << std::to_string(comp.dijkstra.hops) + " edges"
              << std::setw(18) << std::to_string(comp.bfs.hops) + " edges"
              << std::setw(18) << std::to_string(comp.dfs.hops) + " edges"
              << "\n";

    // Nodes visited
    std::cout << std::left << std::setw(24) << "Vertices Expanded"
              << std::setw(20) << comp.dijkstra.nodesVisited
              << std::setw(18) << comp.bfs.nodesVisited
              << std::setw(18) << comp.dfs.nodesVisited
              << "\n";

    // Edge checks
    std::cout << std::left << std::setw(24) << "Edges Evaluated"
              << std::setw(20) << comp.dijkstra.edgeEvaluations
              << std::setw(18) << comp.bfs.edgeEvaluations
              << std::setw(18) << comp.dfs.edgeEvaluations
              << "\n";

    // Execution time
    std::ostringstream dt, bt, ft;
    dt << std::fixed << std::setprecision(1) << comp.dijkstra.executionTimeUs << " µs";
    bt << std::fixed << std::setprecision(1) << comp.bfs.executionTimeUs << " µs";
    ft << std::fixed << std::setprecision(1) << comp.dfs.executionTimeUs << " µs";

    std::cout << std::left << std::setw(24) << "Wall-Clock Duration"
              << std::setw(20) << dt.str()
              << std::setw(18) << bt.str()
              << std::setw(18) << ft.str()
              << "\n";

    std::cout << "--------------------------------------------------------------------------------\n";

    // Paths
    auto printPathSequence = [this](const std::vector<int>& path) {
        for (size_t i = 0; i < path.size(); ++i) {
            auto loc = m_graph.getLocation(path[i]);
            std::cout << (loc ? loc->code : std::to_string(path[i]));
            if (i + 1 < path.size()) std::cout << " -> ";
        }
        std::cout << "\n";
    };

    std::cout << "Dijkstra Path : "; printPathSequence(comp.dijkstra.path);
    std::cout << "BFS Path      : "; printPathSequence(comp.bfs.path);
    std::cout << "DFS Path      : "; printPathSequence(comp.dfs.path);

    std::cout << "\n\033[1mADSA Analysis & Insights:\033[0m\n";
    if (comp.dijkstra.totalDistance < comp.bfs.totalDistance) {
        double saved = comp.bfs.totalDistance - comp.dijkstra.totalDistance;
        std::cout << "* \033[32mDijkstra saved " << std::fixed << std::setprecision(1) 
                  << saved << " meters\033[0m compared to BFS by choosing a path with "
                  << "more intersections but lower cumulative physical weight.\n";
    } else if (comp.dijkstra.totalDistance == comp.bfs.totalDistance) {
        std::cout << "* Both Dijkstra and BFS identified identical physical distance routes.\n";
    }

    if (comp.bfs.hops < comp.dijkstra.hops) {
        std::cout << "* \033[33mBFS found a route with " << (comp.dijkstra.hops - comp.bfs.hops)
                  << " fewer turns/intersections\033[0m, making it simpler to navigate despite higher meterage.\n";
    }

    std::cout << "* DFS explored deeply down its branch, taking " << comp.dfs.hops 
              << " hops (" << std::fixed << std::setprecision(1) << comp.dfs.totalDistance 
              << " m), showing why DFS is designed for connectivity rather than shortest path.\n";
}

void TerminalUI::displayCampusMap() const {
    std::cout << "\n\033[34m================================================================================\n"
              << "                  CAMPUS MAP DIRECTORY & ADJACENCY OVERVIEW                     \n"
              << "================================================================================\033[0m\n";

    auto locs = m_graph.getAllLocations();
    std::cout << "Total Landmarks: " << locs.size() 
              << " | Total Directed Pathways: " << m_graph.getDirectedEdgeCount() << "\n\n";

    for (const auto& loc : locs) {
        std::cout << "\033[1m[" << std::setw(2) << loc.id << "] " << loc.name 
                  << " (" << loc.code << ", " << loc.category << ")\033[0m\n"
                  << "     " << loc.description << "\n";

        const auto& neighbors = m_graph.getNeighbors(loc.id);
        if (neighbors.empty()) {
            std::cout << "     \033[31m[!] No direct outgoing pathways (Isolated landmark)\033[0m\n";
        } else {
            std::cout << "     Connected pathways:\n";
            for (const auto& edge : neighbors) {
                auto targetLoc = m_graph.getLocation(edge.to);
                std::string targetName = targetLoc ? targetLoc->name : ("ID " + std::to_string(edge.to));
                std::cout << "       -> [" << std::setw(2) << edge.to << "] " 
                          << std::left << std::setw(30) << targetName
                          << " : " << std::right << std::setw(5) << std::fixed << std::setprecision(0) 
                          << edge.weight << "m (" << edge.roadType << ")\n";
            }
        }
        std::cout << "\n";
    }
}

void TerminalUI::handleFindShortestPath() {
    std::cout << "\n\033[1m--- FIND SHORTEST ROUTE (DIJKSTRA'S ALGORITHM) ---\033[0m\n";
    int startId = selectLocationPrompt("Enter Starting Location (ID / Name / Code, or 'list'): ");
    if (startId == -1) return;
    int targetId = selectLocationPrompt("Enter Destination Location (ID / Name / Code, or 'list'): ");
    if (targetId == -1) return;

    RouteResult result = PathFinder::dijkstra(m_graph, startId, targetId);
    printRouteResult(result, "DIJKSTRA MINIMUM PHYSICAL DISTANCE ROUTE");
}

void TerminalUI::handleFindFewestHopsPath() {
    std::cout << "\n\033[1m--- FIND FEWEST TURNS ROUTE (BREADTH-FIRST SEARCH) ---\033[0m\n";
    int startId = selectLocationPrompt("Enter Starting Location (ID / Name / Code, or 'list'): ");
    if (startId == -1) return;
    int targetId = selectLocationPrompt("Enter Destination Location (ID / Name / Code, or 'list'): ");
    if (targetId == -1) return;

    RouteResult result = PathFinder::bfs(m_graph, startId, targetId);
    printRouteResult(result, "BFS MINIMUM INTERSECTIONS / FEWEST HOPS ROUTE");
}

void TerminalUI::handleExploreDfsPath() {
    std::cout << "\n\033[1m--- EXPLORE DEEP ROUTE (DEPTH-FIRST SEARCH) ---\033[0m\n";
    int startId = selectLocationPrompt("Enter Starting Location (ID / Name / Code, or 'list'): ");
    if (startId == -1) return;
    int targetId = selectLocationPrompt("Enter Destination Location (ID / Name / Code, or 'list'): ");
    if (targetId == -1) return;

    RouteResult result = PathFinder::dfs(m_graph, startId, targetId);
    printRouteResult(result, "DFS EXPLORATORY DEEP TRAVERSAL ROUTE");
}

void TerminalUI::handleCompareAlgorithms() {
    std::cout << "\n\033[1m--- MULTI-ALGORITHM BENCHMARK COMPARISON ---\033[0m\n";
    int startId = selectLocationPrompt("Enter Starting Location (ID / Name / Code, or 'list'): ");
    if (startId == -1) return;
    int targetId = selectLocationPrompt("Enter Destination Location (ID / Name / Code, or 'list'): ");
    if (targetId == -1) return;

    AlgorithmComparison comp = PathFinder::compareAlgorithms(m_graph, startId, targetId);
    printComparison(comp);
}

void TerminalUI::handleSearchLocations() const {
    std::cout << "\n\033[1m--- SEARCH CAMPUS LOCATIONS ---\033[0m\n";
    std::string query = readLine("Enter search term (name, code, or keyword): ");
    auto results = m_graph.searchLocations(query);

    if (results.empty()) {
        std::cout << "\033[31m[!] No locations found matching '" << query << "'.\033[0m\n";
        return;
    }

    std::cout << "\nFound " << results.size() << " match(es):\n"
              << "--------------------------------------------------------------------------------\n";
    for (const auto& loc : results) {
        std::cout << "ID: " << std::setw(2) << loc.id 
                  << " | " << std::left << std::setw(34) << loc.name
                  << " | Code: " << std::setw(8) << loc.code 
                  << " | " << loc.category << "\n"
                  << "    " << loc.description << "\n";
    }
    std::cout << "--------------------------------------------------------------------------------\n";
}

void TerminalUI::handleAddLocation() {
    std::cout << "\n\033[1m--- ADD NEW CAMPUS LOCATION ---\033[0m\n";
    int nextId = m_graph.getNextAvailableId();
    std::cout << "Suggested Location ID: " << nextId << "\n";
    int id = readInt("Enter ID (" + std::to_string(nextId) + " or custom positive integer): ", 1, 99999);

    if (m_graph.hasLocation(id)) {
        std::cout << "\033[31m[!] Error: Location ID " << id << " already exists.\033[0m\n";
        return;
    }

    std::string name = readLine("Enter Location Name (e.g. Chemical Engineering Wing): ");
    if (m_graph.hasLocation(name)) {
        std::cout << "\033[31m[!] Error: A location with name '" << name << "' already exists.\033[0m\n";
        return;
    }

    std::string code = readLine("Enter Building Code (e.g. CHEM-01): ");
    std::string category = readLine("Enter Category (e.g. Academic, Hostel, Dining, Sports): ");
    std::string desc = readLine("Enter Brief Description: ");

    Location newLoc(id, name, code, category, desc);
    if (m_graph.addLocation(newLoc)) {
        std::cout << "\033[32m[✓] Location successfully added: [" << id << "] " << name << "\033[0m\n";
    } else {
        std::cout << "\033[31m[!] Failed to add location.\033[0m\n";
    }
}

void TerminalUI::handleAddRoad() {
    std::cout << "\n\033[1m--- ADD NEW ROAD / PATHWAY CONNECTION ---\033[0m\n";
    int fromId = selectLocationPrompt("Enter Starting Location (ID / Name / Code): ");
    if (fromId == -1) return;
    int toId = selectLocationPrompt("Enter Ending Location (ID / Name / Code): ");
    if (toId == -1) return;

    if (fromId == toId) {
        std::cout << "\033[31m[!] Self-loops are not permitted in campus navigation.\033[0m\n";
        return;
    }

    double dist = readDouble("Enter Pathway Walking Distance in meters (1.0 to 10000.0): ", 1.0, 10000.0);
    std::string roadType = readLine("Enter Pathway Type (e.g. Paved Walkway, Main Avenue, Covered Trail): ");
    bool bidirectional = readBool("Is this path bidirectional (walkable in both directions)?", true);
    bool accessible = readBool("Is this path wheelchair / barrier-free accessible?", true);

    if (m_graph.addEdge(fromId, toId, dist, roadType, bidirectional, accessible)) {
        std::cout << "\033[32m[✓] Road connection added successfully!\033[0m\n";
    } else {
        std::cout << "\033[31m[!] Failed to add road connection.\033[0m\n";
    }
}

void TerminalUI::handleCheckConnectivity() const {
    std::cout << "\n\033[31m================================================================================\n"
              << "          CAMPUS CONNECTIVITY AUDIT & DISCONNECTED GRAPH ANALYSIS               \n"
              << "================================================================================\033[0m\n";

    auto components = PathFinder::getConnectedComponents(m_graph);
    std::cout << "Identified " << components.size() << " connected component(s) across campus:\n\n";

    for (size_t i = 0; i < components.size(); ++i) {
        bool isPrimary = (components[i].size() > 1 && i == 0);
        std::cout << "\033[1mComponent #" << (i + 1) << " (" << components[i].size() << " locations) "
                  << (isPrimary ? "\033[32m[MAIN INTERCONNECTED CAMPUS]\033[0m" : "\033[31m[DISCONNECTED / ISOLATED SUBGRAPH]\033[0m")
                  << "\033[0m\n";

        for (int id : components[i]) {
            auto loc = m_graph.getLocation(id);
            std::cout << "   - [" << std::setw(2) << id << "] " 
                      << (loc ? loc->name : "ID " + std::to_string(id))
                      << " (" << (loc ? loc->code : "") << ")\n";
        }
        std::cout << "\n";
    }

    if (components.size() > 1) {
        std::cout << "\033[33mADSA Diagnostic Notice:\033[0m\n"
                  << "Campus graph is DISCONNECTED. Routing queries across differing components\n"
                  << "will correctly yield 'Destination Unreachable' with infinite distance.\n";
    } else {
        std::cout << "\033[32m[✓] Full Campus Connectivity Verified: All landmarks mutually reachable.\033[0m\n";
    }
}

void TerminalUI::handleDisplayComplexity() const {
    std::cout << PathFinder::getComplexityReport() << "\n";
}

void TerminalUI::run() {
    printBanner();

    while (true) {
        printMenu();
        int choice = readInt("\nSelect an option [0-10]: ", 0, 10);

        switch (choice) {
            case 1: handleFindShortestPath(); break;
            case 2: handleFindFewestHopsPath(); break;
            case 3: handleExploreDfsPath(); break;
            case 4: handleCompareAlgorithms(); break;
            case 5: handleSearchLocations(); break;
            case 6: displayCampusMap(); break;
            case 7: handleAddLocation(); break;
            case 8: handleAddRoad(); break;
            case 9: handleCheckConnectivity(); break;
            case 10: handleDisplayComplexity(); break;
            case 0:
                std::cout << "\n\033[36mThank you for using Campus Navigator! Have a safe walk.\033[0m\n";
                return;
            default:
                break;
        }

        std::cout << "\nPress Enter to return to main menu...";
        std::string dummy;
        std::getline(std::cin, dummy);
    }
}

} // namespace campus
