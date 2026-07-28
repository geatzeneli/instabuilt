# INSTABUILT: BLUEPRINT EMPIRE
## Production Roadmap v1.0

**Document Type:** Production Roadmap & Development Plan
**Audience:** Studio Leadership, Engineering, Design, Art, QA, Publishing
**Prerequisites:** All 7 design & architecture documents LOCKED
**Status:** Roadmap Approved — Execution Begins

---

## PART 1 — DEVELOPMENT STRATEGY

### 1.1 Strategy Overview

```
PHASE 0: PROTOTYPE      (Months 1-6)     Prove the core loop is fun
PHASE 1: FIRST PLAYABLE (Months 7-12)    One complete player journey
PHASE 2: VERTICAL SLICE (Months 13-18)   Polished demo-quality build
PHASE 3: EARLY ACCESS   (Months 19-24)   Public release, core systems
PHASE 4: FULL RELEASE   (Months 25-30)   Complete game, all features
PHASE 5: LIVE SERVICE   (Months 31+)     DLC, events, community
```

### 1.2 Prototype Goals (Month 6)

**What must be proven:**
1. The construction loop (design → build → deliver) is satisfying in under 15 minutes.
2. The building design tool is intuitive enough that a non-architect can create a plausible house in under 5 minutes.
3. The ECS simulation core can run 50 buildings at 60 FPS.
4. The UE5 + custom ECS integration works without fatal friction.
5. The emotional loop (pride in completed building) is real.

**Deliverable:** Internal prototype. Single building type. One region. No art polish. Playable by studio only.

**Exit criteria:** 10/10 internal playtesters say "I want to play more."

### 1.3 First Playable Goals (Month 12)

**What must exist:**
- Complete single-player journey from Tier 1 start → first contract → design → build → deliver → payment → repeat.
- One building type (POP UP 28 m²), 3 contract variations.
- Save/load works. Game can be closed and resumed.
- UI is functional (not pretty).
- All 5 core systems operational (Time, Building, Construction, Contract, Save).

**Deliverable:** Internal playable build. Studio-wide playtest. First external usability test (10 players).

**Exit criteria:** External playtesters complete the tutorial and first contract unassisted. Retention >80% through first session.

### 1.4 Vertical Slice Goals (Month 18)

**What must exist:**
- Polished, demo-quality build representing final game quality.
- One complete region (starting city, multiple districts).
- 3 building types, full design tool, complete construction workflow.
- First-person walkthrough, drone camera, photo mode.
- Tier 1-2 progression, 10 contract variations.
- Professional UI, placeholder audio replaced with production audio.
- Mentor Pllana voice acting (temporary or final).

**Deliverable:** Press demo build. Trade show playable. Publisher milestone.

**Exit criteria:** Press previews generate positive coverage. Publisher greenlights full production. Playtesters play for 2+ hours voluntarily.

### 1.5 Early Access Goals (Month 24)

**What must exist:**
- All Tier 1-3 content (3 InstaBuilt product lines, 50+ contract variations).
- Multi-region (2 regions).
- Skill trees, crew system, basic economy, reputation.
- Scenario Mode (5 scenarios).
- Community blueprint sharing.
- Steam Workshop integration.

**Deliverable:** Steam Early Access launch.

**Exit criteria:** 80%+ positive Steam reviews. Daily active players >5,000. Crash rate <0.1%. Player-reported bugs addressed within 1 week.

### 1.6 Full Release Goals (Month 30)

**What must exist:**
- All 7 InstaBuilt product lines.
- All 6 company tiers.
- Complete 5-axis reputation.
- Full construction simulation (all 8 phases, safety, weather, quality).
- 4 regions at launch (base game).
- 20+ scenarios.
- Creative Architect Mode, Sandbox Mode.
- 12-language localization.
- All accessibility features.
- Polished to AAA quality.

**Deliverable:** Steam 1.0 launch. Console cert submission.

**Exit criteria:** Metacritic >80. Steam reviews >90% positive. First-week sales meet publisher targets.

---

## PART 2 — DEPENDENCY ANALYSIS

### 2.1 System Dependency Graph

```
                        ┌─────────────┐
                        │  ECS CORE   │ ← FOUNDATION (Week 1-4)
                        └──────┬──────┘
                               │
            ┌──────────────────┼──────────────────┐
            │                  │                  │
     ┌──────▼──────┐   ┌──────▼──────┐   ┌──────▼──────┐
     │  TIME       │   │  EVENT BUS  │   │  COMMAND    │
     │  SYSTEM     │   │  SYSTEM     │   │  PROCESSOR  │
     └──────┬──────┘   └──────┬──────┘   └──────┬──────┘
            │                  │                  │
     ┌──────▼──────┐          │                  │
     │  SAVE/LOAD  │←─────────┘                  │
     └──────┬──────┘                             │
            │                                    │
     ┌──────▼────────────────────────────────────┼──────┐
     │              BUILDING SYSTEM               │      │
     │  (walls, rooms, doors, windows, design)    │      │
     └──────┬─────────────────────────────────────┘      │
            │                                            │
     ┌──────▼──────┐   ┌────────────┐   ┌──────────┐    │
     │ CONSTRUCTION│   │  CONTRACT  │   │  WORKER  │    │
     │   SYSTEM    │───│  SYSTEM    │───│  SYSTEM  │    │
     └──────┬──────┘   └──────┬─────┘   └────┬─────┘    │
            │                  │              │           │
     ┌──────▼──────┐   ┌──────▼─────┐  ┌─────▼──────┐   │
     │ INVENTORY   │   │  ECONOMY   │  │ REPUTATION │   │
     │ MATERIAL    │   │  SYSTEM    │  │  SYSTEM    │   │
     └──────┬──────┘   └──────┬─────┘  └─────┬──────┘   │
            │                  │              │           │
     ┌──────▼──────┐   ┌──────▼─────┐  ┌─────▼──────┐   │
     │  WEATHER    │   │  FINANCE   │  │ RESEARCH   │   │
     │  SYSTEM     │   │  SYSTEM    │  │  SYSTEM    │   │
     └──────┬──────┘   └──────┬─────┘  └────────────┘   │
            │                  │                          │
     ┌──────▼──────────────────▼──────────────────────────▼──┐
     │                  PRESENTATION LAYER                     │
     │  (UI Bridge, Camera Bridge, Audio Bridge, Rendering)   │
     └────────────────────────────────────────────────────────┘
```

### 2.2 Critical Path

```
ECS Core → Time System → Building System → Construction System → Contract System → Full Loop

                                                    ↓
                                              Save/Load (parallel after Time)
                                                    ↓
                                              UI Bridge (parallel after Contract)
```

**Critical path length:** 5 sequential dependencies from project start to playable core loop.

### 2.3 Blocking Systems (must exist before dependent systems)

| Blocker | Blocks | Reason |
|---------|--------|--------|
| ECS Core | Everything | All data lives in ECS |
| Time System | Construction, Economy, Weather, Worker | All time-dependent systems need a clock |
| Building System | Construction System, Contract System | Construction builds buildings; contracts are for buildings |
| Construction System | Contract completion | Contracts complete when construction completes |

### 2.4 Independent Systems (can be built in parallel)

| System | Can start when | Independent of |
|--------|---------------|----------------|
| Weather System | Time System exists | Construction, Contract, Economy |
| Economy System | Time System exists | Construction, Weather |
| Worker System | ECS Core exists | Building (initially) |
| Reputation System | ECS Core exists | Everything (event-driven) |
| Research System | ECS Core exists | Everything |
| Save/Load System | ECS Core + Time System | Presentation |
| UI Bridge | ECS Core + Event Bus | Domain systems (consumes events) |
| Audio Bridge | Event Bus | Domain systems |
| Camera System | UE5 integration | Domain systems |

---

## PART 3 — DEVELOPMENT PHASES

### PHASE 0: PROTOTYPE (Months 1-6)

**Goal:** Prove the core loop is fun and technically feasible.

| Category | Tasks |
|----------|-------|
| **Engineering** | Build ECS Core. Build Time System. Build Building System (walls, rooms, basic design tool). Build Construction System (single phase, hardcoded progress). Build Save/Load (manual save only). Integrate UE5 rendering bridge. Build placeholder UI. |
| **Design** | Define 1 building type (POP UP 28). Define 1 contract template. Define basic material catalog (5 materials). Balance construction speed for prototype pacing. |
| **Art** | Placeholder gray-box building assets (5 meshes). Placeholder UI. Grid texture for design mode. |
| **QA** | Daily playtest of construction loop. Bug reports. Performance monitoring. |
| **Success Criteria** | Designer can place walls to create a house. Construction progresses visibly. Building completes. Save/load preserves state. 60 FPS with 50 buildings. |

### PHASE 1: FIRST PLAYABLE (Months 7-12)

**Goal:** Complete player journey from start to first contract delivery.

| Category | Tasks |
|----------|-------|
| **Engineering** | Full Construction System (8 phases). Contract System (generation, bidding, tracking). Company System (basic financials). Worker System (hire, assign, fatigue). Event Bus completion. Command Processor. UI Bridge (dashboard, HUD). Auto-save. |
| **Design** | 3 contract templates. Tutorial design (Mentor Pllana flow). Balance pass for Tier 1. First-play experience scripting. |
| **Art** | Production building assets for POP UP 28. Worker character models (2 variants). Basic construction site props. Dashboard UI art. |
| **Audio** | Placeholder audio (temp library). Construction sounds. UI sounds. |
| **QA** | External usability test (10 players). Save/load stress test. Performance validation. |
| **Success Criteria** | External testers complete tutorial + first contract unassisted. Zero crashes in 2-hour play session. Save/load round-trip 100% reliable. |

### PHASE 2: VERTICAL SLICE (Months 13-18)

**Goal:** Polished demo representing final quality.

| Category | Tasks |
|----------|-------|
| **Engineering** | Full Building System (all design tools, validation). 3 building types. Region System (1 city, multi-district). Economy System. Weather System. Camera System (all 7 modes). First-person mode. Photo mode. Drone mode. Cinematic camera. UI polish (animations, transitions). |
| **Design** | 10 contract templates. Region definition. Mentor Pllana voice script. Scenario design (2 scenarios). |
| **Art** | Full building assets for 3 types. Region environment art. Worker character polish. Construction site polish. UI polish. Cinematic camera presets. Marketing screenshots. |
| **Audio** | Production audio pass. Dynamic music system. Construction site ambience. Weather audio. UI audio polish. Mentor Pllana voice (temp or final). |
| **QA** | Full test pass. Performance optimization. Trade show readiness. |
| **Success Criteria** | Demo playable at trade show without crashes. Playtesters play 2+ hours voluntarily. Press coverage positive. Publisher milestone approved. |

### PHASE 3: EARLY ACCESS (Months 19-24)

**Goal:** Public release with core content.

| Category | Tasks |
|----------|-------|
| **Engineering** | Remaining Tier 1-3 systems. Safety System. Skill trees. Multi-region (2 regions). Steam Workshop integration. Mod Loader. Analytics integration. Crash reporting. |
| **Design** | 50+ contract templates. Region 2 definition. 5 scenarios. Full tutorial. Skill tree design. |
| **Art** | Region 2 assets. All Tier 1-3 building assets. Additional worker variants. Vehicle models. |
| **Audio** | Full production audio. All voice acting. |
| **QA** | Beta testing (500+ players). Load testing. Save migration testing. |
| **Success Criteria** | 80%+ positive Steam reviews. DAU >5,000. Crash rate <0.1%. |

### PHASE 4: FULL RELEASE (Months 25-30)

**Goal:** Complete AAA game.

| Category | Tasks |
|----------|-------|
| **Engineering** | Tier 4-6 systems. Full reputation system. Research system. Creative Architect Mode. Sandbox Mode. All 20+ scenarios. Console ports. All accessibility features. Optimization pass. |
| **Design** | Remaining contract templates. Region 3 & 4 definitions. Remaining scenarios. Balance pass all tiers. |
| **Art** | Remaining building assets. Region 3 & 4 assets. Final polish pass. Marketing assets. |
| **Audio** | Final mix. Dynamic music for all regions. |
| **QA** | Full regression. Console cert. Performance cert. Localization QA. |
| **Success Criteria** | Metacritic >80. 90%+ positive Steam reviews. Console cert passed. |

### PHASE 5: LIVE SERVICE (Months 31+)

**Goal:** Sustained player engagement.

| Category | Tasks |
|----------|-------|
| **Engineering** | Seasonal events framework. Community challenges. DLC architecture. Multiplayer (research/development). VR mode (research). |
| **Design** | Monthly event designs. DLC designs. Community management. |
| **Art** | DLC assets. Seasonal event assets. |
| **Audio** | Event-specific audio. |
| **QA** | Live monitoring. Hotfix readiness. |

---

## PART 4 — MILESTONE BREAKDOWN

### Milestone Map

```
M0  ──── Project Start (Month 0)
M1  ──── ECS Core Operational (Month 1)
M2  ──── First Building Rendered (Month 2)
M3  ──── Construction Loop Working (Month 3)
M4  ──── Design Tool Functional (Month 4)
M5  ──── Save/Load Working (Month 5)
M6  ──── PROTOTYPE COMPLETE (Month 6) ← GATE
M7  ──── Contract System Working (Month 8)
M8  ──── Worker System Working (Month 9)
M9  ──── Complete Core Loop (Month 10)
M10 ──── UI Dashboard Functional (Month 11)
M11 ──── FIRST PLAYABLE (Month 12) ← GATE
M12 ──── Design Tool Complete (Month 14)
M13 ──── 3 Building Types (Month 15)
M14 ──── Camera & First-Person (Month 16)
M15 ──── Region & Weather (Month 17)
M16 ──── VERTICAL SLICE (Month 18) ← GATE (Publisher)
M17 ──── Tier 2-3 Systems (Month 20)
M18 ──── Multi-Region (Month 21)
M19 ──── Mod Support (Month 22)
M20 ──── Steam Integration (Month 23)
M21 ──── EARLY ACCESS LAUNCH (Month 24) ← GATE (Public)
M22 ──── Tier 4-5 Systems (Month 26)
M23 ──── Creative & Sandbox Modes (Month 27)
M24 ──── Console Ports Start (Month 27)
M25 ──── Full Content Complete (Month 28)
M26 ──── Polish & Optimization (Month 29)
M27 ──── FULL RELEASE v1.0 (Month 30) ← GATE (Launch)
```

### Detailed Milestones

#### M6: PROTOTYPE COMPLETE

| Attribute | Detail |
|-----------|--------|
| **Objective** | Prove the core gameplay loop is fun and technically viable |
| **Deliverables** | Playable prototype with: one building type, design tool, construction loop, save/load |
| **Required Systems** | ECS Core, Time, Building (basic), Construction (basic), Save/Load |
| **Expected Result** | Internal playtesters complete the loop and say "more" |
| **Testing** | Internal playtest (20 people). Performance: 60 FPS with 50 buildings. |
| **Definition of Done** | All prototype features functional. No blocking bugs. Exit survey >80% positive. Tech Director signs off on ECS/UE5 integration. |

#### M11: FIRST PLAYABLE

| Attribute | Detail |
|-----------|--------|
| **Objective** | Complete single-player journey from start to contract delivery |
| **Deliverables** | First playable with: tutorial, 3 contracts, workers, basic economy, dashboard |
| **Required Systems** | All core + Construction (full), Contract, Worker, Company, UI Bridge |
| **Expected Result** | External testers play independently for 1+ hour |
| **Testing** | External usability test (10 players). 100 save/load cycles. |
| **Definition of Done** | All M11 features functional. Testers complete tutorial unassisted. Zero crashes in 2-hour sessions. |

#### M16: VERTICAL SLICE

| Attribute | Detail |
|-----------|--------|
| **Objective** | Polished demo representing final quality |
| **Deliverables** | Demo: 1 region, 3 buildings, all cameras, first-person, photo mode, 10 contracts |
| **Required Systems** | All M11 + Region, Economy, Weather, Camera (7 modes), most UI |
| **Expected Result** | Press coverage. Publisher greenlight. Playtesters play 2+ hours. |
| **Testing** | Full QA pass. Trade show dry run. Performance: 60 FPS on target hardware. |
| **Definition of Done** | Demo-ready. No visible placeholder art. All audio professional. Zero crashes in 10-hour test. Publisher milestone signed. |

#### M21: EARLY ACCESS LAUNCH

| Attribute | Detail |
|-----------|--------|
| **Objective** | Public release on Steam Early Access |
| **Deliverables** | EA build: Tier 1-3, 2 regions, 50+ contracts, 5 scenarios, mod support |
| **Required Systems** | All Tier 1-3 systems. Mod Loader. Analytics. Crash reporting. |
| **Expected Result** | 80%+ positive reviews. Growing player base. |
| **Testing** | Closed beta (500 players). Load test (5,000 concurrent). Save migration test. |
| **Definition of Done** | Steam build live. Store page complete. Community hub active. Crash rate <0.1%. |

#### M27: FULL RELEASE v1.0

| Attribute | Detail |
|-----------|--------|
| **Objective** | Complete AAA game shipped on all target platforms |
| **Deliverables** | v1.0: all 7 product lines, 6 tiers, 4 regions, 20+ scenarios, all modes, 12 languages |
| **Required Systems** | All 28 systems. Console ports. Accessibility. Full localization. |
| **Expected Result** | Metacritic >80. Strong sales. |
| **Testing** | Full regression. Console certification. 12-language QA. Performance cert. |
| **Definition of Done** | All platforms live. Cert passed. 90%+ Steam positive. Metacritic >80. Launch retrospective complete. |

---

## PART 5 — FIRST PLAYABLE VERSION

### 5.1 Design Philosophy

The First Playable is the smallest complete experience that proves the game works. It must contain exactly one of everything needed for the full loop, at minimal quality, with zero extraneous features.

### 5.2 Feature List (exhaustive)

| # | Feature | Why Included |
|---|---------|-------------|
| 1 | ECS Core + Time System | Foundation. Nothing works without entities and time. |
| 2 | Single building type: POP UP 28 m² | Simplest building. One floor. Few rooms. Fast to build. |
| 3 | Design tool: walls, rooms, 2 doors, 4 windows, 1 roof type | Minimum for a plausible house design. |
| 4 | Design validation: basic (walls must enclose, rooms must have doors) | Prevents impossible designs without full structural simulation. |
| 5 | Construction: 3 phases (foundation → frame → finish) | Simplified from 8 phases. Proves the construction loop works. |
| 6 | One contract template: "First Home" | Single contract. Fixed requirements. Teaches the loop. |
| 7 | One client (the "Johnson Family") | Emotional anchor. Named client with reactions. |
| 8 | Worker system: 2 workers (Marku and Arta) | Named characters. Hire, assign, see them build. |
| 9 | Basic economy: cash, material cost, labor cost, payment | Money in, money out. Prove the economic loop. |
| 10 | Company dashboard: cash, active contract, reputation (single number) | Minimum management interface. |
| 11 | Save/Load: manual save, auto-save every 10 minutes | Game must be resumable. |
| 12 | One tutorial contract (Mentor Pllana walks through) | First-time experience. Teaches without manual. |
| 13 | Orbit camera + first-person walkthrough | See your building from above and inside. |
| 14 | Client delivery: walkthrough, reaction, payment | Emotional payoff. Core reward loop. |
| 15 | Start screen → New Game → Tutorial → Dashboard → Contract → Design → Build → Deliver → Repeat | Complete player journey. |

### 5.3 Explicitly NOT Included

- ❌ Multiple building types
- ❌ Multiple floors
- ❌ MEP systems
- ❌ Weather
- ❌ Economy simulation
- ❌ Research
- ❌ Vehicles
- ❌ Regional expansion
- ❌ Scenarios
- ❌ Photo mode
- ❌ Drone camera
- ❌ Multiplayer
- ❌ Mod support
- ❌ Localization (English only)
- ❌ Achievements
- ❌ Polish art (gray-box is fine)

### 5.4 Why This Is Enough

**It validates the core emotional loop:** Can a player feel pride from designing and building a house, however simple? The First Playable answers this question with minimal investment.

**It validates the technical foundation:** The ECS core, UE5 integration, save/load, and construction simulation are the riskiest technical elements. Proving them early derisks the entire project.

**It generates the first real playtest data:** 10 external testers playing through the complete loop provide more valuable feedback than 6 months of internal speculation.

**It's shippable as an internal milestone:** The team can point at something running and say "this is the game, now we make it bigger."

---

## PART 6 — VERTICAL SLICE

### 6.1 Vertical Slice Design

The Vertical Slice is a polished 30-45 minute demo that feels like a complete, shippable game for its duration.

### 6.2 Demo Scenario: "The Riverside Project"

```
OPENING (2 min):
  - Cinematic drone shot of Riverside District
  - Mentor Pllana voiceover: "Riverside Development needs 24 apartments.
    You've got the contract. Let's show them what InstaBuilt can do."
  - Player starts at Company Dashboard

DESIGN (10 min):
  - Player designs a 3-story apartment building
  - Uses full design tool (walls, rooms, floors, doors, windows, roof, materials)
  - Real-time validation shows issues; player fixes them
  - Client requirements tracked in sidebar
  - Player submits design; client approves with minor revision request
  - Player revises and gets final approval

PLANNING (3 min):
  - Permits auto-processed (fast-forwarded for demo pacing)
  - Materials ordered (player sees cost breakdown)
  - Crew assigned (3 crews, 15 workers)
  - Equipment scheduled (crane arrives on Day 5)

CONSTRUCTION (8 min):
  - Phases progress at accelerated speed (5x default)
  - Player manages: responds to rain warning (deploy tarps),
    reallocates crew when framing falls behind,
    approves material substitution when supplier is late
  - Milestone celebrations at foundation, structure, and roof completion
  - Time-lapse shows building rising

INSPECTION & DELIVERY (4 min):
  - First-person walkthrough with city inspector
  - Inspector checks structure, electrical, fire safety
  - One issue found (stair railing height) — player dispatches fix
  - Re-inspection passes
  - Client walkthrough: family explores model unit
  - Client smiles: "It's exactly what we envisioned."
  - Payment received. Building added to portfolio.

CLOSING (1 min):
  - Drone pulls back to show completed building in neighborhood context
  - Other player buildings visible nearby (from previous playthroughs — persistence demo)
  - Mentor Pllana: "Riverside is just the beginning. The city needs a hospital,
    a school, a new downtown tower. Are you ready?"
  - Fade to title: "INSTABUILT: BLUEPRINT EMPIRE — Coming 20XX"
  - Pre-order / Wishlist call to action
```

### 6.3 What Makes It Representative

| Quality | How Demonstrated |
|---------|-----------------|
| **Design depth** | Multi-floor, multi-unit design with real validation |
| **Construction authenticity** | 8 phases, weather response, crew management, supplier issues |
| **Emotional payoff** | Client reaction, portfolio addition, world persistence |
| **Visual quality** | Polished art, dynamic lighting (Lumen), cinematic cameras |
| **Management depth** | Crew allocation, material decisions, budget awareness |
| **Scalability hint** | Closing shows city full of player buildings — what the full game offers |
| **Character** | Mentor Pllana's voice, named workers, named client |

---

## PART 7 — ENGINEERING ORDER

### 7.1 Build Order (Sequential + Parallel Streams)

```
STREAM A: CORE (sequential)
───────────────────────────
Week 1-4:    ECS Core, entity/component registry, archetype storage
Week 5-6:    Time System, event scheduling
Week 7-8:    Event Bus, Command Processor
Week 9-12:   Save/Load System (serialization, compression, versioning)

STREAM B: SIMULATION (sequential, starts after Stream A Week 4)
────────────────────────────────────────────────────────────────
Week 5-10:   Building System (design model, validation)
Week 11-16:  Construction System (phases, progress, quality)
Week 17-20:  Contract System (generation, bidding, lifecycle)
Week 21-24:  Worker System (hiring, skills, fatigue, assignment)
Week 25-28:  Company System (financials, dashboard data)
Week 29-32:  Economy System, Weather System (parallel)
Week 33-36:  Safety System, Reputation System, Research System

STREAM C: PRESENTATION (parallel, starts after Stream A Week 8)
────────────────────────────────────────────────────────────────
Week 9-14:   UE5 Integration Layer (ECS → renderer bridge)
Week 15-20:  UI Bridge (Noesis data binding, dashboard, HUD)
Week 21-26:  Camera System (orbit, free, first-person, drone)
Week 27-30:  Audio Bridge (Wwise integration, dynamic music)
Week 31-36:  UI Polish (animations, transitions, accessibility)

STREAM D: CONTENT (parallel throughout)
───────────────────────────────────────
Ongoing:     Building assets, environment art, worker models
Ongoing:     Contract templates, region definitions, balance data
Ongoing:     Audio assets, localization strings

STREAM E: TOOLS (parallel throughout)
─────────────────────────────────────
Week 1-8:    Asset Validator, Config Schema Validator
Week 9-16:   Building Designer Tool
Week 17-24:  Economy Balancer, Contract Editor
Week 25-36:  Replay Viewer, Performance Profiler, Mod SDK
```

### 7.2 System Build Sequence (number = order built)

```
 1. ECS Core
 2. Time System
 3. Event Bus
 4. Command Processor
 5. Save/Load
 6. Building System
 7. UI Bridge (basic)
 8. UE5 Integration (rendering bridge)
 9. Construction System
10. Contract System
11. Worker System
12. Company System
13. Camera System
14. Economy System
15. Weather System
16. Region System
17. Inventory/Material System
18. Finance System
19. Safety System
20. Reputation System
21. Research System
22. Notification System
23. Audio Bridge
24. Analytics System
25. Achievement System
26. Tutorial System
27. Mod Loader
28. Console Port Layer
```

---

## PART 8 — TEAM REQUIREMENTS

### 8.1 Team Composition (Peak: Month 18)

| Role | Count | Start | Phase |
|------|-------|-------|-------|
| **Engineering** | | | |
| Technical Director | 1 | Day 1 | All |
| Engine/ECS Engineers | 3 | Day 1 | All |
| Simulation Engineers | 4 | Month 3 | 0-2 |
| Gameplay Engineers | 3 | Month 6 | 1-3 |
| UI Engineers | 2 | Month 9 | 1-3 |
| Tools Engineers | 2 | Month 6 | 0-4 |
| Build/DevOps Engineer | 1 | Day 1 | All |
| Console Engineers | 2 | Month 24 | 4 |
| **Design** | | | |
| Design Director | 1 | Day 1 | All |
| Systems Designers | 2 | Month 1 | All |
| Level/World Designers | 2 | Month 12 | 2-4 |
| Economy Designer | 1 | Month 6 | 2-4 |
| UX Designer | 1 | Month 6 | 1-4 |
| **Art** | | | |
| Art Director | 1 | Month 3 | All |
| Environment Artists | 3 | Month 9 | 1-4 |
| Architectural Artists | 2 | Month 9 | 1-4 |
| Character Artist | 1 | Month 12 | 2-3 |
| Prop Artist | 1 | Month 12 | 2-3 |
| Lighting Artist | 1 | Month 15 | 2-4 |
| **Animation** | | | |
| Technical Animator | 1 | Month 12 | 2-4 |
| Animator | 1 | Month 15 | 2-4 |
| **Audio** | | | |
| Audio Director | 1 | Month 12 | 2-4 |
| Sound Designer | 1 | Month 15 | 2-4 |
| Composer | 1 (contract) | Month 18 | 2-4 |
| **Technical Art** | | | |
| Tech Art Lead | 1 | Month 6 | All |
| Tech Artist | 1 | Month 12 | 1-4 |
| **QA** | | | |
| QA Lead | 1 | Month 3 | All |
| QA Testers | 2 | Month 9 | 1-4 |
| QA Testers | +2 | Month 18 | 2-4 |
| QA Testers | +2 | Month 24 | 4-5 |
| **Production** | | | |
| Executive Producer | 1 | Day 1 | All |
| Producer | 1 | Day 1 | All |
| Producer | +1 | Month 12 | 1-4 |
| **Total Peak Team** | **45 people** | Month 18 | |

### 8.2 Team Growth Curve

```
COUNT
 45│                                    ▄████
 40│                              ▄█████
 35│                        ▄█████
 30│                  ▄█████
 25│            ▄█████
 20│      ▄█████
 15│ ▄█████
 10│████
  5│██
   └┴────┴────┴────┴────┴────┴────┴────┴────┴────┴────┴────
    M0   M3   M6   M9  M12  M15  M18  M21  M24  M27  M30
        Proto   FP    VS       EA           Launch
```

---

## PART 9 — RISK MANAGEMENT

### 9.1 Risk Register

| # | Risk | P | I | Score | Mitigation |
|---|------|---|---|-------|------------|
| R1 | Custom ECS/UE5 integration proves too complex | M | H | 12 | Prototype in Month 1. If unworkable after 4 weeks, fall back to UE5 Mass Entity with custom systems. Dedicated integration engineer. |
| R2 | Building design tool too complex for casual players | H | H | 16 | Usability test at Month 4 (prototype). Template/auto-design mode. Iterative UX testing every milestone. |
| R3 | Construction simulation doesn't feel satisfying | M | H | 12 | Prototype loop at Month 3. Daily internal playtests. If unsatisfying, rework progress visualization and milestone celebration before expanding. |
| R4 | City-scale performance (1000+ buildings) | M | H | 12 | Simulation LOD designed from Day 1. Performance benchmark added to CI at Month 6. Optimization sprints built into schedule (Months 25-29). |
| R5 | Save file corruption or migration failures | M | H | 12 | Atomic writes from Day 1. Save migration tested in CI from first versioned save. Save repair tool built. |
| R6 | Content production bottleneck (buildings, regions) | H | M | 9 | Procedural generation for base layouts + artist polish. Reuse InstaBuilt real product specs as content scaffolding. Building Designer Tool for rapid iteration. |
| R7 | Team scaling challenges (rapid growth M12-18) | M | M | 6 | Staggered hiring. Comprehensive onboarding docs. Pair programming. Clear module ownership. |
| R8 | Scope creep during Early Access | H | M | 9 | Feature freeze for EA defined and locked. Player suggestions categorized as post-launch. Dedicated Product Manager enforces scope. |
| R9 | Console certification delays | L | H | 5 | Console ports start M24 (6 months before launch). Dedicated console engineers. Pre-certification check built into CI. |
| R10 | Publisher milestones missed | M | H | 12 | Monthly publisher reviews. Buffer weeks built into schedule (Month 17, 23, 29). Transparent risk reporting. |

*P = Probability (L/M/H), I = Impact (L/M/H), Score = P×I (1-16)*

---

## PART 10 — FEATURE PRIORITIZATION

### MUST HAVE (v1.0 cannot ship without)

| # | Feature | Reason |
|---|---------|--------|
| 1 | Complete construction loop | The game. Non-negotiable. |
| 2 | All 7 InstaBuilt product lines | Core content. Defines the game's identity. |
| 3 | Design tool (full) | Core creative expression. |
| 4 | All 6 company tiers | Full progression. Without it, game ends too soon. |
| 5 | 4 regions at launch | Variety. Without regional variety, late game is repetitive. |
| 6 | Save/Load | Basic expectation. |
| 7 | 5-axis reputation | Consequence system. Without it, choices don't matter. |
| 8 | Worker system (full) | People make construction feel real. |
| 9 | Economy simulation | Financial stakes. Without it, no management depth. |
| 10 | Weather system | Environmental variety and strategic depth. |
| 11 | First-person walkthrough | Emotional payoff. |
| 12 | Photo mode + portfolio | Social sharing drives word-of-mouth. |
| 13 | Career Mode | Primary game mode. |
| 14 | Scenarios (20+) | Replayability and curated challenges. |
| 15 | Creative Architect Mode | Architectural expression. Community content engine. |
| 16 | Sandbox Mode | Player expectation for simulation games. |
| 17 | Tutorial + Mentor Pllana | Onboarding. Without it, retention collapses. |
| 18 | Localization (12 languages) | Market access. |
| 19 | Accessibility (all features) | Ethical requirement + market access. |

### SHOULD HAVE (v1.0 target; cut only if crisis)

| # | Feature | Reason |
|---|---------|--------|
| 20 | Blueprint sharing | Community engagement. Can ship in Day 1 patch. |
| 21 | Time-lapse replay | High shareability. Drives social media presence. |
| 22 | Drone camera mode | Distinctive feature. Press loves it. |
| 23 | Cinematic camera | Portfolio/social quality. |
| 24 | Dynamic music system | Immersion. Can ship with simpler system. |
| 25 | Skill trees (full) | Progression depth. Can simplify to linear unlocks. |
| 26 | Vehicle fleet management | Management depth. Core construction works without vehicles. |

### NICE TO HAVE (v1.0 if time; else post-launch)

| # | Feature | Reason |
|---|---------|--------|
| 27 | Company HQ designable | Fun but not core loop. |
| 28 | Named competitor AI companies | Adds personality to bidding. |
| 29 | In-game awards ceremony | Nice celebration moment. |
| 30 | Worker personality system | Adds flavor. Core worker system is sufficient. |
| 31 | Historical building plaques | Immersive detail. |
| 32 | Steam trading cards | Platform feature. |

### POST RELEASE (v1.1+)

| # | Feature | Timeline |
|---|---------|----------|
| 33 | Multiplayer co-op | 12-18 months post-launch |
| 34 | VR mode | 24 months post-launch |
| 35 | Region DLCs | Every 6 months |
| 36 | Specialization DLCs | Every 9 months |
| 37 | Community blueprint marketplace | 6 months post-launch |
| 38 | Console physical edition | 3 months post-digital |
| 39 | Mobile companion app | 12 months post-launch |

---

## PART 11 — FIRST 90 DAYS

### Week-by-Week Plan

```
WEEK 1-2: STUDIO SETUP
├── Engineering: Set up repo, CI, build system. Choose ECS library base (EnTT).
├── Design: Review all locked documents. Begin contract template design.
├── Production: Set up JIRA, sprint planning, stand-ups.
└── Review: Architecture walkthrough with full team.

WEEK 3-4: ECS CORE
├── Engineering: Implement entity manager, component registry, archetype storage.
├── Engineering: Implement basic system scheduler (single-threaded).
├── Engineering: Unit tests for entity create/destroy, component add/remove.
└── Review: ECS Core demo. Create 1000 entities, add components, query. Verify perf.

WEEK 5-6: TIME + EVENT BUS
├── Engineering: Implement Time System (game clock, speed control, pause).
├── Engineering: Implement Event Bus (publish, subscribe, immediate/deferred).
├── Engineering: Implement Command Processor (validate, queue, execute).
└── Review: Time flows at 1x/3x/10x. Events fire. Commands execute.

WEEK 7-8: SAVE/LOAD + UE5 BRIDGE
├── Engineering: Implement serialization (entity → binary, binary → entity).
├── Engineering: Implement save file write/read, compression, checksum.
├── Engineering: Begin UE5 integration (spawn/update/destroy UE Actors from ECS).
└── Review: Save a world with 1000 entities. Load it. Verify identical state.

WEEK 9-10: BUILDING SYSTEM — MODEL
├── Engineering: Implement Wall, Room, Door, Window components.
├── Engineering: Implement Building entity lifecycle (create → design → approved).
├── Engineering: Implement basic design commands (PlaceWall, AddRoom).
├── Design: Finalize POP UP 28 specs (room sizes, material requirements).
└── Review: Place walls via commands. Query rooms. Validate enclosure.

WEEK 11-12: BUILDING SYSTEM — VISUAL
├── Engineering: Implement building → renderable mesh translation.
├── Engineering: UE5 bridge renders buildings from ECS data.
├── Art: Deliver gray-box wall, floor, roof, door, window meshes.
├── Design: Implement basic design validation (walls enclose, rooms have doors).
└── Review: **FIRST BUILDING RENDERED.** Gray-box house visible in engine. ← MILESTONE

───────────────────────────────────────────────────────────────
★ END OF MONTH 3: FIRST BUILDING VISIBLE ★
───────────────────────────────────────────────────────────────

WEEK 13-14: CONSTRUCTION SYSTEM — CORE
├── Engineering: Implement ConstructionSite, ConstructionPhase entities.
├── Engineering: Implement phase progress accumulation.
├── Engineering: Implement Construction → Building visual update pipeline.
├── Design: Define 3-phase construction for prototype (Foundation → Frame → Finish).
└── Review: Construction progresses visibly. Building changes appearance per phase.

WEEK 15-16: CONSTRUCTION + WORKERS
├── Engineering: Implement Worker entities, C_WorkerStats, C_Assignment.
├── Engineering: Workers assigned to construction site → progress accelerates.
├── Engineering: Implement worker fatigue (basic: accumulate, recover, penalty).
├── Art: Deliver 2 gray-box worker models.
└── Review: **CONSTRUCTION LOOP WORKING.** Workers build a house. ← DEMO

WEEK 17-18: DESIGN TOOL — UI
├── Engineering: Implement design mode UI (grid, wall drawing, room labeling).
├── Engineering: Implement wall placement with snapping.
├── Engineering: Implement undo/redo for design actions.
├── UX: First usability test of design tool (internal, 5 people).
└── Review: Designer creates a house using UI only (no code commands).

WEEK 19-20: CONTRACT + COMPANY
├── Engineering: Implement Contract entity, generation from template.
├── Engineering: Implement Company entity, C_Financials.
├── Engineering: Implement contract → payment flow.
├── Design: Create 3 contract templates ("First Home", "Garage Addition", "Studio").
└── Review: Player can accept contract, build, deliver, receive payment.

WEEK 21-22: SAVE/LOAD — POLISH
├── Engineering: Implement auto-save (10-minute timer + milestone trigger).
├── Engineering: Implement save slots, save browser UI.
├── Engineering: Implement version migration (v1 → v2 test).
└── Review: Save/load 100 times. Load on different machine. Zero failures.

WEEK 23-24: DASHBOARD + TUTORIAL
├── Engineering: Implement company dashboard UI (contracts, finances, reputation).
├── Engineering: Implement tutorial system (scripted walkthrough).
├── Design: Write tutorial script (Mentor Pllana dialogue).
├── UX: Second usability test (external, 5 people).
└── Review: New player completes tutorial → dashboard → contract → design.

───────────────────────────────────────────────────────────────
★ END OF MONTH 6: PROTOTYPE COMPLETE ★
───────────────────────────────────────────────────────────────

WEEK 25-26: PROTOTYPE PLAYTEST
├── QA: Run 20-person internal playtest. Collect data.
├── Engineering: Fix critical bugs from playtest.
├── Design: Analyze playtest feedback. Prioritize changes.
└── Review: Playtest report. Exit survey results.

WEEK 27-28: PROTOTYPE ITERATION
├── Engineering: Address top 5 playtest issues.
├── Design: Iterate on tutorial based on confusion points.
├── UX: Third usability test (if needed for changed systems).
└── Review: Prototype quality check. Is it fun yet?

WEEK 29-30: FIRST PLAYABLE — CONTRACT SYSTEM
├── Engineering: Full Contract System (bidding, AI competitors, milestone payments).
├── Engineering: Deadline tracking, penalty/bonus calculation.
└── Review: Bid on contracts. Win/lose. Track deadlines.

WEEK 31-32: FIRST PLAYABLE — CONSTRUCTION EXPANSION
├── Engineering: Expand to 8 construction phases (from 3).
├── Engineering: Implement phase gating (inspections required).
├── Engineering: Implement quality tracking per phase.
└── Review: Full construction pipeline from foundation to handover.

WEEK 33-34: FIRST PLAYABLE — WORKER EXPANSION
├── Engineering: Implement worker hiring, skills, promotion.
├── Engineering: Implement crew formation and management.
├── Engineering: Implement morale system.
└── Review: Hire 10 workers. Form crews. Manage fatigue and morale.

WEEK 35-36: FIRST PLAYABLE — INTEGRATION
├── Engineering: End-to-end integration of all first playable systems.
├── QA: Full integration test pass.
├── Design: Content complete for first playable (3 contracts, balanced).
└── Review: **FIRST PLAYABLE CANDIDATE.** Full loop functional.

───────────────────────────────────────────────────────────────
★ END OF MONTH 9: FIRST PLAYABLE CANDIDATE ★
───────────────────────────────────────────────────────────────

WEEK 37-40: FIRST PLAYABLE — POLISH & TESTING (Months 10-12)
├── Engineering: Bug fixing. Performance optimization.
├── QA: External usability test (10 players). Save/load stress test.
├── Design: Tutorial refinement based on testing.
├── Art: Upgrade from gray-box to basic production assets.
├── Production: Prepare for Phase 2 (Vertical Slice) planning.
└── Review: **FIRST PLAYABLE APPROVED.** ← GATE FOR PHASE 2
```

---

## PRODUCTION ROADMAP APPROVED

### Final Counts

| Metric | Value |
|--------|-------|
| **Total Milestones** | 27 (M0-M27) |
| **Development Phases** | 5 (Prototype → Live Service) |
| **Critical Path** | ECS → Time → Building → Construction → Contract (5 sequential) |
| **Independent Streams** | 5 (Core, Simulation, Presentation, Content, Tools) |
| **First Playable Target** | Month 12 |
| **Vertical Slice Target** | Month 18 |
| **Early Access Launch** | Month 24 |
| **Full Release v1.0** | Month 30 |
| **Peak Team Size** | 45 people (Month 18) |
| **Must-Have Features** | 19 |
| **Should-Have Features** | 7 |
| **Nice-to-Have Features** | 6 |
| **Post-Release Features** | 7 |
| **Top Risks** | 10 (mitigated) |
| **First 90 Days** | 40 weeks planned to first playable candidate |

### Assumptions Locked

```
[x] 30-month development to v1.0 (realistic AAA simulation timeline)
[x] Prototype proves core loop by Month 6
[x] First vertical slice playable externally by Month 12
[x] Publisher milestone at Month 18 (Vertical Slice)
[x] Early Access at Month 24 funds final production
[x] Peak team of 45 (lean for AAA — enabled by ECS efficiency + UE5 tools)
[x] 5 parallel development streams maximize throughput
[x] Custom ECS is the highest-risk element; mitigated by Month 1 prototype
[x] 20% sprint budget for tech debt from Day 1
[x] Console ports start 6 months before launch (not after)
[x] All 19 must-have features ship in v1.0
```

---

**End of Production Roadmap — InstaBuilt: Blueprint Empire v1.0**

*The plan is locked. The documents are complete. The team is formed. Foundation week begins Monday.*
