# INSTABUILT: BLUEPRINT EMPIRE
## Data Model & Entity Relationship Specification v1.0

**Document Type:** Data Model Specification
**Audience:** Systems Engineers, Database Architects, Gameplay Engineers
**Prerequisites:** GDD (LOCKED), GSS (LOCKED), ARCHITECTURE.md (LOCKED)
**Rule:** This is the single source of truth for every byte of game data.

---

## SECTION 1 — ENTITY CATALOG

### 1.1 Entity Summary

| # | Entity | Owner System | Lifetime | Persistence |
|---|--------|-------------|----------|-------------|
| 1 | PlayerCompany | Company System | Game-long | Full |
| 2 | Employee | Worker System | Hire → Termination | Full |
| 3 | Contract | Contract System | Generation → Completion/Cancellation | Full |
| 4 | Client | Contract System | Persistent | Full |
| 5 | Building | Building System | Design start → Permanent | Full (permanent after completion) |
| 6 | BuildingDesign | Building System | Design session → Approved | Full |
| 7 | ConstructionSite | Construction System | Project start → Project completion | Full |
| 8 | ConstructionPhase | Construction System | Per-phase lifecycle | Full |
| 9 | ConstructionTask | Construction System | Per-task within phase | Transient |
| 10 | Wall | Building System | Building lifetime | Full |
| 11 | Room | Building System | Building lifetime | Full |
| 12 | Door | Building System | Building lifetime | Full |
| 13 | Window | Building System | Building lifetime | Full |
| 14 | Roof | Building System | Building lifetime | Full |
| 15 | Foundation | Building System | Building lifetime | Full |
| 16 | MEPSystem | Building System | Building lifetime | Full |
| 17 | Vehicle | Equipment System | Purchase → Sale/Destruction | Full |
| 18 | Equipment | Equipment System | Purchase → Sale | Full |
| 19 | Material | Inventory System | Persistent (catalog) | Full (catalog), Transient (stockpile instances) |
| 20 | MaterialStockpile | Inventory System | Per-site, per-project | Transient |
| 21 | Supplier | Economy System | Persistent | Full |
| 22 | Invoice | Finance System | Generated → Paid/Overdue | Full |
| 23 | Loan | Finance System | Issued → Repaid/Defaulted | Full |
| 24 | Bank | Economy System | Persistent | Full |
| 25 | Region | Region System | Persistent (static data) | Config only |
| 26 | City | Region System | Persistent | Full |
| 27 | District | Region System | Persistent | Full |
| 28 | Parcel | Region System | Persistent | Full |
| 29 | WeatherProfile | Weather System | Per-region, persistent | Config |
| 30 | WeatherState | Weather System | Per-frame | Transient |
| 31 | EconomyState | Economy System | Per-region | Full |
| 32 | MarketPrice | Economy System | Per-material, per-region | Full |
| 33 | ResearchNode | Research System | Persistent | Full |
| 34 | Technology | Research System | Unlocked → Permanent | Full |
| 35 | ReputationProfile | Reputation System | Game-long | Full |
| 36 | Achievement | Achievement System | Unlocked → Permanent | Full |
| 37 | Notification | Notification System | Generated → Dismissed | Transient |
| 38 | Message | Notification System | Generated → Read/Archived | Full |
| 39 | SaveProfile | Save/Load System | Per-save slot | Metadata |
| 40 | TimeController | Time System | Game-long | Full |
| 41 | Calendar | Time System | Game-long | Full |
| 42 | Permit | Region System | Applied → Issued/Denied | Full |
| 43 | Inspection | Construction System | Scheduled → Completed | Full |
| 44 | SafetyIncident | Safety System | Occurred → Resolved | Full |
| 45 | Skill | Worker System | Persistent (definitions) | Config |
| 46 | Crew | Worker System | Formed → Disbanded | Full |
| 47 | Blueprint | Building System | Saved → Deleted | Full (account-wide) |
| 48 | CompanyPolicy | Company System | Set → Changed | Full |
| 49 | MarketingCampaign | Company System | Started → Ended | Full |
| 50 | GameModeState | Application Layer | Per-session | Transient |
| 51 | CameraBookmark | Camera System | Saved → Deleted | Full (per-save) |
| 52 | AnalyticsSession | Analytics Service | Per-launch | Transient (sent then discarded) |

**Total: 52 entity types.**

### 1.2 Entity Details

#### PlayerCompany

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Root entity representing the player's construction corporation. |
| **Owner System** | Company System |
| **Lifetime** | Created at new game start. Never destroyed (only on game deletion). |
| **Unique ID** | `CompanyId` — singleton (always ID 1). |
| **Creation Rules** | Auto-created by Company System during world initialization. |
| **Destruction Rules** | Never destroyed during normal gameplay. Removed only when save is deleted. |
| **Persistence** | Fully serialized. Core of save data. |

#### Employee (supertype of Worker, Architect, Engineer, Manager)

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Any person working for the player's company. |
| **Owner System** | Worker System |
| **Lifetime** | Created on hire. Destroyed on termination, retirement, or resignation. |
| **Unique ID** | `EmployeeId` — UUID v4, globally unique. |
| **Creation Rules** | Generated by Worker System when player hires. NPCs have generated names, faces, traits. |
| **Destruction Rules** | Termination: entity archived (kept for historical records). Retirement: entity marked inactive. |
| **Persistence** | Fully serialized. Historical employees retained for portfolio context. |

#### Building

| Attribute | Detail |
|-----------|--------|
| **Purpose** | A constructed or in-construction building in the game world. |
| **Owner System** | Building System |
| **Lifetime** | Created at design start. Persists permanently after completion (10+ year lifespan). |
| **Unique ID** | `BuildingId` — UUID v4 + region prefix. |
| **Creation Rules** | Created when player starts designing. Assigned BuildingState::Designing. |
| **Destruction Rules** | Never destroyed in normal gameplay. Can be demolished (rare, reputation penalty, archived). |
| **Persistence** | Active buildings: full serialization. Completed buildings: compressed static data (geometry, materials, metadata only — no simulation state). |

#### Contract

| Attribute | Detail |
|-----------|--------|
| **Purpose** | A binding agreement between a Client and the PlayerCompany to construct a building. |
| **Owner System** | Contract System |
| **Lifetime** | Generated → Bid → Awarded → Active → Completed/Cancelled. |
| **Unique ID** | `ContractId` — UUID v4. |
| **Creation Rules** | Generated by Contract System from templates + procedural variation. |
| **Destruction Rules** | Archived after completion or cancellation. Never fully deleted (audit trail). |
| **Persistence** | Active: full serialization. Completed: archived with summary data. |

#### ConstructionSite

| Attribute | Detail |
|-----------|--------|
| **Purpose** | The active construction operation on a specific parcel for a specific building. |
| **Owner System** | Construction System |
| **Lifetime** | Created when construction begins. Destroyed when building is completed. |
| **Unique ID** | `SiteId` — derived from BuildingId. |
| **Creation Rules** | Created by Construction System when player clicks "Begin Construction". |
| **Destruction Rules** | Archived on completion. Active simulation state removed. |
| **Persistence** | Full serialization while active. Archived on completion. |

#### Region

| Attribute | Detail |
|-----------|--------|
| **Purpose** | A geographic region with distinct climate, economy, building codes, and architectural styles. |
| **Owner System** | Region System |
| **Lifetime** | Static. Loaded from config at world initialization. |
| **Unique ID** | `RegionId` — integer enum (1 = starting region, 2+ = DLC regions). |
| **Creation Rules** | Defined in configuration TOML. Cannot be created at runtime. |
| **Destruction Rules** | Never destroyed. |
| **Persistence** | Config only (not serialized in save). Save references RegionId. |

#### Loan

| Attribute | Detail |
|-----------|--------|
| **Purpose** | A financial obligation: borrowed money with repayment terms. |
| **Owner System** | Finance System |
| **Lifetime** | Issued → Repaid/Defaulted. |
| **Unique ID** | `LoanId` — sequential per company. |
| **Creation Rules** | Created when player accepts loan offer from Bank. |
| **Destruction Rules** | Archived on full repayment or default resolution. |
| **Persistence** | Full serialization. Historical loans retained for credit history. |

#### Blueprint

| Attribute | Detail |
|-----------|--------|
| **Purpose** | A saved building design reusable across saves and shareable with community. |
| **Owner System** | Building System |
| **Lifetime** | Saved → Deleted by player. Account-wide (not save-specific). |
| **Unique ID** | `BlueprintId` — UUID v4 + creator hash. |
| **Creation Rules** | Created when player clicks "Save as Blueprint" from design mode. |
| **Destruction Rules** | Deleted by player. Archived if shared and in use by community. |
| **Persistence** | Account-wide storage. Separate from save files. |

---

## SECTION 2 — ECS COMPONENTS

### 2.1 Component Catalog

**Total components: 87**

### 2.2 Identity Components

#### C_Identity (required on ALL entities)

| Field | Type | Description |
|-------|------|-------------|
| EntityId | UUID (16 bytes) | Globally unique identifier |
| EntityType | Enum (1 byte) | Type tag for debugging/recovery |
| DisplayName | StringId (4 bytes) | Localization key |
| CreatedAt | Timestamp (8 bytes) | In-game time of creation |
| CreatedInVersion | uint32 (4 bytes) | Game version that created this entity |

**Total: ~33 bytes.** Applied to every entity.

**Owner System:** ECS Core
**Update Frequency:** Never (set at creation)
**Memory:** Small, constant per entity.

#### C_Transform

| Field | Type | Description |
|-------|------|-------------|
| Position | Vector3 (12 bytes) | World position (double precision for large worlds) |
| Rotation | Quaternion (16 bytes) | World rotation |
| Scale | Vector3 (12 bytes) | Uniform/non-uniform scale |
| ParentEntity | EntityId (16 bytes, nullable) | Parent transform reference |

**Total: 40 bytes (+16 if parented).**

**Owner System:** ECS Core
**Update Frequency:** Per frame for moving entities; never for static.
**Memory:** Required on ~40% of entities (buildings, vehicles, workers, placed objects).

#### C_Ownership

| Field | Type | Description |
|-------|------|-------------|
| OwnerId | EntityId (16 bytes) | Owning entity (Company, Client, etc.) |
| OwnershipType | Enum (1 byte) | Owned, Leased, Contracted, Public |

**Total: 17 bytes.**

**Owner System:** Applicable domain system.
**Update Frequency:** Rarely (on transfer/ sale).

### 2.3 Construction Components

#### C_ConstructionState

| Field | Type | Description |
|-------|------|-------------|
| CurrentPhase | uint8 (1 byte) | Phase index (1-8) |
| PhaseProgress | float (4 bytes) | 0.0 to 1.0 within current phase |
| OverallProgress | float (4 bytes) | 0.0 to 1.0 total project |
| QualityScore | float (4 bytes) | Current quality (0.0 to 100.0) |
| ScheduledStart | Timestamp (8 bytes) | Planned start |
| ScheduledEnd | Timestamp (8 bytes) | Planned completion |
| ActualStart | Timestamp (8 bytes) | When construction actually began |
| IsPaused | bool (1 byte) | Construction halted flag |
| PauseReason | Enum (1 byte) | Weather, Permit, Client, etc. |

**Total: 31 bytes.**

**Owner System:** Construction System
**Update Frequency:** Every 5 simulation ticks (~200ms at 1x).
**Memory:** Per active construction site (~5-15 active, ~465 bytes max).

#### C_BuildingDesign

| Field | Type | Description |
|-------|------|-------------|
| DesignVersion | uint32 (4 bytes) | Incremented on each design change |
| FloorCount | uint8 (1 byte) | Number of floors |
| TotalArea | float (4 bytes) | Square meters |
| FootprintArea | float (4 bytes) | Ground floor area |
| MaxHeight | float (4 bytes) | Meters above ground |
| BuildingType | Enum (1 byte) | POP_UP_28, MULTIFAMILY, SIGNATURE, etc. |
| MaterialTier | Enum (1 byte) | Budget, Standard, Premium |
| HasBasement | bool (1 byte) | |
| RoofType | Enum (1 byte) | Flat, Gable, Hip, etc. |
| StructuralSystem | Enum (1 byte) | Timber, Steel, Concrete, Hybrid |
| KfWRating | uint8 (1 byte) | Energy efficiency rating |

**Total: ~20 bytes (header) + arrays for walls, rooms, MEP.**

**Owner System:** Building System
**Update Frequency:** During design; read-only during construction.
**Memory:** Per building (large — wall/room arrays are the dominant cost). See Section 9.

#### C_Wall

| Field | Type | Description |
|-------|------|-------------|
| StartPoint | Vector2 (8 bytes) | 2D position on floor plan |
| EndPoint | Vector2 (8 bytes) | 2D position on floor plan |
| Thickness | float (4 bytes) | cm |
| Height | float (4 bytes) | cm |
| WallType | Enum (1 byte) | Interior, Exterior, LoadBearing, FireWall |
| MaterialId | uint32 (4 bytes) | Reference to material catalog |
| FinishId | uint32 (4 bytes) | Surface finish reference |
| FloorIndex | uint8 (1 byte) | Which floor this wall belongs to |

**Total: ~34 bytes per wall.**

**Owner System:** Building System
**Update Frequency:** During design; rarely during construction (change orders).
**Memory:** ~100 walls per small house = 3.4 KB. ~5,000 walls per skyscraper = 170 KB.

#### C_Room

| Field | Type | Description |
|-------|------|-------------|
| RoomType | Enum (1 byte) | Bedroom, Kitchen, Bathroom, etc. |
| Area | float (4 bytes) | Computed m² |
| FloorIndex | uint8 (1 byte) | |
| WallIds | [EntityId] | References to bounding walls (computed) |
| FlooringId | uint32 (4 bytes) | Material reference |
| CeilingFinishId | uint32 (4 bytes) | |
| NaturalLightScore | float (4 bytes) | Computed from windows |

**Total: ~20 bytes + wall references.**

### 2.4 Worker Components

#### C_WorkerStats

| Field | Type | Description |
|-------|------|-------------|
| Role | Enum (1 byte) | Laborer, Electrician, Plumber, etc. |
| SkillLevel | float (4 bytes) | 0.0 to 100.0 |
| Experience | float (4 bytes) | Accumulated XP |
| Fatigue | float (4 bytes) | 0.0 (rested) to 100.0 (exhausted) |
| Morale | float (4 bytes) | 0.0 to 100.0 |
| Productivity | float (4 bytes) | Computed efficiency modifier |
| HourlyWage | float (4 bytes) | Current pay rate |
| HireDate | Timestamp (8 bytes) | |
| TrainingProgress | float (4 bytes) | If in training |

**Total: ~37 bytes.**

**Owner System:** Worker System
**Update Frequency:** Per in-game hour.

#### C_Assignment

| Field | Type | Description |
|-------|------|-------------|
| AssignedSiteId | EntityId (16 bytes) | Current construction site |
| AssignedTaskId | EntityId (16 bytes) | Current specific task |
| AssignedCrewId | EntityId (16 bytes) | Crew membership |
| ShiftStart | Timestamp (8 bytes) | |
| ShiftEnd | Timestamp (8 bytes) | |

**Total: ~48 bytes.**

**Owner System:** Worker System
**Update Frequency:** On assignment change.

### 2.5 Economy Components

#### C_Financials (on PlayerCompany)

| Field | Type | Description |
|-------|------|-------------|
| CashOnHand | double (8 bytes) | Current bank balance |
| RevenueYTD | double (8 bytes) | This fiscal year |
| ExpensesYTD | double (8 bytes) | This fiscal year |
| AssetsValue | double (8 bytes) | Total owned assets |
| LiabilitiesTotal | double (8 bytes) | Outstanding debts |
| CreditRating | uint16 (2 bytes) | 300-850 |
| TaxRate | float (4 bytes) | Current effective rate |
| InsurancePremium | float (4 bytes) | Monthly |

**Total: ~50 bytes.**

#### C_MarketPrice

| Field | Type | Description |
|-------|------|-------------|
| MaterialId | uint32 (4 bytes) | |
| RegionId | uint8 (1 byte) | |
| CurrentPrice | float (4 bytes) | Per unit |
| BasePrice | float (4 bytes) | Unmodified reference price |
| SupplyLevel | float (4 bytes) | 0.0 to 1.0 |
| DemandLevel | float (4 bytes) | 0.0 to 1.0 |
| LastUpdated | Timestamp (8 bytes) | |

**Total: ~25 bytes per material per region.**

### 2.6 Time Components

#### C_TimeState (singleton)

| Field | Type | Description |
|-------|------|-------------|
| CurrentTime | Timestamp (8 bytes) | Absolute game time |
| TimeScale | float (4 bytes) | Current speed multiplier |
| IsPaused | bool (1 byte) | |
| DayOfYear | uint16 (2 bytes) | 1-365 |
| Year | uint16 (2 bytes) | |
| Season | Enum (1 byte) | |

**Total: ~18 bytes.**

### 2.7 Weather Components

#### C_WeatherState (per region)

| Field | Type | Description |
|-------|------|-------------|
| RegionId | uint8 (1 byte) | |
| CurrentCondition | Enum (1 byte) | Clear, Rain, Storm, Snow, etc. |
| Temperature | float (4 bytes) | Celsius |
| WindSpeed | float (4 bytes) | km/h |
| Precipitation | float (4 bytes) | mm/hour |
| CloudCover | float (4 bytes) | 0.0 to 1.0 |
| Forecast7Day | 7 × WeatherSnapshot | Array of 7 daily forecasts |

**Total: ~80 bytes per region (with forecast).**

### 2.8 Reputation Components

#### C_Reputation (on PlayerCompany, per region)

| Field | Type | Description |
|-------|------|-------------|
| RegionId | uint8 (1 byte) | |
| QualityScore | float (4 bytes) | 0-100 |
| ReliabilityScore | float (4 bytes) | 0-100 |
| InnovationScore | float (4 bytes) | 0-100 |
| CommunityScore | float (4 bytes) | 0-100 |
| SafetyScore | float (4 bytes) | 0-100 |
| OverallRating | float (4 bytes) | Weighted average |
| ProjectCount | uint32 (4 bytes) | Projects completed in region |
| LastUpdated | Timestamp (8 bytes) | |

**Total: ~33 bytes per region.**

### 2.9 Save/Load Components

#### C_DirtyFlag (on entities that change)

| Field | Type | Description |
|-------|------|-------------|
| IsDirty | bool (1 byte) | Changed since last save |
| LastSavedTick | uint64 (8 bytes) | Simulation tick of last persistence |

**Total: 9 bytes.** Enables incremental saves.

#### C_SaveMetadata (on SaveProfile entity)

| Field | Type | Description |
|-------|------|-------------|
| SaveVersion | uint32 (4 bytes) | Schema version |
| GameVersion | string (variable) | Game build version |
| PlayTime | double (8 bytes) | Total playtime in seconds |
| SaveTimestamp | Timestamp (8 bytes) | Real-world time of save |
| CompanyName | StringId (4 bytes) | |
| ThumbnailAssetId | uint32 (4 bytes) | Screenshot reference |
| RegionCount | uint8 (1 byte) | |
| TotalBuildings | uint32 (4 bytes) | |
| IsAutoSave | bool (1 byte) | |

---

## SECTION 3 — RELATIONSHIPS

### 3.1 Entity Relationship Diagram (Core)

```
┌─────────────────────────────────────────────────────────────────────────┐
│                          PLAYER COMPANY (1)                             │
│  owns ─────────────────────────────────────────────────────────────┐   │
│  │                                                                  │   │
│  ├── 1:N ── Employee (N)                                           │   │
│  │           ├── subtype: Worker                                    │   │
│  │           ├── subtype: Architect                                 │   │
│  │           ├── subtype: Engineer                                  │   │
│  │           ├── subtype: ProjectManager                            │   │
│  │           └── subtype: Executive                                 │   │
│  │                                                                  │   │
│  ├── 1:N ── Crew (N)                                               │   │
│  │           └── N:M ── Worker (workers belong to crews)            │   │
│  │                                                                  │   │
│  ├── 1:N ── Contract (N) ── N:1 ── Client (N)                      │   │
│  │           │                                                      │   │
│  │           └── 1:1 ── Building (N)                                │   │
│  │                       │                                          │   │
│  │                       ├── 1:1 ── Foundation                      │   │
│  │                       ├── 1:1 ── Roof                            │   │
│  │                       ├── 1:N ── Wall                            │   │
│  │                       │           └── N:1 ── Room (boundary)     │   │
│  │                       ├── 1:N ── Room                            │   │
│  │                       ├── 1:N ── Door                            │   │
│  │                       ├── 1:N ── Window                          │   │
│  │                       ├── 1:1 ── MEPSystem                       │   │
│  │                       └── 1:1 ── ConstructionSite (during build) │   │
│  │                                   │                              │   │
│  │                                   ├── 1:N ── ConstructionPhase   │   │
│  │                                   │           └── 1:N ── Task   │   │
│  │                                   ├── N:M ── Worker (assigned)   │   │
│  │                                   ├── N:M ── Vehicle (on-site)   │   │
│  │                                   └── 1:N ── MaterialStockpile   │   │
│  │                                                                  │   │
│  ├── 1:N ── Vehicle (N)                                            │   │
│  ├── 1:N ── Equipment (N)                                          │   │
│  ├── 1:N ── Loan (N) ── N:1 ── Bank (N)                            │   │
│  ├── 1:N ── Invoice (N) ── N:1 ── Supplier (N)                     │   │
│  ├── 1:N ── MarketingCampaign (N)                                  │   │
│  ├── 1:N ── CompanyPolicy (N)                                      │   │
│  ├── 1:N ── Permit (N)                                             │   │
│  ├── 1:N ── Blueprint (N)  (account-wide, not save-specific)       │   │
│  └── 1:N ── Achievement (N)                                        │   │
│                                                                     │   │
│  has ────────────────────────────────────────────────────────────┐  │   │
│  │                                                                 │  │   │
│  ├── 1:1 ── C_Financials                                           │  │   │
│  ├── 1:N ── C_Reputation (per region)                              │  │   │
│  └── 1:1 ── C_TechnologyTree                                       │  │   │
└────────────────────────────────────────────────────────────────────┘  │   │
                                                                         │   │
┌────────────────────────────────────────────────────────────────────┐   │   │
│                          WORLD (Singleton)                          │   │   │
│  contains ──────────────────────────────────────────────────────┐  │   │   │
│  │                                                               │  │   │   │
│  ├── 1:1 ── TimeController                                      │  │   │   │
│  ├── 1:1 ── Calendar                                            │  │   │   │
│  ├── 1:1 ── EconomyState (per region)                            │  │   │   │
│  ├── 1:N ── Region                                             │  │   │   │
│  │           ├── 1:N ── City                                     │  │   │   │
│  │           │           └── 1:N ── District                     │  │   │   │
│  │           │                       └── 1:N ── Parcel           │  │   │   │
│  │           │                                   └── 0:1 ── Building│   │   │
│  │           ├── 1:1 ── WeatherState                             │  │   │   │
│  │           └── N:M ── Supplier                                  │  │   │   │
│  └── 1:N ── Notification                                        │  │   │   │
└──────────────────────────────────────────────────────────────────┘  │   │   │
└────────────────────────────────────────────────────────────────────┘   │   │
                                                                         │   │
┌──────────────────────────────────────────────────────────────────┐     │   │
│                       SAVE PROFILE (1 per save slot)              │     │   │
│  └── references ── PlayerCompany + World entities                │     │   │
└──────────────────────────────────────────────────────────────────┘     │   │
└────────────────────────────────────────────────────────────────────────┘   │
└──────────────────────────────────────────────────────────────────────────┘
```

### 3.2 Relationship Types Summary

| Relationship | Type | From | To |
|-------------|------|------|-----|
| Company owns Employees | 1:N, composition | PlayerCompany | Employee |
| Employee belongs to Crew | N:M, aggregation | Employee | Crew |
| Contract awarded to Company | N:1 | Contract | PlayerCompany |
| Contract for Building | 1:1 | Contract | Building |
| Building contains Walls | 1:N, composition | Building | Wall |
| Building contains Rooms | 1:N, composition | Building | Room |
| Building on Parcel | 1:1 | Building | Parcel |
| Building has ConstructionSite | 1:1 (during build) | Building | ConstructionSite |
| ConstructionSite assigns Workers | N:M | ConstructionSite | Worker |
| ConstructionSite uses Vehicles | N:M | ConstructionSite | Vehicle |
| Region contains Cities | 1:N, composition | Region | City |
| City contains Districts | 1:N, composition | City | District |
| District contains Parcels | 1:N, composition | District | Parcel |
| PlayerCompany takes Loans | 1:N | PlayerCompany | Loan |
| Loan from Bank | N:1 | Loan | Bank |
| PlayerCompany buys from Supplier | N:M (via Invoice) | PlayerCompany | Supplier |
| Building references Blueprint | N:1 (optional) | Building | Blueprint |

---

## SECTION 4 — IDENTIFIERS

### 4.1 ID Architecture

All entity identifiers use a unified 16-byte format:

```
┌────────────────────────────────────────────────────────────────┐
│                      ENTITY ID (128 bits)                       │
├──────────┬──────────┬──────────┬───────────────────────────────┤
│ Type Tag │ Region   │ Timestamp│ Random / Sequential            │
│ 8 bits   │ 8 bits   │ 32 bits  │ 80 bits                        │
└──────────┴──────────┴──────────┴───────────────────────────────┘
```

| Field | Bits | Purpose |
|-------|------|---------|
| Type Tag | 8 | Entity type enum (0-255). Enables type-safe ID comparison and debugging. |
| Region | 8 | 0 = global, 1-255 = region-scoped. Enables sharding. |
| Timestamp | 32 | In-game tick when entity was created. Enables chronological sorting. |
| Random | 80 | UUID v4 randomness. Collision probability: negligible (2^80 space). |

### 4.2 ID Generation Rules

- EntityId generated by ECS Core at entity creation.
- Uses thread-local RNG seeded from secure entropy source + tick counter.
- No central ID authority (avoids bottleneck).
- IDs are immutable for entity lifetime.
- Archived entities retain their original ID.

### 4.3 ID Collision Prevention

- 80 bits of randomness = 1.2 × 10^24 possible values.
- At 1 million entities created per playthrough, collision probability is ~10^-12 (effectively zero).
- Debug builds verify uniqueness on creation (hash set check, disabled in shipping).

### 4.4 ID Migration

- IDs are opaque. No system parses ID structure (type tag is for debugging only).
- Save migration preserves IDs.
- If an entity is deleted and recreated, it gets a NEW ID (even if "same" entity conceptually).

### 4.5 Special ID Types

| ID Type | Format | Scope |
|---------|--------|-------|
| EntityId | 128-bit UUID | Global |
| BlueprintId | 128-bit UUID + creator hash prefix | Account-wide |
| ConfigId | uint32 sequential | Per-config file |
| AssetId | uint64 (path hash) | Global |
| StringId | uint32 (string table index) | Per-locale |
| SaveSlotId | uint8 (0-9) | Local machine |

---

## SECTION 5 — DATA LIFECYCLE

### 5.1 State Machine for All Entities

```
                    ┌──────────┐
                    │  CREATED │  ← Entity allocated, components attached
                    └────┬─────┘
                         │
                    ┌────▼─────┐
                    │INITIALIZED│ ← All required components set
                    └────┬─────┘
                         │
              ┌──────────┼──────────┐
              │          │          │
         ┌────▼───┐ ┌───▼────┐ ┌──▼──────┐
         │ ACTIVE │ │PAUSED  │ │DORMANT  │
         └────┬───┘ └───┬────┘ └──┬──────┘
              │          │          │
              └──────────┼──────────┘
                         │
              ┌──────────┼──────────┐
              │          │          │
         ┌────▼───┐ ┌───▼────┐ ┌──▼──────┐
         │ARCHIVED│ │MARKED  │ │DESTROYED│
         │(inactive│ │DELETE  │ │(removed)│
         │ history)│ │        │ │         │
         └────────┘ └───┬────┘ └─────────┘
                         │
                    ┌────▼─────┐
                    │DELETED   │ ← Memory freed. Save data removed.
                    └──────────┘
```

### 5.2 Lifecycle Per Entity Type

| Entity Type | ACTIVE Duration | ARCHIVED Policy |
|-------------|----------------|-----------------|
| Worker | Hire → Termination | Historical record kept |
| Building | Design start → Permanent | Never archived (permanent world fixture) |
| ConstructionSite | Build start → Completion | Archived with summary stats |
| Contract | Bid → Completion/Cancel | Archived with financial data |
| Vehicle | Purchase → Sale | Archived sale record |
| Loan | Issue → Repay/Default | Archived for credit history |
| Notification | Generation → Dismiss | Deleted after 30 in-game days |
| Task | Phase start → Task complete | Deleted (too granular to archive) |
| WeatherState | Continuous | Never archived (recomputed) |

### 5.3 Save/Load Lifecycle

```
SAVE:
  1. System Orchestrator sends PREPARE_SAVE to all systems
  2. Each system writes dirty entities to serialization buffer
  3. Save system captures: world metadata + entity data + system state
  4. Binary blob compressed (zstd) + checksummed
  5. Written atomically (temp file → rename)

LOAD:
  1. Read file, verify checksum
  2. Decompress
  3. Validate schema version → apply migrations if needed
  4. Create all entities from serialized data
  5. Restore system state (schedules, RNG seeds, etc.)
  6. Validate entity graph (no dangling references)
  7. Notify all systems: WORLD_RESTORED
  8. Resume simulation
```

---

## SECTION 6 — SERIALIZATION

### 6.1 What Gets Serialized

- All entities marked with C_DirtyFlag where IsDirty == true (incremental) or all entities (full save)
- All components on those entities
- System state (RNG seeds, event schedules, economy cycle position)
- Save metadata (version, timestamp, playtime, thumbnail)
- Player settings (camera bookmarks, UI layout, keybindings)

### 6.2 What NEVER Gets Serialized

- Presentation state (camera position — except bookmarks, UI context — except layout prefs)
- Transient components (C_AnimationState, C_PhysicsIntermediate, C_SelectionHighlight)
- Debug data (profiling markers, debug visualizations)
- Analytics buffers (flushed before save, not saved)
- Notification history beyond 30 in-game days
- Weather forecast (recomputed on load)
- Market price history beyond current values (recomputed)
- Audio state (restarted on load)

### 6.3 Serialization Format

```
┌────────────────────────────────────────────────┐
│              SAVE FILE STRUCTURE                │
├────────────────────────────────────────────────┤
│ HEADER                                          │
│  - Magic bytes: "IBSE" (InstaBuilt Save)       │
│  - Save version: uint32                         │
│  - Game version: string                         │
│  - Save type: uint8 (Manual/Auto/Quick)         │
│  - Timestamp: int64                             │
│  - Compressed size: uint64                      │
│  - Uncompressed size: uint64                    │
│  - Checksum: SHA-256 (32 bytes)                 │
├────────────────────────────────────────────────┤
│ SAVE METADATA SECTION                           │
│  - C_SaveMetadata blob                          │
├────────────────────────────────────────────────┤
│ ENTITY DATA SECTION                             │
│  - Entity count: uint32                         │
│  - For each entity:                             │
│    - EntityId (16 bytes)                        │
│    - Component count: uint16                    │
│    - For each component:                        │
│      - ComponentTypeId: uint16                  │
│      - Component size: uint16                   │
│      - Component data: variable                 │
├────────────────────────────────────────────────┤
│ SYSTEM STATE SECTION                            │
│  - System count: uint16                         │
│  - For each system:                             │
│    - SystemId: uint16                           │
│    - State size: uint32                         │
│    - State data: variable                       │
├────────────────────────────────────────────────┤
│ TRAILER                                         │
│  - Section count: uint16                        │
│  - End magic: "IBSE" (reversed)                 │
└────────────────────────────────────────────────┘
```

### 6.4 Reference Handling

- Entity references stored as EntityId.
- On load, EntityId → pointer resolution happens after all entities are deserialized.
- Dangling references (entity was deleted between save and load) are detected during validation. Nulled with warning.
- Config references stored as ConfigId. Resolved against current config on load (config may have changed between versions).

### 6.5 Versioning & Migration

- SaveVersion incremented when schema changes.
- Migration functions: `Migrate_vN_to_vN+1(world)` — transforms entities/components.
- Migration chain applied sequentially: v3 → v4 → v5 → current.
- If migration gap is too large (>10 versions), offer player option to load with warning.
- Unrecognized component types are preserved as opaque blobs (forward compatibility).

### 6.6 Compression

- zstd compression level 3 (balance speed/size).
- Typical 100-hour save: ~80MB uncompressed → ~15MB compressed.
- Completed building geometry stored as delta-compressed mesh data.
- String table deduplicated before compression.

### 6.7 Integrity Validation

- SHA-256 checksum over compressed data.
- Magic bytes at start and end (detect truncation).
- Entity count consistency check.
- Entity ID uniqueness verification on load.

---

## SECTION 7 — DATA VALIDATION

### 7.1 Validation Rules Catalog

| # | Rule | Enforcement | Recovery |
|---|------|------------|----------|
| V1 | EntityId uniqueness | Debug: at creation. Load: post-deserialize. | If duplicate detected on load: regenerate one ID, log warning. |
| V2 | No dangling EntityId references | On load, after resolution phase | Null dangling reference, log warning, notify player: "Save repaired." |
| V3 | Building must have Foundation | Design validation (GSS Step 13) | Reject design until foundation added. |
| V4 | Room must have ≥1 access door | Design validation | Warning; player may override. |
| V5 | CashOnHand ≥ 0 | On every financial transaction | Block transaction that would cause negative (except loans). If negative via bug: emergency loan auto-triggered. |
| V6 | Worker cannot be assigned to >1 site | On assignment | Reject duplicate assignment. |
| V7 | Contract must have valid Client reference | Contract generation | Cannot generate contract without client. |
| V8 | Building cannot exceed parcel boundaries | Design validation | Reject placement. |
| V9 | Save file checksum must match | On load | Refuse load. Offer previous save. |
| V10 | RNG seed must be non-zero | On system init | Re-seed from entropy. |
| V11 | Construction phase order | Phase transition | Cannot skip phases. Must complete gated phases before next. |
| V12 | Loan repayment ≤ remaining balance | On payment | Clamp payment to remaining balance. |
| V13 | Weather state valid for region climate | Weather update | Clamp to region-legal values. |
| V14 | Reputation scores 0-100 | On calculation | Clamp to range. |

### 7.2 Automatic Recovery

- **Corrupted component:** If a single component fails to deserialize, entity loads without it. Component re-initialized to defaults. Warning logged.
- **Missing system state:** If a system's state section is missing from save, system initializes to defaults. Warning.
- **Duplicate IDs at creation:** Extremely unlikely (Section 4.3). If detected, retry with new random ID (max 3 retries). If still fails, crash with diagnostic (hardware entropy failure).

---

## SECTION 8 — MODDING DATA

### 8.1 Data Classification

| Category | Files | Moddable | Safety |
|----------|-------|----------|--------|
| **Core Data** | Engine constants, ECS type registry | NO | Game stability |
| **Game Content** | Building defs, materials, contracts, regions, vehicles | YES | Sandboxed, validated |
| **Configuration** | Balance values, difficulty, economy params | YES | Merged with base |
| **Localization** | PO string tables | YES | Add new languages |
| **Assets** | 3D models, textures, audio | YES (references only) | Mod provides assets |
| **Scripts** | Lua behavior extensions (post-launch) | YES | Sandboxed VM |
| **Save Data** | Player saves | NO | Player data integrity |

### 8.2 Mod Data Merge Rules

1. Mod provides override TOML files in `Mods/ModName/Config/`.
2. Mod Loader merges overrides on top of base config (Layer 5 over Layer 2 per Architecture doc).
3. Array values: mod can append or replace (declared in mod metadata).
4. Conflicting mods: both mods' values presented to player in Mod Manager. Player chooses winner per conflict.
5. Mod-added entities get IDs with mod namespace prefix (high bit of region field).

---

## SECTION 9 — MEMORY STRATEGY

### 9.1 Memory Budgets

| Scenario | Entities | Estimated Memory | Target |
|----------|----------|-----------------|--------|
| 100 workers, 10 sites, 50 buildings | ~500 entities | ~8 MB | <16 MB |
| 500 workers, 25 sites, 200 buildings | ~2,000 entities | ~30 MB | <64 MB |
| 1,000 workers, 50 sites, 1,000 buildings | ~5,000 entities | ~80 MB | <128 MB |
| 10,000 workers, 200 sites, 10,000 buildings | ~30,000 entities | ~500 MB | <1 GB |

### 9.2 Per-Component Size Estimates

| Component | Size (bytes) | Entities with this component | Total at 1,000 buildings |
|-----------|-------------|------------------------------|--------------------------|
| C_Identity | 33 | All (~30K) | ~1 MB |
| C_Transform | 40 | ~12K | ~0.5 MB |
| C_Wall | 34 | ~500K walls (avg 500/building) | ~17 MB |
| C_Room | 20 | ~200K rooms | ~4 MB |
| C_Door | 16 | ~200K doors | ~3.2 MB |
| C_Window | 16 | ~300K windows | ~4.8 MB |
| C_WorkerStats | 37 | 1,000 | ~37 KB |
| C_Financials | 50 | 1 (singleton) | ~50 bytes |
| C_Reputation | 33 | ~10 (regions) | ~330 bytes |
| C_WeatherState | 80 | ~10 (regions) | ~800 bytes |

**Dominant cost:** Wall components in large cities. 10,000 buildings × 500 walls × 34 bytes = ~170 MB for walls alone.

### 9.3 Optimization Strategies

1. **SoA (Struct of Arrays) layout:** Wall components stored as parallel arrays (all StartPoints contiguous, all EndPoints contiguous). Cache-friendly for batch operations (structural validation iterates all walls).

2. **Completed building compression:** After construction completes, wall/room/door/window data is compressed (delta encoding + zstd) and stored as a blob. The full component structure is destroyed; only the compressed blob + C_Transform + metadata remains.

3. **Simulation LOD:** Buildings >500m from camera are not fully simulated. Their components are unloaded, only C_Transform + C_BuildingSummary (32 bytes: type, height, footprint, completion status) remain.

4. **Object pooling:** Transient entities (tasks, notifications) are pooled. Entity slots are reused rather than allocated/deallocated.

5. **String interning:** All entity names and material labels share a string table. Duplicate strings stored once.

---

## SECTION 10 — ENTITY RELATIONSHIP DIAGRAMS

### 10.1 Building Hierarchy

```
Building (1)
├── Foundation (1)
├── MEPSystem (1)
├── Roof (1)
├── Floor[0..N]
│   ├── Wall[0..N]
│   ├── Room[0..N]
│   │   ├── Door[0..N]  (connects rooms)
│   │   └── Window[0..N]
│   └── Stair[0..N]  (connects floors)
└── ConstructionSite (0..1)  (only during construction)
```

### 10.2 Company Organization

```
PlayerCompany (1)
├── Department: Operations
│   ├── Crew[0..N]
│   │   └── Worker[3..10] (via C_Assignment)
│   ├── Vehicle[0..N]
│   └── Equipment[0..N]
├── Department: Projects
│   ├── Contract[0..N]
│   │   └── Building[1] (via contract)
│   └── Blueprint[0..N] (account-wide)
├── Department: Finance
│   ├── Loan[0..N]
│   ├── Invoice[0..N]
│   └── Bank[0..N] (relationships)
├── Department: Administration
│   ├── CompanyPolicy[0..N]
│   ├── MarketingCampaign[0..N]
│   └── Permit[0..N]
└── Specialists
    ├── Architect[0..N]
    ├── Engineer[0..N]
    └── ProjectManager[0..N]
```

### 10.3 World Geography

```
World
└── Region[1..N]
    ├── WeatherState[1]
    ├── EconomyState[1]
    ├── ReputationProfile[1] (player's rep in this region)
    └── City[1..N]
        └── District[1..N]
            └── Parcel[1..N]
                ├── Building[0..1] (if developed)
                └── ZoningRestrictions[1]
```

### 10.4 Contract Lifecycle Relationships

```
ContractTemplate (Config) ──generates──→ Contract[1]
                                            │
                          ┌─────────────────┼─────────────────┐
                          │                 │                 │
                     Client[1]       Building[1]        PlayerCompany[1]
                     (requester)     (deliverable)      (contractor)
                          │                 │
                          │            ConstructionSite[1]
                          │                 │
                          │      ┌──────────┼──────────┐
                          │      │          │          │
                          │  Worker[N]  Vehicle[N]  Material[N]
                          │  (assigned) (on-site)   (consumed)
                          │
                     Invoice[N]
                     (payments)
```

---

## SECTION 11 — DESIGN DECISIONS

### DD-001: EntityId as 128-bit UUID

**Decision:** All entity identifiers are 128-bit UUIDs with embedded type tag.

**Alternatives:** 32-bit sequential IDs, 64-bit IDs, string-based IDs.

**Reasoning:** 128 bits eliminates collision risk across saves, mods, and future multiplayer. Embedded type tag enables fast type filtering without a lookup. Opaque IDs prevent systems from encoding meaning in ID structure.

**Trade-off:** 16 bytes per reference vs 4 bytes for uint32. Acceptable given memory budgets.

### DD-002: SoA Component Storage

**Decision:** Components of the same type stored in Struct-of-Arrays layout, not Array-of-Structs.

**Alternatives:** AoS (standard OOP), columnar database approach.

**Reasoning:** Construction simulation processes walls in bulk (validate all walls, compute all room areas). SoA keeps the working set in cache. Systems that read only StartPoint don't load EndPoint, Thickness, etc. into cache.

**Trade-off:** Adding a new field to a component requires adding a new array. Slightly more complex code. Worth it for simulation performance.

### DD-003: Completed Building Compression

**Decision:** After construction completes, building component data is compressed and archived. The building transitions from "active simulation entity" to "static world entity."

**Alternatives:** Keep all buildings fully simulated forever. Stream buildings in/out.

**Reasoning:** With 10,000+ buildings in a long playthrough, simulating every wall of every building is wasteful. Completed buildings don't change. Compressing them to <1KB average frees memory for active construction.

**Trade-off:** Player cannot edit completed buildings without "unarchiving" them (possible feature: renovation contracts that reactivate a building).

### DD-004: Transient vs Persistent Components

**Decision:** Components are tagged Transient or Persistent. Transient components are never serialized.

**Alternatives:** Serialize everything. Manual opt-out per component.

**Reasoning:** Many components hold frame-level state (animation progress, physics velocities, selection highlights). Serializing these is wasteful and can cause visual glitches on load (character frozen mid-animation). The Transient tag is declarative and checked by the Save System automatically.

### DD-005: Save File as Single Binary Blob

**Decision:** Save data is a single compressed binary file, not a directory of files.

**Alternatives:** Multi-file saves (one per system), SQLite database, JSON directory.

**Reasoning:** Single file is easy for players to manage (backup, share, cloud sync). Atomic write (temp + rename) prevents corruption. Binary is compact. Multi-file saves create consistency problems (what if one file is missing?).

**Trade-off:** Cannot partially load a save. Entire blob must be read. Acceptable; typical save <20MB loads in <1 second.

### DD-006: Config as Code (TOML), Not Database

**Decision:** All game configuration stored in TOML files in the repository, not in a database.

**Alternatives:** SQLite config database, JSON, custom binary config format.

**Reasoning:** TOML is human-readable, diff-friendly (git blame works), and supports comments. Designers can edit in any text editor. Modders can override with their own TOML files. Build pipeline validates TOML schema at compile time.

**Trade-off:** No transactional config updates. Not a problem — config is read at startup, not modified at runtime.

---

## DATA MODEL APPROVED

### Final Counts

| Metric | Count |
|--------|-------|
| Entity Types | 52 |
| ECS Components | 87 |
| Relationship Types | 28 (1:1), 19 (1:N), 8 (N:M) = 55 total |
| Validation Rules | 14 core + 8 per-system = 22 total |
| Future Extension Points | 7 |

### Extension Points

1. **New component types:** Register in ECS type registry. No existing code modified.
2. **New entity types:** Add EntityType enum value. Add creation rules to owning system.
3. **New relationship types:** Define in relationship registry. Validated by relationship rules engine.
4. **New validation rules:** Add to validation pipeline. Runs at design validation or load validation phase.
5. **New mod data categories:** Add to Mod Loader's allowed-categories list.
6. **New save sections:** Add section type enum + serializer. Save/Load system is section-extensible.
7. **New ID types:** Add to ID registry. Opaque to all systems.

---

**End of Data Model & Entity Relationship Specification — InstaBuilt: Blueprint Empire v1.0**
