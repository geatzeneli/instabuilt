# INSTABUILT: BLUEPRINT EMPIRE
## Engineering Standards v1.0

**Document Type:** Engineering Standards & Operations
**Audience:** All Engineering Team Members
**Prerequisites:** All prior documents LOCKED
**Enforcement:** CI validates. PR reviewers enforce. Leads audit quarterly.

---

## SECTION 1 — PERFORMANCE BUDGETS

### 1.1 Frame Budget (Target: 60 FPS = 16.67ms)

| System Group | Budget (ms) | % of Frame |
|-------------|-------------|------------|
| UE5 Rendering (Nanite, Lumen) | 8.0 | 48% |
| Custom Simulation (all systems) | 4.0 | 24% |
| UI Rendering (Noesis) | 1.5 | 9% |
| Audio (Wwise update) | 0.5 | 3% |
| Input Processing | 0.3 | 2% |
| UE5 Overhead (ticking, GC) | 1.5 | 9% |
| **Reserve** | **0.87** | **5%** |

### 1.2 Simulation Budget Breakdown (4.0ms)

| System | Budget (ms) | Notes |
|--------|-------------|-------|
| Construction System | 1.5 | Heaviest system; scales with active sites |
| Worker System | 0.4 | Batch-processed |
| Economy System | 0.1 | Per-day update amortized |
| Weather System | 0.05 | Per-hour update |
| Building System | 0.3 | Primarily design-mode; idle during construction |
| Safety System | 0.1 | Per-hour |
| All other systems | 1.0 | Combined |
| **Reserve** | **0.55** | Burst headroom |

### 1.3 Memory Budget (Target: 8 GB for min spec, 16 GB recommended)

| Category | Budget (GB) | Notes |
|----------|-------------|-------|
| UE5 Engine + Rendering | 2.5 | Baseline engine |
| Game Assets (loaded) | 2.0 | Textures, meshes, audio |
| Simulation State | 0.5 | All ECS data for active entities |
| Completed Building Cache | 0.5 | Compressed static building data |
| UI + Noesis | 0.3 | UI textures, fonts, layouts |
| Audio (Wwise) | 0.2 | Loaded sound banks |
| Streaming Pool | 1.0 | Async asset loading |
| OS + Background | 1.0 | Windows, Steam, etc. |

### 1.4 GPU Budget (Target: GTX 1060 / RX 580 minimum)

| Category | Budget (ms GPU) |
|----------|----------------|
| Nanite rasterization | 5.0 |
| Lumen GI + reflections | 3.0 |
| Shadow maps | 2.0 |
| Post-processing | 1.5 |
| UI compositing | 0.5 |
| **Total GPU** | **12.0** (target <16.67) |

---

## SECTION 2 — LOD STRATEGY

### 2.1 Rendering LOD

| LOD Level | Screen % | Transition Distance | Poly Reduction |
|-----------|----------|---------------------|----------------|
| LOD0 | >10% | 0m | 100% (full detail) |
| LOD1 | 5-10% | 50m | 50% polys |
| LOD2 | 2-5% | 150m | 25% polys |
| LOD3 | <2% | 500m | 10% polys (Nanite auto) |
| Culled | <0.1% | 2000m+ | Not rendered |

Nanite handles LOD for static geometry automatically. Custom LOD chains only for skeletal meshes (workers, vehicles).

### 2.2 Simulation LOD

| Distance from Camera | Simulation Detail |
|---------------------|-------------------|
| 0-200m | Full simulation (all components active) |
| 200-500m | Reduced (workers simplified to point agents, no individual tasks) |
| 500-2000m | Summary only (building progress bar, no internal simulation) |
| 2000m+ | Frozen (static completion state, no simulation) |

Simulation LOD only applies to buildings the player doesn't own. The player's active construction sites always run at full simulation.

### 2.3 Audio LOD

- 0-50m: Full spatial audio, individual source rendering.
- 50-150m: Reduced sources (merge nearby sounds, lower quality).
- 150m+: Ambient only (construction site becomes distant hum).

---

## SECTION 3 — OBJECT POOLING

### 3.1 Pooled Entity Types

| Entity Type | Pool Size | Growth Policy |
|-------------|-----------|---------------|
| ConstructionTask | 500 | Doubles on exhaustion, max 2000 |
| Notification | 100 | Fixed (oldest dismissed) |
| SafetyIncident | 50 | Fixed |
| ParticleEmitter | 200 | Fixed (managed by UE5 Niagara) |
| PathfindingRequest | 100 | Fixed (queue-based) |

### 3.2 Pool Rules

- Pooled entities are never truly destroyed (EntityId persists).
- On "destroy," entity resets to default state and returns to pool.
- On "create," pool is checked first; new allocation only if pool exhausted.
- Pool exhaustion is a warning event (logged, analyzed).

---

## SECTION 4 — MULTITHREADING

### 4.1 Thread Architecture

```
┌────────────────────────────────────────────────┐
│              MAIN THREAD (UE5 Game Thread)      │
│  Input → Commands → State Machines → UI Update │
└────────────────┬───────────────────────────────┘
                 │
    ┌────────────┼────────────┐
    │            │            │
┌───▼────┐ ┌────▼────┐ ┌────▼────┐
│ RENDER │ │SIMULATION│ │  AUDIO  │
│ THREAD │ │ THREAD  │ │ THREAD  │
│ (UE5)  │ │ (Custom)│ │ (Wwise) │
└────────┘ └────┬────┘ └─────────┘
                 │
    ┌────────────┼────────────┐
    │            │            │
┌───▼────┐ ┌────▼────┐ ┌────▼────┐
│WORKER 1│ │WORKER 2 │ │WORKER N │  ← Task graph jobs
│(build- │ │(economy)│ │(weather)│
│ ings)  │ │         │ │         │
└────────┘ └─────────┘ └─────────┘
```

### 4.2 Parallelization Strategy

- **Frame-level:** Simulation thread runs in parallel with render thread. Sync point at frame end.
- **System-level:** Independent systems (Weather, Economy) run as task graph jobs with declared dependencies.
- **Data-level:** Construction System parallelizes per-site (each site is independent). Worker System parallelizes per-crew.
- **No locks:** Systems communicate via lock-free event queue. Component storage uses atomic operations for rare writes, single-writer principle for common writes.

### 4.3 Synchronization Points

| Point | Frequency | Mechanism |
|-------|-----------|-----------|
| Input → Simulation | Per frame | Command queue (lock-free SPSC) |
| Simulation → Render | Per frame | Presentation Bus snapshot (atomic swap) |
| Simulation → Audio | Per frame | Event queue (lock-free MPSC) |
| Save | On trigger | All systems drained to save state (rare) |

---

## SECTION 5 — PROFILING

### 5.1 Profiling Stack

| Tool | Purpose | When |
|------|---------|------|
| Unreal Insights | UE5 rendering, game thread profiling | Weekly, pre-release |
| Custom Simulation Profiler | Per-system CPU, entity counts, event throughput | Daily in CI, on-demand in dev |
| Tracy | Micro-level CPU/GPU tracing | On-demand for optimization |
| PIX (Xbox) / Razor (PS5) | Console GPU profiling | Pre-certification |
| Superluminal | Windows CPU sampling | On-demand |
| Wwise Profiler | Audio CPU/memory | Weekly |

### 5.2 Automated Performance Regression

- CI runs benchmark scenario (100 buildings, 500 workers, 10 active sites, 5 minutes at 10x speed).
- Records: average FPS, 99th percentile frame time, memory high-water mark, simulation tick time.
- Compares to baseline from previous build.
- Regression >5% in any metric → build flagged, engineer assigned.

---

## SECTION 6 — TESTING STRATEGY

### 6.1 Test Pyramid

```
           ┌─────────┐
           │ACCEPTANCE│  10 tests  (GSS compliance: "Can player complete a contract?")
           │  TESTS   │
           ├─────────┤
           │ SIMULATION│ 50 tests  (Headless: run 1000 projects, verify no crash/state corruption)
           │  TESTS   │
           ├───────────┤
           │INTEGRATION│ 200 tests (Multi-system: "Does weather affect construction?")
           │  TESTS    │
           ├─────────────┤
           │  UNIT TESTS  │ 1000+ tests (Per-system, per-function)
           └───────────────┘
```

### 6.2 Unit Testing

- **Framework:** Catch2 (custom core), UE5 Automation Framework (engine code).
- **Coverage target:** 80% line coverage for domain systems. 60% for application layer. 40% for presentation.
- **Mocking:** Systems tested in isolation with mock dependencies (mock EventBus, mock TimeSystem).
- **Per-system requirements:** Every public function has at least one test. Every event handler has at least one test. Every failure case has a test.

### 6.3 Simulation Testing (Headless)

- Custom headless build of simulation core (no rendering, no audio, no UE5).
- Runs on Linux CI agents. Fast: 1000-project simulation in <30 seconds.
- Tests:
  - Determinism: same seed + same inputs → same outputs.
  - Stability: run for 100 in-game years, verify no crash, no memory leak, no NaN/Inf propagation.
  - Economy: verify prices stay within sane bounds.
  - Construction: verify all building types complete successfully.
  - Save/load: save mid-simulation, load, verify state matches.

### 6.4 Regression Testing

- Save file library: 100 player saves from various game versions, playtimes, tiers.
- On every build: load each save, run simulation for 1 in-game month, verify no crash.
- Screenshot comparison: load each save, capture screenshot at known camera positions, pixel-diff against baseline. Flag differences >1%.

### 6.5 Performance Testing

- Benchmark scenarios (small city, medium city, large city).
- Run on reference hardware (min spec, recommended spec, high-end).
- Record FPS, frame time distribution, memory, load time.
- Trend over time: performance dashboard shows FPS per build. Regression alert.

---

## SECTION 7 — CI/CD

### 7.1 Branch Strategy

```
main ────────────────────────────────────────────────────── (releasable)
  │
  ├── develop ───────────────────────────────────────────── (integration)
  │     │
  │     ├── feature/building-validation ────── (short-lived, 1-5 days)
  │     ├── feature/weather-system
  │     └── feature/xxx
  │
  ├── release/v1.0 ──────────────────────────────────────── (stabilization)
  │     │
  │     └── hotfix/crash-on-load ────────────── (urgent, cherry-pick to main)
  │
  └── tags: v1.0.0, v1.0.1, v1.1.0
```

### 7.2 CI Pipeline (per commit to develop/feature)

```
1. Lint (5s)           → naming, formatting, include order
2. Build (5 min)       → incremental compile
3. Unit Tests (2 min)  → all unit tests
4. Integration (3 min) → integration test suite
5. Asset Validate (1m) → asset compliance
6. Sim Regression (30s)→ headless simulation
─────────────────────────────────────────
Total: ~12 minutes per commit
```

### 7.3 Nightly Pipeline

```
1. Full rebuild (1 hour)
2. All tests (15 min)
3. Full sim regression (10 min)
4. Performance benchmarks (30 min)
5. Package editor + client (30 min)
6. Deploy to internal Steam branch (5 min)
─────────────────────────────────────────
Total: ~2.5 hours
Available for playtest by 09:00 daily
```

### 7.4 Release Pipeline

```
1. Create release/vX.Y branch from main
2. Feature freeze: only bug fixes
3. Full test pass (manual QA + automated)
4. Performance validation (all target platforms)
5. Certification check (platform TRCs)
6. Build + sign (all platforms)
7. Deploy to Steam (staging branch → default branch)
8. Monitor crash rate for 24 hours
9. If <0.1% crash rate: promote to all players
```

---

## SECTION 8 — CODE REVIEW

### 8.1 Review Requirements

- All changes require review before merge.
- Minimum 1 reviewer (2 for core systems, 1 for tools/scripts).
- Reviewer cannot be the author.
- CI must pass before review can be approved.
- Review checklist:
  - [ ] Architecture compliance (dependency direction, forbidden deps)
  - [ ] Performance (no O(N²) on hot path, no allocation per frame)
  - [ ] Test coverage (new code has tests, modified behavior has updated tests)
  - [ ] Memory (no leaks, pooled where appropriate)
  - [ ] Thread safety (correct synchronization, no data races)
  - [ ] Error handling (no bare exceptions, crash resilience)
  - [ ] Documentation (public API documented, complex logic commented)
  - [ ] Naming conventions followed

### 8.2 Review SLAs

| PR Size | Review Within |
|---------|---------------|
| <50 lines | 2 hours |
| 50-200 lines | 4 hours |
| 200-500 lines | 1 business day |
| >500 lines | 2 business days (strongly discouraged; split into smaller PRs) |

---

## SECTION 9 — NAMING CONVENTIONS

### 9.1 C++

| Element | Convention | Example |
|---------|-----------|---------|
| Namespaces | PascalCase | `InstaBuilt::Construction` |
| Classes/Structs | PascalCase | `BuildingSystem`, `C_Wall` |
| Functions | PascalCase | `CalculateQualityScore()` |
| Variables | camelCase | `workerCount`, `currentPhase` |
| Constants | kPascalCase | `kMaxWorkersPerCrew` |
| Enums | PascalCase | `BuildingState::UnderConstruction` |
| Macros | UPPER_SNAKE | `INSTABUILT_VERSION_MAJOR` |
| Member fields | camelCase_ | `phaseProgress_`, `workerId_` |
| Static members | s_camelCase | `s_instanceCount` |
| Component types | C_PascalCase | `C_Transform`, `C_WorkerStats` |
| System types | PascalCase + "System" | `ConstructionSystem` |

### 9.2 Files

| Type | Convention | Example |
|------|-----------|---------|
| Headers | PascalCase.h | `BuildingSystem.h` |
| Source | PascalCase.cpp | `BuildingSystem.cpp` |
| Tests | PascalCaseTest.cpp | `BuildingSystemTest.cpp` |
| Config | snake_case.toml | `building_defs.toml` |
| Assets | snake_case | `popup_28_exterior.fbx` |

### 9.3 Git

| Element | Convention | Example |
|---------|-----------|---------|
| Branches | kebab-case | `feature/building-validation` |
| Commits | imperative, <=72 chars | `Add wall collision validation` |
| PR titles | imperative, descriptive | `[Construction] Fix phase transition crash` |

---

## SECTION 10 — LOGGING STANDARDS

### 10.1 Log Levels

| Level | Usage | Shipping |
|-------|-------|----------|
| TRACE | Per-frame details, entity lifecycle | NO |
| DEBUG | System state transitions, computed values | NO |
| INFO | Major events: contract awarded, building completed, tier up | YES |
| WARN | Non-critical issues: material shortage, worker fatigue, approaching deadline | YES |
| ERROR | Failures: inspection failed, payment rejected, save corrupted | YES |
| FATAL | Unrecoverable: crash imminent | YES |

### 10.2 Log Format

```
[2028-07-15 14:32:01.234] [INFO] [Construction] Site Riverside-12: Phase 2 (Foundation) completed. Quality: 94.2%
[2028-07-15 14:32:01.456] [WARN] [Worker] Employee Marku (ID: a3f2...): Fatigue critical (87%). Consider rest.
[2028-07-15 14:32:02.001] [ERROR] [Save] Auto-save failed: Disk full (path: C:/Saves/auto_3.ibsave)
```

### 10.3 Privacy Rule

**No personally identifiable information in logs, ever.** No player names, no file paths containing usernames, no hardware IDs, no IP addresses. Logged EntityIds are fine (opaque).

---

## SECTION 11 — TELEMETRY

### 11.1 Event Categories

| Category | Example Events | Opt-out Allowed |
|----------|---------------|-----------------|
| Progression | Tier unlocked, contract completed, building completed | No (essential for balancing) |
| Economy | Cash balance sampled daily, loan taken, bankruptcy | No |
| Performance | FPS, frame time, memory, load time | No |
| Feature Usage | Camera mode switches, design tool usage, time speed changes | Yes |
| Errors | Crashes, assertion failures, save failures | No (essential for stability) |
| Session | Playtime, session count, return rate | Yes |

### 11.2 Privacy

- Telemetry is opt-in (GDPR/CCPA consent on first launch).
- No personal data transmitted.
- Telemetry ID is random per install, not linked to Steam/Platform account.
- Data retained for 2 years, then aggregated and anonymized.
- Players can request their telemetry data (GDPR access) or deletion.

### 11.3 Crash Reporting

- Stack trace + minidump captured on crash.
- Last 60 seconds of Event Bus activity included.
- System states snapshot included (entity counts, active sites, financial state).
- Player offered option to include save file (helps reproduction).
- Crash report sent via Sentry/Backtrace.

---

## SECTION 12 — TECHNICAL DEBT

### 12.1 Tracking

- Technical debt items tracked as JIRA issues with label `tech-debt`.
- Categorized: Performance, Architecture, Test Coverage, Tooling, Documentation.
- Estimated effort in story points.
- Reviewed quarterly by Engineering Leads.

### 12.2 Budget

- 20% of each sprint allocated to tech debt reduction.
- Priority: items causing bugs, blocking features, or slowing iteration.
- "Tech Debt Day" on last Friday of each sprint: team focuses entirely on debt.

### 12.3 Prevention

- Architecture compliance enforced by CI (forbidden dependencies fail build).
- Complexity budget: any function >50 lines flagged for review. Any class >500 lines flagged for refactor discussion.
- "Boy Scout Rule": leave code cleaner than you found it. Small cleanups in every PR.

---

## SECTION 13 — RELEASE CHECKLIST

### 13.1 Pre-Release

```
[ ] All automated tests passing (unit, integration, simulation, performance)
[ ] Zero P0/P1 bugs open
[ ] Performance within budgets on all target platforms
[ ] Memory within budget on min spec hardware
[ ] Save/load compatibility verified (all archived saves load)
[ ] Localization complete for all 12 languages (no missing keys)
[ ] Accessibility features verified (screen reader, colorblind, controller)
[ ] No debug logging enabled in shipping build
[ ] Crash reporting configured and tested
[ ] Analytics configured and tested
[ ] Steam achievements configured
[ ] Steam Cloud save configured
[ ] Build signed with appropriate certificates
[ ] Store page updated (screenshots, description, system requirements)
[ ] Legal review complete (EULA, privacy policy, third-party licenses)
[ ] Press build sent to PR team
```

### 13.2 Post-Release (24 hours)

```
[ ] Monitor crash rate (<0.1% target)
[ ] Monitor player support tickets
[ ] Monitor Steam reviews
[ ] Monitor social media / Discord
[ ] Engineering on-call rotation active
[ ] Hotfix ready if critical issue found
```

### 13.3 Console Certification (additional)

```
[ ] TRC/XR compliance verified (Sony, Microsoft)
[ ] Age rating submitted and approved
[ ] First-party store page live
[ ] Trophy/achievement set configured per platform
[ ] Save data migration path tested (PS4→PS5, XB1→XSX)
[ ] Controller features: haptic feedback, adaptive triggers, light bar
```

---

## SECTION 14 — RISK MANAGEMENT

| Risk | Mitigation | Owner | Review Cadence |
|------|-----------|-------|---------------|
| Key person dependency | Documentation, pair programming, knowledge sharing | Tech Director | Monthly |
| Build breakage blocks team | Pre-commit CI, revert-on-red policy | Build Engineer | Continuous |
| Performance regression | Automated benchmarks, trend dashboard | Performance Lead | Weekly |
| Save corruption | Atomic writes, backup strategy, repair tool | Systems Lead | Per-release |
| Scope creep | Feature gate review, ADR for major additions | Prod Lead | Sprint planning |
| Burnout | 20% tech debt time, no crunch policy, rotating on-call | Studio Director | Quarterly |

---

## ENGINEERING STANDARDS APPROVED

### Final Counts

| Category | Count/Spec |
|----------|-----------|
| Performance Budgets | 4 (frame, simulation, memory, GPU) |
| LOD Strategies | 3 (rendering, simulation, audio) |
| Pooled Types | 5 |
| Thread Architecture | 4 thread types + N workers |
| Test Types | 5 (unit, integration, simulation, regression, performance) |
| CI Pipelines | 3 (per-commit, nightly, release) |
| Code Review SLA | 5 size tiers |
| Naming Conventions | 30+ rules |
| Supported Languages | 12 |
| Release Checklist Items | 22 pre-release, 4 post-release |

---

## APPENDIX: DOCUMENT MAP

```
InstaBuilt/
├── GDD.md              ← Game Design Document (what we're building)
├── GSS.md              ← Gameplay Systems Specification (how it plays)
├── ARCHITECTURE.md     ← Software Architecture (how systems connect)
├── DATA_MODEL.md       ← Data Model & ECS (what data looks like)
├── SIMULATION.md       ← Simulation Systems (how simulation runs)
├── PIPELINE.md         ← Production Pipeline (how we build it)
└── STANDARDS.md        ← Engineering Standards (how we work)
```

*These seven documents define the complete vision, design, architecture, data, simulation, pipeline, and standards for InstaBuilt: Blueprint Empire. No ambiguity remains. Build begins now.*

---

**End of Engineering Standards — InstaBuilt: Blueprint Empire v1.0**
