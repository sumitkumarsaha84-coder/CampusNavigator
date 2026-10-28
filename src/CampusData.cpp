#include "CampusData.hpp"

namespace campus {

void CampusData::loadDefaultCampus(CampusGraph& graph) {
    graph.clear();

    // 1. Add Vertices (University Campus Landmarks & Facilities)
    graph.addLocation(Location(1, "Main North Gate", "GATE-01", "Entrance", 
                               "Primary northern campus portal, visitor parking & security", 500.0, 60.0));
    graph.addLocation(Location(2, "Administrative Complex", "ADM-01", "Administrative", 
                               "Vice Chancellor's Office, Registrar, Admissions & Finance", 500.0, 160.0));
    graph.addLocation(Location(3, "Central Library & Knowledge Centre", "LIB-01", "Academic", 
                               "5-floor research repository, digital media lab & silent reading", 340.0, 250.0));
    graph.addLocation(Location(4, "Turing Computer Science Department", "CS-01", "Academic", 
                               "CS & IT laboratories, server clusters, computing classrooms", 660.0, 250.0));
    graph.addLocation(Location(5, "Ramanujan Mathematics & AI Block", "MATH-01", "Academic", 
                               "Pure mathematics seminar halls and Machine Learning lab", 500.0, 310.0));
    graph.addLocation(Location(6, "Bose Physics & Electronics Labs", "PHY-01", "Academic", 
                               "Applied physics instrumentation and quantum optics wing", 760.0, 330.0));
    graph.addLocation(Location(7, "APJ Abdul Kalam Auditorium", "AUD-01", "Cultural", 
                               "2000-seat university convention centre and grand theatre", 320.0, 370.0));
    graph.addLocation(Location(8, "Student Activity Centre & Food Court", "SAC-01", "Recreation", 
                               "Clubs headquarters, student lounges, cafes and multi-cuisine dining", 500.0, 430.0));
    graph.addLocation(Location(9, "Olympic Sports Complex & Gym", "SPT-01", "Sports", 
                               "Indoor badminton courts, gymnasium, swimming pool and track", 820.0, 190.0));
    graph.addLocation(Location(10, "University Health Centre", "HLT-01", "Healthcare", 
                               "24x7 emergency medical triage, pharmacy and ambulance bay", 670.0, 530.0));
    graph.addLocation(Location(11, "Vivekananda Boys Hostel", "HST-01", "Residential", 
                               "North campus residence halls and dining mess", 780.0, 80.0));
    graph.addLocation(Location(12, "Sarojini Naidu Girls Hostel", "HST-02", "Residential", 
                               "South campus residence halls and recreational greens", 330.0, 530.0));
    graph.addLocation(Location(13, "South Campus Gate", "GATE-02", "Entrance", 
                               "Southern vehicular portal connecting city metro station", 500.0, 640.0));
    graph.addLocation(Location(14, "Innovation Hub & Robotics Incubator", "INC-01", "Research", 
                               "Startup incubators, rapid prototyping lab and makerspace", 650.0, 390.0));
    graph.addLocation(Location(15, "Lakeside Pavillion & Botanical Garden", "LAKE-01", "Recreation", 
                               "Scenic nature trail, biodiversity garden and open-air gazebo", 170.0, 450.0));

    // 16. Isolated Landmark (Disconnected component for testing & audit)
    graph.addLocation(Location(16, "High-Altitude Space Observatory", "OBS-01", "Research", 
                               "Isolated hilltop astrophysics observatory (separate satellite sector)", 890.0, 590.0));

    // 2. Add Pathways / Corridors / Roads (Weighted Edges in meters)
    // North Sector Connectivity
    graph.addEdge(1, 2, 120.0, "North Main Avenue", true, true);
    graph.addEdge(1, 4, 280.0, "Computing Boulevard", true, true);
    graph.addEdge(1, 11, 250.0, "North Hostel Walkway", true, true);
    // Express Ring Road (High physical distance, but only 1 direct hop)
    graph.addEdge(1, 8, 600.0, "Perimeter Express Ring Road", true, true);

    // Administrative & Academic Core
    graph.addEdge(2, 3, 150.0, "Central Campus Mall", true, true);
    graph.addEdge(2, 4, 160.0, "Heritage Academic Walk", true, true);
    graph.addEdge(2, 7, 180.0, "Ceremonial Boulevard", true, true);

    graph.addEdge(3, 5, 110.0, "Scholar's Concourse", true, true);
    graph.addEdge(3, 8, 190.0, "Library Commons Path", true, true);

    graph.addEdge(4, 5, 90.0,  "Silicon Alley", true, true);
    graph.addEdge(4, 6, 130.0, "Engineering Quad Walkway", true, true);
    graph.addEdge(4, 14, 100.0, "Technology Spine", true, true);

    graph.addEdge(5, 6, 80.0,  "Science Arcade", true, true);
    graph.addEdge(14, 8, 140.0, "Incubator Plaza Promenade", true, true);

    // Cultural & Student Life Core
    graph.addEdge(7, 8, 120.0, "Festival Walk", true, true);
    graph.addEdge(7, 15, 230.0, "Lakeside Boardwalk", true, false);

    // Sports & Recreation
    graph.addEdge(8, 9, 220.0, "Athletics Parkway", true, true);
    graph.addEdge(9, 11, 180.0, "Varsity Lane", true, true);

    // Healthcare & Southern Access
    graph.addEdge(8, 10, 170.0, "Wellness Path", true, true);
    graph.addEdge(8, 12, 240.0, "South Hostel Promenade", true, true);

    graph.addEdge(10, 13, 200.0, "Emergency Boulevard", true, true);
    graph.addEdge(12, 13, 160.0, "South Gate Avenue", true, true);
    graph.addEdge(15, 13, 270.0, "Eco Nature Trail", true, false);
}

} // namespace campus
