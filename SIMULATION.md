# INSTABUILT: BLUEPRINT EMPIRE
## Simulation Systems Technical Design v1.0

**Document Type:** Simulation Systems Specification
**Audience:** Simulation Engineers, Gameplay Programmers, QA Engineers
**Prerequisites:** GDD (LOCKED), GSS (LOCKED), ARCHITECTURE.md (LOCKED), DATA_MODEL.md (LOCKED)

---

## SYSTEM CATALOG

Total systems: **28**

### Core Systems (5)
1. Time System
2. ECS Core System
3. Event Bus System
4. Command Processor System
5. Save/Load System

### World Systems (4)
6. Weather System
7. Region System
8. Economy System
9. Calendar System

### Construction Domain (5)
10. Building System
11. Construction System
12. Material System
13. Inventory System
14. Safety System

### Business Domain (6)
15. Contract System
16. Company System
17. Finance System
18. Worker System
19. Reputation System
20. Research System

### Presentation Bridge (3)
21. UI Bridge System
22. Camera Bridge System
23. Audio Bridge System

### Support (5)
24. Notification System
25. Analytics System
26. Achievement System
27. Tutorial System
28. Mod Runtime System

---

## DETAILED SYSTEM DESIGNS

---

### SYSTEM 1: TIME SYSTEM

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Central authority for game time. Drives all time-dependent systems. |
| **Responsibilities** | Time advancement at configurable speeds; pause/resume; calendar tracking; working hours enforcement; deadline scheduling; seasonal transitions. |
| **Owned Data** | C_TimeState (singleton), C_Calendar, event schedule queue |
| **Update Frequency** | Every frame (at current speed multiplier). Emits granularity-dependent events. |
| **Inputs** | Speed change commands from player; pause from UI, loading, or emergency; system registration for scheduled events. |
| **Outputs** | TimeTick (every frame), TimeHourTick, TimeDayTick, TimeWeekTick, TimeMonthTick, TimeSeasonTick, TimeYearTick. Current time data via polling. |
| **Dependencies** | None (leaf system). |
| **Events Emitted** | `TimeAdvanced(delta)`, `HourChanged(hour)`, `DayChanged(day, month, year)`, `WeekChanged(week)`, `MonthChanged(month)`, `SeasonChanged(season)`, `YearChanged(year)`, `WorkingHoursStarted`, `WorkingHoursEnded`, `WeekendStarted`, `WeekendEnded`, `TimeScaleChanged(newScale)`, `TimePaused`, `TimeResumed` |
| **Events Consumed** | None. |
| **Failure Cases** | Speed set to 0 (pause — normal). Negative delta (rejected, clamp to 0). Time overflow (128-bit timestamp, effectively never). |
| **Performance** | O(1) per frame. Scheduled event dispatch: O(log N) with priority queue. |
| **Scalability** | N/A (singleton system). |

**Update Logic:**

```
TimeSystem::Tick(deltaRealSeconds):
  if IsPaused: return
  
  deltaGameTime = deltaRealSeconds * TimeScale
  accumulatedTime += deltaGameTime
  
  while accumulatedTime >= TICK_INTERVAL:
    CurrentTime += TICK_INTERVAL
    accumulatedTime -= TICK_INTERVAL
    
    Emit(TimeAdvanced(TICK_INTERVAL))
    
    if newHour:   Emit(HourChanged), check working hours
    if newDay:    Emit(DayChanged), Emit(CalendarDayAdvanced)
    if newWeek:   Emit(WeekChanged)
    if newMonth:  Emit(MonthChanged)
    if newSeason: Emit(SeasonChanged)
    if newYear:   Emit(YearChanged)
    
    ProcessScheduledEvents(currentTime)
```

---

### SYSTEM 2: WEATHER SYSTEM

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Generates region-specific weather; affects construction and visual presentation. |
| **Responsibilities** | Weather generation from climate profiles; 7-day forecast; weather event triggering; construction impact calculation. |
| **Owned Data** | C_WeatherState per region, WeatherProfile per region (config reference) |
| **Update Frequency** | Per in-game hour. Forecast recalculated per in-game day. |
| **Inputs** | TimeHourTick, RegionId (for per-region update), climate profile from config. |
| **Outputs** | Current weather per region (pollable), 7-day forecast (pollable), weather impact multipliers (for Construction System). |
| **Dependencies** | Time System (events), Region System (climate profiles). |
| **Events Emitted** | `WeatherChanged(regionId, oldCondition, newCondition)`, `WeatherWarning(regionId, warningType, severity)`, `StormStarted(regionId)`, `StormEnded(regionId)`, `ExtremeTemperature(regionId, temp, type)` |
| **Events Consumed** | `HourChanged` — triggers weather recalculation. `SeasonChanged` — shifts climate baseline. |
| **Failure Cases** | Climate profile missing for region → use temperate default with warning. Weather state corruption → reinitialize from climate profile. |
| **Performance** | O(R) per hour where R = active regions (~10-50). Negligible. |
| **Scalability** | Per-region weather is independent. Fully parallelizable. |

**Weather Generation Algorithm (per region, per hour):**

```
1. Get climate baseline from Region's WeatherProfile:
   - BaseTemperature(season)
   - BaseHumidity(season)
   - PrevailingWind(season)
   - StormFrequency(season)

2. Apply Markov chain transition from previous hour's condition:
   Clear → (90%) Clear, (8%) Cloudy, (2%) LightRain
   Cloudy → (60%) Cloudy, (20%) Clear, (15%) LightRain, (5%) HeavyRain
   LightRain → (50%) LightRain, (25%) Cloudy, (15%) HeavyRain, (10%) Clear
   HeavyRain → (40%) HeavyRain, (30%) LightRain, (20%) Cloudy, (10%) Storm
   Storm → (60%) HeavyRain, (30%) Storm, (10%) LightRain
   Snow → (80%) Snow, (15%) Cloudy, (5%) Clear

3. Apply seasonal modifiers to transition probabilities.

4. Calculate temperature:
   CurrentTemp = BaseTemp + Random(-5, +5)
   Apply time-of-day curve (cooler at night, warmer mid-day)

5. Calculate wind: PrevailingWind + Random(-20, +20) km/h

6. Generate construction impact multipliers:
   OutdoorWorkMultiplier: 0.0 (stopped) to 1.0 (full speed)
   QualityMultiplier: 0.85 (adverse) to 1.05 (ideal)
   FatigueMultiplier: 1.0 to 1.3 (extreme heat/cold)
```

---

### SYSTEM 3: ECONOMY SYSTEM

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Simulates regional and global economy: material prices, labor market, interest rates, property values. |
| **Responsibilities** | Price fluctuation per material per region; inflation; labor market dynamics; interest rates; economic cycle (boom/bust/recovery); competitor pricing. |
| **Owned Data** | EconomyState per region, MarketPrice entities per material per region. |
| **Update Frequency** | Per in-game day. |
| **Inputs** | TimeDayTick, RegionId, player actions (large purchases affect local demand). |
| **Outputs** | Current prices (pollable by Contract, Construction, UI), economic indicators. |
| **Dependencies** | Time System, Region System. |
| **Events Emitted** | `PricesUpdated(regionId, materials[])`, `EconomicEvent(regionId, eventType)`, `RecessionStarted(regionId)`, `RecessionEnded(regionId)`, `BoomStarted(regionId)`, `InterestRateChanged(regionId, newRate)` |
| **Events Consumed** | `DayChanged` — triggers daily price update. `PlayerLargePurchase(material, quantity)` — adjusts local demand. `SeasonChanged` — adjusts construction demand cycle. |
| **Failure Cases** | Price goes negative → clamp to minimum (material base cost). Inflation spiral → capped at 50% annual. |
| **Performance** | O(M × R) per day where M = materials (~200), R = regions (~10). ~2,000 operations. Negligible. |
| **Scalability** | Per-material per-region prices are independent. Fully parallelizable. |

**Price Model:**

```
CurrentPrice = BasePrice × SupplyDemandFactor × EconomicCycleFactor × SeasonFactor × RandomNoise

SupplyDemandFactor:
  Supply = BaseSupply + LocalProduction - Exports
  Demand = BaseDemand + ConstructionActivity + PlayerPurchases
  Factor = clamp(0.5, 1 + (Demand - Supply) / Supply, 2.0)

EconomicCycleFactor:
  Boom:    1.2
  Stable:  1.0
  Slowdown: 0.9
  Recession: 0.75

SeasonFactor:
  Spring: 1.05 (high construction activity)
  Summer: 1.0
  Autumn: 1.02
  Winter: 0.95 (reduced construction)
```

---

### SYSTEM 4: CONSTRUCTION SYSTEM

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Simulates construction progress on active building sites. |
| **Responsibilities** | Phase sequencing; task scheduling; crew productivity calculation; material consumption; equipment utilization; weather impact; problem generation; quality tracking. |
| **Owned Data** | ConstructionSite entities, ConstructionPhase entities, ConstructionTask entities (pooled). |
| **Update Frequency** | Every 5 simulation ticks (~200ms at 1x) for active sites. |
| **Inputs** | Building design data (from Building System); worker assignments; equipment assignments; material availability; weather conditions; time. |
| **Outputs** | Construction progress events; phase completion; quality scores; material consumption records; problem notifications. |
| **Dependencies** | Building System, Worker System, Equipment System, Inventory System, Weather System, Time System, Safety System. |
| **Events Emitted** | `ConstructionProgress(siteId, phaseProgress, overallProgress)`, `PhaseStarted(siteId, phase)`, `PhaseCompleted(siteId, phase)`, `ConstructionMilestone(siteId, milestone)`, `QualityUpdated(siteId, newScore)`, `MaterialConsumed(siteId, materialId, quantity)`, `ProblemDetected(siteId, problem)`, `ConstructionPaused(siteId, reason)`, `ConstructionResumed(siteId)`, `ConstructionComplete(siteId)`, `TaskAssigned(taskId, workerId)` |
| **Events Consumed** | `HourChanged` — progress tick (every 5th). `WeatherChanged` — recalculate productivity. `WorkerAssigned/Unassigned` — update crew composition. `MaterialDelivered` — update stockpile. `EquipmentAssigned` — update equipment availability. |
| **Failure Cases** | Phase order violation → rejected, log error. No workers assigned → progress = 0, emit warning. Materials exhausted → pause construction, emit `MaterialShortage`. Extreme weather → pause outdoor phases. Equipment breakdown → reassign tasks to manual or pause. Quality below threshold → flag for inspection failure risk. |
| **Performance** | O(S × W) per tick where S = active sites (~5-50), W = workers per site (~5-50). ~250-2,500 operations. Target: <2ms at 50 sites. |
| **Scalability** | Sites are independent. Update loop parallelized per site. Maximum active sites capped by company tier (Tier 6: ~50). |

**Construction Progress Formula:**

```
Per tick (for each active task):
  BaseProgress = TaskComplexity × DeltaTime
  
  Modifiers (multiplicative):
  × CrewProductivity  = f(worker skill, crew size, crew cohesion)
  × EquipmentBonus    = 1.0 (manual) to 2.5 (optimal equipment)
  × WeatherImpact     = 0.0 (stopped) to 1.0 (ideal)
  × FatiguePenalty    = 0.7 (exhausted) to 1.0 (rested)
  × MaterialQuality   = 0.9 (budget) to 1.1 (premium)
  × OvertimeBonus     = 1.0 (normal) to 1.3 (night work, but quality penalty)
  × AutomationLevel    = 1.0 (manual) to 1.5 (delegated, with PM bonus)
  
  TaskProgress += BaseProgress × Product(all modifiers)
  
  If TaskProgress >= 1.0:
    Task complete.
    Quality contribution = f(modifiers) → accumulated to phase quality.
```

---

### SYSTEM 5: BUILDING SYSTEM

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Manages building design, structural validation, and the building entity lifecycle. |
| **Responsibilities** | Design creation and editing; structural validation; building-to-renderable translation; blueprint save/load; building state management. |
| **Owned Data** | Building entities, Wall entities, Room entities, Door entities, Window entities, Foundation entities, Roof entities, MEPSystem entities, Blueprint entities. |
| **Update Frequency** | Design mode: per user action. Construction: read-only queries. Completed: dormant. |
| **Inputs** | Design commands (PlaceWall, MoveWall, AddRoom, SetMaterial, etc.); structural rules from config; material catalog. |
| **Outputs** | Building design data; validation results; renderable geometry (for Presentation Bus); blueprint data. |
| **Dependencies** | Material System (catalog), Region System (zoning constraints). |
| **Events Emitted** | `DesignChanged(buildingId)`, `DesignValidated(buildingId, issues[])`, `DesignApproved(buildingId)`, `DesignRejected(buildingId, reasons[])`, `BuildingStateChanged(buildingId, oldState, newState)`, `BlueprintSaved(blueprintId)`, `BlueprintLoaded(blueprintId)` |
| **Events Consumed** | `ConstructionComplete` — transition building to Completed state. |
| **Failure Cases** | Structural instability detected → reject design. Zoning violation → reject with specific error. Wall collision → snap or reject. Room without access → warning, not rejection. |

**Structural Validation Pipeline:**

```
ValidateBuilding(buildingId):
  issues = []
  
  // 1. Zoning
  if building.height > parcel.maxHeight: issues += HEIGHT_VIOLATION
  if building.footprint outside parcel.setbacks: issues += SETBACK_VIOLATION
  
  // 2. Structural
  for each floor:
    if floor has unsupported spans > MAX_SPAN: issues += STRUCTURAL_WEAKNESS
    if load path broken: issues += LOAD_PATH_BROKEN
  
  // 3. Code compliance
  if bedrooms without windows: issues += NATURAL_LIGHT
  if floor without fire exit: issues += FIRE_SAFETY
  if bathrooms < required: issues += CODE_BATHROOM_COUNT
  
  // 4. Client requirements
  for each requirement in contract:
    if not met: issues += CLIENT_REQUIREMENT_UNMET
  
  // 5. Energy
  energyRating = CalculateEnergyRating(building)
  if energyRating < required (KfW 40): issues += ENERGY_BELOW_REQUIRED
  
  return issues
```

---

### SYSTEM 6: WORKER SYSTEM

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Manages all employee entities: hiring, skills, assignment, morale, fatigue. |
| **Responsibilities** | Worker skill calculation; productivity computation; fatigue accumulation/recovery; morale dynamics; assignment resolution; payroll processing; training progress. |
| **Owned Data** | Employee entities, Crew entities, Skill definitions (config). |
| **Update Frequency** | Per in-game hour (fatigue, morale). Per assignment change. Per day (payroll). |
| **Inputs** | TimeHourTick, assignment commands, training commands, safety incidents. |
| **Outputs** | Worker productivity values (consumed by Construction System); payroll data (consumed by Finance System). |
| **Dependencies** | Time System, Safety System (incidents affect workers). |
| **Events Emitted** | `WorkerHired(employeeId)`, `WorkerTerminated(employeeId)`, `WorkerPromoted(employeeId, newRole)`, `WorkerFatigueChanged(employeeId, level)`, `WorkerMoraleChanged(employeeId, level)`, `WorkerSkillIncreased(employeeId, skill, newLevel)`, `WorkerAssigned(employeeId, siteId)`, `WorkerUnassigned(employeeId)`, `CrewFormed(crewId)`, `CrewDisbanded(crewId)` |
| **Events Consumed** | `HourChanged` — update fatigue/morale. `DayChanged` — process payroll, training ticks. `SafetyIncident` — injured workers. `ConstructionMilestone` — morale boost. `WeatherChanged` — fatigue modifier. |
| **Failure Cases** | Worker assigned to two sites simultaneously → reject second assignment. Worker terminated while critical task in progress → task reassigned to next available. Mass resignation (morale collapse) → emergency notification, temporary labor available at premium. |

**Fatigue Model:**

```
Per working hour:
  Fatigue += BASE_FATIGUE_RATE × WeatherFatigueModifier × OvertimeModifier
  
  BASE_FATIGUE_RATE: 2.0/hour (normal), 3.0 (overtime), 4.0 (night)
  WeatherFatigueModifier: 1.0 (normal), 1.3 (extreme heat), 0.9 (pleasant)
  
Per resting hour (off-shift):
  Fatigue -= RECOVERY_RATE × RestQuality
  RECOVERY_RATE: 15.0/hour
  
Fatigue effects:
  0-30:   No penalty
  31-60:  Productivity × 0.9, Safety risk × 1.2
  61-80:  Productivity × 0.7, Safety risk × 1.5
  81-100: Productivity × 0.4, Safety risk × 2.0, chance of refusing work
```

**Morale Model:**

```
Morale changes from:
  +5:   Project milestone completed
  +2:   Paid on time
  +1:   Working in good weather
  +3:   Promotion / bonus received
  -3:   Safety incident on site
  -2:   Extended overtime (3+ consecutive days)
  -5:   Coworker terminated
  -1:   Bad weather (continuous)
  -10:  Pay cut
  +10:  Company wins major award
  
Morale effects:
  80-100: Productivity × 1.1, low quit risk
  50-79:  Normal
  20-49:  Productivity × 0.9, moderate quit risk
  0-19:   Productivity × 0.7, high quit risk, strike possible
```

---

### SYSTEM 7: CONTRACT SYSTEM

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Manages contract lifecycle: generation, bidding, tracking, completion. |
| **Responsibilities** | Procedural contract generation; bid evaluation (player vs AI); requirement tracking; payment milestone processing; deadline enforcement; penalty/bonus calculation. |
| **Owned Data** | Contract entities, Client entities, ContractTemplate config. |
| **Update Frequency** | Per day (deadline checks, new contract generation). Per event (bid processing, milestone). |
| **Inputs** | Player bids; market conditions; player reputation; region data; building completion events. |
| **Outputs** | Available contracts (for UI); active contract status; payment events; reputation events. |
| **Dependencies** | Economy System, Reputation System, Region System, Building System, Company System. |
| **Events Emitted** | `ContractGenerated(contractId)`, `ContractAwarded(contractId, winner)`, `ContractBidPlaced(contractId, companyId, amount)`, `ContractMilestoneReached(contractId, milestone)`, `ContractCompleted(contractId)`, `ContractBreached(contractId, reason)`, `ContractCancelled(contractId)`, `PaymentDue(contractId, amount)`, `PaymentReceived(contractId, amount)`, `DeadlineWarning(contractId, daysRemaining)`, `DeadlineMissed(contractId)` |
| **Events Consumed** | `ReputationChanged` — affects bid competitiveness. `BuildingCompleted` — triggers contract completion. `DayChanged` — deadline checks. |
| **Failure Cases** | Client goes bankrupt → contract cancelled, partial payment. Player misses deadline → penalty applied, reputation hit. Bid below minimum profitability → rejection with warning (cannot bid yourself into bankruptcy on purpose). |

---

### SYSTEM 8: FINANCE SYSTEM

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Manages company financials: cash flow, loans, taxes, insurance, invoices. |
| **Responsibilities** | Transaction processing; loan management; tax calculation; insurance premium processing; invoice generation and payment; financial report generation. |
| **Owned Data** | C_Financials (on PlayerCompany), Loan entities, Invoice entities, Bank entities. |
| **Update Frequency** | Per transaction (real-time). Per day (interest accrual). Per quarter (taxes, insurance). |
| **Inputs** | Payment events; purchase events; loan requests; tax rules (config). |
| **Outputs** | Financial state (for UI dashboard); reports; warnings (low cash, loan due). |
| **Dependencies** | Time System, Economy System (interest rates). |
| **Events Emitted** | `Transaction(amount, category, description)`, `LoanIssued(loanId, amount, terms)`, `LoanPaymentDue(loanId)`, `LoanRepaid(loanId)`, `LoanDefaulted(loanId)`, `TaxDue(amount, deadline)`, `InsuranceDue(amount)`, `CashLow(warning)`, `CashCritical`, `BankruptcyWarning`, `BankruptcyDeclared` |
| **Events Consumed** | `DayChanged` — interest accrual. `QuarterChanged` — tax & insurance processing. `PaymentReceived` — credit cash. `WorkerPayroll` — debit cash. `MaterialPurchased` — debit cash. |
| **Failure Cases** | Cash negative without available credit → bankruptcy trigger. Loan payment missed → credit rating downgrade, penalty interest. Tax payment missed → penalty + interest, legal risk. |

---

### SYSTEM 9: SAFETY SYSTEM

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Simulates construction site safety: risk evaluation, incident generation, impact propagation. |
| **Responsibilities** | Risk factor aggregation; incident probability calculation; incident generation and resolution; safety reputation impact. |
| **Owned Data** | SafetyIncident entities (pooled). |
| **Update Frequency** | Per in-game hour per active site. |
| **Inputs** | Worker fatigue levels; weather conditions; equipment condition; site phase; safety investment level; difficulty setting. |
| **Outputs** | Incident events; reputation impacts; worker state changes. |
| **Dependencies** | Worker System, Weather System, Equipment System, Construction System, Time System. |
| **Events Emitted** | `SafetyIncident(siteId, type, severity, affectedWorkers[])`, `SafetyIncidentResolved(incidentId, resolution)`, `SafetyRiskWarning(siteId, riskLevel)` |
| **Events Consumed** | `HourChanged` — risk evaluation. `PhaseStarted` — risk recalibration per phase. |
| **Failure Cases** | Worker fatality (vetoed by GSS — injuries only). Incident during inspection → automatic failure. |

**Risk Model:**

```
RiskScore = Σ(factors):

  BaseRisk(phase):
    Phase 1 (Site Prep):    15
    Phase 2 (Foundation):   20
    Phase 3 (Structure):    35  ← highest risk
    Phase 4 (Envelope):     25
    Phase 5 (MEP):          15
    Phase 6 (Interior):     10
    Phase 7 (Systems):      8
    Phase 8 (Punch):        5

  Modifiers:
    + WorkerFatigue / 2
    + (100 - SafetyInvestment) / 5
    + WeatherRisk (0-20)
    + EquipmentConditionRisk (0-15)
    - WorkerSkill / 5
    - AutomationLevel × 10

  IncidentProbability = RiskScore / 1000 per hour per site
  
  If incident:
    Severity = Random(RiskScore/3, RiskScore)
    Type = WeightedRandom: Minor(70%), Moderate(25%), Serious(5%)
```

---

### SYSTEM 10: REPUTATION SYSTEM

| Attribute | Detail |
|-----------|--------|
| **Purpose** | Tracks multi-axis reputation per region and globally. |
| **Responsibilities** | Score calculation across 5 axes; reputation event processing; reputation-based gating. |
| **Owned Data** | C_Reputation per region. |
| **Update Frequency** | On event trigger (not continuous). |
| **Inputs** | Quality scores from Construction; deadline data from Contracts; community projects; safety incidents; innovation metrics. |
| **Outputs** | Reputation scores (pollable by Contract, UI); tier-gating decisions. |
| **Dependencies** | Region System. |
| **Events Emitted** | `ReputationChanged(regionId, axis, oldScore, newScore)`, `ReputationMilestone(regionId, milestone)`, `ReputationTierUnlocked(tier)` |
| **Events Consumed** | `ContractCompleted` — adjust quality/reliability. `SafetyIncident` — adjust safety. `CommunityProjectCompleted` — adjust community. `InnovationUsed` — adjust innovation. |
| **Failure Cases** | Score outside 0-100 → clamp. Multiple rapid changes → smoothed (exponential moving average prevents volatility). |

**Calculation:**

```
QualityScore     = EMA(project quality scores, α=0.3)
ReliabilityScore = EMA(on-time delivery %, α=0.2)
InnovationScore  = cumulative from new tech usage, unique designs
CommunityScore   = cumulative from pro-bono, affordable housing, local hiring
SafetyScore      = 100 - (incident severity sum / project count)

OverallRating = 0.30×Quality + 0.25×Reliability + 0.15×Innovation + 0.15×Community + 0.15×Safety
```

---

## UPDATE ORDER DIAGRAM

```
EVERY FRAME:
┌─────────────────────────────────────────────────────────────┐
│ 1. INPUT PROCESSING                                          │
│    └── Raw input → Command objects → Command validation       │
│                                                              │
│ 2. TIME SYSTEM                                               │
│    └── Advance time → emit granularity events                │
│                                                              │
│ 3. EVENT DISPATCH (immediate events from frame 1-2)          │
│                                                              │
│ 4. PER-HOUR SYSTEMS (if HourChanged event)                   │
│    ├── Weather System → update weather                       │
│    ├── Worker System → fatigue/morale tick                   │
│    └── Safety System → risk evaluation                       │
│                                                              │
│ 5. PER-5-TICK SYSTEMS (every 5th simulation tick)             │
│    └── Construction System → progress all active sites       │
│                                                              │
│ 6. PER-DAY SYSTEMS (if DayChanged event)                     │
│    ├── Economy System → update prices                        │
│    ├── Contract System → deadline checks, generate contracts │
│    ├── Finance System → interest, payroll                    │
│    ├── Worker System → training progress                     │
│    └── Calendar System → schedule events                     │
│                                                              │
│ 7. EVENT-DRIVEN SYSTEMS (triggered by accumulated events)    │
│    ├── Reputation System → recalculate if triggered          │
│    ├── Achievement System → check unlock conditions          │
│    └── Notification System → generate/deliver notifications  │
│                                                              │
│ 8. PRESENTATION BUS UPDATE                                   │
│    └── Snapshot domain state → notify UI/Audio/Camera        │
│                                                              │
│ 9. RENDER                                                    │
└─────────────────────────────────────────────────────────────┘
```

---

## EVENT CATALOG

Total events: **86**

### Event Categories

| Category | Count | Example |
|----------|-------|---------|
| Time events | 12 | HourChanged, DayChanged, SeasonChanged |
| Weather events | 5 | WeatherChanged, StormStarted |
| Construction events | 10 | PhaseCompleted, ProblemDetected |
| Building events | 6 | DesignValidated, BuildingStateChanged |
| Worker events | 9 | WorkerHired, WorkerFatigueChanged |
| Contract events | 11 | ContractAwarded, DeadlineWarning |
| Finance events | 12 | Transaction, LoanIssued, BankruptcyWarning |
| Economy events | 5 | PricesUpdated, RecessionStarted |
| Safety events | 3 | SafetyIncident, SafetyRiskWarning |
| Reputation events | 3 | ReputationChanged, ReputationMilestone |
| Achievement events | 2 | AchievementUnlocked |
| UI/Presentation | 8 | NotificationDelivered, CameraModeChanged |

---

## SIMULATION SYSTEMS APPROVED

### Final Counts

| Metric | Count |
|--------|-------|
| Total Systems | 28 |
| Core Systems | 5 |
| World Systems | 4 |
| Construction Domain | 5 |
| Business Domain | 6 |
| Presentation Bridge | 3 |
| Support Systems | 5 |
| Total Events | 86 |
| Systems with per-frame update | 3 (Time, ECS, EventBus) |
| Systems with per-hour update | 3 (Weather, Worker, Safety) |
| Systems with per-5-tick update | 1 (Construction) |
| Systems with per-day update | 5 (Economy, Contract, Finance, Worker-payroll, Calendar) |
| Event-driven systems | 7 (Reputation, Achievement, Notification, etc.) |

### Future Extension Points

1. **New system:** Register in System Orchestrator with dependencies and update frequency.
2. **New event type:** Define event struct, register in Event Bus type registry.
3. **New update frequency:** Add to Orchestrator schedule. No existing system changes.
4. **System replacement:** Swap implementation behind same interface. Consumers unaffected.
5. **Multiplayer replication:** Add `[Replicate]` tag to events. Networking layer handles transmission.

---

**End of Simulation Systems Technical Design — InstaBuilt: Blueprint Empire v1.0**
