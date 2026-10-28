#ifndef CAMPUS_NAVIGATOR_CAMPUSGRAPH_HPP
#define CAMPUS_NAVIGATOR_CAMPUSGRAPH_HPP

#include "Location.hpp"
#include "Edge.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <optional>

namespace campus {

/**
 * @brief Adjacency-list weighted directed/undirected graph representation of the campus.
 * 
 * Vertices: Campus locations (buildings, gates, facilities).
 * Edges: Pathways, corridors, avenues with physical walking distances.
 */
class CampusGraph {
public:
    CampusGraph() = default;

    /**
     * @brief Adds a location vertex to the graph.
     * @param loc Location object with valid id, name, and details.
     * @return true if added; false if id or name already exists.
     */
    bool addLocation(const Location& loc);

    /**
     * @brief Adds a weighted edge between two existing locations.
     * @param fromId Origin location ID.
     * @param toId Destination location ID.
     * @param weight Distance in meters (must be > 0.0).
     * @param roadType Pathway type description.
     * @param bidirectional If true, inserts both (from->to) and (to->from).
     * @param accessible Whether path is wheelchair friendly.
     * @return true on success; false if either vertex does not exist or weight <= 0.
     */
    bool addEdge(int fromId, int toId, double weight,
                 const std::string& roadType = "Paved Walkway",
                 bool bidirectional = true,
                 bool accessible = true);

    /**
     * @brief Checks if a location ID exists in the graph.
     */
    bool hasLocation(int id) const;

    /**
     * @brief Checks if a location name exists in the graph (case-insensitive).
     */
    bool hasLocation(const std::string& name) const;

    /**
     * @brief Retrieves location metadata by ID.
     */
    std::optional<Location> getLocation(int id) const;

    /**
     * @brief Retrieves location metadata by exact name (case-insensitive).
     */
    std::optional<Location> getLocation(const std::string& name) const;

    /**
     * @brief Looks up a location by ID string, exact name, or unique prefix.
     * @return Vertex ID, or -1 if not found.
     */
    int findLocationId(const std::string& query) const;

    /**
     * @brief Searches locations matching query string in name, code, or description.
     */
    std::vector<Location> searchLocations(const std::string& query) const;

    /**
     * @brief Structure representing a directed edge with source vertex for full map serialization.
     */
    struct GraphEdge {
        int from;
        int to;
        double weight;
        std::string roadType;
        bool accessible;
    };

    /**
     * @brief Returns a flattened list of all directed edges in the graph.
     */
    std::vector<GraphEdge> getAllEdges() const;

    /**
     * @brief Returns outgoing adjacency list for a vertex.
     */
    const std::vector<Edge>& getNeighbors(int id) const;

    /**
     * @brief Returns sorted list of all vertex IDs.
     */
    std::vector<int> getAllLocationIds() const;

    /**
     * @brief Returns list of all locations.
     */
    std::vector<Location> getAllLocations() const;

    /**
     * @brief Finds a specific directed edge from fromId to toId.
     */
    bool findEdge(int fromId, int toId, Edge& outEdge) const;

    /**
     * @brief Removes an edge between two locations.
     */
    bool removeEdge(int fromId, int toId, bool bidirectional = true);

    /**
     * @brief Returns total number of vertices |V|.
     */
    size_t getVertexCount() const noexcept;

    /**
     * @brief Returns total number of directed edges |E|.
     */
    size_t getDirectedEdgeCount() const noexcept;

    /**
     * @brief Generates the next available unique location ID.
     */
    int getNextAvailableId() const noexcept;

    /**
     * @brief Clears all vertices and edges.
     */
    void clear();

private:
    std::unordered_map<int, Location> m_locations;
    std::unordered_map<std::string, int> m_nameToId;
    std::unordered_map<int, std::vector<Edge>> m_adjList;

    static std::string toLower(const std::string& str);
};

} // namespace campus

#endif // CAMPUS_NAVIGATOR_CAMPUSGRAPH_HPP
