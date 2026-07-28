# INSTABUILT: BLUEPRINT EMPIRE
## Software Architecture Blueprint v1.0

**Document Type:** Software Architecture Blueprint
**Audience:** Engineering Team, Technical Directors, Systems Architects
**Status:** Architecture Locked — Ready for Detailed System Design
**Target Lifespan:** 10+ years of active development and maintenance
**Rule 0:** This document defines HOW systems connect, NOT what they do. The GSS defines what. This defines how.

---

## 1. ARCHITECTURAL PHILOSOPHY

### 1.1 Primary Architectural Style

**Entity-Component-System (ECS) with Data-Oriented Design, layered within a Service-Oriented Core.**

```
┌─────────────────────────────────────────────────────────────┐
│                    PRESENTATION LAYER                        │
│  (Rendering, Audio, UI, Input — Unreal Engine 5)            │
├─────────────────────────────────────────────────────────────┤
│                    APPLICATION LAYER                         │
│  (Game Modes, State Machines, Player Workflows)             │
├─────────────────────────────────────────────────────────────┤
│                      DOMAIN LAYER                            │
│  (ECS Simulation: Buildings, Workers, Economy, Weather)     │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐          │
│  │Building │ │ Worker  │ │Contract │ │Economy  │  ...      │
│  │ System  │ │ System  │ │ System  │ │ System  │          │
│  └─────────┘ └─────────┘ └─────────┘ └─────────┘          │
├─────────────────────────────────────────────────────────────┤
│                  INFRASTRUCTURE LAYER                        │
│  (Serialization, Networking, Analytics, Localization)       │
├─────────────────────────────────────────────────────────────┤
│                   PERSISTENCE LAYER                          │
│  (Save/Load, Configuration, Asset Database, Cloud Sync)     │
└─────────────────────────────────────────────────────────────┘
```

### 1.2 Why This Style

| Factor | Reasoning |
|--------|-----------|
| **ECS** | The game simulates thousands of independent entities (buildings, workers, vehicles, materials, contracts, city elements). ECS provides cache-friendly data layout, trivial parallelism, and clean separation of data from behavior. Adding a new building type means adding components, not modifying existing code. |
| **Data-Oriented Design** | Construction simulation involves processing large batches of homogeneous data (all workers on all sites, all material deliveries, all weather effects). DOD keeps data contiguous in memory, maximizing CPU cache utilization and enabling SIMD operations where beneficial. |
| **Service-Oriented Core** | Cross-cutting concerns (logging, analytics, save/load, localization) are provided as services. Systems request services via interfaces, not concrete implementations. This enables mocking for tests, swapping implementations per platform, and adding new services without touching existing code. |
| **Layered Architecture** | Each layer depends only on the layer below it. The presentation layer never touches simulation data directly. The domain layer never knows about rendering. This enables: (a) headless simulation for testing and server-side computation, (b) swapping the entire UI without touching game logic, (c) future multiplayer where simulation runs on a server and presentation on clients. |

### 1.3 Trade-Offs Accepted

| Trade-Off | Why Accepted |
|-----------|--------------|
| **ECS learning curve** | ECS requires the team to think in data flows rather than object hierarchies. However, the 10-year lifespan justifies this investment. Once internalized, ECS is more maintainable than deep inheritance trees. |
| **More boilerplate than monolithic OOP** | ECS requires explicit system registration, component definition, and query setup. The payoff is that adding features requires adding components and systems — never modifying existing code. Mod support benefits enormously from this. |
| **Double bookkeeping during prototyping** | ECS separates data (components) from behavior (systems), which can feel unnatural during rapid prototyping. We mitigate this with a lightweight scripting layer for prototyping that compiles down to ECS systems for production. |
| **UE5 rendering + custom simulation layer** | Using Unreal for rendering but a custom ECS core for simulation creates an integration boundary. However, UE5 provides world-class rendering, tools, and asset pipeline that would take years to build in-house. The simulation layer is where our unique value lives — we build that ourselves. |

### 1.4 Long-Term Benefits

1. **Parallelism scales with hardware:** ECS queries naturally parallelize. As CPUs add cores, our simulation scales without architectural changes.
2. **Mod support is architectural, not bolted on:** Components and systems are data-driven. Modders add components, systems, and data tables — same mechanism as our internal developers.
3. **Multiplayer is deferred, not impossible:** The simulation layer is designed to be deterministic. When multiplayer is added, the simulation runs on the server and syncs state deltas to clients — no architectural rewrite needed.
4. **10-year maintainability:** Systems are isolated. A developer can understand the Weather System by reading one folder. Bugs in weather cannot corrupt construction data because the systems communicate through defined interfaces.
5. **Testability by construction:** Every system can be tested in isolation by providing mock inputs and asserting outputs. The headless simulation mode enables automated regression testing of entire gameplay scenarios.

---

## 2. HIGH-LEVEL ARCHITECTURE

### 2.1 System Diagram

```
┌─────────────────────────────────────────────────────────────────────────────────────┐
│                              INSTABUILT ENGINE                                       │
│                                                                                      │
│  ┌─────────────────────── CLIENT (Player-Facing) ───────────────────────────────┐   │
│  │                                                                               │   │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐       │   │
│  │  │ RENDERER │  │   AUDIO  │  │    UI    │  │  INPUT   │  │  CAMERA  │       │   │
│  │  │ (UE5)    │  │  (Wwise) │  │ (Noesis) │  │ (UE5)    │  │ System   │       │   │
│  │  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬─────┘       │   │
│  │       │              │              │              │              │            │   │
│  │       └──────────────┴──────────────┴──────────────┴──────────────┘            │   │
│  │                                     │                                          │   │
│  │                          ┌──────────┴──────────┐                               │   │
│  │                          │  PRESENTATION BUS   │                               │   │
│  │                          │  (Observer Pattern)  │                               │   │
│  │                          └──────────┬──────────┘                               │   │
│  └─────────────────────────────────────┼──────────────────────────────────────────┘   │
│                                        │                                              │
│  ┌─────────────────────────────────────┼──────────────────────────────────────────┐   │
│  │                          APPLICATION LAYER                                      │   │
│  │                                     │                                          │   │
│  │  ┌──────────────────────────────────┴──────────────────────────────────┐       │   │
│  │  │                      GAME MODE CONTROLLER                            │       │   │
│  │  │  (Career | Sandbox | Scenario | Creative | Multiplayer — future)     │       │   │
│  │  └──────────────────────────────────┬──────────────────────────────────┘       │   │
│  │                                     │                                          │   │
│  │  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐       │   │
│  │  │  DESIGN      │  │ CONSTRUCTION │  │  MANAGEMENT  │  │  ONBOARDING  │       │   │
│  │  │  WORKFLOW    │  │  WORKFLOW    │  │  WORKFLOW    │  │  CONTROLLER  │       │   │
│  │  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘       │   │
│  │         │                  │                  │                  │              │   │
│  └─────────┼──────────────────┼──────────────────┼──────────────────┼──────────────┘   │
│            │                  │                  │                  │                  │
│  ┌─────────┼──────────────────┼──────────────────┼──────────────────┼──────────────┐   │
│  │         │           COMMAND BUS (CQRS)          │                  │              │   │
│  │         └──────────────────┴──────────────────┴──────────────────┘              │   │
│  │                                     │                                          │   │
│  │                          ┌──────────┴──────────┐                               │   │
│  │                          │    SYSTEM ORCHESTRATOR   │                           │   │
│  │                          │  (Frame Update Scheduler) │                           │   │
│  │                          └──────────┬──────────┘                               │   │
│  │                                     │                                          │   │
│  │    ┌────────────────────────────────┼────────────────────────────────┐         │   │
│  │    │                    DOMAIN LAYER (ECS Core)                       │         │   │
│  │    │                                                                 │         │   │
│  │    │  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐           │         │   │
│  │    │  │ BUILDING │ │  WORKER  │ │ CONTRACT │ │ ECONOMY  │           │         │   │
│  │    │  │ SYSTEM   │ │  SYSTEM  │ │ SYSTEM   │ │ SYSTEM   │  ...      │         │   │
│  │    │  └────┬─────┘ └────┬─────┘ └────┬─────┘ └────┬─────┘           │         │   │
│  │    │       │                                                        │         │   │
│  │    │  ┌────┴─────┐ ┌────┴─────┐ ┌────┴─────┐ ┌────┴─────┐           │         │   │
│  │    │  │ WEATHER  │ │  TIME    │ │REPUTATION│ │ RESEARCH │           │         │   │
│  │    │  │ SYSTEM   │ │  SYSTEM  │ │ SYSTEM   │ │ SYSTEM   │           │         │   │
│  │    │  └──────────┘ └──────────┘ └──────────┘ └──────────┘           │         │   │
│  │    │                                                                 │         │   │
│  │    │              All systems communicate via EVENT BUS              │         │   │
│  │    │                                                                 │         │   │
│  │    └─────────────────────────────────────────────────────────────────┘         │   │
│  │                                                                                │   │
│  └────────────────────────────────────────────────────────────────────────────────┘   │
│                                                                                      │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐              │
│  │  SAVE /  │  │  ASSET   │  │  CONFIG  │  │  MOD     │  │ANALYTICS │              │
│  │  LOAD    │  │DATABASE  │  │ MANAGER │  │  LOADER  │  │ SERVICE  │              │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘  └──────────┘              │
│                                                                                      │
│  ┌──────────────────────────────────────────────────────────────────────────┐       │
│  │                        DEVELOPER TOOLS & EDITOR                           │       │
│  │  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐       │       │
│  │  │ BUILDING │ │CONTRACT  │ │ECONOMY   │ │REPLAY    │ │PERFORM.  │       │       │
│  │  │ DESIGNER │ │ EDITOR   │ │BALANCER  │ │ VIEWER   │ │PROFILER  │       │       │
│  │  └──────────┘ └──────────┘ └──────────┘ └──────────┘ └──────────┘       │       │
│  └──────────────────────────────────────────────────────────────────────────┘       │
│                                                                                      │
└─────────────────────────────────────────────────────────────────────────────────────┘
```

### 2.2 Data Flow Philosophy

Data flows in ONE direction through the system:

```
INPUT → COMMAND → SYSTEM ORCHESTRATOR → ECS SYSTEMS → EVENT BUS → PRESENTATION
                                                              ↓
                                                         PERSISTENCE
```

- **Input** (keyboard, mouse, controller, UI clicks) produces **Commands**.
- **Commands** are validated by the Application Layer and translated into domain operations.
- **ECS Systems** process the domain state update each frame.
- **Events** emitted by systems flow to the Presentation Layer (for rendering updates) and Persistence Layer (for save data).
- **NEVER** does the Presentation Layer directly modify domain state. It can only issue Commands.
- **NEVER** does a domain system directly call a presentation system. It emits events and the presentation layer subscribes.

### 2.3 Engine Foundation

**Recommendation: Custom ECS Core integrated with Unreal Engine 5 for rendering.**

```
┌──────────────────────────────────────┐
│          UNREAL ENGINE 5             │
│  ┌────────────────────────────────┐  │
│  │  Rendering (Nanite, Lumen)     │  │
│  │  Physics (Chaos)               │  │
│  │  Audio (MetaSounds → Wwise)    │  │
│  │  Input System                  │  │
│  │  Asset Pipeline                │  │
│  │  Editor Framework              │  │
│  │  Blueprint (prototyping only)  │  │
│  └────────────────────────────────┘  │
│                                      │
│  ┌────────────────────────────────┐  │
│  │  INSTABUILT SIMULATION CORE    │  │
│  │  (Custom C++ ECS)              │  │
│  │  - Entity Manager              │  │
│  │  - Component Storage (SoA)     │  │
│  │  - System Scheduler            │  │
│  │  - Event Bus                   │  │
│  │  - Command Processor           │  │
│  │  - Deterministic RNG           │  │
│  └────────────────────────────────┘  │
└──────────────────────────────────────┘
```

**Why UE5 for rendering:** Nanite provides automatic LOD for thousands of buildings. Lumen provides dynamic global illumination that makes buildings look photorealistic at any time of day. The asset pipeline is production-proven. This saves 3-4 years of rendering engine development.

**Why custom ECS core:** UE's built-in Actor/Component model is object-oriented and not designed for simulating 10,000+ independent entities at 60fps. Mass Entity (UE5's ECS) is promising but immature for production (as of UE 5.4). A custom ECS core gives us full control over memory layout, scheduling, and determinism. The integration cost with UE5 rendering is acceptable (~6 months of engine work) given the 10-year lifespan.

**Alternative considered:** Pure Unity DOTS. Rejected because Unity's rendering pipeline does not match UE5's visual quality for architectural visualization, which is critical for player satisfaction. Building photorealism is core to the "pride in your creation" emotional loop.

---

## 3. CORE MODULES

### 3.1 Entity-Component-System Core

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Foundation of all simulation. Manages entity lifecycle, component storage, and system execution. |
| **Responsibilities** | Entity creation/destruction, archetype-based component storage (struct-of-arrays), system dependency resolution, parallel job scheduling, entity query execution. |
| **Inputs** | Commands from Application Layer (CreateEntity, AddComponent, RemoveComponent, DestroyEntity). |
| **Outputs** | Events to Event Bus (EntityCreated, ComponentChanged, EntityDestroyed). |
| **Dependencies** | None (lowest-level module). |
| **Ownership** | Core Engine Team. |
| **Lifecycle** | Initialized at engine startup. Persists until shutdown. |
| **Communication** | Direct API calls for entity operations. Event Bus for state change notifications. |

### 3.2 Building System

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Manages all building entities: their design data, construction state, physical representation, and lifecycle. |
| **Responsibilities** | Building design storage (walls, rooms, MEP, materials), construction phase tracking, structural validation, quality computation, building-to-renderable translation. |
| **Inputs** | Design commands (PlaceWall, AddRoom, SetMaterial), construction progress from Construction System, inspection results. |
| **Outputs** | Building state events (PhaseChanged, ConstructionMilestone, QualityUpdated), renderable data for Presentation Layer. |
| **Dependencies** | ECS Core, Material Database, Structural Validation Rules. |
| **Ownership** | Building & Design Team. |
| **Lifecycle** | Per-building: created during design, persists through construction, persists permanently after completion. |
| **Communication** | Commands for design changes. Events for state updates. Direct queries for validation. |

### 3.3 Construction System

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Simulates construction progress on active building sites. Manages phases, crews, equipment, materials, and site events. |
| **Responsibilities** | Phase sequencing, task scheduling, crew productivity calculation, material consumption tracking, equipment utilization, weather impact on progress, problem generation (delays, quality issues). |
| **Inputs** | Building design data, crew assignments, equipment assignments, material availability from Inventory System, weather from Weather System, time from Time System. |
| **Outputs** | Construction progress events, phase completion events, quality scores, material consumption records, problem notifications. |
| **Dependencies** | Building System, Worker System, Equipment System, Inventory System, Weather System, Time System, Safety System. |
| **Ownership** | Construction & Simulation Team. |
| **Lifecycle** | Active per construction site. System wakes/sleeps based on whether any sites are actively building. |
| **Communication** | Event-driven: subscribes to TimeTick, WeatherChange, WorkerAssignment. Publishes ConstructionProgress, PhaseCompleted, ProblemDetected. |

### 3.4 Worker System

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Manages all worker entities: hiring, skills, assignments, morale, fatigue, and career progression. |
| **Responsibilities** | Worker skill calculation, productivity computation, fatigue accumulation/recovery, morale dynamics, task assignment resolution, payroll processing, training tracking. |
| **Inputs** | Commands (Hire, Assign, Train, Promote, Terminate), time from Time System, events from Safety System (injuries affecting workers). |
| **Outputs** | Worker state events (SkillChanged, MoraleChanged, Fatigued), productivity values consumed by Construction System. |
| **Dependencies** | ECS Core, Time System, Economy System (for wage data). |
| **Ownership** | Management Systems Team. |
| **Lifecycle** | Workers exist from hire to termination/retirement. Persistent across game sessions. |
| **Communication** | Direct queries for worker stats (Construction System asks "how productive is this crew?"). Events for state changes. |

### 3.5 Contract System

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Manages contract lifecycle: generation, bidding, awarding, tracking, completion, and disputes. |
| **Responsibilities** | Contract generation (procedural from templates), bid evaluation (player vs AI competitors), contract award logic, requirement tracking, milestone payment processing, deadline enforcement, penalty/bonus calculation. |
| **Inputs** | Player bids (price, timeline, quality tier), market conditions from Economy System, player reputation from Reputation System, region data. |
| **Outputs** | Contract events (Awarded, MilestoneReached, Completed, Breached), payment events to Economy System, reputation events to Reputation System. |
| **Dependencies** | Economy System, Reputation System, Region System, Building System (for requirement validation). |
| **Ownership** | Business Systems Team. |
| **Lifecycle** | Contracts are generated continuously. Active contracts tracked. Completed contracts archived. |
| **Communication** | Event-driven for contract lifecycle. Query-based for available contract listing. |

### 3.6 Economy System

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Simulates the regional and global economy: material prices, labor markets, interest rates, property values, economic cycles. |
| **Responsibilities** | Price fluctuation (supply/demand per material per region), inflation modeling, interest rate adjustment (central bank simulation), labor market dynamics (wage pressure), economic cycle (boom/bust/recovery), competitor pricing. |
| **Inputs** | Time progression, player actions (large purchases affect local demand), region events. |
| **Outputs** | Price data consumed by Contract System, Construction System, and Management UI. Economic indicators for dashboards. |
| **Dependencies** | Time System, Region System. |
| **Ownership** | Economy & Balance Team. |
| **Lifecycle** | Continuous simulation. Runs at reduced frequency (1 tick per in-game day) compared to frame-level systems. |
| **Communication** | Polled by other systems ("what's the current price of concrete in North Region?"). Events for major shifts (recession announced). |

### 3.7 Weather System

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Generates region-specific weather patterns affecting construction, worker productivity, and visual presentation. |
| **Responsibilities** | Weather generation (procedural based on region climate profile + season), 7-day forecast, weather event triggering (storms, heatwaves), construction impact calculation. |
| **Inputs** | Time System (date, season), Region System (climate profile). |
| **Outputs** | Current weather state, forecast data, weather events (RainStarted, StormWarning, HeatwaveActive). |
| **Dependencies** | Time System, Region System. |
| **Ownership** | Simulation Team. |
| **Lifecycle** | Continuous. Weather calculated per in-game day. |
| **Communication** | Events for weather changes. Polled by Construction System for impact calculations. |

### 3.8 Time System

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Central time authority. Manages game time progression, speed control, calendar, working hours. |
| **Responsibilities** | Time advancement (at configurable speeds), pause/resume, calendar tracking (date, day of week, season, year), working hours enforcement, deadline tracking, event scheduling. |
| **Inputs** | Speed change commands from player, pause commands. |
| **Outputs** | TimeTick events at configured intervals (frame, hour, day, week, month, season, year). Current time data for all other systems. |
| **Dependencies** | None (low-level system). |
| **Ownership** | Core Engine Team. |
| **Lifecycle** | Initialized at game start. Persists until shutdown. |
| **Communication** | Events at every time granularity. Direct polling for current time. |

### 3.9 Reputation System

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Tracks the player company's multi-axis reputation across regions. |
| **Responsibilities** | Reputation calculation (Quality, Reliability, Innovation, Community, Safety), per-region tracking, reputation event processing, reputation-based unlocking (contract tiers, client access). |
| **Inputs** | Events from Construction System (quality scores), Contract System (delivery timeliness), Safety System (incidents), Economy System (community projects). |
| **Outputs** | Reputation scores consumed by Contract System, Client AI, Dashboard UI. |
| **Dependencies** | Region System. |
| **Ownership** | Business Systems Team. |
| **Lifecycle** | Continuous. Updated after each project completion and significant event. |
| **Communication** | Event subscriber (listens to all reputation-affecting events). Polled for current scores. |

### 3.10 Safety System

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Simulates construction site safety: incident generation, severity calculation, and impact propagation. |
| **Responsibilities** | Incident probability calculation (based on crew fatigue, weather, equipment condition, difficulty setting, safety investments), incident resolution (injury severity, response options), safety reputation impact. |
| **Inputs** | Worker fatigue data, weather conditions, equipment condition, site phase, safety investment level, difficulty setting. |
| **Outputs** | Safety events (IncidentOccurred with severity), reputation impacts, worker state changes (injured workers). |
| **Dependencies** | Worker System, Weather System, Equipment System, Time System. |
| **Ownership** | Simulation Team. |
| **Lifecycle** | Active during construction phases. Dormant otherwise. |
| **Communication** | Event publisher for incidents. Polls other systems for risk factors. |

### 3.11 Save/Load System

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Persists and restores complete game state. Supports save slots, auto-save, and cloud sync. |
| **Responsibilities** | Game state serialization (all ECS components, system states, configuration), save file management (slots, versioning, compression), auto-save scheduling, save file validation, backward compatibility (loading old saves on new versions), cloud storage integration. |
| **Inputs** | Save command (manual or auto-save trigger), entire ECS world state. |
| **Outputs** | Save files (structured binary format), load events (WorldRestored). |
| **Dependencies** | All domain systems (reads their state). None (writes). |
| **Ownership** | Core Engine Team. |
| **Lifecycle** | Available at all times. Auto-save runs on 15-minute timer + milestone triggers. |
| **Communication** | Queries all systems for state during save. Restores all systems during load. Version migration layer handles schema changes. |

### 3.12 UI System

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Renders all user interface: dashboards, HUDs, menus, tooltips, notifications. |
| **Responsibilities** | UI rendering via Noesis GUI (vector-based, data-bound), UI state management, data binding to domain models, input handling for UI elements, accessibility features (screen reader, scaling). |
| **Inputs** | Domain state via Presentation Bus, player input events, accessibility settings. |
| **Outputs** | Command objects to Application Layer (player actions). |
| **Dependencies** | Localization System, Accessibility System, Presentation Bus. |
| **Ownership** | UI Team. |
| **Lifecycle** | Always active. Different UI contexts loaded per game mode. |
| **Communication** | Subscribes to Presentation Bus for data. Publishes Commands for player actions. NEVER directly accesses domain systems. |

### 3.13 Camera System

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Implements the seven camera modes defined in the GSS with smooth transitions. |
| **Responsibilities** | Camera mode management, smooth interpolation between modes, collision detection, bookmarks, first-person controller, drone flight model, cinematic presets. |
| **Inputs** | Player input (mouse, keyboard, controller), camera mode commands, focus targets. |
| **Outputs** | View matrix, projection matrix, camera state events (ModeChanged). |
| **Dependencies** | Input System, Physics (for collision). |
| **Ownership** | Rendering Team. |
| **Lifecycle** | Always active. Mode-configurable. |
| **Communication** | Direct rendering pipeline integration. Commands from Input System. |

### 3.14 Analytics Service

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Collects anonymized gameplay telemetry for balancing, bug detection, and player behavior understanding. |
| **Responsibilities** | Event ingestion, batching, transmission, privacy filtering, opt-out support. |
| **Inputs** | Analytics events from all other systems (gameplay actions, performance metrics, error events). |
| **Outputs** | Batched telemetry to analytics backend. |
| **Dependencies** | None (all systems push events to it). |
| **Ownership** | Platform & Services Team. |
| **Lifecycle** | Always active. Buffers events locally, flushes periodically. |
| **Communication** | Fire-and-forget event submission. Never blocks gameplay. |

### 3.15 Localization System

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Provides translated strings and localized assets for all supported languages. |
| **Responsibilities** | String table management, locale detection, fallback chain, pluralization, gendered text, localized number/date/currency formatting, asset swapping (region-specific textures). |
| **Inputs** | Current locale setting, string keys from all UI and game systems. |
| **Outputs** | Localized strings and formatted values. |
| **Dependencies** | None (service consumed by all other modules). |
| **Ownership** | Localization Team. |
| **Lifecycle** | Loaded at startup. Hot-reloadable for development. |
| **Communication** | Function call interface: `Loc.Get("building.roof.error.pitch")` returns localized string. |

### 3.16 Mod Loader

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Loads, validates, and integrates player-created modifications. |
| **Responsibilities** | Mod discovery (filesystem + Steam Workshop), dependency resolution, version compatibility checking, sandboxed loading, data table merging, system injection, conflict detection between mods. |
| **Inputs** | Mod files (data tables, scripts, assets), mod metadata. |
| **Outputs** | Modified game state (merged data tables, registered additional systems/components). |
| **Dependencies** | ECS Core (registers new components/systems), Config Manager (merges data tables), Asset Database. |
| **Ownership** | Platform & Services Team. |
| **Lifecycle** | Executes during game initialization before any domain system starts. |
| **Communication** | Extends core systems through defined extension points. Never patches binaries directly. |

---

## 4. MODULE BOUNDARIES

### 4.1 Allowed Communication Matrix

```
                     FROM →
              ↓ TO   │ECS│BLD│CNS│WRK│CNT│ECO│WTH│TIM│REP│SAF│SAV│ UI │CAM│ANA│LOC│MOD│
     ┌───────────────┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
     │ ECS Core      │ - │   │   │   │   │   │   │   │   │   │   │   │   │   │   │   │
     │ Building      │ ✓ │ - │   │   │   │   │   │   │   │   │   │   │   │   │   │   │
     │ Construction  │ ✓ │ ✓ │ - │ ✓ │   │   │ ✓ │ ✓ │   │ ✓ │   │   │   │   │   │   │
     │ Worker        │ ✓ │   │   │ - │   │   │   │ ✓ │   │   │   │   │   │   │   │   │
     │ Contract      │ ✓ │ ✓ │   │   │ - │ ✓ │   │ ✓ │ ✓ │   │   │   │   │   │   │   │
     │ Economy       │ ✓ │   │   │   │   │ - │   │ ✓ │   │   │   │   │   │   │   │   │
     │ Weather       │ ✓ │   │   │   │   │   │ - │ ✓ │   │   │   │   │   │   │   │   │
     │ Time          │ ✓ │   │   │   │   │   │   │ - │   │   │   │   │   │   │   │   │
     │ Reputation    │ ✓ │   │   │   │ ✓ │   │   │   │ - │ ✓ │   │   │   │   │   │   │
     │ Safety        │ ✓ │   │   │ ✓ │   │   │ ✓ │ ✓ │   │ - │   │   │   │   │   │   │
     │ Save/Load     │ ✓ │ ✓ │ ✓ │ ✓ │ ✓ │ ✓ │ ✓ │ ✓ │ ✓ │ ✓ │ - │   │   │   │   │   │
     │ UI            │   │   │   │   │   │   │   │   │   │   │   │ - │   │   │   │   │
     │ Camera        │   │   │   │   │   │   │   │   │   │   │   │   │ - │   │   │   │
     │ Analytics     │ ✓ │ ✓ │ ✓ │ ✓ │ ✓ │ ✓ │ ✓ │ ✓ │ ✓ │ ✓ │ ✓ │ ✓ │ ✓ │ - │   │   │
     │ Localization  │   │   │   │   │   │   │   │   │   │   │   │ ✓ │   │   │ - │   │
     │ Mod Loader    │ ✓ │   │   │   │   │   │   │   │   │   │   │   │   │   │   │ - │
     └───────────────┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
     
     ✓ = Allowed communication (via Event Bus or Commands)
     Blank = Forbidden (must go through intermediary or redesign)
     - = Self-reference (not applicable)
```

### 4.2 Forbidden Dependencies

1. **No domain system may depend on the UI System.** The domain layer does not know presentation exists. It emits events; UI subscribes.
2. **No domain system may depend on the Camera System.** Camera is presentation concern.
3. **No system may directly modify another system's entities.** Use Commands or Events.
4. **No circular dependencies between systems.** The dependency graph is acyclic. Enforced by build system validation.
5. **The Save/Load System may depend on all domain systems, but no domain system may depend on Save/Load.** Save/Load queries state; it never pushes.
6. **Presentation Layer systems (UI, Camera, Audio, Renderer) may never directly access ECS component data.** They receive data via the Presentation Bus, which provides a read-only mirror of relevant domain state.

### 4.3 Loose Coupling Mechanisms

| Mechanism | Usage |
|-----------|-------|
| **Event Bus** | Primary inter-system communication. Systems publish events; interested systems subscribe. Publishers do not know subscribers exist. |
| **Command Bus** | Player actions and system-to-system requests. Commands are validated objects, not raw function calls. |
| **Service Locator (read-only)** | For cross-cutting services (Logging, Localization, Analytics). Systems request service interfaces, not concrete implementations. Constructor injection preferred; service locator as fallback. |
| **Data Contracts** | When System A needs data from System B, it requests a Data Contract — a structured read-only snapshot. This prevents System A from mutating System B's internal state. |
| **Interface Segregation** | Every module exposes a minimal public interface. Internal implementation is hidden. Tests only exercise the public interface. |

---

## 5. PROJECT ORGANIZATION

### 5.1 Repository Structure

```
InstaBuilt/
├── README.md
├── LICENSE.md
├── .github/
│   └── workflows/                          # CI/CD pipelines
├── Docs/
│   ├── GDD.md                              # Game Design Document
│   ├── GSS.md                              # Gameplay Systems Specification
│   ├── ARCHITECTURE.md                     # This document
│   ├── CODING_STANDARDS.md
│   ├── TOOLING_GUIDE.md
│   └── ADR/                                # Architecture Decision Records
├── Engine/
│   ├── Core/                               # ECS core, event bus, command processor
│   ├── Simulation/                         # All domain systems
│   │   ├── Building/
│   │   ├── Construction/
│   │   ├── Worker/
│   │   ├── Contract/
│   │   ├── Economy/
│   │   ├── Weather/
│   │   ├── Time/
│   │   ├── Reputation/
│   │   ├── Safety/
│   │   ├── Research/
│   │   ├── Region/
│   │   └── Company/
│   ├── Application/                        # Game modes, workflows, state machines
│   ├── Presentation/                       # UI, Camera, rendering bridge
│   ├── Infrastructure/                     # Save/Load, networking, analytics, localization
│   └── Integration/                        # UE5 integration layer
├── Game/
│   ├── Content/                            # All game assets (UE5 Content Browser)
│   │   ├── Buildings/
│   │   ├── Materials/
│   │   ├── Vehicles/
│   │   ├── Characters/
│   │   ├── UI/
│   │   ├── Audio/
│   │   ├── Environments/
│   │   └── Blueprints/                     # Prototype-only; migrated to C++ before ship
│   ├── Config/                             # Data-driven configuration
│   │   ├── Balance/                        # Tuning values
│   │   ├── Buildings/                      # Building definitions
│   │   ├── Contracts/                      # Contract templates
│   │   ├── Regions/                        # Region definitions
│   │   ├── Research/                       # Tech tree definitions
│   │   └── Difficulty/                     # Difficulty presets
│   ├── Localization/                       # String tables per language
│   └── Mods/                               # User mod directory (shipped empty)
├── Tools/
│   ├── BuildingDesigner/                   # Standalone building design tool
│   ├── ContractEditor/                     # Contract template editor
│   ├── EconomyBalancer/                    # Economy simulation & tuning tool
│   ├── ReplayViewer/                       # Time-lapse replay tool
│   ├── PerformanceProfiler/               # Custom simulation profiler
│   ├── AssetValidator/                    # Asset compliance checker
│   └── ModSDK/                            # Mod creation kit
├── Tests/
│   ├── Unit/                               # Per-system unit tests
│   ├── Integration/                        # Multi-system interaction tests
│   ├── Simulation/                         # Headless simulation verification
│   ├── Performance/                        # Benchmark and regression tests
│   └── Acceptance/                         # GSS-compliance verification
├── Plugins/                                # UE5 plugins for third-party integrations
│   ├── Wwise/                              # Audio middleware
│   ├── NoesisGUI/                          # UI middleware
│   ├── Simplygon/                          # LOD generation
│   └── Steamworks/                         # Steam integration
└── ThirdParty/                             # Vendored third-party libraries
    ├── EnTT/                               # ECS library (modified)
    ├── fmt/                                # String formatting
    ├── nlohmann/                           # JSON parsing
    └── catch2/                             # Test framework
```

### 5.2 Module Ownership

| Module Group | Owner Team | Lead |
|-------------|-----------|------|
| Engine/Core | Core Engine | Engine Director |
| Engine/Simulation | Simulation | Simulation Lead |
| Engine/Application | Gameplay | Gameplay Lead |
| Engine/Presentation | UI + Rendering | Presentation Lead |
| Engine/Infrastructure | Platform | Platform Lead |
| Game/Content | Content | Art Director |
| Game/Config | Design + Balance | Design Director |
| Tools/* | Tools | Tools Lead |
| Tests/* | QA + Engineering | QA Lead |
| Plugins/* | Platform | Platform Lead |

---

## 6. LAYERED ARCHITECTURE

### 6.1 Layer Descriptions

#### Presentation Layer

| Aspect | Detail |
|--------|--------|
| **Responsibility** | Everything the player sees, hears, and interacts with. Rendering, audio, UI, camera, input processing. |
| **Technology** | Unreal Engine 5 (rendering, physics, input), Wwise (audio), Noesis GUI (UI), custom camera system. |
| **Key Rule** | NEVER accesses domain state directly. Subscribes to Presentation Bus for data. Issues Commands for actions. |
| **Testing** | Visual regression tests for UI. Automated screenshot comparison. Audio playback tests. |

#### Application Layer

| Aspect | Detail |
|--------|--------|
| **Responsibility** | Orchestrates player workflows. Manages game mode state machines. Validates player commands before passing to domain. |
| **Technology** | Custom C++ state machine framework. Command validation pipeline. |
| **Key Rule** | Translates player intent into domain commands. Does not contain business logic. |
| **Testing** | State machine transition tests. Command validation tests. |

#### Domain Layer (ECS Core + Systems)

| Aspect | Detail |
|--------|--------|
| **Responsibility** | All gameplay simulation. Building design, construction progress, worker behavior, economy, weather, reputation. |
| **Technology** | Custom ECS (EnTT-derived), data-oriented system design. |
| **Key Rule** | Pure simulation logic. No presentation code. No platform-specific code. Deterministic where possible (same inputs → same outputs). |
| **Testing** | Unit tests per system. Integration tests per system interaction. Headless simulation replay tests. |

#### Infrastructure Layer

| Aspect | Detail |
|--------|--------|
| **Responsibility** | Cross-cutting services: serialization, networking stubs, analytics, localization, logging, crash reporting. |
| **Technology** | Custom serialization, platform SDKs. |
| **Key Rule** | Services are consumed via interfaces. Implementations are swappable per platform. |
| **Testing** | Round-trip serialization tests. Mock service verification. |

#### Persistence Layer

| Aspect | Detail |
|--------|--------|
| **Responsibility** | Save/load, configuration loading, asset database, cloud sync, mod data loading. |
| **Technology** | Custom binary serialization, JSON/TOML for configuration, SQLite for asset metadata. |
| **Key Rule** | Save files are versioned with migration paths. Configuration is hot-reloadable in development. |
| **Testing** | Save/load round-trip tests. Backward compatibility tests (old saves load on new builds). |

#### Tools Layer

| Aspect | Detail |
|--------|--------|
| **Responsibility** | Developer and content creator tooling. Editors, balancers, profilers, validators. |
| **Technology** | UE5 Editor extensions, standalone Qt applications, web dashboards. |
| **Key Rule** | Tools operate on the same data formats as the game. Tools that modify data produce validated output. |
| **Testing** | Tool output validation tests. |

### 6.2 Dependency Direction

```
Presentation ──→ Application ──→ Domain ──→ Infrastructure
                      │                         │
                      └──────────→ Persistence ←┘
                                        ↑
                                   Tools (same level, can read Domain + Config)
```

Dependencies flow DOWNWARD only. Upper layers depend on lower layers through interfaces. Lower layers NEVER reference upper layers.

---

## 7. SYSTEM COMMUNICATION

### 7.1 Event Bus Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        EVENT BUS                             │
│                                                              │
│  ┌──────────┐   publish()    ┌──────────────┐               │
│  │ System A │ ──────────────→│              │               │
│  └──────────┘                │  Event Bus   │               │
│                              │              │               │
│  ┌──────────┐   publish()    │ ┌──────────┐ │  subscribe()  │
│  │ System B │ ──────────────→│ │  Event   │ │←──────────────│
│  └──────────┘                │ │  Queue   │ │               │
│                              │ │ (ring    │ │               │
│  ┌──────────┐                │ │  buffer) │ │  ┌──────────┐ │
│  │ System C │ ←── delivery ──│ └──────────┘ │──│ System D │ │
│  └──────────┘                └──────────────┘  └──────────┘ │
│                                                              │
│  Event Types:                                                │
│  ┌─────────────────┬────────────────────────────────────┐   │
│  │ Immediate       │ Delivered this frame, before next    │   │
│  │                 │ system tick. For state changes       │   │
│  │                 │ needed in current frame.             │   │
│  ├─────────────────┼────────────────────────────────────┤   │
│  │ Deferred        │ Delivered next frame. For reactions  │   │
│  │                 │ that don't need to be immediate.     │   │
│  ├─────────────────┼────────────────────────────────────┤   │
│  │ Broadcast        │ Delivered to all subscribers.       │   │
│  ├─────────────────┼────────────────────────────────────┤   │
│  │ Targeted         │ Delivered to specific system(s).    │   │
│  └─────────────────┴────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

### 7.2 Command Bus

Commands represent player intent or system-to-system requests. They are validated objects.

```
PLAYER ACTION → Input System → Command Factory → Command Validator → Command Bus → Domain System
                                               ↓ (if invalid)
                                          Rejection + User Feedback
```

**Command Types:**
- **Immediate Commands:** Execute this frame (pause, camera switch, UI navigation).
- **Queued Commands:** Execute next simulation tick (place wall, assign worker, bid on contract).
- **Composite Commands:** Bundles of related commands that succeed or fail atomically (design a complete room = many wall placements batched).

### 7.3 Presentation Bus

A read-only mirror of domain state optimized for rendering. Updated once per frame after all domain systems have ticked.

```
Domain Systems → Presentation Bus → UI Binding → Screen Update
                                  → Renderer → Frame Render
                                  → Audio → Sound Playback
```

**Key design:** The Presentation Bus is a snapshot, not a live view. Domain systems write to their authoritative state. Once per frame, the Presentation Bus captures a read-only copy. This eliminates race conditions between simulation and rendering.

### 7.4 Observer Pattern Usage

Used for:
- UI components observing domain data changes (data binding)
- Audio system observing game events (play sound on construction milestone)
- Analytics observing all game events (fire-and-forget logging)
- Achievement system observing milestone events

### 7.5 State Synchronization (Future Multiplayer)

The architecture reserves a synchronization boundary:

```
┌──────────────────┐         ┌──────────────────┐
│  AUTHORITATIVE    │  sync   │  CLIENT MIRROR   │
│  SIMULATION       │ ←─────→ │  (Presentation)  │
│  (Server)         │  delta  │                  │
└──────────────────┘         └──────────────────┘
```

The domain layer is designed for deterministic simulation. When multiplayer is added:
- The server runs the authoritative simulation.
- Client inputs are sent as Commands to the server.
- The server broadcasts state deltas to clients.
- Clients run a mirror simulation for prediction, corrected by server deltas.

This architecture is documented now to ensure single-player decisions don't preclude future multiplayer.

---

## 8. DATA OWNERSHIP

### 8.1 Ownership Table

| Data Type | Owner System | Reason |
|-----------|-------------|--------|
| Building design (walls, rooms, MEP) | Building System | Single source of truth for all building structural data |
| Construction state (phase, progress, quality) | Construction System | Owns the construction lifecycle |
| Worker data (skills, morale, assignment) | Worker System | Only system that modifies worker state |
| Contract data (requirements, bids, status) | Contract System | Owns contract lifecycle |
| Economy data (prices, rates, cycles) | Economy System | Single authority on economic state |
| Weather state (current, forecast) | Weather System | Generates and owns weather |
| Time state (current time, calendar) | Time System | Only system that advances time |
| Reputation scores | Reputation System | Calculates and owns reputation |
| Safety state (incidents, risk) | Safety System | Owns safety calculations |
| Equipment state (condition, location) | Equipment System | Owns equipment lifecycle |
| Research progress | Research System | Owns tech tree state |
| Player company (finances, policies) | Company System | Central authority for company state |
| Region data (zoning, demographics) | Region System | Static data, region definitions |
| Save metadata (version, timestamp) | Save/Load System | Owns persistence concerns |

### 8.2 No Duplicate Ownership

Every piece of data has exactly one owner. Other systems access data through:
1. **Queries to the owner system** (synchronous read of current state)
2. **Events published by the owner system** (asynchronous notification of changes)
3. **Data Contracts** (read-only snapshots provided by the owner)

**Example:** The Construction System needs to know worker fatigue. It does NOT store worker fatigue. It queries the Worker System: `WorkerSystem.GetFatigue(workerId)`. If the Construction System needs to react to fatigue changes, it subscribes to `WorkerFatigueChanged` events from the Worker System.

### 8.3 Data Mutation Rules

1. Only the owning system may mutate its data.
2. Other systems may request mutations by issuing Commands to the owning system.
3. The owning system validates and executes or rejects the Command.
4. If executed, the owning system publishes the resulting Event.

---

## 9. UPDATE ORDER

### 9.1 Frame Update Lifecycle

```
FRAME START
│
├── 1. INPUT PROCESSING (async)
│   ├── Poll input devices
│   ├── Dispatch to active UI context
│   └── Generate Commands
│
├── 2. COMMAND PROCESSING
│   ├── Validate all pending Commands
│   ├── Queue valid domain Commands
│   └── Reject invalid Commands (return feedback)
│
├── 3. DOMAIN SIMULATION TICK  ──────────────────────┐
│   │  (All systems execute in dependency order)      │
│   │                                                 │
│   ├── 3.1  Time System → Advance game time          │
│   │         (at current speed multiplier)            │
│   │         Publish: TimeHourTick, TimeDayTick       │
│   │                                                 │
│   ├── 3.2  Weather System → Update weather          │
│   │         (if new in-game hour/day)                │
│   │         Publish: WeatherChanged                  │
│   │                                                 │
│   ├── 3.3  Economy System → Update prices/rates     │
│   │         (if new in-game day)                     │
│   │         Publish: PricesUpdated                   │
│   │                                                 │
│   ├── 3.4  Worker System → Update fatigue/morale    │
│   │         Publish: WorkerStateChanged              │
│   │                                                 │
│   ├── 3.5  Safety System → Evaluate risk/incidents  │
│   │         Publish: SafetyIncident (if triggered)   │
│   │                                                 │
│   ├── 3.6  Construction System → Progress all sites │
│   │         Publish: ConstructionProgress,           │
│   │         PhaseCompleted, ProblemDetected           │
│   │                                                 │
│   ├── 3.7  Contract System → Check deadlines,       │
│   │         process milestones                       │
│   │         Publish: MilestoneReached,               │
│   │         DeadlineWarning                          │
│   │                                                 │
│   ├── 3.8  Reputation System → Recalculate scores   │
│   │         (if triggered by events)                 │
│   │         Publish: ReputationChanged               │
│   │                                                 │
│   └── 3.9  Other systems as needed                  │
│             (Research, Company, Region)              │
│                                                      │
├── 4. PRESENTATION BUS UPDATE  ─────────────────────┘
│   ├── Capture read-only snapshot of domain state
│   ├── Diff against previous snapshot
│   └── Emit change notifications to subscribers
│
├── 5. RENDER UPDATE
│   ├── UI System → Rebind changed data, redraw
│   ├── Camera System → Update view/projection
│   ├── Rendering → Submit draw calls
│   └── Audio → Process queued sound events
│
├── 6. PERSISTENCE CHECK
│   ├── Auto-save timer check
│   └── Queue save if triggered (async on background thread)
│
└── FRAME END
```

### 9.2 Simulation Frequency by System

| System | Tick Frequency | Rationale |
|--------|---------------|-----------|
| Time System | Every frame (at speed multiplier) | Must be responsive to speed changes |
| Weather | Per in-game hour | Weather doesn't change minute-to-minute |
| Economy | Per in-game day | Economic data changes slowly |
| Worker fatigue | Per in-game hour | Fatigue accumulates gradually |
| Safety evaluation | Per in-game hour | Risk evaluation is batched |
| Construction progress | Every 5 simulation ticks (~200ms at 1x) | Smooth progress visualization |
| Reputation | On event trigger | Only recalculates when relevant events occur |
| Contract checks | Per in-game day | Deadlines are daily granularity |

### 9.3 Initialization Lifecycle

```
STARTUP SEQUENCE
│
├── 1. Engine Boot
│   ├── Initialize UE5 core systems
│   ├── Initialize custom ECS core
│   ├── Register all component types
│   └── Register all system types
│
├── 2. Service Initialization
│   ├── Start Logging Service
│   ├── Start Localization Service (load string tables)
│   ├── Start Analytics Service
│   └── Start Save/Load Service
│
├── 3. Configuration Loading
│   ├── Load game configuration
│   ├── Load difficulty settings
│   ├── Load region definitions
│   └── Load building/material/vehicle catalogs
│
├── 4. Mod Loading
│   ├── Discover installed mods
│   ├── Validate compatibility
│   ├── Load mod data tables (merge with base)
│   └── Register mod components/systems
│
├── 5. World Initialization
│   ├── NEW GAME: Create initial world state
│   │   ├── Generate starting region
│   │   ├── Create player company entity
│   │   ├── Create mentor NPC entity
│   │   └── Generate initial contract pool
│   └── LOAD GAME: Restore world from save file
│       ├── Deserialize all entities & components
│       ├── Apply version migrations if needed
│       └── Validate restored state
│
├── 6. Presentation Initialization
│   ├── Build UI context for current game mode
│   ├── Initialize camera (default orbit mode)
│   ├── Start audio (ambient + music)
│   └── Fade from loading screen
│
└── READY → Begin Frame Loop
```

### 9.4 Shutdown Sequence

```
SHUTDOWN (triggered by player exit or crash)
│
├── 1. Auto-Save (if dirty state exists)
├── 2. Flush Analytics (send pending telemetry)
├── 3. Stop all domain systems
├── 4. Tear down ECS (destroy all entities)
├── 5. Shut down services
├── 6. Release UE5 resources
└── EXIT
```

### 9.5 Crash Recovery

- Auto-save runs every 15 minutes + at major milestones.
- On crash, the crash reporter captures: stack trace, last 60 seconds of event bus activity, system states snapshot.
- On next launch, the game detects the crash and offers to load the most recent auto-save.
- Save files are atomic (write to temp, rename on completion) to prevent corruption from crash during save.

---

## 10. CONFIGURATION STRATEGY

### 10.1 Configuration Hierarchy

```
┌─────────────────────────────────────────────────────┐
│  LAYER 1: Engine Defaults (hardcoded fallbacks)     │
│  ↓ overridden by                                     │
│  LAYER 2: Base Game Config (.toml files in Game/Config/) │
│  ↓ overridden by                                     │
│  LAYER 3: Difficulty Preset (Apprentice/Pro/Veteran) │
│  ↓ overridden by                                     │
│  LAYER 4: Player Settings (user preferences)         │
│  ↓ overridden by                                     │
│  LAYER 5: Mod Overrides (loaded mods)                │
│  ↓ overridden by                                     │
│  LAYER 6: Developer Override (dev builds only)      │
└─────────────────────────────────────────────────────┘
```

### 10.2 Data-Driven Configuration Catalog

| Category | Format | Hot-Reloadable | Moddable |
|----------|--------|---------------|----------|
| Building definitions (POP UP 28, 52, 104, etc.) | TOML | Yes | Yes |
| Material catalog (prices, properties, textures) | TOML + Asset refs | Yes | Yes |
| Vehicle definitions (stats, costs, maintenance) | TOML | Yes | Yes |
| Worker roles & skill curves | TOML | Yes | Yes |
| Contract templates | TOML | Yes | Yes |
| Region definitions (climate, zoning, costs) | TOML | No (requires world gen) | Yes |
| Economy balance (base prices, cycles) | TOML | Yes | Yes |
| Research tree definitions | TOML | Yes | Yes |
| Difficulty presets | TOML | Yes | Yes |
| Weather profiles per region | TOML | Yes | Yes |
| UI layout definitions | XML (Noesis) | Yes | Limited |
| Localization strings | PO files | Yes | Yes |
| Achievement definitions | TOML | No | No |
| Tutorial flow definitions | TOML | Yes | No |

### 10.3 Mod Support

All TOML configuration files support mod merging. When a mod is loaded:
1. The Mod Loader reads the mod's config overrides.
2. Overrides are merged on top of the base configuration (Layer 5 over Layer 2).
3. If two mods conflict on the same value, the Mod Loader flags the conflict. The player resolves it via the Mod Manager UI.
4. Mods can add new entries (new building types, new materials) in addition to overriding existing ones.

---

## 11. CROSS-CUTTING CONCERNS

### 11.1 Logging

| Aspect | Implementation |
|--------|---------------|
| **Levels** | TRACE, DEBUG, INFO, WARN, ERROR, FATAL |
| **Categories** | Per system (Building, Construction, Economy, etc.) |
| **Output** | File (rotating), console (dev), in-game console (cheat-enabled) |
| **Shipping** | INFO and above only. Debug stripped. |
| **Privacy** | Player-identifiable data never logged in shipping builds. |
| **Performance** | Log calls use format strings evaluated lazily. Fatal logging synchronous; all else async. |

### 11.2 Diagnostics

- **In-game profiler:** FPS, frame time breakdown by system, entity counts, memory usage. Toggle with Ctrl+Shift+P (dev builds).
- **System health monitor:** Tracks per-system tick time. Alerts if any system exceeds budget (e.g., Construction tick > 5ms triggers warning).
- **Memory tracker:** Tracks allocation per system. Detects leaks in dev builds.
- **Determinism checker:** In test builds, runs simulation twice with same inputs, compares results. Flags non-deterministic systems.

### 11.3 Localization

- All user-facing strings are stored in PO files, keyed by string ID.
- Locale is auto-detected from OS, overridable in settings.
- Supported languages at launch: English, German, French, Spanish, Italian, Japanese, Korean, Simplified Chinese, Brazilian Portuguese, Polish, Russian, Arabic.
- Number formatting, date formatting, and currency formatting are locale-aware.
- UI layouts must accommodate 30% text expansion (German) and right-to-left (Arabic).

### 11.4 Accessibility

Per GSS Section 13, the architecture supports:
- **Colorblind modes:** Post-processing shader that remaps color palette. Colorblind profiles stored as LUTs (look-up tables) in config.
- **Text scaling:** UI system uses relative units (em, rem). Scaling factor applied at root.
- **Screen reader:** UI elements expose accessibility labels and roles. Platform screen reader APIs bridged through abstraction layer.
- **Input remapping:** All actions are named ("CameraOrbit", "PlaceWall"). Key bindings map action names to physical inputs. Remapping changes the mapping table, not the code.
- **Motion reduction:** Camera interpolation, particle systems, and screen transitions check a global accessibility flag.

### 11.5 Telemetry

- **Opt-in:** Players must consent to telemetry (GDPR/CCPA compliance).
- **Anonymous:** No personally identifiable information.
- **Batched:** Events buffered locally, sent in batches every 5 minutes.
- **Categories:** Gameplay actions, performance metrics, error events, progression milestones.
- **Backend:** Events flow to an analytics pipeline (e.g., Snowplow → BigQuery → Looker). Design team queries for balancing data.

### 11.6 Debug Tools

- **Developer Console:** Accessed via tilde key. Supports commands: `spawn building X`, `add money N`, `set time HH:MM`, `complete phase`, `trigger storm`, etc.
- **Entity Inspector:** Click any entity in the world → see all components and their current values.
- **Event Log:** Real-time display of all events flowing through the Event Bus. Filterable by system. Recordable for replay.
- **Simulation Rewind:** In dev builds, the simulation can be rewound to any previous tick. Invaluable for reproducing bugs.

### 11.7 Error Handling

| Error Type | Handling |
|------------|----------|
| **Assertion failure** | In dev: break into debugger. In shipping: log, attempt graceful degradation. |
| **System exception** | Isolate to the failing system. Other systems continue. Log + telemetry. |
| **Save corruption** | Detect via checksum. Offer previous save. Report corruption details. |
| **Out of memory** | Emergency flush (dump caches, unload unused assets). If still OOM, graceful shutdown with save. |
| **GPU hang** | UE5 TDR handling. Attempt recovery. If unrecoverable, save and restart. |
| **Invalid mod** | Sandbox violation caught by Mod Loader. Mod disabled. Player notified. |

### 11.8 Versioning

- **Save file version:** Integer incrementing with each save schema change. Migration functions for each version jump.
- **Asset version:** Semantic versioning for all data tables. Mod compatibility checked against asset versions.
- **Protocol version:** For future networking. Client/server negotiate compatible protocol version.
- **API version:** For future mod API. Deprecated APIs retained for one major version before removal.

---

## 12. EXTENSIBILITY STRATEGY

### 12.1 Adding a New Building Type

**No code changes required.** Steps:

1. Create a new TOML file in `Game/Config/Buildings/MyNewBuilding.toml`.
2. Define: dimensions, room requirements, material constraints, structural system, cost factors, tier unlock, construction phases.
3. Reference existing or new 3D assets and material definitions.
4. The Mod Loader (or base config loader) picks it up automatically.
5. The Contract System can now generate contracts for this building type.
6. The Building System can now validate designs for this building type.
7. The Construction System uses the phase definitions to build it.

**Why this works:** Building type is data, not code. The systems consume building definitions, not hardcoded building types.

### 12.2 Adding a New Region (DLC)

**Minimal code changes.** Steps:

1. Create region definition TOML: climate profile, zoning rules, labor market, architectural styles, material costs, contract pool.
2. Create terrain data, city layout, road network (procedural from parameters + artist touch-up).
3. Create region-specific 3D assets (architectural details, vegetation).
4. Add localization strings for region name, districts, landmarks.
5. The Region System loads it. All dependent systems (Contract, Economy, Weather) consume it automatically.

**Why this works:** Region is data. All systems reference region by ID, not by hardcoded enum.

### 12.3 Adding a New Construction Phase

**Configuration change, may require system extension if new mechanics involved.**

1. If the new phase is a variant of existing phases (e.g., "Prefab Assembly" as a new framing method): add to phase configuration. System uses existing phase machinery.
2. If the new phase introduces new mechanics (e.g., "3D Printing" phase with fundamentally different behavior): create a new PhaseProcessor class registered with the Construction System. The Construction System dispatches to the appropriate processor per phase type.

**Why this works:** Phase processing is polymorphic. Adding a new phase type means adding a new processor, not modifying the Construction System.

### 12.4 Adding a New Vehicle or Equipment

**No code changes.**

1. Add vehicle definition TOML: stats, cost, maintenance schedule, 3D asset reference, operator requirements.
2. The Equipment System loads it. Purchasing, assignment, maintenance all work automatically.

### 12.5 Adding New AI Competitor Behaviors

**Code change in the AI system only.**

1. Define behavior profile in TOML (bidding aggressiveness, specialization, growth rate).
2. Implement behavior strategy class in the AI system.
3. Competitor entities use the configured strategy.

**Why this works:** AI behavior is strategy pattern. Competitors are configured with behavior profiles, not hardcoded logic.

### 12.6 Adding a New Game Mode (DLC or Major Update)

**Application Layer change.**

1. Create a new GameModeController class that orchestrates the mode's workflow.
2. Register with the Application Layer.
3. The mode is selectable from the main menu.

All domain systems remain unchanged — the game mode is an orchestrator, not a system modifier.

### 12.7 Adding Multiplayer

**Infrastructure Layer change, no domain system changes.**

1. Enable the synchronization boundary already reserved in the architecture (Section 7.5).
2. The server runs the authoritative simulation.
3. Client inputs are transmitted as Commands.
4. Server broadcasts state deltas.

Domain systems were designed for determinism from the start. The only new code is the networking layer, which sits in Infrastructure, not Domain.

---

## 13. ARCHITECTURAL RISKS

### 13.1 Risk Register

| # | Risk | Severity | Likelihood | Mitigation |
|---|------|----------|------------|------------|
| R1 | **ECS integration with UE5 is friction-heavy.** UE5's Actor model and custom ECS don't naturally interoperate. | High | Medium | Dedicated Integration Layer team. Clear ownership boundary. Automated tests for the bridge. Prototype this in pre-production month 1. |
| R2 | **Simulation performance at city scale.** 1000+ buildings, 500+ workers, 10+ active sites may exceed frame budget. | High | Medium | Data-oriented design (SoA layout). Parallel system execution. LOD for simulation (distant sites updated less frequently). Performance budgets per system enforced by CI. |
| R3 | **Save file size grows unboundedly.** Every building ever constructed persists. 100-hour saves could hit gigabytes. | Medium | High | Completed buildings stored as compressed static data (geometry only, no simulation state). Only active sites store full simulation state. Save file size targeting <100MB for 100-hour saves. |
| R4 | **Mod API abuse causes instability.** Modders find ways to crash or corrupt the game. | Medium | High | Sandboxed mod loading. Mod crash = mod disabled, not game crash. Mod data corruption = mod save isolated from main saves. |
| R5 | **Determinism is hard.** Achieving deterministic simulation for future multiplayer is technically challenging. | Medium | Low | Determinism is a goal, not a hard requirement. If full determinism proves impossible, multiplayer will use state synchronization (server authority) instead of lockstep, which tolerates minor non-determinism. |
| R6 | **Team scaling with ECS.** New hires unfamiliar with ECS may be less productive initially. | Medium | Medium | Comprehensive onboarding docs. ECS training module. Pair programming for first 2 weeks. Internal ECS utility library to reduce boilerplate. |
| R7 | **Configuration explosion.** Thousands of TOML values become unmanageable. | Low | High | Validation tools that check config consistency. "Balancer" tool that visualizes config dependencies. CI validates config on every commit. |
| R8 | **Dependency creep.** Over 10 years, forbidden dependencies emerge through "temporary" workarounds. | Medium | High | Architecture tests in CI that enforce the dependency matrix (Section 4.1). Build fails on violation. Quarterly architecture review. |

### 13.2 Technical Debt Management

- **Architecture Review:** Quarterly. Entire engineering leadership reviews one system per quarter against its original design.
- **Deprecation Policy:** Public APIs deprecated with one major version warning before removal.
- **Refactor Budget:** 20% of each sprint allocated to technical debt reduction.
- **Complexity Budget:** Any new system must justify its existence in an ADR. Systems that exceed a complexity threshold (measured by cyclomatic complexity of public interface) trigger automatic review.

---

## 14. ARCHITECTURE DECISION RECORDS (ADR)

### ADR-001: Engine Foundation

**Decision:** Use Unreal Engine 5 for rendering, physics, audio, and input; custom C++ ECS for simulation.

**Alternatives Considered:**
- Pure Unity with DOTS — rejected for insufficient architectural visualization quality.
- Fully custom engine — rejected for 3-4 year rendering engine development cost.
- UE5 with Mass Entity — rejected due to Mass Entity's immaturity for production simulation at this scale.

**Why this wins:** Best rendering quality for architectural visualization (core to player satisfaction) + full control over simulation architecture (core to scalability and mod support). The integration cost is front-loaded and acceptable given 10-year lifespan.

### ADR-002: Data-Oriented ECS vs Object-Oriented

**Decision:** Data-oriented ECS with Struct-of-Arrays component storage.

**Alternatives Considered:**
- Object-oriented with deep inheritance — rejected for poor cache performance and fragile base class problem.
- Pure functional with immutable state — rejected for memory overhead of copying large simulation state each frame.

**Why this wins:** Construction simulation involves thousands of homogeneous entities. SoA provides optimal cache utilization, trivial parallelization, and clean separation of data from behavior. Mod support is natural — adding components never requires modifying existing systems.

### ADR-003: Event Bus vs Direct Function Calls

**Decision:** Event Bus for inter-system communication; direct function calls only within a system.

**Alternatives Considered:**
- All communication via direct function calls — rejected because it creates tight coupling and circular dependency risk.
- All communication via events — rejected because some queries are naturally synchronous (e.g., "what is the current weather?").

**Why this wins:** Events decouple systems (Weather doesn't know Construction exists). Direct calls are efficient for simple queries. The hybrid approach provides both decoupling and performance.

### ADR-004: Command Pattern for Player Input

**Decision:** All player actions pass through the Command Bus as validated Command objects.

**Alternatives Considered:**
- Direct function calls from input handlers to domain systems — rejected for lack of validation, undo support, and multiplayer extensibility.
- MVC-style controller methods — rejected as less structured than explicit Command objects.

**Why this wins:** Commands are explicit, validatable, loggable, and replayable. Undo is natural (reverse the command). Multiplayer is natural (serialize the command, transmit to server). Analytics is natural (log the command type).

### ADR-005: Save File Format

**Decision:** Custom binary format with version header and migration layer. Components serialize themselves.

**Alternatives Considered:**
- JSON serialization — rejected for file size (textual representation of thousands of entities would be megabytes).
- UE5 built-in serialization — rejected because it couples save format to UE version and makes headless simulation saves difficult.
- SQLite database — rejected for load time (thousands of queries to reconstruct entity graph).

**Why this wins:** Binary format is compact and fast. Custom format decouples save from engine version. Component self-serialization means adding a component type never requires modifying save code.

### ADR-006: UI Technology

**Decision:** Noesis GUI for data-bound vector UI.

**Alternatives Considered:**
- UMG (UE5's built-in UI) — rejected for limited data binding and poor scaling/text support.
- HTML/CSS via embedded browser — rejected for performance overhead and integration complexity.
- Custom immediate-mode UI — rejected for development time.

**Why this wins:** Noesis provides production-quality data binding (XAML-style), vector rendering (scales perfectly for accessibility), and excellent localization support. It's used in AAA games (The Coalition, Arkane) with UE5 integration.

### ADR-007: Audio Middleware

**Decision:** Wwise for interactive audio.

**Alternatives Considered:**
- UE5 MetaSounds — rejected for limited dynamic mixing capabilities needed for adaptive construction site audio.
- FMOD — viable alternative; Wwise selected for better spatial audio and existing team expertise.

**Why this wins:** Construction sites require highly dynamic audio (number of active tools, proximity to work, weather, time of day). Wwise's RTPC system is purpose-built for this.

### ADR-008: Deterministic Simulation

**Decision:** Aim for deterministic simulation but do not hard-require it. Multiplayer will use server-authoritative state sync, not lockstep.

**Alternatives Considered:**
- Hard lockstep determinism requirement — rejected as it constrains too many design decisions (no floating-point, no unordered containers, etc.).
- No determinism at all — rejected because determinism enables replay, time-lapse, and simpler testing.

**Why this wins:** "Deterministic where practical" gives us replay and testing benefits without constraining every design choice. If future multiplayer can't achieve lockstep, state sync works fine.

### ADR-009: Modding Architecture

**Decision:** Data-driven modding via config merging + optional script extensions. No binary patching.

**Alternatives Considered:**
- Full scripting API (Lua, C#) — considered for post-launch; too complex for launch scope.
- Workshop-only asset mods — rejected as insufficient for simulation game modding depth.

**Why this wins:** Config merging gives modders enormous power (new buildings, regions, contracts, materials) with zero code. Script extensions (post-launch) add behavior modding. The architecture supports gradual deepening of mod capabilities.

### ADR-010: Build System

**Decision:** Unreal Build Tool (UBT) for UE5 components + CMake for custom simulation core.

**Alternatives Considered:**
- Pure UBT — rejected because custom ECS core should build independently of UE5 for headless simulation and testing.
- Pure CMake — rejected because UE5 integration requires UBT.

**Why this wins:** UBT handles UE5 complexity. CMake handles the custom core. The core can be built and tested without UE5 (faster iteration, CI-friendly). Integration layer links them.

---

## ARCHITECTURE APPROVED FOR DETAILED SYSTEM DESIGN

### Locked Assumptions

The following assumptions are now locked. Future phases must not violate them:

```
[x] UE5 for rendering/physics/audio/input; custom C++ ECS for simulation
[x] Data-oriented ECS with Struct-of-Arrays component storage
[x] Event Bus for inter-system communication; Commands for player input
[x] Presentation Layer reads domain state via read-only Presentation Bus
[x] No domain system may depend on any presentation system
[x] Dependency graph is acyclic; enforced by CI
[x] Every data type has exactly one owning system
[x] Frame update follows fixed order: Input → Commands → Simulation → Presentation → Render
[x] Save format is custom binary with versioned migration
[x] Configuration uses TOML with 6-layer override hierarchy
[x] Mod support via config merging + component/system registration
[x] Noesis GUI for UI; Wwise for audio
[x] Determinism is a goal, not a hard requirement
[x] 20% sprint budget for technical debt reduction
[x] Quarterly architecture review mandatory
[x] System tick frequency calibrated per system (not all systems tick every frame)
[x] Multiplayer architecture reserved via sync boundary; domain systems designed for eventual networking
[x] All user-facing strings in PO files; 12 launch languages
[x] Telemetry opt-in; anonymous; batched
[x] Crash recovery via atomic auto-saves on 15-minute timer + milestone triggers
```

---

**End of Software Architecture Blueprint — InstaBuilt: Blueprint Empire v1.0**

*Architecture is the art of deciding what to build once and never rebuild. This document is that decision.*
