/**
 * ============================================================================
 * CAMPUS NAVIGATOR - CLIENT APPLICATION CONTROLLER
 * Connects directly to the C++17 Embedded REST Backend
 * Single Source of Truth for all Graph Algorithms & Topologies
 * ============================================================================
 */

(function () {
  'use strict';

  // API Host Base (auto-detects origin or falls back to port 8080)
  const API_BASE = window.location.origin.startsWith('http') 
    ? window.location.origin 
    : 'http://localhost:8080';

  class CampusNavigatorApp {
    constructor() {
      this.locations = [];
      this.edges = [];
      this.selectedOrigin = null;
      this.selectedDest = null;
      this.activeAlgorithm = 'dijkstra';
      this.currentRoute = null;
      this.showRoadLabels = true;
      this.zoomLevel = 1.0;
      this.viewBox = { x: 0, y: 0, w: 1000, h: 700 };

      this.dom = {
        selectOrigin: document.getElementById('select-origin'),
        selectDest: document.getElementById('select-dest'),
        backendStatus: document.getElementById('backend-status'),
        backendStatusText: document.getElementById('backend-status-text'),
        statVertices: document.getElementById('stat-vertices'),
        statEdges: document.getElementById('stat-edges'),
        statComponents: document.getElementById('stat-components'),
        statComponentsSub: document.getElementById('stat-components-sub'),
        statActiveAlgo: document.getElementById('stat-active-algo'),
        layerEdges: document.getElementById('layer-edges'),
        layerRoute: document.getElementById('layer-route'),
        layerEdgeLabels: document.getElementById('layer-edge-labels'),
        layerNodes: document.getElementById('layer-nodes'),
        svgViewport: document.getElementById('campus-svg'),
        mapTooltip: document.getElementById('map-tooltip'),
        resultPlaceholder: document.getElementById('result-placeholder'),
        resultDisconnected: document.getElementById('result-disconnected'),
        resultActiveContent: document.getElementById('result-active-content'),
        comparisonContent: document.getElementById('comparison-content'),
        resDistance: document.getElementById('res-distance'),
        resWalkTime: document.getElementById('res-walk-time'),
        resHops: document.getElementById('res-hops'),
        resNodesVisited: document.getElementById('res-nodes-visited'),
        resEdgesEvaluated: document.getElementById('res-edges-evaluated'),
        resExecTime: document.getElementById('res-exec-time'),
        resNotes: document.getElementById('res-notes'),
        itineraryTbody: document.getElementById('itinerary-tbody'),
        comparisonTbody: document.getElementById('comparison-tbody'),
        comparisonInsightsList: document.getElementById('comparison-insights-list'),
        searchResultsList: document.getElementById('search-results-list'),
        newRoadFrom: document.getElementById('new-road-from'),
        newRoadTo: document.getElementById('new-road-to')
      };
    }

    async init() {
      this.setupEventListeners();
      await this.checkBackendStatus();
      await this.loadCampusData();
      
      // Default to landmark scenario (North Gate to Computer Science)
      if (this.locations.length >= 4) {
        this.loadScenario(1, 4);
      }
    }

    setupEventListeners() {
      // Algorithm radio selector pills
      document.querySelectorAll('.algo-pill').forEach(pill => {
        pill.addEventListener('click', () => {
          document.querySelectorAll('.algo-pill').forEach(p => p.classList.remove('active'));
          pill.classList.add('active');
          const radio = pill.querySelector('input[type="radio"]');
          radio.checked = true;
          this.activeAlgorithm = radio.value;
          this.dom.statActiveAlgo.textContent = radio.value.toUpperCase();
          if (this.selectedOrigin && this.selectedDest) {
            this.handleFindRoute();
          }
        });
      });

      // Dropdown changes
      this.dom.selectOrigin.addEventListener('change', (e) => {
        this.selectedOrigin = parseInt(e.target.value, 10);
        this.updateNodeHighlights();
      });

      this.dom.selectDest.addEventListener('change', (e) => {
        this.selectedDest = parseInt(e.target.value, 10);
        this.updateNodeHighlights();
      });
    }

    async checkBackendStatus() {
      try {
        const res = await fetch(`${API_BASE}/api/status`);
        if (!res.ok) throw new Error('Status not OK');
        const data = await res.json();
        
        this.dom.backendStatus.className = 'status-pill status-online';
        this.dom.backendStatusText.textContent = `C++ REST Engine Online (v${data.version})`;
        this.dom.statVertices.textContent = data.vertices;
        this.dom.statEdges.textContent = data.directedEdges;
        this.dom.statComponents.textContent = data.connectedComponents;
        this.dom.statComponentsSub.textContent = data.isFullyConnected ? 'Fully Connected' : '1 Disconnected Satellite';
      } catch (err) {
        this.dom.backendStatus.className = 'status-pill status-offline';
        this.dom.backendStatusText.textContent = 'C++ Backend Offline';
      }
    }

    async loadCampusData() {
      try {
        const [locRes, edgeRes] = await Promise.all([
          fetch(`${API_BASE}/api/locations`),
          fetch(`${API_BASE}/api/edges`)
        ]);

        this.locations = await locRes.json();
        this.edges = await edgeRes.json();

        this.populateDropdowns();
        this.renderSchematicMap();
      } catch (err) {
        console.error('Failed to load campus graph data:', err);
      }
    }

    populateDropdowns() {
      const makeOptions = (list) => {
        let html = '<option value="" disabled selected>Choose campus landmark...</option>';
        list.forEach(loc => {
          html += `<option value="${loc.id}">[${loc.code}] ${loc.name} (${loc.category})</option>`;
        });
        return html;
      };

      const options = makeOptions(this.locations);
      this.dom.selectOrigin.innerHTML = options;
      this.dom.selectDest.innerHTML = options;
      this.dom.newRoadFrom.innerHTML = options;
      this.dom.newRoadTo.innerHTML = options;

      if (this.selectedOrigin) this.dom.selectOrigin.value = this.selectedOrigin;
      if (this.selectedDest) this.dom.selectDest.value = this.selectedDest;
    }

    /* ------------------------------------------------------------------------
       SVG Map Rendering
       ------------------------------------------------------------------------ */
    renderSchematicMap() {
      this.dom.layerEdges.innerHTML = '';
      this.dom.layerEdgeLabels.innerHTML = '';
      this.dom.layerNodes.innerHTML = '';

      // Build location lookup map
      const locMap = new Map();
      this.locations.forEach(l => locMap.set(l.id, l));

      // Deduplicate undirected edges for drawing lines
      const drawnPairs = new Set();
      this.edges.forEach(e => {
        const pairKey = e.from < e.to ? `${e.from}-${e.to}` : `${e.to}-${e.from}`;
        if (drawnPairs.has(pairKey)) return;
        drawnPairs.add(pairKey);

        const u = locMap.get(e.from);
        const v = locMap.get(e.to);
        if (!u || !v) return;

        // Draw Edge Line
        const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
        line.setAttribute('x1', u.x);
        line.setAttribute('y1', u.y);
        line.setAttribute('x2', v.x);
        line.setAttribute('y2', v.y);
        line.setAttribute('class', `map-edge ${e.accessible ? 'accessible-edge' : ''}`);
        line.setAttribute('id', `edge-${e.from}-${e.to}`);
        this.dom.layerEdges.appendChild(line);

        // Draw Edge Distance Badge
        if (this.showRoadLabels) {
          const midX = (u.x + v.x) / 2;
          const midY = (u.y + v.y) / 2;

          const g = document.createElementNS('http://www.w3.org/2000/svg', 'g');
          g.setAttribute('class', 'edge-label-group');

          const rect = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
          rect.setAttribute('x', midX - 18);
          rect.setAttribute('y', midY - 8);
          rect.setAttribute('width', 36);
          rect.setAttribute('height', 16);
          rect.setAttribute('class', 'edge-label-bg');

          const text = document.createElementNS('http://www.w3.org/2000/svg', 'text');
          text.setAttribute('x', midX);
          text.setAttribute('y', midY);
          text.setAttribute('class', 'edge-label-text');
          text.textContent = `${Math.round(e.weight)}m`;

          g.appendChild(rect);
          g.appendChild(text);
          this.dom.layerEdgeLabels.appendChild(g);
        }
      });

      // Render Nodes (Landmarks)
      this.locations.forEach(loc => {
        const g = document.createElementNS('http://www.w3.org/2000/svg', 'g');
        g.setAttribute('class', `map-node node-${loc.id}`);
        g.setAttribute('transform', `translate(${loc.x}, ${loc.y})`);
        g.setAttribute('data-id', loc.id);

        // Outer Halo for Start / Dest Highlights
        const halo = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
        halo.setAttribute('r', '26');
        halo.setAttribute('class', 'node-halo');

        // Main Node Card Box
        const box = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
        box.setAttribute('x', '-35');
        box.setAttribute('y', '-20');
        box.setAttribute('width', '70');
        box.setAttribute('height', '40');
        const catClass = this.getCategoryClass(loc.category, loc.id);
        box.setAttribute('class', `node-box ${catClass}`);

        // Building Code Text
        const codeText = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        codeText.setAttribute('y', '3');
        codeText.setAttribute('class', 'node-text-code');
        codeText.textContent = loc.code;

        // Building Name Subtext below box
        const nameText = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        nameText.setAttribute('y', '32');
        nameText.setAttribute('class', 'node-text-name');
        nameText.textContent = loc.name.length > 18 ? loc.name.substr(0, 16) + '..' : loc.name;

        g.appendChild(halo);
        g.appendChild(box);
        g.appendChild(codeText);
        g.appendChild(nameText);

        // Hover Tooltip Events
        g.addEventListener('mouseenter', (e) => this.showTooltip(e, loc));
        g.addEventListener('mouseleave', () => this.hideTooltip());
        
        // Click to Select Node as Origin / Destination
        g.addEventListener('click', () => this.handleNodeClick(loc.id));

        this.dom.layerNodes.appendChild(g);
      });

      this.updateNodeHighlights();
    }

    getCategoryClass(category, id) {
      if (id === 16) return 'cat-isolated';
      const cat = (category || '').toLowerCase();
      if (cat.includes('entrance') || cat.includes('gate')) return 'cat-entrance';
      if (cat.includes('academic') || cat.includes('cs') || cat.includes('math')) return 'cat-academic';
      if (cat.includes('admin')) return 'cat-administrative';
      if (cat.includes('recreation') || cat.includes('food')) return 'cat-recreation';
      if (cat.includes('sports') || cat.includes('gym')) return 'cat-sports';
      if (cat.includes('health') || cat.includes('clinic')) return 'cat-healthcare';
      if (cat.includes('hostel') || cat.includes('residence')) return 'cat-residential';
      if (cat.includes('research') || cat.includes('incubator')) return 'cat-research';
      return 'cat-academic';
    }

    showTooltip(e, loc) {
      const tt = this.dom.mapTooltip;
      tt.innerHTML = `
        <div style="font-weight:700; color:#60a5fa; margin-bottom:2px;">[${loc.code}] ${loc.name}</div>
        <div style="font-size:10px; color:#cbd5e1; margin-bottom:4px;">${loc.category}</div>
        <div style="font-size:11px; line-height:1.3;">${loc.description}</div>
        <div style="font-size:10px; color:#93c5fd; margin-top:4px;">Click to set as destination</div>
      `;
      tt.style.display = 'block';

      const rect = this.dom.svgViewport.getBoundingClientRect();
      const x = e.clientX - rect.left + 15;
      const y = e.clientY - rect.top + 15;
      tt.style.left = `${Math.min(x, rect.width - 240)}px`;
      tt.style.top = `${Math.min(y, rect.height - 110)}px`;
    }

    hideTooltip() {
      this.dom.mapTooltip.style.display = 'none';
    }

    handleNodeClick(id) {
      if (!this.selectedOrigin) {
        this.selectedOrigin = id;
        this.dom.selectOrigin.value = id;
      } else if (!this.selectedDest || this.selectedOrigin === id) {
        this.selectedDest = id;
        this.dom.selectDest.value = id;
        this.handleFindRoute();
      } else {
        this.selectedDest = id;
        this.dom.selectDest.value = id;
        this.handleFindRoute();
      }
      this.updateNodeHighlights();
    }

    updateNodeHighlights() {
      document.querySelectorAll('.map-node').forEach(n => {
        n.classList.remove('node-start', 'node-dest');
      });

      if (this.selectedOrigin) {
        const startNode = document.querySelector(`.map-node.node-${this.selectedOrigin}`);
        if (startNode) startNode.classList.add('node-start');
      }

      if (this.selectedDest) {
        const destNode = document.querySelector(`.map-node.node-${this.selectedDest}`);
        if (destNode) destNode.classList.add('node-dest');
      }
    }

    swapEndpoints() {
      const tmp = this.selectedOrigin;
      this.selectedOrigin = this.selectedDest;
      this.selectedDest = tmp;

      this.dom.selectOrigin.value = this.selectedOrigin || '';
      this.dom.selectDest.value = this.selectedDest || '';
      this.updateNodeHighlights();

      if (this.selectedOrigin && this.selectedDest) {
        this.handleFindRoute();
      }
    }

    /* ------------------------------------------------------------------------
       Route Calculation via C++ REST API
       ------------------------------------------------------------------------ */
    async handleFindRoute() {
      if (!this.selectedOrigin || !this.selectedDest) {
        alert('Please choose both an Origin and a Destination landmark.');
        return;
      }

      try {
        const url = `${API_BASE}/api/route?from=${this.selectedOrigin}&to=${this.selectedDest}&algo=${this.activeAlgorithm}`;
        const res = await fetch(url);
        const data = await res.json();

        this.dom.comparisonContent.style.display = 'none';
        this.dom.resultPlaceholder.style.display = 'none';

        if (!data.found) {
          // Disconnected Graph Handling
          this.dom.resultActiveContent.style.display = 'none';
          this.dom.resultDisconnected.style.display = 'flex';
          this.dom.layerRoute.innerHTML = '';
          return;
        }

        // Active Route Found
        this.dom.resultDisconnected.style.display = 'none';
        this.dom.resultActiveContent.style.display = 'block';

        this.renderRoutePath(data.path);
        this.renderRouteDetails(data);
      } catch (err) {
        console.error('Error fetching route from C++ engine:', err);
      }
    }

    renderRoutePath(path) {
      this.dom.layerRoute.innerHTML = '';
      if (!path || path.length < 2) return;

      const locMap = new Map();
      this.locations.forEach(l => locMap.set(l.id, l));

      for (let i = 0; i + 1 < path.length; ++i) {
        const u = locMap.get(path[i]);
        const v = locMap.get(path[i + 1]);
        if (!u || !v) continue;

        const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
        line.setAttribute('x1', u.x);
        line.setAttribute('y1', u.y);
        line.setAttribute('x2', v.x);
        line.setAttribute('y2', v.y);
        line.setAttribute('class', 'map-edge route-edge');
        line.setAttribute('marker-end', 'url(#arrowhead)');
        this.dom.layerRoute.appendChild(line);
      }
    }

    renderRouteDetails(data) {
      this.dom.resDistance.textContent = `${data.totalDistance.toFixed(1)} m`;
      const walkMinutes = (data.totalDistance / 83.33);
      this.dom.resWalkTime.textContent = walkMinutes < 1 
        ? `${Math.round(walkMinutes * 60)} sec walk`
        : `${Math.floor(walkMinutes)}m ${Math.round((walkMinutes % 1) * 60)}s walk`;

      this.dom.resHops.textContent = `${data.hops} hops`;
      this.dom.resNodesVisited.textContent = data.nodesVisited;
      this.dom.resEdgesEvaluated.textContent = data.edgeEvaluations;
      this.dom.resExecTime.textContent = `${data.executionTimeUs.toFixed(1)} µs`;
      this.dom.resNotes.textContent = data.notes;

      // Build turn-by-turn table
      let html = '';
      (data.legs || []).forEach(leg => {
        const mins = (leg.distance / 83.33);
        const paceStr = mins < 1 ? `${Math.round(mins * 60)}s` : `${Math.floor(mins)}m ${Math.round((mins % 1) * 60)}s`;

        html += `
          <tr>
            <td><strong>${leg.step}</strong></td>
            <td>${leg.fromName} <span class="search-code">${leg.fromCode}</span></td>
            <td>${leg.toName} <span class="search-code">${leg.toCode}</span></td>
            <td><strong>${leg.distance.toFixed(0)} m</strong></td>
            <td>${leg.cumDist.toFixed(0)} m</td>
            <td>${paceStr}</td>
            <td>${leg.roadType}</td>
            <td>${leg.accessible ? '<span style="color:#16a34a;">&#10003; Yes</span>' : '<span style="color:#64748b;">No</span>'}</td>
          </tr>
        `;
      });
      this.dom.itineraryTbody.innerHTML = html;
    }

    /* ------------------------------------------------------------------------
       Compare All 3 Algorithms
       ------------------------------------------------------------------------ */
    async handleCompareAll() {
      if (!this.selectedOrigin || !this.selectedDest) {
        alert('Please choose both an Origin and a Destination landmark to run the multi-algorithm comparison.');
        return;
      }

      try {
        const url = `${API_BASE}/api/compare?from=${this.selectedOrigin}&to=${this.selectedDest}`;
        const res = await fetch(url);
        const data = await res.json();

        this.dom.resultPlaceholder.style.display = 'none';
        this.dom.resultActiveContent.style.display = 'none';

        if (!data.isConnected) {
          this.dom.resultDisconnected.style.display = 'flex';
          this.dom.comparisonContent.style.display = 'none';
          this.dom.layerRoute.innerHTML = '';
          return;
        }

        this.dom.resultDisconnected.style.display = 'none';
        this.dom.comparisonContent.style.display = 'block';

        // Draw Dijkstra path by default
        this.renderRoutePath(data.dijkstra.path);

        const formatPath = (path) => {
          const locMap = new Map();
          this.locations.forEach(l => locMap.set(l.id, l));
          return path.map(id => {
            const loc = locMap.get(id);
            return loc ? loc.code : id;
          }).join(' &rarr; ');
        };

        const d = data.dijkstra;
        const b = data.bfs;
        const f = data.dfs;

        this.dom.comparisonTbody.innerHTML = `
          <tr>
            <td><strong>Optimality Status</strong></td>
            <td class="col-dijkstra"><strong>Globally Optimal Distance</strong></td>
            <td class="col-bfs"><strong>Optimal Minimum Hops</strong></td>
            <td class="col-dfs">Contiguous Exploration</td>
          </tr>
          <tr>
            <td><strong>Total Walking Distance</strong></td>
            <td class="col-dijkstra"><strong>${d.totalDistance.toFixed(1)} m</strong></td>
            <td class="col-bfs">${b.totalDistance.toFixed(1)} m</td>
            <td class="col-dfs">${f.totalDistance.toFixed(1)} m</td>
          </tr>
          <tr>
            <td><strong>Navigational Hops (Edges)</strong></td>
            <td class="col-dijkstra">${d.hops} edges</td>
            <td class="col-bfs"><strong>${b.hops} edges</strong></td>
            <td class="col-dfs">${f.hops} edges</td>
          </tr>
          <tr>
            <td><strong>Vertices Expanded (Settled)</strong></td>
            <td class="col-dijkstra">${d.nodesVisited}</td>
            <td class="col-bfs">${b.nodesVisited}</td>
            <td class="col-dfs">${f.nodesVisited}</td>
          </tr>
          <tr>
            <td><strong>Edge Checks / Relaxations</strong></td>
            <td class="col-dijkstra">${d.edgeEvaluations}</td>
            <td class="col-bfs">${b.edgeEvaluations}</td>
            <td class="col-dfs">${f.edgeEvaluations}</td>
          </tr>
          <tr>
            <td><strong>C++ Execution Time</strong></td>
            <td class="col-dijkstra">${d.executionTimeUs.toFixed(1)} µs</td>
            <td class="col-bfs">${b.executionTimeUs.toFixed(1)} µs</td>
            <td class="col-dfs">${f.executionTimeUs.toFixed(1)} µs</td>
          </tr>
          <tr>
            <td><strong>Path Waypoints Sequence</strong></td>
            <td class="col-dijkstra"><small>${formatPath(d.path)}</small></td>
            <td class="col-bfs"><small>${formatPath(b.path)}</small></td>
            <td class="col-dfs"><small>${formatPath(f.path)}</small></td>
          </tr>
        `;

        // Insights Box
        let insightsHtml = '<strong>ADSA Theoretical Findings & Insights:</strong><ul>';
        (data.insights || []).forEach(inText => {
          insightsHtml += `<li>${inText}</li>`;
        });
        insightsHtml += '</ul>';
        this.dom.comparisonInsightsList.innerHTML = insightsHtml;
      } catch (err) {
        console.error('Error fetching algorithm comparison:', err);
      }
    }

    loadScenario(fromId, toId) {
      this.selectedOrigin = fromId;
      this.selectedDest = toId;
      this.dom.selectOrigin.value = fromId;
      this.dom.selectDest.value = toId;
      this.updateNodeHighlights();
      this.handleFindRoute();
    }

    /* ------------------------------------------------------------------------
       Live Search
       ------------------------------------------------------------------------ */
    handleLiveSearch(query) {
      const q = (query || '').toLowerCase().trim();
      if (!q) {
        this.dom.searchResultsList.innerHTML = '';
        document.querySelectorAll('.map-node').forEach(n => n.style.opacity = '1');
        return;
      }

      const matches = this.locations.filter(loc => 
        loc.name.toLowerCase().includes(q) ||
        loc.code.toLowerCase().includes(q) ||
        loc.category.toLowerCase().includes(q) ||
        loc.description.toLowerCase().includes(q)
      );

      const matchedIds = new Set(matches.map(m => m.id));
      document.querySelectorAll('.map-node').forEach(n => {
        const id = parseInt(n.getAttribute('data-id'), 10);
        n.style.opacity = matchedIds.has(id) ? '1' : '0.25';
      });

      let html = '';
      matches.slice(0, 6).forEach(loc => {
        html += `
          <div class="search-item" onclick="window.app.selectFromSearch(${loc.id})">
            <div class="search-item-header">
              <span>${loc.name}</span>
              <span class="search-code">${loc.code}</span>
            </div>
            <div class="search-item-desc">${loc.category} &bull; ${loc.description}</div>
          </div>
        `;
      });
      this.dom.searchResultsList.innerHTML = html;
    }

    selectFromSearch(id) {
      if (!this.selectedOrigin) {
        this.selectedOrigin = id;
        this.dom.selectOrigin.value = id;
      } else {
        this.selectedDest = id;
        this.dom.selectDest.value = id;
        this.handleFindRoute();
      }
      this.updateNodeHighlights();
      this.dom.searchResultsList.innerHTML = '';
      document.querySelectorAll('.map-node').forEach(n => n.style.opacity = '1');
    }

    /* ------------------------------------------------------------------------
       Dynamic Graph Modification Forms
       ------------------------------------------------------------------------ */
    async submitAddLocation() {
      const name = document.getElementById('new-loc-name').value.trim();
      const code = document.getElementById('new-loc-code').value.trim();
      const category = document.getElementById('new-loc-category').value;
      const description = document.getElementById('new-loc-desc').value.trim();
      const x = parseFloat(document.getElementById('new-loc-x').value) || 450;
      const y = parseFloat(document.getElementById('new-loc-y').value) || 350;

      const payload = { name, code, category, description, x, y };
      try {
        const res = await fetch(`${API_BASE}/api/locations`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(payload)
        });
        const data = await res.json();
        if (data.success) {
          this.closeModal('modal-add-location');
          await this.loadCampusData();
          await this.checkBackendStatus();
          alert(`Success: [${code}] ${name} has been added to the campus graph!`);
        } else {
          alert(`Error: ${data.error}`);
        }
      } catch (err) {
        alert('Failed to connect to backend server.');
      }
    }

    async submitAddRoad() {
      const from = parseInt(document.getElementById('new-road-from').value, 10);
      const to = parseInt(document.getElementById('new-road-to').value, 10);
      const weight = parseFloat(document.getElementById('new-road-weight').value);
      const roadType = document.getElementById('new-road-type').value.trim() || 'Paved Walkway';
      const bidirectional = document.getElementById('new-road-bidi').checked;
      const accessible = document.getElementById('new-road-accessible').checked;

      const payload = { from, to, weight, roadType, bidirectional, accessible };
      try {
        const res = await fetch(`${API_BASE}/api/edges`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(payload)
        });
        const data = await res.json();
        if (data.success) {
          this.closeModal('modal-add-road');
          await this.loadCampusData();
          await this.checkBackendStatus();
          if (this.selectedOrigin && this.selectedDest) {
            this.handleFindRoute();
          }
          alert('Pathway connection successfully added to C++ adjacency list!');
        } else {
          alert(`Error: ${data.error}`);
        }
      } catch (err) {
        alert('Failed to connect to backend server.');
      }
    }

    async connectObservatoryTrail() {
      // Connects Lakeside Pavillion (15) to Isolated Space Observatory (16)
      const payload = {
        from: 15,
        to: 16,
        weight: 450.0,
        roadType: 'Observatory Hill Trail',
        bidirectional: true,
        accessible: false
      };

      try {
        const res = await fetch(`${API_BASE}/api/edges`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(payload)
        });
        const data = await res.json();
        if (data.success) {
          await this.loadCampusData();
          await this.checkBackendStatus();
          this.loadScenario(1, 16);
          alert('Observatory Hill Trail constructed! Space Observatory is now connected to the campus network.');
        } else {
          alert(`Error: ${data.error}`);
        }
      } catch (err) {
        alert('Failed to connect to backend server.');
      }
    }

    async resetCampusData() {
      if (!confirm('Reset campus graph to default benchmark layout?')) return;
      try {
        await fetch(`${API_BASE}/api/reset`, { method: 'POST' });
        await this.loadCampusData();
        await this.checkBackendStatus();
        this.loadScenario(1, 4);
      } catch (err) {
        alert('Failed to reset campus graph.');
      }
    }

    /* ------------------------------------------------------------------------
       UI Helpers
       ------------------------------------------------------------------------ */
    toggleRoadLabels() {
      this.showRoadLabels = !this.showRoadLabels;
      document.getElementById('btn-toggle-labels').textContent = `Distances: ${this.showRoadLabels ? 'ON' : 'OFF'}`;
      this.dom.layerEdgeLabels.style.display = this.showRoadLabels ? 'block' : 'none';
    }

    zoomMap(factor) {
      this.zoomLevel *= factor;
      this.zoomLevel = Math.max(0.6, Math.min(2.5, this.zoomLevel));
      this.applyViewBox();
    }

    resetMapZoom() {
      this.zoomLevel = 1.0;
      this.applyViewBox();
    }

    applyViewBox() {
      const baseW = 1000;
      const baseH = 700;
      const curW = baseW / this.zoomLevel;
      const curH = baseH / this.zoomLevel;
      const curX = (baseW - curW) / 2;
      const curY = (baseH - curH) / 2;
      this.dom.svgViewport.setAttribute('viewBox', `${curX} ${curY} ${curW} ${curH}`);
    }

    openModal(modalId) {
      const modal = document.getElementById(modalId);
      if (modal) modal.style.display = 'flex';
    }

    closeModal(modalId) {
      const modal = document.getElementById(modalId);
      if (modal) modal.style.display = 'none';
    }

    toggleAccordion(bodyId) {
      const el = document.getElementById(bodyId);
      if (el) el.classList.toggle('open');
    }
  }

  // Initialize and mount globally
  window.app = new CampusNavigatorApp();
  document.addEventListener('DOMContentLoaded', () => window.app.init());

})();
