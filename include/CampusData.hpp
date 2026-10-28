#ifndef CAMPUS_NAVIGATOR_CAMPUSDATA_HPP
#define CAMPUS_NAVIGATOR_CAMPUSDATA_HPP

#include "CampusGraph.hpp"

namespace campus {

/**
 * @brief Utility for populating realistic university campus benchmarks and topologies.
 */
class CampusData {
public:
    /**
     * @brief Populates the graph with a rich, realistic university campus map:
     *        15 interconnected landmark buildings/avenues plus 1 isolated satellite facility
     *        to demonstrate disconnected graph handling.
     */
    static void loadDefaultCampus(CampusGraph& graph);
};

} // namespace campus

#endif // CAMPUS_NAVIGATOR_CAMPUSDATA_HPP
