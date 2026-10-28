#ifndef CAMPUS_NAVIGATOR_HTTPSERVER_HPP
#define CAMPUS_NAVIGATOR_HTTPSERVER_HPP

#include "CampusGraph.hpp"
#include "RouteResult.hpp"
#include <string>
#include <atomic>
#include <cstdint>

namespace campus {

/**
 * @brief Lightweight, zero-dependency embedded C++17 HTTP/JSON Server for Campus Navigator.
 * 
 * Directly interfaces with the in-memory CampusGraph and PathFinder algorithms,
 * ensuring C++ remains the single source of truth for all graph data structures
 * and ADSA pathfinding computations.
 */
class HttpServer {
public:
    /**
     * @brief Constructs the HTTP server with reference to the campus graph.
     * @param graph Reference to live campus graph instance.
     * @param port TCP listening port (default: 8080).
     * @param webRoot Path to directory containing static frontend assets.
     */
    explicit HttpServer(CampusGraph& graph, int port = 8080, std::string webRoot = "web");
    ~HttpServer();

    /**
     * @brief Starts server listening and handling connections.
     * @param blocking If true, runs synchronously on calling thread.
     */
    bool start(bool blocking = true);

    /**
     * @brief Gracefully terminates server loop and closes sockets.
     */
    void stop();

    int getPort() const noexcept { return m_port; }
    bool isRunning() const noexcept { return m_running; }

    static std::string urlDecode(const std::string& in);
    static std::string jsonEscape(const std::string& in);
    static std::string getMimeType(const std::string& filepath);

private:
    CampusGraph& m_graph;
    int m_port;
    std::string m_webRoot;
    std::atomic<bool> m_running{false};
    std::uintptr_t m_listenSocket{0};

    void runLoop();
    void handleConnection(std::uintptr_t clientSock);

    std::string routeRequest(const std::string& method, const std::string& path,
                             const std::string& query, const std::string& body,
                             std::string& outContentType, int& outStatus);

    std::string serializeLocation(const Location& loc) const;
    std::string serializeLocations() const;
    std::string serializeEdges() const;
    std::string serializeRouteResult(const RouteResult& res) const;
    std::string serializeComparison(int fromId, int toId) const;
    std::string serializeConnectivity() const;
    std::string serializeComplexity() const;
};

} // namespace campus

#endif // CAMPUS_NAVIGATOR_HTTPSERVER_HPP
