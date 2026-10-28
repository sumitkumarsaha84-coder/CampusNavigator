#ifndef CAMPUS_NAVIGATOR_EDGE_HPP
#define CAMPUS_NAVIGATOR_EDGE_HPP

#include <string>
#include <utility>

namespace campus {

/**
 * @brief Represents a walkable road or corridor between two locations (directed weighted edge).
 */
struct Edge {
    int to;                   // Destination vertex ID
    double weight;            // Distance in meters (must be > 0.0)
    std::string roadType;     // e.g. "Paved Walkway", "Main Road", "Covered Pathway", "Scenic Trail"
    bool accessible;          // Barrier-free / wheelchair accessible

    Edge(int to_, double weight_, std::string roadType_ = "Paved Walkway", bool accessible_ = true)
        : to(to_), weight(weight_), roadType(std::move(roadType_)), accessible(accessible_) {}
};

} // namespace campus

#endif // CAMPUS_NAVIGATOR_EDGE_HPP
