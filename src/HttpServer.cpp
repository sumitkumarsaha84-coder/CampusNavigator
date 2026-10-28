#include "HttpServer.hpp"
#include "PathFinder.hpp"
#include "CampusData.hpp"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <limits>

namespace campus {

std::string HttpServer::urlDecode(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '%') {
            if (i + 2 < in.size()) {
                int hexVal = 0;
                std::istringstream hexStream(in.substr(i + 1, 2));
                if (hexStream >> std::hex >> hexVal) {
                    out += static_cast<char>(hexVal);
                    i += 2;
                } else {
                    out += in[i];
                }
            }
        } else if (in[i] == '+') {
            out += ' ';
        } else {
            out += in[i];
        }
    }
    return out;
}

std::string HttpServer::jsonEscape(const std::string& in) {
    std::string out;
    out.reserve(in.size() + 16);
    for (char c : in) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b";  break;
            case '\f': out += "\\f";  break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 32) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                    out += buf;
                } else {
                    out += c;
                }
                break;
        }
    }
    return out;
}

std::string HttpServer::getMimeType(const std::string& filepath) {
    auto dotPos = filepath.rfind('.');
    if (dotPos == std::string::npos) return "text/plain";
    std::string ext = filepath.substr(dotPos);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (ext == ".html" || ext == ".htm") return "text/html; charset=utf-8";
    if (ext == ".css") return "text/css; charset=utf-8";
    if (ext == ".js") return "application/javascript; charset=utf-8";
    if (ext == ".json") return "application/json; charset=utf-8";
    if (ext == ".svg") return "image/svg+xml";
    if (ext == ".png") return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".ico") return "image/x-icon";
    if (ext == ".txt") return "text/plain; charset=utf-8";
    return "application/octet-stream";
}

HttpServer::HttpServer(CampusGraph& graph, int port, std::string webRoot)
    : m_graph(graph), m_port(port), m_webRoot(std::move(webRoot)) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

HttpServer::~HttpServer() {
    stop();
#ifdef _WIN32
    WSACleanup();
#endif
}

void HttpServer::stop() {
    m_running = false;
    if (m_listenSocket != 0 && m_listenSocket != static_cast<std::uintptr_t>(~0ULL)) {
        closesocket(static_cast<SOCKET>(m_listenSocket));
        m_listenSocket = 0;
    }
}

bool HttpServer::start(bool blocking) {
    SOCKET serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == INVALID_SOCKET) {
        std::cerr << "[HttpServer] Failed to create socket.\n";
        return false;
    }

    int opt = 1;
#ifdef _WIN32
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));
#else
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(static_cast<uint16_t>(m_port));

    if (bind(serverFd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "[HttpServer] Bind failed on port " << m_port << ".\n";
        closesocket(serverFd);
        return false;
    }

    if (listen(serverFd, 16) == SOCKET_ERROR) {
        std::cerr << "[HttpServer] Listen failed.\n";
        closesocket(serverFd);
        return false;
    }

    m_listenSocket = static_cast<std::uintptr_t>(serverFd);
    m_running = true;

    std::cout << "\n================================================================================\n"
              << "   CAMPUS NAVIGATOR - SMART CAMPUS REST API & WEB DASHBOARD                     \n"
              << "   HTTP Server listening on: http://localhost:" << m_port << "\n"
              << "   Web assets root:          " << m_webRoot << "\n"
              << "   C++ ADSA Graph Backend:   ACTIVE (Single Source of Truth)\n"
              << "================================================================================\n" << std::endl;

    if (blocking) {
        runLoop();
    } else {
        std::thread([this]() { runLoop(); }).detach();
    }

    return true;
}

void HttpServer::runLoop() {
    while (m_running) {
        sockaddr_in clientAddr{};
        
sockaddr_in clientAddr{};

#ifdef _WIN32
int clientLen = sizeof(clientAddr);
#else
socklen_t clientLen = sizeof(clientAddr);
#endif

SOCKET clientSock = accept(static_cast<SOCKET>(m_listenSocket),
                           reinterpret_cast<sockaddr*>(&clientAddr),
                           &clientLen);

        if (clientSock == INVALID_SOCKET) {
            if (!m_running) break;
            continue;
        }

        std::thread([this, clientSock]() {
            handleConnection(static_cast<std::uintptr_t>(clientSock));
        }).detach();
    }
}

void HttpServer::handleConnection(std::uintptr_t clientSock) {
    SOCKET sock = static_cast<SOCKET>(clientSock);

    // Read until we have the full HTTP header (ends with \r\n\r\n)
    std::string requestStr;
    requestStr.reserve(4096);
    {
        std::vector<char> buf(4096, 0);
        while (requestStr.find("\r\n\r\n") == std::string::npos) {
            int n = recv(sock, buf.data(), static_cast<int>(buf.size()), 0);
            if (n <= 0) { closesocket(sock); return; }
            requestStr.append(buf.data(), static_cast<size_t>(n));
            if (requestStr.size() > 65536) break; // Safety limit
        }
    }

    std::istringstream reqStream(requestStr);
    std::string method, fullUri, httpVer;
    reqStream >> method >> fullUri >> httpVer;

    // Handle CORS preflight
    if (method == "OPTIONS") {
        std::string res = "HTTP/1.1 204 No Content\r\n"
                          "Access-Control-Allow-Origin: *\r\n"
                          "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                          "Access-Control-Allow-Headers: Content-Type, Accept\r\n"
                          "Content-Length: 0\r\n\r\n";
        send(sock, res.data(), static_cast<int>(res.size()), 0);
        closesocket(sock);
        return;
    }

    // Split path and query
    std::string path = fullUri;
    std::string query;
    auto qPos = fullUri.find('?');
    if (qPos != std::string::npos) {
        path = fullUri.substr(0, qPos);
        query = fullUri.substr(qPos + 1);
    }

    // Parse Content-Length header (case-insensitive search)
    size_t contentLength = 0;
    {
        std::string lowerReq = requestStr;
        std::transform(lowerReq.begin(), lowerReq.end(), lowerReq.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        auto clPos = lowerReq.find("content-length:");
        if (clPos != std::string::npos) {
            clPos += 15; // skip "content-length:"
            while (clPos < lowerReq.size() && (lowerReq[clPos] == ' ' || lowerReq[clPos] == '\t')) ++clPos;
            contentLength = static_cast<size_t>(std::stoul(lowerReq.substr(clPos)));
        }
    }

    // Extract whatever body bytes arrived with the headers
    std::string body;
    auto headerEndPos = requestStr.find("\r\n\r\n");
    if (headerEndPos != std::string::npos) {
        body = requestStr.substr(headerEndPos + 4);
    }

    // If body is still incomplete, read remaining bytes from socket
    if (body.size() < contentLength) {
        size_t remaining = contentLength - body.size();
        std::vector<char> bodyBuf(remaining);
        size_t totalRead = 0;
        while (totalRead < remaining) {
            int n = recv(sock, bodyBuf.data() + totalRead,
                         static_cast<int>(remaining - totalRead), 0);
            if (n <= 0) break;
            totalRead += static_cast<size_t>(n);
        }
        body.append(bodyBuf.data(), totalRead);
    }

    std::string responseBody;
    std::string contentType = "application/json; charset=utf-8";
    int statusCode = 200;

    if (path.rfind("/api/", 0) == 0) {
        responseBody = routeRequest(method, path, query, body, contentType, statusCode);
    } else {
        // Serve static web asset
        if (path == "/" || path.empty()) path = "/index.html";
        std::string localPath = m_webRoot + path;

        // Security check against directory traversal
        if (localPath.find("..") != std::string::npos) {
            statusCode = 403;
            responseBody = "Forbidden";
            contentType = "text/plain";
        } else {
            std::ifstream file(localPath, std::ios::binary);
            if (file) {
                std::ostringstream ss;
                ss << file.rdbuf();
                responseBody = ss.str();
                contentType = getMimeType(localPath);
                statusCode = 200;
            } else {
                statusCode = 404;
                responseBody = "404 Not Found";
                contentType = "text/plain";
            }
        }
    }

    std::ostringstream resp;
    resp << "HTTP/1.1 " << statusCode << " " << (statusCode == 200 ? "OK" : (statusCode == 404 ? "Not Found" : "Error")) << "\r\n"
         << "Content-Type: " << contentType << "\r\n"
         << "Content-Length: " << responseBody.size() << "\r\n"
         << "Access-Control-Allow-Origin: *\r\n"
         << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
         << "Access-Control-Allow-Headers: Content-Type, Accept\r\n"
         << "Connection: close\r\n\r\n"
         << responseBody;

    std::string respStr = resp.str();
    send(sock, respStr.data(), static_cast<int>(respStr.size()), 0);
    closesocket(sock);
}

static std::string getQueryParam(const std::string& query, const std::string& key) {
    std::string pattern = key + "=";
    auto pos = query.find(pattern);
    if (pos == std::string::npos) return "";
    pos += pattern.size();
    auto endPos = query.find('&', pos);
    std::string val = (endPos == std::string::npos) ? query.substr(pos) : query.substr(pos, endPos - pos);
    return HttpServer::urlDecode(val);
}

static std::string getJsonStringField(const std::string& json, const std::string& key) {
    std::string pattern = "\"" + key + "\"";
    auto pos = json.find(pattern);
    if (pos == std::string::npos) return "";
    pos = json.find(':', pos);
    if (pos == std::string::npos) return "";
    pos = json.find('"', pos);
    if (pos == std::string::npos) return "";
    auto endPos = json.find('"', pos + 1);
    if (endPos == std::string::npos) return "";
    return json.substr(pos + 1, endPos - pos - 1);
}

static double getJsonNumberField(const std::string& json, const std::string& key, double defaultVal = 0.0) {
    std::string pattern = "\"" + key + "\"";
    auto pos = json.find(pattern);
    if (pos == std::string::npos) return defaultVal;
    pos = json.find(':', pos);
    if (pos == std::string::npos) return defaultVal;
    // skip whitespace
    pos = json.find_first_not_of(" \t\r\n", pos + 1);
    if (pos == std::string::npos) return defaultVal;
    try {
        size_t idx = 0;
        return std::stod(json.substr(pos), &idx);
    } catch (...) {
        return defaultVal;
    }
}

static bool getJsonBoolField(const std::string& json, const std::string& key, bool defaultVal = true) {
    std::string pattern = "\"" + key + "\"";
    auto pos = json.find(pattern);
    if (pos == std::string::npos) return defaultVal;
    pos = json.find(':', pos);
    if (pos == std::string::npos) return defaultVal;
    pos = json.find_first_not_of(" \t\r\n", pos + 1);
    if (pos == std::string::npos) return defaultVal;
    if (json.substr(pos, 4) == "true") return true;
    if (json.substr(pos, 5) == "false") return false;
    return defaultVal;
}

std::string HttpServer::routeRequest(const std::string& method, const std::string& path,
                                     const std::string& query, const std::string& body,
                                     std::string& outContentType, int& outStatus) {
    outContentType = "application/json; charset=utf-8";
    outStatus = 200;

    // GET /api/status
    if (path == "/api/status" && method == "GET") {
        auto components = PathFinder::getConnectedComponents(m_graph);
        std::ostringstream ss;
        ss << "{"
           << "\"status\":\"ok\","
           << "\"version\":\"2.0.0\","
           << "\"appName\":\"Campus Navigator\","
           << "\"vertices\":" << m_graph.getVertexCount() << ","
           << "\"directedEdges\":" << m_graph.getDirectedEdgeCount() << ","
           << "\"connectedComponents\":" << components.size() << ","
           << "\"isFullyConnected\":" << (components.size() <= 1 ? "true" : "false")
           << "}";
        return ss.str();
    }

    // GET /api/locations
    if (path == "/api/locations" && method == "GET") {
        return serializeLocations();
    }

    // GET /api/edges
    if (path == "/api/edges" && method == "GET") {
        return serializeEdges();
    }

    // GET /api/search?q=...
    if (path == "/api/search" && method == "GET") {
        std::string q = getQueryParam(query, "q");
        auto results = m_graph.searchLocations(q);
        std::ostringstream ss;
        ss << "[";
        for (size_t i = 0; i < results.size(); ++i) {
            ss << serializeLocation(results[i]);
            if (i + 1 < results.size()) ss << ",";
        }
        ss << "]";
        return ss.str();
    }

    // GET /api/route?from=X&to=Y&algo=dijkstra|bfs|dfs
    if (path == "/api/route" && method == "GET") {
        std::string fromStr = getQueryParam(query, "from");
        std::string toStr = getQueryParam(query, "to");
        std::string algo = getQueryParam(query, "algo");
        if (algo.empty()) algo = "dijkstra";

        int fromId = m_graph.findLocationId(fromStr);
        int toId = m_graph.findLocationId(toStr);

        if (fromId == -1 || toId == -1) {
            outStatus = 400;
            return "{\"error\":\"Invalid source or destination location identifier.\"}";
        }

        RouteResult res;
        std::string lowerAlgo = algo;
        std::transform(lowerAlgo.begin(), lowerAlgo.end(), lowerAlgo.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        if (lowerAlgo == "bfs") {
            res = PathFinder::bfs(m_graph, fromId, toId);
        } else if (lowerAlgo == "dfs") {
            res = PathFinder::dfs(m_graph, fromId, toId);
        } else {
            res = PathFinder::dijkstra(m_graph, fromId, toId);
        }

        return serializeRouteResult(res);
    }

    // GET /api/compare?from=X&to=Y
    if (path == "/api/compare" && method == "GET") {
        std::string fromStr = getQueryParam(query, "from");
        std::string toStr = getQueryParam(query, "to");
        int fromId = m_graph.findLocationId(fromStr);
        int toId = m_graph.findLocationId(toStr);

        if (fromId == -1 || toId == -1) {
            outStatus = 400;
            return "{\"error\":\"Invalid source or destination location identifier.\"}";
        }

        return serializeComparison(fromId, toId);
    }

    // GET /api/connectivity
    if (path == "/api/connectivity" && method == "GET") {
        return serializeConnectivity();
    }

    // GET /api/complexity
    if (path == "/api/complexity" && method == "GET") {
        return serializeComplexity();
    }

    // POST /api/locations
    if (path == "/api/locations" && method == "POST") {
        int id = static_cast<int>(getJsonNumberField(body, "id", m_graph.getNextAvailableId()));
        std::string name = getJsonStringField(body, "name");
        std::string code = getJsonStringField(body, "code");
        std::string cat  = getJsonStringField(body, "category");
        std::string desc = getJsonStringField(body, "description");
        double x = getJsonNumberField(body, "x", 500.0);
        double y = getJsonNumberField(body, "y", 350.0);

        if (name.empty()) {
            outStatus = 400;
            return "{\"success\":false,\"error\":\"Location name cannot be empty.\"}";
        }
        if (m_graph.hasLocation(id)) {
            outStatus = 400;
            return "{\"success\":false,\"error\":\"Location ID already exists.\"}";
        }
        if (m_graph.hasLocation(name)) {
            outStatus = 400;
            return "{\"success\":false,\"error\":\"A location with this name already exists.\"}";
        }

        Location newLoc(id, name, code, cat, desc, x, y);
        if (m_graph.addLocation(newLoc)) {
            return "{\"success\":true,\"location\":" + serializeLocation(newLoc) + "}";
        } else {
            outStatus = 500;
            return "{\"success\":false,\"error\":\"Failed to insert location into graph.\"}";
        }
    }

    // POST /api/edges
    if (path == "/api/edges" && method == "POST") {
        int from = static_cast<int>(getJsonNumberField(body, "from", -1));
        int to = static_cast<int>(getJsonNumberField(body, "to", -1));
        double weight = getJsonNumberField(body, "weight", 0.0);
        std::string roadType = getJsonStringField(body, "roadType");
        if (roadType.empty()) roadType = "Paved Walkway";
        bool bidi = getJsonBoolField(body, "bidirectional", true);
        bool accessible = getJsonBoolField(body, "accessible", true);

        if (from <= 0 || to <= 0 || from == to) {
            outStatus = 400;
            return "{\"success\":false,\"error\":\"Invalid endpoint IDs (self-loops prohibited).\"}";
        }
        if (weight <= 0.0) {
            outStatus = 400;
            return "{\"success\":false,\"error\":\"Pathway distance must be strictly greater than zero.\"}";
        }
        if (!m_graph.hasLocation(from) || !m_graph.hasLocation(to)) {
            outStatus = 400;
            return "{\"success\":false,\"error\":\"One or both endpoint locations do not exist.\"}";
        }

        if (m_graph.addEdge(from, to, weight, roadType, bidi, accessible)) {
            return "{\"success\":true,\"message\":\"Pathway connection successfully registered.\"}";
        } else {
            outStatus = 500;
            return "{\"success\":false,\"error\":\"Failed to insert edge into adjacency list.\"}";
        }
    }

    // POST /api/reset
    if (path == "/api/reset" && method == "POST") {
        CampusData::loadDefaultCampus(m_graph);
        return "{\"success\":true,\"message\":\"Campus graph reset to default benchmark topology.\"}";
    }

    outStatus = 404;
    return "{\"error\":\"Unknown API endpoint.\"}";
}

std::string HttpServer::serializeLocation(const Location& loc) const {
    std::ostringstream ss;
    ss << "{"
       << "\"id\":" << loc.id << ","
       << "\"name\":\"" << jsonEscape(loc.name) << "\","
       << "\"code\":\"" << jsonEscape(loc.code) << "\","
       << "\"category\":\"" << jsonEscape(loc.category) << "\","
       << "\"description\":\"" << jsonEscape(loc.description) << "\","
       << "\"x\":" << std::fixed << std::setprecision(1) << loc.x << ","
       << "\"y\":" << std::fixed << std::setprecision(1) << loc.y
       << "}";
    return ss.str();
}

std::string HttpServer::serializeLocations() const {
    auto locs = m_graph.getAllLocations();
    std::ostringstream ss;
    ss << "[";
    for (size_t i = 0; i < locs.size(); ++i) {
        ss << serializeLocation(locs[i]);
        if (i + 1 < locs.size()) ss << ",";
    }
    ss << "]";
    return ss.str();
}

std::string HttpServer::serializeEdges() const {
    auto edges = m_graph.getAllEdges();
    std::ostringstream ss;
    ss << "[";
    for (size_t i = 0; i < edges.size(); ++i) {
        ss << "{"
           << "\"from\":" << edges[i].from << ","
           << "\"to\":" << edges[i].to << ","
           << "\"weight\":" << std::fixed << std::setprecision(1) << edges[i].weight << ","
           << "\"roadType\":\"" << jsonEscape(edges[i].roadType) << "\","
           << "\"accessible\":" << (edges[i].accessible ? "true" : "false")
           << "}";
        if (i + 1 < edges.size()) ss << ",";
    }
    ss << "]";
    return ss.str();
}

std::string HttpServer::serializeRouteResult(const RouteResult& res) const {
    std::ostringstream ss;
    ss << "{"
       << "\"found\":" << (res.found ? "true" : "false") << ","
       << "\"algorithm\":\"" << jsonEscape(res.algorithmName) << "\","
       << "\"startId\":" << res.startId << ","
       << "\"targetId\":" << res.targetId << ","
       << "\"totalDistance\":" << (res.found ? res.totalDistance : -1.0) << ","
       << "\"hops\":" << res.hops << ","
       << "\"nodesVisited\":" << res.nodesVisited << ","
       << "\"edgeEvaluations\":" << res.edgeEvaluations << ","
       << "\"executionTimeUs\":" << std::fixed << std::setprecision(1) << res.executionTimeUs << ","
       << "\"notes\":\"" << jsonEscape(res.notes) << "\",";

    // Path sequence array
    ss << "\"path\":[";
    for (size_t i = 0; i < res.path.size(); ++i) {
        ss << res.path[i];
        if (i + 1 < res.path.size()) ss << ",";
    }
    ss << "],";

    // Detailed legs
    ss << "\"legs\":[";
    double cumDist = 0.0;
    for (size_t i = 0; i + 1 < res.path.size(); ++i) {
        int u = res.path[i];
        int v = res.path[i + 1];
        auto uLoc = m_graph.getLocation(u);
        auto vLoc = m_graph.getLocation(v);
        Edge edge(0, 0);
        m_graph.findEdge(u, v, edge);
        cumDist += edge.weight;

        ss << "{"
           << "\"step\":" << (i + 1) << ","
           << "\"fromId\":" << u << ","
           << "\"fromName\":\"" << jsonEscape(uLoc ? uLoc->name : "") << "\","
           << "\"fromCode\":\"" << jsonEscape(uLoc ? uLoc->code : "") << "\","
           << "\"toId\":" << v << ","
           << "\"toName\":\"" << jsonEscape(vLoc ? vLoc->name : "") << "\","
           << "\"toCode\":\"" << jsonEscape(vLoc ? vLoc->code : "") << "\","
           << "\"distance\":" << std::fixed << std::setprecision(1) << edge.weight << ","
           << "\"cumDist\":" << std::fixed << std::setprecision(1) << cumDist << ","
           << "\"roadType\":\"" << jsonEscape(edge.roadType) << "\","
           << "\"accessible\":" << (edge.accessible ? "true" : "false")
           << "}";
        if (i + 2 < res.path.size()) ss << ",";
    }
    ss << "]";

    ss << "}";
    return ss.str();
}

std::string HttpServer::serializeComparison(int fromId, int toId) const {
    AlgorithmComparison comp = PathFinder::compareAlgorithms(m_graph, fromId, toId);

    std::ostringstream ss;
    ss << "{"
       << "\"fromId\":" << fromId << ","
       << "\"toId\":" << toId << ","
       << "\"isConnected\":" << (comp.isConnected ? "true" : "false") << ","
       << "\"dijkstra\":" << serializeRouteResult(comp.dijkstra) << ","
       << "\"bfs\":" << serializeRouteResult(comp.bfs) << ","
       << "\"dfs\":" << serializeRouteResult(comp.dfs) << ",";

    // Insights
    ss << "\"insights\":[";
    std::vector<std::string> insights;
    if (comp.isConnected) {
        if (comp.dijkstra.totalDistance < comp.bfs.totalDistance) {
            double saved = comp.bfs.totalDistance - comp.dijkstra.totalDistance;
            std::ostringstream in1;
            in1 << "Dijkstra saved " << std::fixed << std::setprecision(1) << saved 
                << " meters compared to BFS by choosing a path with lower cumulative physical weight.";
            insights.push_back(in1.str());
        } else {
            insights.push_back("Dijkstra and BFS identified identical physical distance routes.");
        }

        if (comp.bfs.hops < comp.dijkstra.hops) {
            std::ostringstream in2;
            in2 << "BFS found a route with " << (comp.dijkstra.hops - comp.bfs.hops)
                << " fewer turns/intersections, minimizing navigational complexity.";
            insights.push_back(in2.str());
        }

        std::ostringstream in3;
        in3 << "DFS traversed " << comp.dfs.hops << " hops (" 
            << std::fixed << std::setprecision(1) << comp.dfs.totalDistance 
            << " m), demonstrating non-optimal branch exploration.";
        insights.push_back(in3.str());
    } else {
        insights.push_back("Destination is unreachable from origin in the campus graph (disconnected components).");
    }

    for (size_t i = 0; i < insights.size(); ++i) {
        ss << "\"" << jsonEscape(insights[i]) << "\"";
        if (i + 1 < insights.size()) ss << ",";
    }
    ss << "]}";

    return ss.str();
}

std::string HttpServer::serializeConnectivity() const {
    auto components = PathFinder::getConnectedComponents(m_graph);
    std::ostringstream ss;
    ss << "{"
       << "\"componentCount\":" << components.size() << ","
       << "\"isFullyConnected\":" << (components.size() <= 1 ? "true" : "false") << ","
       << "\"components\":[";

    for (size_t i = 0; i < components.size(); ++i) {
        ss << "{\"id\":" << (i + 1) << ",\"count\":" << components[i].size() << ",\"locations\":[";
        for (size_t j = 0; j < components[i].size(); ++j) {
            auto loc = m_graph.getLocation(components[i][j]);
            if (loc) {
                ss << serializeLocation(*loc);
                if (j + 1 < components[i].size()) ss << ",";
            }
        }
        ss << "]}";
        if (i + 1 < components.size()) ss << ",";
    }
    ss << "]}";
    return ss.str();
}

std::string HttpServer::serializeComplexity() const {
    std::string report = PathFinder::getComplexityReport();
    std::ostringstream ss;
    ss << "{"
       << "\"reportText\":\"" << jsonEscape(report) << "\","
       << "\"algorithms\":["
       << "{\"name\":\"Dijkstra's Algorithm\",\"time\":\"O((|V| + |E|) log |V|)\",\"space\":\"O(|V|)\",\"dataStructure\":\"Binary Min-Heap (std::priority_queue)\",\"optimality\":\"Shortest physical distance in meters\"},"
       << "{\"name\":\"Breadth-First Search (BFS)\",\"time\":\"O(|V| + |E|)\",\"space\":\"O(|V|)\",\"dataStructure\":\"FIFO Queue (std::queue)\",\"optimality\":\"Minimum hop count (fewest intersections)\"},"
       << "{\"name\":\"Depth-First Search (DFS)\",\"time\":\"O(|V| + |E|)\",\"space\":\"O(|V|)\",\"dataStructure\":\"LIFO Stack (std::stack)\",\"optimality\":\"First reachable path (exploratory)\"},"
       << "{\"name\":\"Connected Components\",\"time\":\"O(|V| + |E|)\",\"space\":\"O(|V|)\",\"dataStructure\":\"Queue-based partition\",\"optimality\":\"Identifies isolated subgraphs\"}"
       << "]"
       << "}";
    return ss.str();
}

} // namespace campus
