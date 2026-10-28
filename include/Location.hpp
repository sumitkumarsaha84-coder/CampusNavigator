#ifndef CAMPUS_NAVIGATOR_LOCATION_HPP
#define CAMPUS_NAVIGATOR_LOCATION_HPP

#include <string>
#include <iostream>
#include <iomanip>

namespace campus {

/**
 * @brief Represents a physical campus landmark or building (vertex in graph).
 */
struct Location {
    int id;
    std::string name;
    std::string code;         // e.g., "CS-01", "LIB-MAIN", "GATE-N"
    std::string category;     // e.g., "Academic", "Administrative", "Hostel", "Sports", "Entrance"
    std::string description;  // e.g., "Department of Computer Science & Engineering"
    double x{0.0};            // Schematic map X coordinate
    double y{0.0};            // Schematic map Y coordinate

    Location() : id(-1), name(""), code(""), category(""), description(""), x(0.0), y(0.0) {}

    Location(int id_, std::string name_, std::string code_, 
             std::string category_, std::string description_ = "",
             double x_ = 0.0, double y_ = 0.0)
        : id(id_), name(std::move(name_)), code(std::move(code_)),
          category(std::move(category_)), description(std::move(description_)),
          x(x_), y(y_) {}

    bool operator==(const Location& other) const {
        return id == other.id;
    }
};

} // namespace campus

#endif // CAMPUS_NAVIGATOR_LOCATION_HPP
