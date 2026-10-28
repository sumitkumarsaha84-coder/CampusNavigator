#ifndef CAMPUS_NAVIGATOR_TERMINALUI_HPP
#define CAMPUS_NAVIGATOR_TERMINALUI_HPP

#include "CampusGraph.hpp"
#include "PathFinder.hpp"
#include "RouteResult.hpp"
#include <string>

namespace campus {

/**
 * @brief Interactive terminal interface for the Campus Navigator application.
 * 
 * Provides an ANSI-colored menu system, bulletproof input validation,
 * turn-by-turn route directions, algorithm benchmarking, and data modification.
 */
class TerminalUI {
public:
    explicit TerminalUI(CampusGraph& graph);

    /**
     * @brief Launches the main interactive REPL menu loop.
     */
    void run();

    // Menu Action Handlers
    void displayCampusMap() const;
    void handleFindShortestPath();
    void handleFindFewestHopsPath();
    void handleExploreDfsPath();
    void handleCompareAlgorithms();
    void handleSearchLocations() const;
    void handleAddLocation();
    void handleAddRoad();
    void handleCheckConnectivity() const;
    void handleDisplayComplexity() const;

    // Output & Formatting Utilities
    void printBanner() const;
    void printMenu() const;
    void printRouteResult(const RouteResult& result, const std::string& title) const;
    void printComparison(const AlgorithmComparison& comp) const;

    // Bulletproof Input Validation Helpers
    static int readInt(const std::string& prompt, int minVal, int maxVal);
    static double readDouble(const std::string& prompt, double minVal, double maxVal);
    static std::string readLine(const std::string& prompt, bool allowEmpty = false);
    static bool readBool(const std::string& prompt, bool defaultVal = true);
    int selectLocationPrompt(const std::string& rolePrompt) const;

private:
    CampusGraph& m_graph;
    bool m_useColor{true};

    // ANSI Colors
    static constexpr const char* RESET   = "\033[0m";
    static constexpr const char* BOLD    = "\033[1m";
    static constexpr const char* RED     = "\033[31m";
    static constexpr const char* GREEN   = "\033[32m";
    static constexpr const char* YELLOW  = "\033[33m";
    static constexpr const char* BLUE    = "\033[34m";
    static constexpr const char* MAGENTA = "\033[35m";
    static constexpr const char* CYAN    = "\033[36m";
    static constexpr const char* WHITE   = "\033[37m";

    static std::string formatMinutes(double meters);
};

} // namespace campus

#endif // CAMPUS_NAVIGATOR_TERMINALUI_HPP
