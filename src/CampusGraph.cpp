#include "CampusGraph.hpp"
#include <algorithm>
#include <cctype>

namespace campus {

std::string CampusGraph::toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lower;
}

bool CampusGraph::addLocation(const Location& loc) {
    if (loc.id < 0 || loc.name.empty()) {
        return false;
    }
    // Check for ID collision
    if (m_locations.find(loc.id) != m_locations.end()) {
        return false;
    }
    // Check for name collision
    std::string lowerName = toLower(loc.name);
    if (m_nameToId.find(lowerName) != m_nameToId.end()) {
        return false;
    }

    m_locations[loc.id] = loc;
    m_nameToId[lowerName] = loc.id;
    m_adjList[loc.id] = std::vector<Edge>();
    return true;
}

bool CampusGraph::addEdge(int fromId, int toId, double weight,
                          const std::string& roadType,
                          bool bidirectional,
                          bool accessible) {
    // Input validation: weight must be strictly positive, vertices must exist
    if (weight <= 0.0 || fromId == toId) {
        return false;
    }
    if (!hasLocation(fromId) || !hasLocation(toId)) {
        return false;
    }

    // Check if edge already exists from -> to
    auto& fromEdges = m_adjList[fromId];
    for (auto& edge : fromEdges) {
        if (edge.to == toId) {
            // Update existing edge weight and metadata
            edge.weight = weight;
            edge.roadType = roadType;
            edge.accessible = accessible;
            if (bidirectional) {
                auto& toEdges = m_adjList[toId];
                for (auto& revEdge : toEdges) {
                    if (revEdge.to == fromId) {
                        revEdge.weight = weight;
                        revEdge.roadType = roadType;
                        revEdge.accessible = accessible;
                        return true;
                    }
                }
                toEdges.emplace_back(fromId, weight, roadType, accessible);
            }
            return true;
        }
    }

    // Insert new edge
    fromEdges.emplace_back(toId, weight, roadType, accessible);

    if (bidirectional) {
        auto& toEdges = m_adjList[toId];
        bool found = false;
        for (auto& edge : toEdges) {
            if (edge.to == fromId) {
                edge.weight = weight;
                edge.roadType = roadType;
                edge.accessible = accessible;
                found = true;
                break;
            }
        }
        if (!found) {
            toEdges.emplace_back(fromId, weight, roadType, accessible);
        }
    }

    return true;
}

bool CampusGraph::hasLocation(int id) const {
    return m_locations.find(id) != m_locations.end();
}

bool CampusGraph::hasLocation(const std::string& name) const {
    return m_nameToId.find(toLower(name)) != m_nameToId.end();
}

std::optional<Location> CampusGraph::getLocation(int id) const {
    auto it = m_locations.find(id);
    if (it != m_locations.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<Location> CampusGraph::getLocation(const std::string& name) const {
    auto it = m_nameToId.find(toLower(name));
    if (it != m_nameToId.end()) {
        return getLocation(it->second);
    }
    return std::nullopt;
}

int CampusGraph::findLocationId(const std::string& query) const {
    if (query.empty()) return -1;

    // Check if query is an integer ID
    try {
        size_t idx = 0;
        int parsedId = std::stoi(query, &idx);
        if (idx == query.length() && hasLocation(parsedId)) {
            return parsedId;
        }
    } catch (...) {
        // Not a direct integer, continue to name search
    }

    std::string lq = toLower(query);

    // Exact name match
    auto it = m_nameToId.find(lq);
    if (it != m_nameToId.end()) {
        return it->second;
    }

    // Code exact match (e.g. "CS-01")
    for (const auto& [id, loc] : m_locations) {
        if (toLower(loc.code) == lq) {
            return id;
        }
    }

    // Substring match in name
    for (const auto& [id, loc] : m_locations) {
        if (toLower(loc.name).find(lq) != std::string::npos) {
            return id;
        }
    }

    return -1;
}

std::vector<Location> CampusGraph::searchLocations(const std::string& query) const {
    std::vector<Location> results;
    if (query.empty()) return results;

    std::string lq = toLower(query);
    for (const auto& [id, loc] : m_locations) {
        if (toLower(loc.name).find(lq) != std::string::npos ||
            toLower(loc.code).find(lq) != std::string::npos ||
            toLower(loc.category).find(lq) != std::string::npos ||
            toLower(loc.description).find(lq) != std::string::npos) {
            results.push_back(loc);
        }
    }

    std::sort(results.begin(), results.end(), [](const Location& a, const Location& b) {
        return a.id < b.id;
    });

    return results;
}

std::vector<CampusGraph::GraphEdge> CampusGraph::getAllEdges() const {
    std::vector<GraphEdge> allEdges;
    allEdges.reserve(getDirectedEdgeCount());
    for (const auto& [fromId, edges] : m_adjList) {
        for (const auto& edge : edges) {
            allEdges.push_back({fromId, edge.to, edge.weight, edge.roadType, edge.accessible});
        }
    }
    return allEdges;
}

const std::vector<Edge>& CampusGraph::getNeighbors(int id) const {
    static const std::vector<Edge> emptyList;
    auto it = m_adjList.find(id);
    if (it != m_adjList.end()) {
        return it->second;
    }
    return emptyList;
}

std::vector<int> CampusGraph::getAllLocationIds() const {
    std::vector<int> ids;
    ids.reserve(m_locations.size());
    for (const auto& [id, _] : m_locations) {
        ids.push_back(id);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

std::vector<Location> CampusGraph::getAllLocations() const {
    std::vector<Location> locs;
    locs.reserve(m_locations.size());
    for (const auto& [_, loc] : m_locations) {
        locs.push_back(loc);
    }
    std::sort(locs.begin(), locs.end(), [](const Location& a, const Location& b) {
        return a.id < b.id;
    });
    return locs;
}

bool CampusGraph::findEdge(int fromId, int toId, Edge& outEdge) const {
    auto it = m_adjList.find(fromId);
    if (it == m_adjList.end()) return false;

    for (const auto& edge : it->second) {
        if (edge.to == toId) {
            outEdge = edge;
            return true;
        }
    }
    return false;
}

bool CampusGraph::removeEdge(int fromId, int toId, bool bidirectional) {
    bool removed = false;
    auto it = m_adjList.find(fromId);
    if (it != m_adjList.end()) {
        auto& edges = it->second;
        auto origSize = edges.size();
        edges.erase(std::remove_if(edges.begin(), edges.end(),
                                   [toId](const Edge& e) { return e.to == toId; }),
                    edges.end());
        if (edges.size() < origSize) removed = true;
    }

    if (bidirectional) {
        auto revIt = m_adjList.find(toId);
        if (revIt != m_adjList.end()) {
            auto& revEdges = revIt->second;
            revEdges.erase(std::remove_if(revEdges.begin(), revEdges.end(),
                                          [fromId](const Edge& e) { return e.to == fromId; }),
                           revEdges.end());
        }
    }

    return removed;
}

size_t CampusGraph::getVertexCount() const noexcept {
    return m_locations.size();
}

size_t CampusGraph::getDirectedEdgeCount() const noexcept {
    size_t count = 0;
    for (const auto& [_, edges] : m_adjList) {
        count += edges.size();
    }
    return count;
}

int CampusGraph::getNextAvailableId() const noexcept {
    int maxId = 0;
    for (const auto& [id, _] : m_locations) {
        if (id > maxId) maxId = id;
    }
    return maxId + 1;
}

void CampusGraph::clear() {
    m_locations.clear();
    m_nameToId.clear();
    m_adjList.clear();
}

} // namespace campus
