# INSTABUILT: BLUEPRINT EMPIRE
## Phase 6 — Early Access Production Roadmap v1.0

**Document Type:** Production Roadmap — Phase 6
**Target:** Steam Early Access Launch (Month 24 per ROADMAP.md)
**Starting Point:** Vertical Slice Complete (11 systems, 19 components, 20 tests)
**Rule:** Every milestone expands existing systems. Nothing is rewritten.

---

## 1. MILESTONE OVERVIEW

```
M1  — Construction Depth         (8-phase pipeline, MEP, weather, defects)
M2  — Building Variety           (multi-floor, roofs, 7 product lines)
M3  — Company Management         (departments, loans, insurance, marketing)
M4  — Employee AI                (skills, morale, personalities, training)
M5  — World Simulation           (multi-district city, zoning, population)
M6  — Contract System            (bidding, negotiation, government, types)
M7  — Economy & Supply Chain     (suppliers, logistics, market dynamics)
M8  — Content Production         (25+ buildings, 100+ contracts, 2 regions)
M9  — Performance & Polish       (60 FPS city, LOD, streaming, save perf)
M10 — Early Access Assembly      (career mode, sandbox, full loop)
```

### Dependency Graph

```
M1 (Construction) ──┬──→ M2 (Building Variety) ──→ M8 (Content)
                    │
                    ├──→ M4 (Employee AI) ──→ M3 (Company Mgmt)
                    │                              │
                    ├──→ M6 (Contracts) ───────────┤
                    │                              │
                    └──→ M7 (Economy) ──→ M5 (World) ──→ M9 (Performance)
                                                              │
                    M8 (Content) ────────────────────────────→ M10 (EA Assembly)
```

---

## 2. DETAILED MILESTONES

---

### M1 — CONSTRUCTION DEPTH

**Goal:** Expand from 4-phase prototype construction to the full 8-phase pipeline with MEP, weather effects, safety, and defect systems.

**Expands:** FConstructionSystem, C_ConstructionState, C_BuildingQuality

| Feature | Description | Depends On |
|---------|-------------|------------|
| 8-phase pipeline | Expand from 4 to 8 phases: SitePrep, Foundation, Structure, Envelope, MEP_RoughIn, InteriorFinishes, FinalSystems, PunchList | C_ConstructionState |
| MEP systems | Electrical, plumbing, HVAC rough-in tracking per phase | C_MEPSystem (new) |
| Weather effects | C_WeatherState impacts construction speed and quality | FWeatherSystem (new) |
| Safety incidents | C_SafetyIncident with risk calculation from fatigue, weather, phase | FSafetySystem (new) |
| Construction defects | C_BuildingQuality expanded: per-phase defect generation with severity | C_BuildingQuality |
| Material logistics | C_MaterialStockpile per site, consumption tracking, shortage events | FInventorySystem (new) |
| Equipment usage | C_EquipmentState, condition tracking, breakdown probability | FEquipmentSystem (new) |
| Critical inspections | Inspection gates at phases 2, 3, 5, 6, 8 — failure requires rework | C_Inspection (new) |

**New Components (6):**
- C_MEPSystem — electrical/plumbing/HVAC state per building
- C_WeatherState — temperature, wind, precipitation, forecast
- C_SafetyIncident — type, severity, affected workers, resolution
- C_MaterialStockpile — per-site inventory of materials
- C_EquipmentState — type, condition, operator, utilization
- C_Inspection — phase, inspector, result, required fixes

**New Systems (3):**
- FWeatherSystem — climate profiles, Markov chain weather, construction impact
- FSafetySystem — risk aggregation, incident generation, reputation impact
- FInventorySystem — stockpile management, consumption, shortage detection

**Engineering:** ~8,000 lines — expand ConstructionSystem, add 3 new systems, 6 components
**Art:** Weather particles, safety incident VFX, inspection NPC model
**Design:** 8 phase definitions, weather profiles per climate, safety risk curves, defect probability tables
**QA:** Test all 8 phases complete successfully, weather impacts verified, safety incidents fire
**Risk:** Medium — ConstructionSystem is already modular; expansion is additive

**Definition of Done:** Player watches a building go through all 8 phases with weather interruptions and a minor safety incident resolved.

---

### M2 — BUILDING VARIETY

**Goal:** Multi-floor buildings, roof systems, structural validation, and the first 3 InstaBuilt product lines.

**Expands:** FBuildingDesigner, C_BuildingDesignData, C_Room, C_Wall

| Feature | Description |
|---------|-------------|
| Multi-floor | Floor management panel, stair placement, floor copy/mirror, floor-height configuration |
| Roof systems | 7 roof types (Flat, Gable, Hip, Mansard, Gambrel, Shed, Butterfly), pitch, overhang, dormers |
| Structural validation | Load path analysis, span checking, beam/column requirements, seismic zones |
| Room customization | Room templates, fixture placement, lighting, trim, flooring per room |
| Advanced materials | 50+ materials across 8 categories, each with cost/quality/energy ratings |
| Architectural styles | 5 style presets (Modern, Traditional, Colonial, Craftsman, Mediterranean) |
| Building codes | Per-region code requirements: setbacks, height, fire, accessibility, energy |
| POP UP Solutions | 28/52/104 m² modular units — prefab placement, configurable layouts |
| Multistory-Multifamily | Multi-unit buildings, unit numbering, shared walls, common areas |
| Traditional Homes | Regional styles, pitched roofs, brick/wood facades, character details |

**New Components (4):**
- C_Floor — floor index, height, room/wall references
- C_Roof — type, pitch, overhang, material, dormers
- C_StructuralElement — beam, column, load path data
- C_BuildingCode — per-region code requirements

**Engineering:** ~6,000 lines — expand BuildingDesigner, add floor/roof/structural subsystems
**Art:** Roof meshes, material textures (50+), style preset assets, multi-floor templates
**Design:** Product line specs, code requirements per region, material catalog
**QA:** Design a 3-story building with custom roof, validate structurally, build successfully
**Risk:** Medium — design tool complexity increases significantly; UX testing critical

**Definition of Done:** Player designs a 3-story Traditional Home with gable roof, passes structural validation, and builds it.

---

### M3 — COMPANY MANAGEMENT

**Goal:** Full company management: departments, loans, insurance, marketing, supplier relationships, equipment ownership.

**Expands:** FCompanyManager, C_EmployeeContract, C_Financials

| Feature | Description |
|---------|-------------|
| Departments | Operations, Projects, Finance, Admin — each with budgets and staff |
| Loans | 5 loan types (equipment, construction, credit line, expansion, emergency), interest, repayment |
| Insurance | Liability, workers comp, equipment, builder's risk — mandatory, premiums, claims |
| Marketing | Campaign types (portfolio, social, events, billboards), brand awareness, cost/benefit |
| Equipment ownership | Buy vs rent analysis, depreciation, maintenance scheduling, resale value |
| Suppliers | 3-5 suppliers per material, relationships, bulk discounts, reliability ratings |
| Company valuation | Asset-based + revenue multiple + reputation modifier |
| Financial reports | P&L, balance sheet, cash flow projection, project profitability |

**New Components (5):**
- C_Department — name, budget, headcount, manager
- C_Loan — type, principal, rate, term, remaining, status
- C_InsurancePolicy — type, coverage, premium, claims history
- C_MarketingCampaign — type, cost, reach, duration, effectiveness
- C_Supplier — name, materials, base price, reliability, relationship

**Engineering:** ~5,000 lines — expand CompanyManager, add FinanceSystem subsystems
**Design:** Loan terms, insurance rates, marketing effectiveness curves, supplier catalog
**QA:** Take a loan, buy equipment, run a marketing campaign, file an insurance claim
**Risk:** Low — management systems are additive, don't affect construction loop

**Definition of Done:** Player manages a company with 3 departments, an active loan, insurance policies, and a marketing campaign while building.

---

### M4 — EMPLOYEE AI

**Goal:** Named workers with skills, personalities, morale, fatigue, training, and career progression.

**Expands:** FWorkerSystem, C_WorkerStats, C_EmployeeContract

| Feature | Description |
|---------|-------------|
| Skill progression | 5 skill trees (Architecture, Construction, Business, Technology, Leadership), XP from projects |
| Personalities | 12 personality traits (Perfectionist, FastLearner, Lazy, Leader, etc.) affecting behavior |
| Morale system | Event-driven morale changes, threshold effects (quit risk, productivity, strike risk) |
| Fatigue system | Accumulation during work, recovery during rest, overtime penalties, refusal threshold |
| Training | Send workers to training (cost + time → skill increase), certification requirements |
| Specializations | Workers develop specialties: Framing, Electrical, Finishing, etc. — productivity bonuses |
| Crew dynamics | Crew cohesion bonus, conflict between incompatible personalities, leader influence |
| Career path | Laborer → Journeyman → Master → Supervisor → Manager, with salary expectations |

**New Components (3):**
- C_Personality — traits, compatibilities, mood
- C_SkillTree — 5 axes, levels, XP, unlocked abilities
- C_Crew — members, cohesion, leader, history

**Engineering:** ~4,000 lines — expand WorkerSystem with AI behaviors
**Design:** Personality matrix, skill curves, morale/fatigue balance, training catalog
**Art:** Worker model variants (hard hats, tool belts, role-specific gear)
**QA:** Hire 10 workers with different personalities, form crews, observe dynamics over 5 projects
**Risk:** Medium — AI behaviors must feel natural, not robotic; iteration needed

**Definition of Done:** A worker hired as Laborer with "FastLearner" trait reaches Journeyman after 5 projects and requests a raise.

---

### M5 — WORLD SIMULATION

**Goal:** Multi-district city, zoning, population simulation, NPC buildings, economic districts.

**Expands:** FWorldManager, C_District, C_Plot, C_Road

| Feature | Description |
|---------|-------------|
| Multi-district city | 5+ districts (Downtown, Suburban, Industrial, Riverside, Historic) with distinct character |
| Zoning | Residential (R1-R4), Commercial (C1-C3), Industrial (I1-I2), Mixed-Use — affects what can be built |
| Population simulation | District population, growth rate, income levels, housing demand |
| NPC buildings | AI-built structures fill the world — competitors, generic buildings, landmarks |
| Economic districts | Land values, property taxes, development incentives per district |
| Traffic | Road network with vehicle simulation, construction traffic impacts |
| City services | Water, power, sewer, internet — availability affects plot value and buildability |
| City council | Periodic zoning changes, development approvals, community opposition events |

**New Components (4):**
- C_Population — count, density, growth, income distribution
- C_ZoningRegulation — type, height limit, setbacks, use restrictions, variances
- C_LandValue — current value, trend, tax rate
- C_CityService — type, coverage area, capacity, reliability

**Engineering:** ~5,000 lines — expand WorldManager, add population/zone/service subsystems
**Design:** 5 district definitions, zoning rules, population curves, city service configurations
**Art:** District-specific building assets, road variations, city props, NPC building variants
**QA:** Build in 3 different districts, verify zoning restrictions, see population changes
**Risk:** Medium — world simulation must not tank performance with large city

**Definition of Done:** Player browses 5 districts with different zoning, builds in 2, sees NPC buildings fill unbuilt plots.

---

### M6 — CONTRACT SYSTEM

**Goal:** Multiple contract types, competitive bidding, negotiation, client relationships, government RFPs.

**Expands:** FContractSystem, C_ContractData, C_ClientData

| Feature | Description |
|---------|-------------|
| Multiple types | Residential, Commercial, Industrial, Institutional, Infrastructure, Emergency, Government |
| Competitive bidding | AI competitors bid against player; bid competitiveness gauge; win/lose with feedback |
| Negotiation | Counter-offer on price, timeline, scope; client personality affects flexibility |
| Client relationships | Returning clients, loyalty discounts, referral chains, reputation per client |
| Government RFPs | Complex proposals, strict requirements, high reward, public scrutiny |
| Emergency contracts | Disaster recovery, urgent repairs — fast timeline, premium pay, huge reputation |
| Contract templates | Designer tool for creating new contract types (dev tool) |
| Milestone payments | Partial payments at phase completions, not just final delivery |

**New Components (3):**
- C_Competitor — name, size, specialization, aggression, current workload
- C_Bid — contract, bidder, amount, timeline, quality tier, competitiveness
- C_ClientRelationship — history, trust level, discount expectation, referral count

**Engineering:** ~4,000 lines — expand ContractSystem with bidding/negotiation/competitor AI
**Design:** Competitor profiles, bid evaluation formulas, negotiation parameters, contract type catalog
**QA:** Bid on 20 contracts against AI, negotiate 5, win 10, track relationship growth
**Risk:** Low — additive system; core contract loop already proven

**Definition of Done:** Player bids against 3 AI competitors, loses to a lower bid, negotiates the next one, wins with a higher margin.

---

### M7 — ECONOMY & SUPPLY CHAIN

**Goal:** Dynamic economy with supply/demand, supplier relationships, logistics, market cycles.

**Expands:** FEconomySystem, C_Financials, C_MarketPrice

| Feature | Description |
|---------|-------------|
| Supply/demand | Material prices fluctuate per region based on construction activity, season, events |
| Supplier network | Multiple suppliers per material, negotiation, reliability, delivery times |
| Logistics | Material delivery scheduling, transport costs, warehouse storage |
| Market cycles | Boom/bust cycles (6-12 month period), interest rate changes, inflation |
| Competitor economy | AI companies also build, affecting demand and prices |
| Property market | Land values change based on development, building resale value |
| Economic events | Recession, material shortage, labor strike, regulatory change, tax incentive |
| Price forecasting | 30-day price trend graphs for planning material purchases |

**New Components (3):**
- C_SupplyChain — source, transport, delivery time, reliability
- C_EconomicCycle — phase (boom/stable/slowdown/recession), duration, intensity
- C_PropertyValue — current, purchase price, appreciation rate, tax assessment

**Engineering:** ~3,000 lines — expand EconomySystem with supply/demand and market cycles
**Design:** Base prices, supply/demand curves, cycle parameters, economic event catalog
**QA:** Run 10-year simulation, verify price stability, market cycles, supplier behavior
**Risk:** Low — economy is self-contained; other systems just query current prices

**Definition of Done:** Player sees lumber prices spike during a construction boom, decides to delay a project, prices normalize 3 months later.

---

### M8 — CONTENT PRODUCTION

**Goal:** 25+ building types, 100+ contracts, 2 complete regions for Early Access.

**Expands:** All config-driven systems

| Feature | Description |
|---------|-------------|
| Building catalog | 25+ building types across 7 product lines, each with full specs |
| Contract catalog | 100+ contracts across 7 categories, procedurally varied |
| Region 1: Riverside | Starting region — suburban/riverside, 5 districts, temperate climate |
| Region 2: Northwood | Unlockable region — forested, cold climate, different codes and styles |
| Material catalog | 80+ materials, each with cost, quality, energy, availability properties |
| Blueprint library | 20 starter blueprints for quick design, community sharing foundation |
| Scenario mode | 5 scenarios (First Home, The Big One, Tight Deadline, Green Mandate, Luxury Estate) |

**New Content (data-driven, not code):**
- 25 building definition TOML files
- 100+ contract template TOML files
- 2 region definition TOML files
- 80+ material catalog entries
- 20 blueprint files
- 5 scenario definition files

**Engineering:** Minimal code — content is configuration. Maybe 1,000 lines for scenario mode.
**Art:** Building assets, region-specific props, material textures, blueprint thumbnails
**Design:** Primary workload — all TOML content, balance testing, scenario scripting
**QA:** Validate all 25 buildings build successfully, all 100 contracts completable
**Risk:** Medium — content volume is high; requires designer tooling to be efficient

**Definition of Done:** Player can browse 25 building types, select any, and see a valid contract for it.

---

### M9 — PERFORMANCE & POLISH

**Goal:** 60 FPS in a large city, simulation LOD, streaming, save optimization, memory budgets.

**Expands:** All systems (optimization, not new features)

| Feature | Description |
|---------|-------------|
| Simulation LOD | Buildings >200m from camera: reduce to summary state (progress only) |
| Render LOD | Nanite for static geometry, custom LOD chains for dynamic objects |
| Streaming | District streaming — only load assets for visible + adjacent districts |
| Save compression | Completed buildings stored as compressed static data (<1KB avg) |
| Memory budget | Simulation state <500MB, render <2GB, audio <200MB, UI <300MB |
| Parallel update | Construction sites updated in parallel (task graph), workers batched |
| Object pooling | ConstructionTask, Notification, SafetyIncident pooled to reduce allocations |
| UI optimization | Noesis data binding updated only on change, not per frame |
| Profiling | Per-system CPU budget with automated regression detection in CI |
| 60 FPS cert | Min spec (GTX 1060): Large city (1,000 buildings, 10 active sites) at 60 FPS |

**Engineering:** ~4,000 lines — optimization across all systems, profiling infrastructure
**QA:** Performance test suite: small/medium/large city benchmarks, memory tracking, frame analysis
**Risk:** Medium — optimization may require algorithm changes in hot-path systems

**Definition of Done:** Benchmark: 1,000 buildings in city, 10 active construction sites, steady 60 FPS on min spec hardware.

---

### M10 — EARLY ACCESS ASSEMBLY

**Goal:** Complete Early Access build: career mode, sandbox mode, full loop, Steam integration.

**Feature** | **Description**
**Career Mode** | Tier 1-3 progression, 25+ buildings, 100+ contracts, 2 regions, full company management
**Sandbox Mode** | Unlimited money, all buildings unlocked, free build anywhere
**Tutorial** | 30-minute guided first experience (Mentor Pllana dialogue scripted)
**Steam Integration** | Achievements (20), Cloud Save, Workshop (blueprint sharing), Rich Presence
**Main Menu** | New Game, Continue, Load, Scenarios, Sandbox, Settings, Credits
**Settings** | Graphics, Audio, Controls, Accessibility, Language (English + German at EA)
**Crash Reporting** | Sentry/Backtrace integration, auto-save on crash detection
**Analytics** | Opt-in telemetry: progression, economy, performance, errors
**EA Disclaimer** | "This game is in Early Access. Features may change. Your feedback shapes development."
**Patch Pipeline** | Hotfix (<50MB, hours), Patch (<500MB, weekly), Update (1-5GB, monthly)

**Engineering:** ~6,000 lines — game modes, Steam SDK, menu system, analytics, crash reporting
**Art:** Menu backgrounds, achievement icons, loading screens, EA branding
**Design:** Tutorial script, achievement definitions, analytics event catalog
**QA:** Full regression pass, Steamworks validation, 100-player closed beta
**Production:** Store page, trailer, press kit, community hub, Discord

**Definition of Done:** Steam Early Access build live. Player downloads, plays tutorial, completes first contract, saves, returns next day.

---

## 3. CONTENT PIPELINE

### Building Creation Workflow

```
Designer specs → BuildingDesigner tool → TOML config
  → Art creates assets → Asset pipeline validation
  → QA builds test building → Iterate
  → Deploy in next content update
```

### Contract Creation Workflow

```
Designer defines template → ContractEditor tool → TOML config
  → Procedural variation rules defined
  → QA validates all variations completable
  → Deploy
```

### Testing Workflow

```
Per-commit: lint, build, unit tests, integration tests (~12 min)
Nightly: full rebuild, all tests, simulation regression, performance benchmark (~2.5 hrs)
Weekly: QA manual pass, content validation, save compatibility
Pre-release: 100-player beta, 48-hour stability test
```

---

## 4. PERFORMANCE TARGETS

| Metric | Target |
|--------|--------|
| FPS (min spec, large city) | 60 FPS |
| Frame time budget (sim) | 4.0ms |
| Frame time budget (render) | 8.0ms |
| Memory (simulation state) | <500 MB |
| Memory (total) | <4 GB |
| Save file size (100hr) | <50 MB |
| Load time (100hr save) | <10 seconds |
| Crash rate | <0.1% |

---

## 5. RISK REGISTER

| # | Risk | P | I | Mitigation |
|---|------|---|---|------------|
| R1 | Construction 8-phase complexity delays M1 | M | H | Keep phases data-driven; hardcode first 4, config-drive last 4 |
| R2 | Design tool UX fails player testing | M | H | Weekly usability tests during M2; template system as fallback |
| R3 | Large city performance below 60 FPS | M | H | Simulation LOD from start; perf benchmarks in CI from M1 |
| R4 | Content volume (25 buildings, 100 contracts) not met | H | M | Hire 2 content designers at M8 start; use procedural generation |
| R5 | Employee AI feels robotic | M | M | Personality-driven dialogue; fake it well; iterative testing |
| R6 | Economy balancing is unstable | L | H | 10-year sim regression in CI from M7; automated balance alerts |

---

## EARLY ACCESS PRODUCTION PLAN APPROVED

| Metric | Value |
|--------|-------|
| Milestones | 10 (M1-M10) |
| New Components | 28 |
| New Systems | 9 |
| Building Types | 25+ |
| Contracts | 100+ |
| Regions | 2 |
| Product Lines | 7 (all InstaBuilt) |
| Performance Target | 60 FPS large city |
| Team Size (peak) | 28 (12 eng, 6 art, 4 design, 3 QA, 3 prod) |
| Duration | ~12 months from VS to EA |

**Ready to begin M1 implementation. Waiting for approval.**
