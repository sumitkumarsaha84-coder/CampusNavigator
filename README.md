# 🏛️ Campus Navigator — Smart Campus Route Finder

**ADSA Semester Project | C++17 | Graph Algorithms | REST API | Web Dashboard**

> A fully integrated university navigation system with a C++17 adjacency-list graph backend, embedded HTTP REST API, and a responsive browser-based dashboard. All route-finding algorithms (Dijkstra, BFS, DFS) are computed in C++ — no fake JavaScript results.

---

## 📋 Table of Contents

1. [Project Overview](#project-overview)
2. [Architecture](#architecture)
3. [Features](#features)
4. [Campus Dataset](#campus-dataset)
5. [Quick Start — Web Dashboard](#quick-start--web-dashboard)
6. [Quick Start — Terminal Mode](#quick-start--terminal-mode)
7. [Build Instructions](#build-instructions)
8. [REST API Reference](#rest-api-reference)
9. [Algorithm Comparison](#algorithm-comparison)
10. [File Structure](#file-structure)
11. [Test Results](#test-results)
12. [VS Code Tasks](#vs-code-tasks)

---

## Project Overview

Campus Navigator solves the **real-world graph problem** of finding optimal routes between 16 campus locations connected by 22 bidirectional pathways (46 directed edges). It demonstrates three distinct ADSA algorithms:

| Algorithm | Optimizes | Data Structure | Complexity |
|-----------|-----------|---------------|------------|
| **Dijkstra** | Minimum physical distance (metres) | Binary min-heap `std::priority_queue` | O((V+E) log V) |
| **BFS** | Minimum hop count (fewest intersections) | FIFO queue `std::queue` | O(V+E) |
| **DFS** | First reachable path (exploratory) | LIFO stack `std::stack` | O(V+E) |

**Benchmark route — GATE-01 → SAC-01:**
- Dijkstra: **420 m**, 3 hops via ADM-01 → AUD-01 (optimal by weight)
- BFS: **600 m**, 1 hop direct (optimal by intersections)
- DFS: 600 m, 1 hop (arbitrary first path found)

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    CAMPUS NAVIGATOR v2.0                        │
│                                                                 │
│  Browser (index.html + app.js + styles.css)                     │
│    │  fetch() HTTP/JSON requests                                │
│    ▼                                                            │
│  C++ HTTP Server (HttpServer.cpp — WinSock2)                    │
│    │  Routes API calls to C++ graph algorithms                  │
│    ▼                                                            │
│  CampusGraph (adjacency-list weighted directed graph)           │
│    ├── Dijkstra (PathFinder.cpp) — min-priority queue           │
│    ├── BFS (PathFinder.cpp) — FIFO queue                        │
│    ├── DFS (PathFinder.cpp) — LIFO stack                        │
│    └── Connected Components (PathFinder.cpp)                    │
│                                                                 │
│  Campus Dataset (CampusData.cpp) — 16 locations, 22 roads      │
└─────────────────────────────────────────────────────────────────┘
```

**Single Source of Truth:** All graph computations happen in C++. JavaScript only renders the results.

---

## Features

### Web Dashboard
- 🗺️ **Interactive SVG schematic campus map** — 16 labeled buildings with coordinates
- 🔵 **Click-to-select nodes** — click any building on the map to set source/destination
- 🛣️ **Route highlighting** — active path rendered in gold on the SVG graph
- 📊 **Algorithm comparison matrix** — Dijkstra vs BFS vs DFS side-by-side
- 🔍 **Location search** — real-time search by name, code, or keyword
- ➕ **Add locations** — POST new buildings dynamically to the live graph
- 🔗 **Add roads** — POST new pathways, instantly usable for routing
- ⚠️ **Disconnected-route handling** — Space Observatory (ID 16) is intentionally isolated
- 📈 **Performance metrics** — nodes visited, edge evaluations, execution time

### Terminal Mode
- Interactive ANSI menu with all algorithms
- Location search, route display, algorithm comparison
- Complexity explanations and viva Q&A mode

---

## Campus Dataset

| ID | Code | Name | Category |
|----|------|------|----------|
| 1 | GATE-01 | Main North Gate | Entrance |
| 2 | ADM-01 | Administrative Complex | Administrative |
| 3 | LIB-01 | Central Library | Academic |
| 4 | CS-01 | Turing CS Department | Academic |
| 5 | MATH-01 | Ramanujan Mathematics & AI | Academic |
| 6 | PHY-01 | Bose Physics & Electronics | Academic |
| 7 | AUD-01 | APJ Abdul Kalam Auditorium | Cultural |
| 8 | SAC-01 | Student Activity Centre | Recreation |
| 9 | SPT-01 | Olympic Sports Complex | Sports |
| 10 | HLT-01 | University Health Centre | Healthcare |
| 11 | HST-01 | Vivekananda Boys Hostel | Residential |
| 12 | HST-02 | Sarojini Naidu Girls Hostel | Residential |
| 13 | GATE-02 | South Campus Gate | Entrance |
| 14 | INC-01 | Innovation Hub & Robotics | Research |
| 15 | LAKE-01 | Lakeside Pavillion | Recreation |
| 16 | OBS-01 | **Space Observatory** *(isolated)* | Research |

> **Educational note:** Location 16 (OBS-01) is intentionally isolated in a separate connected component to demonstrate disconnected-graph handling.

---

## Quick Start — Web Dashboard

### Option 1: Double-click launcher (recommended)
```
Double-click:  run_web.bat
```
This builds the server, opens your browser to `http://localhost:8080`, and starts serving.

### Option 2: PowerShell manual steps
```powershell
# Set up compiler
$env:PATH = "C:\Users\sumit\.gemini\antigravity\scratch\w64devkit\bin;" + $env:PATH

# Go to project root
cd "C:\Users\sumit\.gemini\antigravity\scratch\CampusNavigator"

# Build
g++ -std=c++17 -Wall -O2 -Iinclude `
    src/main.cpp src/CampusGraph.cpp src/PathFinder.cpp `
    src/CampusData.cpp src/TerminalUI.cpp src/HttpServer.cpp `
    -lws2_32 -o CampusNavigatorServer.exe

# Start server
.\CampusNavigatorServer.exe --web --port 8080 --webroot web

# Open in browser:  http://localhost:8080
```

### Option 3: Use a custom port (if 8080 is busy)
```
run_web.bat 8081
```
or
```powershell
.\CampusNavigatorServer.exe --web --port 8081 --webroot web
```

---

## Quick Start — Terminal Mode

```powershell
$env:PATH = "C:\Users\sumit\.gemini\antigravity\scratch\w64devkit\bin;" + $env:PATH
cd "C:\Users\sumit\.gemini\antigravity\scratch\CampusNavigator"
.\CampusNavigatorServer.exe --cli
```
Or double-click `run_terminal.bat`.

---

## Build Instructions

### Prerequisites
- **GCC 16.2** via w64devkit (already installed at `scratch\w64devkit\bin`)
- Or: MinGW-w64, TDM-GCC, or any g++ supporting C++17
- CMake 3.15+ (optional — for CMake builds)

### Direct g++ Build (fastest)
```powershell
$env:PATH = "C:\Users\sumit\.gemini\antigravity\scratch\w64devkit\bin;" + $env:PATH
cd "C:\Users\sumit\.gemini\antigravity\scratch\CampusNavigator"

# Main server + web dashboard
g++ -std=c++17 -O2 -Iinclude src/main.cpp src/CampusGraph.cpp src/PathFinder.cpp src/CampusData.cpp src/TerminalUI.cpp src/HttpServer.cpp -lws2_32 -o CampusNavigatorServer.exe

# Unit tests
g++ -std=c++17 -O2 -Iinclude tests/test_main.cpp src/CampusGraph.cpp src/PathFinder.cpp src/CampusData.cpp src/TerminalUI.cpp src/HttpServer.cpp -lws2_32 -o CampusNavigatorTests.exe
```

### CMake Build
```powershell
$env:PATH = "C:\Users\sumit\.gemini\antigravity\scratch\w64devkit\bin;" + $env:PATH
cd "C:\Users\sumit\.gemini\antigravity\scratch\CampusNavigator"
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build
.\build\CampusNavigatorTests.exe
```

### Run Tests
```powershell
.\CampusNavigatorTests.exe        # Direct binary
# or
.\run_tests.bat                   # Batch launcher
# or
cd build && ctest -V              # CTest integration
```

---

## REST API Reference

Base URL: `http://localhost:8080`

### GET Endpoints

| Endpoint | Description | Example Response |
|----------|-------------|-----------------|
| `GET /api/status` | Graph summary | `{"vertices":16,"directedEdges":46,"connectedComponents":2}` |
| `GET /api/locations` | All 16 campus locations with x,y coords | JSON array |
| `GET /api/edges` | All 46 directed edges | JSON array |
| `GET /api/search?q=gate` | Search locations by name/code/keyword | JSON array |
| `GET /api/route?from=1&to=8&algo=dijkstra` | Find route (dijkstra/bfs/dfs) | RouteResult JSON |
| `GET /api/compare?from=1&to=8` | Run all 3 algorithms, return comparison | Comparison JSON |
| `GET /api/connectivity` | Connected component analysis | Component JSON |
| `GET /api/complexity` | Algorithm complexity report | Complexity JSON |

**`algo` parameter values:** `dijkstra` (default), `bfs`, `dfs`

**`from`/`to` parameter:** Location ID (integer 1–16) or location code (e.g., `GATE-01`)

### POST Endpoints

#### `POST /api/locations` — Add a new campus location
```json
{
  "name": "New Building",
  "code": "NEW-01",
  "category": "Academic",
  "description": "Description here",
  "x": 400.0,
  "y": 300.0
}
```
Response: `{"success":true,"location":{...}}`

#### `POST /api/edges` — Add a new road/pathway
```json
{
  "from": 15,
  "to": 16,
  "weight": 450.0,
  "roadType": "Trail",
  "bidirectional": true,
  "accessible": false
}
```
Response: `{"success":true,"message":"Pathway connection successfully registered."}`

#### `POST /api/reset` — Reset graph to default campus dataset
Response: `{"success":true,"message":"Campus graph reset to default benchmark topology."}`

### Route Result JSON Schema
```json
{
  "found": true,
  "algorithm": "Dijkstra (Min-Priority Queue)",
  "startId": 1, "targetId": 8,
  "totalDistance": 420,
  "hops": 3,
  "nodesVisited": 10,
  "edgeEvaluations": 28,
  "executionTimeUs": 32.0,
  "notes": "Optimal minimum walking distance found.",
  "path": [1, 2, 7, 8],
  "legs": [
    {"step":1, "fromId":1, "fromCode":"GATE-01", "toId":2, "toCode":"ADM-01",
     "distance":120.0, "cumDist":120.0, "roadType":"North Main Avenue", "accessible":true},
    ...
  ]
}
```

---

## Algorithm Comparison

### GATE-01 (1) → SAC-01 (8) — Benchmark Route

| Metric | Dijkstra | BFS | DFS |
|--------|----------|-----|-----|
| **Distance** | **420 m** ✅ | 600 m | 600 m |
| **Hops** | 3 | **1** ✅ | 1 |
| **Path** | 1→2→7→8 | 1→8 | 1→8 |
| **Nodes Visited** | 10 | 5 | 2 |
| **Time Complexity** | O((V+E)logV) | O(V+E) | O(V+E) |
| **Use Case** | Shortest walk | Fewest turns | Exploration |

### Disconnected Route — GATE-01 (1) → OBS-01 (16)
```json
{"found": false, "notes": "Disconnected Graph: Target is unreachable from origin."}
```

---

## File Structure

```
CampusNavigator/
├── include/
│   ├── Location.hpp         # Location struct (id, name, code, category, x, y)
│   ├── Edge.hpp             # Directed weighted edge struct
│   ├── RouteResult.hpp      # Algorithm result container
│   ├── CampusGraph.hpp      # Adjacency-list graph class
│   ├── PathFinder.hpp       # Dijkstra, BFS, DFS, ConnectedComponents declarations
│   ├── CampusData.hpp       # Dataset loader header
│   ├── HttpServer.hpp       # Embedded HTTP/REST server
│   └── TerminalUI.hpp       # Interactive terminal menu
├── src/
│   ├── CampusGraph.cpp      # Graph implementation + getAllEdges()
│   ├── PathFinder.cpp       # All algorithm implementations
│   ├── CampusData.cpp       # 16 locations, 22 bidirectional pathways
│   ├── HttpServer.cpp       # WinSock2 HTTP server, 12 API endpoints
│   ├── TerminalUI.cpp       # ANSI terminal UI
│   └── main.cpp             # Entry point (--cli / --web / --port / --webroot)
├── web/
│   ├── index.html           # Dashboard UI (26 KB)
│   ├── styles.css           # Blue-and-white design system (22 KB)
│   └── app.js               # API client + SVG graph renderer (29 KB)
├── tests/
│   └── test_main.cpp        # 8 test suites, all passing
├── build/                   # CMake output directory
├── CMakeLists.txt           # CMake build (includes HttpServer + ws2_32)
├── run_web.bat              # Build + launch web dashboard
├── run_terminal.bat         # Launch terminal mode
├── run_tests.bat            # Run test suite
├── README.md
├── ADSA_VIVA_NOTES.md
└── .gitignore
```

---

## Test Results

All 8 test suites pass (last verified: compiled with GCC 16.2, no warnings):

```
[RUNNING] Test_Graph_Vertex_And_Edge_Validation       ... PASSED
[RUNNING] Test_Dijkstra_Shortest_Path_Accuracy        ... PASSED
[RUNNING] Test_BFS_Minimum_Hop_Count_Contrasted_With_Dijkstra ... PASSED
[RUNNING] Test_DFS_Path_Validity                      ... PASSED
[RUNNING] Test_Disconnected_Graph_Handling            ... PASSED
[RUNNING] Test_Connected_Components_Analysis          ... PASSED
[RUNNING] Test_Dynamic_Topology_Add_Road_And_Reroute  ... PASSED
[RUNNING] Test_Location_Search_And_Autocomplete       ... PASSED
================================================================================
TEST EXECUTION SUMMARY: 8 / 8 SUITES PASSED.
```

---

## VS Code Tasks

Open the project folder in VS Code. Use **Terminal → Run Task** or `Ctrl+Shift+B`:

- **Build Server** — compiles `CampusNavigatorServer.exe`
- **Build Tests** — compiles `CampusNavigatorTests.exe`
- **Run Tests** — executes the test suite
- **Start Web Dashboard** — starts server on port 8080

---

## Compiler & Toolchain

| Tool | Version | Path |
|------|---------|------|
| g++ | GCC 16.2 (w64devkit) | `scratch\w64devkit\bin\g++.exe` |
| cmake | 3.x+ | `scratch\w64devkit\bin\cmake.exe` |
| mingw32-make | bundled | `scratch\w64devkit\bin\mingw32-make.exe` |

**Compiler flags used:** `-std=c++17 -Wall -O2 -Iinclude -lws2_32`

---

*ADSA Semester Project — C++17 Graph Algorithms: Dijkstra · BFS · DFS · Connected Components*
