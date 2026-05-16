# FPS Trainer — Realistic Scenarios PRD

**Version:** 0.1 (draft)
**Status:** Proposed — follows Assessment Module ship
**Scope:** Training arm of the product; ships after Assessment v1

---

## 1. Overview

The Realistic Scenarios Module is the training arm that follows the Assessment Module. Where the assessment diagnoses weaknesses, scenarios remediate them through short drill sessions set in game-style environments. The module ships *after* the Assessment Module is live and producing analysis sessions.

In v1 of this module, an LLM is **not** in the critical path of drill prescription. Drills are selected from a fixed library by deterministic logic (weakness → tag → drill). LLM-driven parameter tuning and runtime drill generation are explicit follow-on phases on the roadmap.

## 2. Goals

- Provide short (5–10 min) drill playlists that target weaknesses identified by the assessment.
- Use mixed indoor/outdoor environments to root drills in realistic FPS context, not abstract aim-trainer geometry.
- Use a mix of target types — static dummies, scripted moving targets, and simple AI bots — across drills, matched to the construct each is training.
- Produce a separate "training score" per session that gives feedback without contaminating assessment scoring.
- Establish a drill library architecture that scales cleanly into LLM-driven tuning (phase 2) and LLM-driven generation (phase 3).

## 3. Non-goals (v1 of this module)

- LLM-generated drills or runtime parameter tuning — explicitly deferred.
- Replacing assessment scores with drill performance — drills are training-only.
- Game-specific weapons, recoil, or movement systems.
- Competitive ranking, leaderboards, or social play.

## 4. Connection to the Assessment Module

- Each subtest weakness maps to one or more drill tags.
- After an assessment session, the analysis output includes a **prescribed playlist of 3–5 drills** (5–10 min total) targeting the lowest-percentile constructs.
- Playlist construction is **rule-based in v1**: rank weaknesses, pick the highest-rated drill per weakness, balance for variety, fit within the time budget.
- The LLM (already in the loop for the assessment report) can narrate the playlist's rationale to the user, but does not pick the drills in v1.

## 5. Drill library architecture

Each drill is a self-contained UE scenario with structured metadata:

```
Drill {
  id, name,
  environment_id, environment_type: indoor | outdoor | mixed,
  target_type: static | scripted | ai_bot,
  duration_seconds,
  difficulty_levels: [...],
  weakness_tags: [flick, tracking, switching, pre_aim, reaction],
  scoring: { metrics_captured: [...], composite_formula }
}
```

Drills are pure content in v1 — code-defined parameters, no runtime tuning. The schema deliberately exposes parameter ranges so phase-2 LLM tuning can plug in without schema changes.

## 6. Drill types (initial library plan)

The library starts small and grows. One drill per weakness tag is the minimum bar for v1 of this module; richer coverage follows. Each drill maps to one or more weakness tags.

- **Flick weakness drills** — short-engagement scenarios with targets appearing in peripheral vision. Variants in indoor (rooms, doorways) and outdoor (long-sightline ridges).
- **Tracking weakness drills** — AI bots strafing along predictable lines in open environments; jiggle-peek targets.
- **Switching weakness drills** — multi-bot rooms where targets activate in sequence.
- **Pre-aim weakness drills** — "clear the building" scenarios where targets spawn at known angles and crosshair placement is the decisive variable.
- **Reaction weakness drills** — door-breach scenarios with very short reaction windows.

Final library size for v1 is deferred until UE content production capacity is scoped.

## 7. Scoring within drills

- Per-drill score: composite of accuracy, time-to-kill, shots-fired ratio, time-on-target, etc. — composition is drill-specific.
- Per-session **training score**: aggregate of drills played in that session.
- Training scores are stored on a separate track from assessment scores and **never** used to update z-scores or percentiles.
- Training scores feed into a separate "training history" view in the dashboard, parallel to the assessment history.

## 8. Technical considerations

- Drills are UE-side content. The library is loaded at runtime from a manifest, so adding drills doesn't require a binary patch.
- Drill telemetry uses the **same trial data shape** as the Assessment Module where applicable, so analytics and re-scoring tooling are shared.
- Playlist prescription logic runs server-side (FastAPI) so it can be evolved (rules → LLM tuning → LLM generation) without UE patches.
- Drill content is the largest cost driver in this module — environment art, AI bot behaviour, and animation must be scoped per drill.

## 9. Roadmap

- **Phase 1 (this PRD):** fixed drill library, rule-based prescription, separate training scores.
- **Phase 2:** LLM tunes drill parameters within designer-defined ranges (target size, speed, spawn rate) based on the user's specific weakness profile. Library stays fixed; parameters are dynamic.
- **Phase 3:** LLM composes new drills from primitives (target type × environment × spawn pattern × duration) and back-tests them against the rule-based baseline before they reach players.

## 10. Open questions

- Final v1 library size — depends on UE content production capacity per drill.
- AI bot complexity ceiling for v1 — Behaviour Trees are sufficient; advanced perception/pathing is a later concern.
- How many drills per weakness tag are needed to keep playlists varied across repeat sessions (avoid players seeing the same drill every time).
- Whether to expose drill-level difficulty selection to users, or auto-pick from their assessment scores.
- Cooldown / freshness logic — should recently-played drills be deprioritised in playlists?
