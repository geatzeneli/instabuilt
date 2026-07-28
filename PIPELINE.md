# INSTABUILT: BLUEPRINT EMPIRE
## Production Pipeline Design v1.0

**Document Type:** Production Pipeline Specification
**Audience:** Build Engineers, Technical Artists, Tools Team, DevOps
**Prerequisites:** All prior documents LOCKED

---

## SECTION 1 — SAVE SYSTEM

### 1.1 Save Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    SAVE SYSTEM                           │
│                                                          │
│  ┌──────────────┐    ┌──────────────┐    ┌────────────┐ │
│  │ AUTO-SAVE    │    │ MANUAL SAVE  │    │ QUICK SAVE │ │
│  │ (15 min +    │    │ (player      │    │ (F5,       │ │
│  │  milestone)  │    │  initiated)  │    │  single    │ │
│  └──────┬───────┘    └──────┬───────┘    │  slot)     │ │
│         │                   │            └─────┬──────┘ │
│         └───────────────────┼──────────────────┘        │
│                             │                           │
│                    ┌────────▼────────┐                  │
│                    │  SAVE ORCHESTRATOR│                │
│                    │  - Serialize      │                │
│                    │  - Compress       │                │
│                    │  - Checksum       │                │
│                    │  - Atomic write   │                │
│                    └────────┬────────┘                  │
│                             │                           │
│              ┌──────────────┼──────────────┐            │
│              │              │              │            │
│     ┌────────▼─────┐ ┌─────▼──────┐ ┌────▼────────┐   │
│     │ LOCAL STORAGE│ │CLOUD SYNC  │ │ SAVE BROWSER│   │
│     │ (SSD/HDD)    │ │(Steam Cloud│ │ (UI for     │   │
│     │              │ │ / platform)│ │  load/del)  │   │
│     └──────────────┘ └────────────┘ └─────────────┘   │
└─────────────────────────────────────────────────────────┘
```

### 1.2 Auto-Save Triggers

| Trigger | Frequency |
|---------|-----------|
| Timer | Every 15 real minutes |
| Contract milestone | On completion of each construction phase |
| Contract award | When player wins a bid |
| Company tier up | On tier promotion |
| Major purchase | Equipment/vehicle >$100K |
| Before risky action | Before accepting loan, before major bid |
| Application pause | When game loses focus (configurable) |
| Pre-crash guard | Emergency save if memory pressure detected |

### 1.3 Manual Saves

- 10 save slots per profile.
- Named by player (default: "Save [N] — [Date] [Company] [Tier]").
- Thumbnail auto-captured from current camera view.
- Save/load from in-game pause menu or dashboard.
- Save files portable between machines (same platform account).

### 1.4 Save Version Migration

```
Save v3  →  Migrate_v3_to_v4()  →  Save v4  →  ...  →  Save vCurrent

Migration functions:
  - Registered in migration chain by version
  - Each migration transforms entity/components in place
  - Tested with archived saves from every previous version
  - If migration gap >10 versions, warn player, attempt best-effort
```

### 1.5 Incremental Saves

- Full save: all entities. (~15MB, every 5th save).
- Incremental save: only entities with C_DirtyFlag set. (~2-5MB, 4 of 5 saves).
- Incremental saves reference the most recent full save.
- On load: load full save, replay incremental deltas.
- Full save automatically triggered if incremental chain exceeds 4.

### 1.6 Backup Strategy

- Previous save retained as `.bak` before overwrite.
- Rolling backup: last 3 saves kept (configurable).
- Auto-saves in separate folder, rolling 5.
- Steam Cloud keeps version history (platform feature).

### 1.7 Corruption Recovery

- Checksum verification on load.
- If corrupted: offer to load `.bak` or previous auto-save.
- If all copies corrupted: offer new game. Log corruption details.
- Save Repair Tool: standalone utility that attempts to salvage readable entities from corrupted saves. Runs offline.

---

## SECTION 2 — ASSET PIPELINE

### 2.1 Pipeline Overview

```
SOURCE ASSETS                     ENGINE ASSETS                    GAME
────────────                      ─────────────                    ────
                                                          
.blend / .fbx  ──→  Mesh Processor  ──→  .uasset (static mesh)
.png / .tga    ──→  Texture Processor ──→ .uasset (texture)
.sbs / .sbsar  ──→  Material Processor──→ .uasset (material)
.png / .svg     ──→  UI Asset Proc    ──→  .noesis (UI asset)
.wav / .flac   ──→  Audio Processor   ──→  .wem / .bnk (Wwise)
.po            ──→  Loc Processor     ──→  .locres (localization)
.toml          ──→  Config Validator  ──→  .bin (compiled config)
```

### 2.2 Mesh Pipeline

```
Source Art (Blender/Maya)
  │
  ├── Export: FBX 2023, meters, Z-up
  │
  ├── Mesh Processor (automated via UE5 Datasmith + custom rules)
  │   ├── Validate: naming convention, pivot position, scale (1 unit = 1m)
  │   ├── Generate: collision mesh, LODs (LOD0=100%, LOD1=50%, LOD2=25%, LOD3=10%)
  │   ├── UV check: lightmap UVs on channel 1
  │   ├── Material assignment: slot names match material catalog
  │   └── Nanite: enable for architectural meshes (>500 tris)
  │
  ├── Output: .uasset (StaticMesh), .uasset (NaniteMesh)
  │
  └── Validation Report: errors block commit; warnings logged
```

**LOD Generation Rules:**

| Object Type | LOD0 Distance | LOD1 | LOD2 | LOD3 |
|-------------|--------------|------|------|------|
| Building exterior | 0-50m | 50-150m | 150-500m | 500m+ |
| Building interior | 0-15m | 15-40m | 40-100m | 100m+ |
| Vehicles | 0-30m | 30-100m | 100m+ | — |
| Workers | 0-20m | 20-60m | 60m+ | — |
| Vegetation | 0-30m | 30-80m | 80-200m | 200m+ |
| Props/furniture | 0-10m | 10-30m | 30m+ | — |

### 2.3 Material Pipeline

```
Source (Substance Designer / Painter)
  │
  ├── Export: .sbsar (parametric) or texture set (BaseColor, Normal, ORM, Roughness)
  │
  ├── Material Processor
  │   ├── Validate: texture resolution (2K standard, 4K hero assets)
  │   ├── Pack: ORM (Occlusion+Roughness+Metallic) into single texture
  │   ├── Generate: material instance from master material
  │   └── Assign: physical material properties (for audio + physics)
  │
  ├── Master Material variants: Standard, Architectural, Vegetation, Vehicle, UI
  │
  └── Output: .uasset (MaterialInstance)
```

### 2.4 Audio Pipeline

```
Source (.wav 48kHz/24bit, .flac)
  │
  ├── Import into Wwise Authoring Tool
  │
  ├── Audio Processor
  │   ├── Categorize: SFX, Ambience, Music, Voice, UI
  │   ├── Apply: compression (Vorbis for SFX, Opus for voice)
  │   ├── Configure: attenuation curves, spatialization, RTPC bindings
  │   ├── Generate: SoundBanks (.bnk), streamed audio (.wem)
  │   └── Validate: loudness standards (LUFS), peak levels
  │
  └── Output: Wwise project + generated banks → engine loads via Wwise SDK
```

### 2.5 UI Pipeline

```
Source (Figma / Adobe XD designs → exported specs)
  │
  ├── UI Artist creates Noesis XAML + styles
  │
  ├── UI Processor
  │   ├── Validate: XAML syntax, data binding paths, localization keys
  │   ├── Compile: XAML → binary .noesis
  │   ├── Atlas: UI textures packed into atlas sheets
  │   └── Test: automated layout validation at 4 resolutions
  │
  └── Output: .noesis (compiled UI), texture atlases
```

---

## SECTION 3 — CONTENT PIPELINE

### 3.1 Designer Workflow

```
BALANCE DESIGNER                    QUEST/CONTRACT DESIGNER
────────────────                    ──────────────────────
  │                                    │
  ├── Edit .toml in VSCode             ├── Edit .toml in VSCode
  │   (with schema validation)         │   (with schema validation)
  │                                    │
  ├── git commit                       ├── git commit
  │                                    │
  ├── CI validates TOML schema        ├── CI validates TOML schema
  │   ├── Economy balancer runs        │   ├── Contract validator runs
  │   └── Reports balance issues       │   └── Reports impossible requirements
  │                                    │
  ├── Hot-reload in running game       ├── Hot-reload in running game
  │   (dev build only)                 │   (dev build only)
  │                                    │
  └── Playtest → iterate               └── Playtest → iterate
```

### 3.2 Artist Workflow

```
3D ARTIST                            TECHNICAL ARTIST
─────────                            ────────────────
  │                                    │
  ├── Create asset in DCC              ├── Review asset
  │   (Blender/Maya/Substance)         │   ├── Naming ✓
  │                                    │   ├── Scale ✓
  ├── Export to project source dir     │   ├── Pivot ✓
  │                                    │   ├── UVs ✓
  ├── git add + commit                 │   └── Poly budget ✓
  │                                    │
  ├── CI triggers asset pipeline       ├── If failed → return to artist
  │   ├── Mesh processing              │   with report
  │   ├── Validation checks             │
  │   ├── Generate LODs                ├── If passed → approve
  │   └── Output to Content/            │
  │                                    │
  └── Asset appears in UE5 editor      └── Houdini procedural tools
      for level design use                 for large-scale generation
```

### 3.3 Level Designer Workflow

```
LEVEL DESIGNER
──────────────
  │
  ├── UE5 Editor with custom InstaBuilt tooling
  │
  ├── Place parcels, roads, terrain (procedural + manual polish)
  │
  ├── Assign zoning, climate, economic profiles to districts
  │
  ├── Place landmark buildings, parks, infrastructure
  │
  ├── Validate: all parcels reachable, no overlapping, zoning consistent
  │
  ├── Export region definition .toml (auto-generated from level data)
  │
  └── git commit → CI bakes lighting → region appears in game
```

### 3.4 QA Workflow

```
QA ENGINEER
───────────
  │
  ├── Pull latest build from CI
  │
  ├── Run automated test suite
  │   ├── Headless simulation regression (1,000 projects, verify no crash)
  │   ├── UI screenshot comparison (pixel diff vs baseline)
  │   ├── Performance benchmarks (FPS, memory, load time)
  │   └── Save/load round-trip (100 saves, verify integrity)
  │
  ├── Manual test pass (assigned areas)
  │   ├── Record bugs in JIRA with repro steps + save file
  │   └── Verify fixed bugs from previous build
  │
  └── Sign-off: build meets quality bar for this milestone
```

---

## SECTION 4 — MOD SUPPORT

### 4.1 Mod Distribution

- **Steam Workshop** (primary): One-click subscribe, auto-download, auto-update.
- **Manual install:** Drop folder into `Game/Mods/`. Detected on launch.
- **In-game Mod Manager:** Enable/disable, reorder, resolve conflicts.

### 4.2 Mod Packaging

```
MyMod/
├── mod.toml              # Metadata: name, version, author, dependencies
├── Config/               # TOML overrides (merged with base)
│   ├── buildings.toml
│   ├── materials.toml
│   └── contracts.toml
├── Assets/               # Custom 3D models, textures, audio
│   ├── Meshes/
│   ├── Textures/
│   └── Audio/
├── Localization/         # String overrides or new languages
│   └── en.po
├── Scripts/              # Lua behaviors (post-launch feature)
│   └── custom_phase.lua
└── Preview/              # Workshop thumbnail + screenshots
    └── thumbnail.png
```

### 4.3 Mod Validation

- Schema validation of all TOML files.
- Asset reference resolution (no dangling references).
- Lua sandboxing (no file I/O, no network, limited CPU).
- Conflict detection between active mods.
- Mod crash → mod disabled, not game crash.

---

## SECTION 5 — EDITOR TOOLS

### 5.1 Building Designer Tool

**Purpose:** Standalone or in-editor tool for creating building templates and testing designs.

**Features:**
- Full design workflow from GSS Section 6.
- Structural validation with visual overlays.
- Cost estimation in real-time.
- Export as Blueprint (for game) or FBX (for external use).
- Template library browser.
- Procedural generation test (generate 100 variants of a template, validate all).

### 5.2 Economy Balancer Tool

**Purpose:** Visualize and tune the economy simulation.

**Features:**
- Time-series graphs of all tracked prices.
- "What-if" scenario testing (change base price, see ripple effects over 10 game-years).
- Inflation/deflation trend analysis.
- Balance issue detection (price spiral, starvation, wealth accumulation).
- Export balancing recommendations.

### 5.3 Contract Editor

**Purpose:** Create and validate contract templates.

**Features:**
- Template editor with live preview.
- Requirement conflict detection.
- Profitability simulation (run 100 AI bidders against this contract, show win rate).
- Difficulty rating auto-calculation.

### 5.4 Replay Viewer

**Purpose:** Review recorded gameplay sessions for debugging and content creation.

**Features:**
- Record: all Commands + initial state + RNG seed.
- Playback: deterministic replay of recorded session.
- Scrub: jump to any point in the recording.
- Export: render to video for trailers / bug reports.
- Diff: compare two replays (expected vs actual behavior).

### 5.5 Performance Profiler

**Purpose:** In-engine profiling of simulation and rendering performance.

**Features:**
- Per-system CPU time breakdown.
- Memory allocation tracking per system.
- Entity/component count trending over time.
- Cache miss visualization.
- Automated regression detection (compare to baseline).
- Export to Chrome Tracing format for external analysis.

### 5.6 Asset Validator

**Purpose:** Automated validation of all game assets before they enter the build.

**Features:**
- Naming convention enforcement.
- Texture resolution and format checks.
- Mesh polygon budget verification.
- Material slot consistency.
- Audio loudness compliance.
- Localization completeness check (no missing keys).

---

## SECTION 6 — IMPORT / EXPORT

### 6.1 Blueprint Sharing

**Export:**
- Player clicks "Export Blueprint" in Creative Architect mode.
- Generates .ibbp file (InstaBuilt Blueprint Package).
- Contains: wall/room/MEP data, materials, thumbnail.
- Can be shared via Steam Workshop or exported to disk.

**Import:**
- Player clicks "Import Blueprint" in Design mode.
- Selects .ibbp file or browses Workshop.
- Blueprint appears in Blueprint Library.
- Can be used as starting point for any compatible contract.

### 6.2 Portfolio Export

- Export building as: FBX (3D model), PNG (screenshot), JSON (data).
- For 3D printing: STL export of simplified building geometry.
- For content creators: 360° video export of building walkthrough.

### 6.3 Save Export/Import

- Export save as .ibsave file (portable between machines).
- Import save from file.
- Cloud sync provides this automatically, but manual option for offline.

---

## SECTION 7 — LOCALIZATION PIPELINE

### 7.1 Workflow

```
ENGLISH SOURCE (.po template)
  │
  ├── Extract: build tool scans all .toml, .xaml, C++ for localizable strings
  │
  ├── Generate: .pot (template) file with all string keys
  │
  ├── Send to localization vendor / community translators
  │
  ├── Receive: .po files per language
  │
  ├── Validate:
  │   ├── No missing keys
  │   ├── No broken format specifiers (%s, %d, {0})
  │   ├── No HTML/XAML tag breakage
  │   └── Plurals correctly defined
  │
  ├── Compile: .po → .locres (binary, fast lookup)
  │
  └── Test: automated screenshot comparison per language (UI layout check)
```

### 7.2 Supported Languages (Launch)

| # | Language | Code | RTL |
|---|----------|------|-----|
| 1 | English | en | No |
| 2 | German | de | No |
| 3 | French | fr | No |
| 4 | Spanish | es | No |
| 5 | Italian | it | No |
| 6 | Japanese | ja | No |
| 7 | Korean | ko | No |
| 8 | Simplified Chinese | zh-CN | No |
| 9 | Brazilian Portuguese | pt-BR | No |
| 10 | Polish | pl | No |
| 11 | Russian | ru | No |
| 12 | Arabic | ar | Yes |

### 7.3 Continuous Localization

- String freeze: 2 weeks before each major update.
- Loc kit sent to translators; returned within 1 week.
- Late string additions flagged; urgent translation path available.
- Community translation program for post-launch languages.

---

## SECTION 8 — PATCHING & HOTFIXES

### 8.1 Patch Tiers

| Tier | Size | Frequency | Content | Approval |
|------|------|-----------|---------|----------|
| **Hotfix** | <50 MB | As needed (hours) | Critical bug fixes, crash fixes, economy emergency | Lead Engineer |
| **Patch** | <500 MB | Weekly | Bug fixes, minor balance, small features | QA Lead |
| **Update** | 1-5 GB | Monthly | New content, features, scenarios | Prod Lead |
| **Major Update** | 5-20 GB | Quarterly | DLC, regions, major features | Studio Director |

### 8.2 Hotfix Pipeline

```
1. Bug identified (crash report, player report, QA)
2. Engineer fixes in hotfix branch (off release tag)
3. Code review (1 reviewer, expedited)
4. Automated tests pass
5. QA smoke test (1 hour)
6. Build + sign
7. Deploy to Steam (default branch)
8. Players receive update on next launch
9. Monitor crash rate, rollback if >0.1% increase
```

### 8.3 Delta Patching

- Binary diff between versions (courgette/bsdiff).
- Players download only changed bytes.
- Typical patch: 50-200 MB (not full 500 MB).
- Steam handles delta patching automatically for Steam builds.

---

## SECTION 9 — DLC PIPELINE

### 9.1 DLC Architecture

```
Base Game
├── Core systems (always present)
├── Base regions (Region 1)
├── Base buildings (all InstaBuilt lines)
└── Base scenarios (10)

DLC: Scandinavia Region
├── New region definition (.toml)
├── Region-specific assets (meshes, textures, audio)
├── Region-specific contracts
├── New weather profiles (extreme cold, northern lights)
├── New architectural style (Nordic)
└── New scenarios (5)

DLC: Green Builder
├── New research nodes
├── New materials (sustainable catalog)
├── New certification system (LEED-style)
├── New contracts (green-focused)
└── New scenarios (3)
```

### 9.2 DLC Development Flow

```
1. DLC scoped → GDD supplement written → Architecture impact assessed
2. Feature branch created off main
3. DLC content developed (same pipeline as base game)
4. DLC systems use extension points: register new region, add components
5. Base game systems remain unchanged
6. DLC tested standalone + integrated with base game
7. Base game patched if needed (add extension points, no breaking changes)
8. DLC released on Steam as separate App ID (depot)
9. Players without DLC: DLC regions grayed out with "Buy" prompt
10. Players with DLC: seamless access
```

---

## SECTION 10 — BUILD PIPELINE

### 10.1 CI/CD Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     GITHUB / PERFORCE                        │
│                  (Source Control)                            │
└────────────────────────┬────────────────────────────────────┘
                         │
                         │ git push / p4 submit
                         ▼
┌─────────────────────────────────────────────────────────────┐
│                    JENKINS CI SERVER                         │
│                                                              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐       │
│  │ PRE-COMMIT   │  │  NIGHTLY     │  │  RELEASE     │       │
│  │ (every push) │  │  (01:00 UTC) │  │  (on tag)    │       │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘       │
│         │                 │                 │                │
│         ▼                 ▼                 ▼                │
│  ┌──────────────────────────────────────────────────────┐   │
│  │                 BUILD STAGES                          │   │
│  │                                                       │   │
│  │  1. Checkout + dependency resolution                  │   │
│  │  2. Compile (UE5 + custom core)                       │   │
│  │  3. Asset pipeline (process all assets)               │   │
│  │  4. Unit tests                                        │   │
│  │  5. Integration tests                                 │   │
│  │  6. Simulation regression (headless, 1000 projects)   │   │
│  │  7. Performance benchmarks                            │   │
│  │  8. Package (platform-specific)                       │   │
│  │  9. Sign (platform cert)                              │   │
│  │  10. Deploy (Steam, internal distribution)            │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

### 10.2 Build Targets

| Target | Platform | Frequency | Purpose |
|--------|----------|-----------|---------|
| Editor | Win64 | Every push (incremental) | Developer iteration |
| Editor | Win64 | Nightly (full rebuild) | Daily stable editor |
| Game Client | Win64 | Nightly | Daily playtest build |
| Game Client | Win64 | Weekly | QA test pass |
| Game Client | Win64 + Linux | Release | Steam deployment |
| Game Client | PS5 / XSX | Release | Console cert |
| Simulation Core | Linux (headless) | Nightly | Server simulation testing |
| Tools | Win64 | Weekly | Tool releases for content team |

### 10.3 Build Times (Targets)

| Build Type | Target Time | Strategy |
|------------|-------------|----------|
| Incremental (code change) | <2 min | Precompiled headers, unity builds, distributed compilation (Incredibuild) |
| Incremental (asset change) | <5 min | Asset cache, incremental processing |
| Full rebuild (nightly) | <4 hours | Parallelized across 20 build agents |
| Release build | <6 hours | Full LTO, extra validation, signing |

---

## PIPELINE APPROVED

### Final Counts

| Pipeline | Tools/Processes |
|----------|----------------|
| Save System | 7 features (auto, manual, quick, incremental, backup, recovery, cloud) |
| Asset Pipeline | 5 processors (mesh, material, texture, audio, UI) |
| Content Pipeline | 4 workflows (designer, artist, LD, QA) |
| Mod Support | 3 distribution channels, 4 validation checks |
| Editor Tools | 6 tools |
| Localization | 12 launch languages, continuous pipeline |
| Patching | 4 tiers, delta patching |
| DLC | Extension point architecture |
| Build Pipeline | 3 CI pipelines, 7 build targets |

---

**End of Production Pipeline Design — InstaBuilt: Blueprint Empire v1.0**
