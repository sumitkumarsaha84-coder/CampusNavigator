# Advanced Data Structures & Algorithms (ADSA) Viva Reference Notes
## Project: Campus Navigator — Smart Campus Route Finder

---

## 1. Graph Representation: Adjacency List vs Adjacency Matrix

### Q1: Why did you choose an Adjacency List over an Adjacency Matrix for this project?
* **Graph Density:** Campus road networks are **sparse graphs**. In our university campus, each building connects to approximately 2 to 5 adjacent avenues (average vertex degree $d \approx 3$).
* **Space Complexity:**
  * **Adjacency Matrix:** Requires $\Theta(|V|^2)$ space regardless of how few edges exist. For a campus with $|V| = 1000$ buildings, an adjacency matrix allocates $1,000,000$ cells, almost all containing zeros.
  * **Adjacency List:** Requires $\Theta(|V| + |E|)$ space. Since $|E| \approx 3|V|$, space usage is strictly linear in the number of landmarks.
* **Neighborhood Iteration:**
  * To inspect the neighbors of vertex $u$, an Adjacency Matrix requires scanning all $|V|$ columns ($\Theta(|V|)$ time).
  * An Adjacency List scans only the actual outgoing edges ($\Theta(\text{deg}(u))$ time), which is crucial for high-performance Dijkstra, BFS, and DFS traversals.

### Q2: How did you implement the Adjacency List in C++17?
* We used `std::unordered_map<int, std::vector<Edge>>`:
  * Key: `int` vertex ID.
  * Value: `std::vector<Edge>` containing `{int to, double weight, string roadType, bool accessible}`.
* For fast name and code resolution, we maintain an auxiliary hash map `std::unordered_map<std::string, int>` providing $O(1)$ average-time lookups from landmark names or codes to internal vertex IDs.

---

## 2. Dijkstra's Algorithm & Priority Queue Mechanics

### Q3: What is the core invariant of Dijkstra's Algorithm?
* Dijkstra is a **greedy algorithm** based on the **optimal substructure property**.
* **Invariant:** At every step, when a vertex $u$ with the minimum tentative distance is extracted from the priority queue, its shortest distance from the source is finalized and guaranteed to be optimal.
* This holds because edge weights are strictly non-negative ($w \ge 0$). Any other path to $u$ must pass through another unvisited frontier node whose tentative distance is already greater than or equal to $dist[u]$.

### Q4: Why did you use a Min-Priority Queue instead of an array or linear search?
* **Comparison of Implementations:**
  | Implementation | Extract-Min | Edge Relaxation (Decrease-Key) | Total Time Complexity | Best For |
  | :--- | :--- | :--- | :--- | :--- |
  | **Linear Array** | $O(|V|)$ | $O(1)$ | $O(|V|^2 + |E|) = O(|V|^2)$ | Dense graphs ($|E| \approx |V|^2$) |
  | **Binary Min-Heap** | $O(\log |V|)$ | $O(\log |V|)$ | $O((|V| + |E|) \log |V|)$ | **Sparse graphs (Campus maps)** |
  | **Fibonacci Heap** | $O(\log |V|)$ amortized | $O(1)$ amortized | $O(|E| + |V| \log |V|)$ | Theoretical; high constant factors |
* Since campus graphs are sparse ($|E| \ll |V|^2$), the binary heap reduces time from quadratic $O(|V|^2)$ to quasi-linear $O(|V| \log |V|)$.

### Q5: How is "Decrease-Key" handled in modern C++ with `std::priority_queue`?
* Standard C++ `std::priority_queue` does not expose a native `decrease_key` method.
* Instead, we use the standard industry technique called **Lazy Deletion** (or duplicate insertion):
  1. When a shorter distance to $v$ is discovered, we push a new pair `(new_dist, v)` into the min-heap.
  2. When popping an element `(d, u)` from the heap, we check `if (visited[u] || d > dist[u]) continue;`.
  3. If already visited or outdated, we discard it immediately in $O(1)$ time.
  4. At most $|E|$ elements are ever pushed into the queue, keeping space $O(|E|)$ and each pop $O(\log |E|) = O(\log |V|)$.

### Q6: Why does Dijkstra fail when edge weights are negative?
* Dijkstra relies on the assumption that adding an edge to a path cannot decrease its total length.
* If a negative edge exists, a path that currently appears longer could later traverse a large negative edge and become shorter overall. Since Dijkstra marks vertices as visited/settled once popped and never re-evaluates them, it would miss this shorter path.
* **Solution for negative weights:** **Bellman-Ford Algorithm** ($O(|V| \cdot |E|)$) or **Johnson's Algorithm** for all-pairs.

---

## 3. BFS vs Dijkstra on Weighted Graphs

### Q7: Can Breadth-First Search (BFS) find the shortest path on a weighted graph?
* **No**, in general, BFS finds the shortest path **only in terms of edge count (hops)**, not cumulative physical weight.
* In BFS, all edges are implicitly treated as having identical weight (1 unit).
* **Exception:** If all edge weights in the graph are identical constants, BFS is equivalent to Dijkstra and runs in $O(|V| + |E|)$.

### Q8: Describe the benchmark case where BFS and Dijkstra disagree in your project.
* In our campus dataset:
  * Route from **Main North Gate (1)** to **Student Activity Centre (8)**:
    * **Direct Route:** Perimeter Express Road (1 hop, distance = **600 meters**).
    * **Multi-hop Route:** Via Admin (2) and Auditorium (7) (3 hops: $120\text{m} + 180\text{m} + 120\text{m}$ = **420 meters**).
  * **BFS output:** Selects the 1-hop path ($1 \to 8$) because it minimizes intersections. Total distance: $600\text{m}$.
  * **Dijkstra output:** Selects the 3-hop path ($1 \to 2 \to 7 \to 8$) because it minimizes physical walking meters ($420\text{m}$).
  * This demonstrates why transportation apps offer both "Shortest Distance" and "Fewest Transfers" modes.

---

## 4. Depth-First Search (DFS) & Applications

### Q9: What is the purpose of DFS in this navigation system?
* While Dijkstra finds optimal distance and BFS finds minimum hops, DFS serves to:
  1. Explore deep contiguous routes.
  2. Prove graph reachability without maintaining priority heaps.
  3. Form the foundational traversal for cycle detection and topological sorting.
* In our application, DFS also illustrates how a greedy deep branch dive can result in a much longer route than Dijkstra or BFS.

### Q10: Why did you use an iterative DFS with `std::stack` instead of recursion?
* Recursive DFS consumes space on the operating system call stack.
* In deep graphs or large networks with millions of vertices, recursive DFS can trigger a **stack overflow** segmentation fault.
* Iterative DFS allocates memory on the process **heap** via `std::stack`, supporting arbitrarily deep graphs safely.

---

## 5. Disconnected Graph Handling & Connected Components

### Q11: How does your system detect and handle disconnected vertices?
* In our campus model, vertex 16 (**High-Altitude Space Observatory**) is intentionally isolated (no edges connecting it to the main campus).
* **Algorithmic Handling:**
  1. `dist[target]` remains initialized to `std::numeric_limits<double>::infinity()`.
  2. The priority queue or BFS queue empties before the target vertex is ever reached.
  3. `PathFinder` checks `if (dist[target] == INF || parent.find(target) == parent.end())`.
  4. It returns `RouteResult.found = false` and sets informative diagnostics rather than crashing or looping.
* **Component Audit:**
  * We run `PathFinder::getConnectedComponents(graph)` which executes an outer loop over all vertices, launching a BFS from every unvisited vertex.
  * It detects exactly **2 components**: Component 1 (15 interconnected buildings) and Component 2 (the isolated observatory).

---

## 6. Comprehensive Complexity Cheat Sheet

| Operation / Algorithm | Time Complexity | Auxiliary Space | Key Data Structure |
| :--- | :--- | :--- | :--- |
| **Add Vertex** | $O(1)$ amortized | $O(1)$ | Hash table insertion |
| **Add Edge** | $O(1)$ amortized | $O(1)$ | Dynamic array push |
| **Lookup Location** | $O(1)$ average | $O(1)$ | `std::unordered_map` |
| **Dijkstra's Algorithm** | $O((|V| + |E|) \log |V|)$ | $O(|V|)$ | `std::priority_queue` (Min-Heap) |
| **Breadth-First Search** | $O(|V| + |E|)$ | $O(|V|)$ | `std::queue` (FIFO) |
| **Depth-First Search** | $O(|V| + |E|)$ | $O(|V|)$ | `std::stack` (LIFO) |
| **Connected Components** | $O(|V| + |E|)$ | $O(|V|)$ | Queue-based component clustering |
| **Path Reconstruction** | $O(L)$ where $L \le |V|$ | $O(L)$ | Parent back-pointers |


---

## 7. REST API Design & Web Integration (v2.0)

### Q12: Why does the C++ backend serve a REST API instead of a standalone terminal program?
* A REST API decouples the algorithmic backend from the presentation layer.
* The C++ graph engine remains the **Single Source of Truth** — all route computations happen in native C++ with zero JavaScript replication.
* This architecture mirrors real-world navigation services (e.g., Google Maps backend) where the server computes routes and the client renders them.

### Q13: How does your embedded HTTP server work without a web framework?
* `HttpServer.cpp` implements a raw WinSock2 TCP server:
  1. `socket()` + `bind()` + `listen()` on the chosen port.
  2. `accept()` loop in the main thread; each accepted connection spawns a `std::thread`.
  3. The handler reads the complete HTTP request — first receiving until `\r\n\r\n` (end of headers), then parsing `Content-Length` and reading the remaining body bytes in a second `recv()` loop.
  4. HTTP method, path, query string, and JSON body are parsed using simple string scanning.
  5. `Content-Type: application/json` + CORS headers (`Access-Control-Allow-Origin: *`) are added to every response.
* **Key lesson:** Separating header-reading from body-reading is critical for POST requests, because HTTP clients (including .NET `WebClient`) often send headers and body in separate TCP segments.

### Q14: What is CORS and why is it needed here?
* **CORS (Cross-Origin Resource Sharing)** is a browser security policy that blocks JavaScript from making requests to a different origin than the one that served the page.
* When the browser serves `index.html` from `http://localhost:8080`, JavaScript `fetch()` calls to `http://localhost:8080/api/...` are same-origin — no CORS issue.
* If the user opens `index.html` directly from the file system (`file://`), the origin is `null`, which would trigger CORS errors without the `Access-Control-Allow-Origin: *` header.
* Our server adds this header on every response including OPTIONS preflight responses.

### Q15: How does the browser dashboard avoid faking algorithm results?
* The JavaScript in `app.js` never implements Dijkstra, BFS, or DFS.
* Every route request calls `fetch('/api/route?from=X&to=Y&algo=dijkstra')` and waits for the C++ server to compute and return the result as JSON.
* The JavaScript only renders the JSON response — it has no knowledge of graph topology, edge weights, or traversal logic.
* This is verifiable by inspecting `app.js`: no adjacency list, no visited-set, no queue, no priority queue.

### Q16: How did you solve the POST body-truncation bug?
* Initial implementation: single `recv(sock, buffer, 65536, 0)`. This reads what arrives in the first TCP segment — for GET requests that's the complete request; for POST requests with a body it might be just the headers.
* Root cause: TCP is a stream protocol. HTTP headers and the body are sent as separate `write()` calls by .NET `WebClient`, arriving in separate TCP segments.
* Fix:
  1. Loop `recv()` until `\r\n\r\n` is found in the accumulated data (full headers received).
  2. Parse the `Content-Length` header (case-insensitive).
  3. If the body already present after `\r\n\r\n` is shorter than `Content-Length`, loop `recv()` again to read the remaining bytes.
* This pattern (header-then-body) is the standard way to implement HTTP/1.1 parsing.

---

## 8. System Architecture Summary

```
Frontend (Browser)                   Backend (C++17)
────────────────────────────────     ────────────────────────────────────────
index.html  ──────────────────────►  HttpServer (WinSock2 TCP, port 8080)
styles.css  (static assets served     │
app.js       by C++ server)           ├── GET /api/locations  → CampusGraph::getAllLocations()
                                      ├── GET /api/edges      → CampusGraph::getAllEdges()
app.js fetch('/api/route?...')  ────► ├── GET /api/route      → PathFinder::dijkstra/bfs/dfs()
                                      ├── GET /api/compare    → PathFinder::compareAlgorithms()
                                      ├── GET /api/search     → CampusGraph::searchLocations()
                                      ├── GET /api/connectivity → PathFinder::getConnectedComponents()
                                      ├── POST /api/locations → CampusGraph::addLocation()
                                      └── POST /api/edges     → CampusGraph::addEdge()

CampusGraph (adjacency list: unordered_map<int, vector<Edge>>)
  ├── 16 vertices (campus locations with x,y SVG coordinates)
  ├── 22 bidirectional pathways = 46 directed edges
  └── 2 connected components (15 main + 1 isolated observatory)
```

**Data flow for a route request:**
1. User selects GATE-01 → SAC-01 with Dijkstra in browser.
2. `app.js` calls `fetch('/api/route?from=1&to=8&algo=dijkstra')`.
3. `HttpServer` parses query, calls `PathFinder::dijkstra(graph, 1, 8)`.
4. Dijkstra runs with `std::priority_queue<pair<double,int>, vector, greater>`.
5. Result serialized to JSON with path `[1,2,7,8]`, distance 420m, 3 hops.
6. JSON returned to browser; `app.js` highlights the path on the SVG canvas.
