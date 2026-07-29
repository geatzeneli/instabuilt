# INSTABUILT: BLUEPRINT EMPIRE
## Phases 7-9 — Launch & Live Operations Plan

**Document Type:** Launch + Live Operations Roadmap
**Prerequisites:** All prior documents LOCKED. Phase 6 code foundation built.

---

## PHASE 7 — EARLY ACCESS LAUNCH (Month 24)

### Steam Launch Checklist

| # | Item | Owner | Deadline |
|---|------|-------|----------|
| 1 | Store page live (capsule art, screenshots, trailer, description) | Marketing | M-90 days |
| 2 | Steam Achievements configured (20 achievements) | Engineering | M-30 |
| 3 | Steam Cloud Save tested (cross-machine sync) | Engineering + QA | M-14 |
| 4 | Steam Workshop integration (blueprint sharing) | Engineering | M-14 |
| 5 | Rich Presence (shows "Designing a Traditional Home" etc.) | Engineering | M-7 |
| 6 | Build Depot configured (Windows + Linux) | DevOps | M-7 |
| 7 | Demo build (tutorial + first contract, 30 min) | Production | M-7 |
| 8 | Press build sent to 50 outlets | PR | M-14 |
| 9 | Community Hub: Discord, subreddit, forums | Community | M-30 |
| 10 | EA Disclaimer: "This game is in Early Access..." in-game | Engineering | M-7 |
| 11 | Crash reporting: Sentry/Backtrace integrated | Engineering | M-14 |
| 12 | Analytics: progression, economy, errors, opt-in | Engineering | M-14 |
| 13 | Localization: English + German at EA launch | Localization | M-14 |
| 14 | Accessibility: colorblind, text scaling, remapping | Engineering | M-7 |
| 15 | Legal: EULA, privacy policy, third-party licenses | Legal | M-7 |
| 16 | Pricing: $29.99 EA, $39.99 full release | Publishing | M-30 |
| 17 | Launch discount: 10% first week | Publishing | Launch day |

### Launch Day Operations

```
T-24h:  Build frozen. Final QA pass.
T-12h:  Steam page switched to "Coming Soon" → "Available"
T-2h:   Build deployed to Steam default branch
T-0:    LAUNCH. Store page live. Tweet/announcement posted.
T+1h:   Monitor: crash rate, concurrent players, reviews
T+4h:   Engineering on-call: ready to hotfix critical issues
T+24h:  First 24-hour report: sales, CCU, review score, top bugs
T+72h:  First patch if needed (hotfix for critical bugs only)
T+7d:   First weekly patch (bug fixes from player reports)
T+30d:  First content update (1 new scenario, 5 new contracts)
```

### Community Systems

- **In-game feedback:** "Report Bug" / "Suggest Feature" buttons → backend
- **Public roadmap:** Trello/Notion showing what's coming next
- **Dev blog:** Bi-weekly posts on Steam (what we're working on)
- **Discord:** #bug-reports, #suggestions, #showcase, #dev-chat
- **Player surveys:** Monthly NPS survey, feature priority voting

---

## PHASE 8 — FULL RELEASE v1.0 (Month 30)

### v1.0 Completion Targets

| System | EA State | v1.0 State |
|--------|----------|------------|
| Building types | 25+ | 50+ |
| Contracts | 100+ | 250+ |
| Regions | 2 | 4 |
| Product lines | 7 (all) | 7 (polished) |
| Scenarios | 5 | 20+ |
| Company tiers | 1-3 | 1-6 |
| Career mode | Complete | Polished + narrative |
| Sandbox mode | Basic | Full creative freedom |
| Creative Architect | — | Full design mode |

### Console Readiness

- **PS5 / Xbox Series X|S:** Native ports
- **Certification:** TRC/XR compliance pre-check built into CI
- **Controller:** Full support with adaptive UI
- **Performance:** Locked 60 FPS on consoles
- **Save:** Cross-progression via platform cloud

### VR Preparation

- First-person walkthrough mode adapted for VR
- Teleport + smooth locomotion options
- Room-scale building inspection
- Target: Post-launch update, not v1.0

### Release Day Checklist

```
[ ] Metacritic score tracking setup
[ ] Review embargo lifted (3 days before launch)
[ ] Launch trailer finalized
[ ] All 12 languages localized and QA'd
[ ] Console certs approved
[ ] Physical edition manufactured (if applicable)
[ ] Press interviews scheduled (launch week)
[ ] Streamer/influencer keys distributed (1 week before)
[ ] Server infrastructure scaled for launch load
[ ] Customer support team trained and ready
[ ] Refund policy clear on store page
```

---

## PHASE 9 — LIVE OPERATIONS (Month 31+)

### Content Cadence

| Frequency | Content Type | Example |
|-----------|-------------|---------|
| **Weekly** | Bug fix patch | Crash fixes, balance tweaks |
| **Monthly** | Small update | 5 new contracts, 1 new building, seasonal event |
| **Quarterly** | Major update | New region, new product line features, new scenarios |
| **Bi-annual** | DLC | Scandinavia Region, Green Builder, Multiplayer |

### DLC Strategy

| DLC | Content | Price | Timeline |
|-----|---------|-------|----------|
| Region: Scandinavia | Nordic architecture, extreme cold weather, timber focus | $14.99 | M36 |
| Region: Japan | Seismic engineering, compact urban, traditional + modern | $14.99 | M42 |
| Specialization: Green Builder | Net-zero, carbon accounting, renewables, LEED certs | $9.99 | M39 |
| Specialization: Historic Restoration | Heritage buildings, period materials, preservation | $9.99 | M45 |
| Multiplayer Co-Op | 2-4 player shared company | Free (major update) | M42 |
| VR Mode | Full VR support | $19.99 standalone | M48 |

### Community Challenges

- **Monthly theme:** "Build the best Traditional Home" — community votes
- **Global goals:** "As a community, build 1 million housing units this month"
- **Featured builds:** Developer picks showcased on Steam + social media
- **Creator program:** Top blueprint creators get revenue share from marketplace

### Mod Support Expansion

- **Blueprint Marketplace:** Players sell designs (Studio takes 15%)
- **Script modding:** Lua API for custom behaviors (post-M36)
- **Asset modding:** Custom 3D models, textures, audio
- **Total conversion:** New regions, new product lines, new mechanics

### Metrics & Health

| Metric | Target | Alert Threshold |
|--------|--------|-----------------|
| Daily Active Users | >10,000 | <5,000 |
| Crash Rate | <0.1% | >0.2% |
| Steam Review Score | >90% | <80% |
| Session Length (median) | >45 min | <30 min |
| D1 Retention | >60% | <40% |
| D7 Retention | >30% | <15% |
| D30 Retention | >15% | <8% |
| Revenue (monthly) | >$200K | <$100K |

### Team Structure (Live Ops)

| Role | Count | Focus |
|------|-------|-------|
| Live Producer | 1 | Content cadence, community, metrics |
| Engineers | 4 | Bug fixes, performance, DLC development |
| Designers | 2 | Balance, scenarios, contract templates |
| Artists | 2 | DLC assets, seasonal content |
| QA | 2 | Regression, player-reported bugs |
| Community Manager | 1 | Discord, social, feedback aggregation |
| **Total Live Team** | **12** | (Scaled down from 45 at peak) |

---

## LIFECYCLE SUMMARY

```
M0     Project Start
M6     Prototype Complete — core loop proven
M12    First Playable — complete single journey
M18    Vertical Slice — polished demo, publisher greenlight
M24    EARLY ACCESS LAUNCH — Steam, 25+ buildings, 2 regions
M30    FULL RELEASE v1.0 — All 7 product lines, 4 regions, console
M36+   LIVE SERVICE — DLC every 6 months, monthly updates
M48+   VR + Multiplayer — Platform expansion
M60+   INSTABUILT 2 — Next generation (if v1.0 successful)
```

---

**LIVE OPERATIONS PLAN APPROVED**

*The foundation is built. The game is shipped. Now we listen to players and make it better every month.*
