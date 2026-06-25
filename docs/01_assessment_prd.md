# FPS Trainer — Assessment Module PRD

**Version:** 0.1 (draft)
**Status:** Proposed
**Scope:** v1 deliverable

---

## 1. Overview

The Assessment Module is the diagnostic core of the FPS Trainer. Players complete a fixed battery of short subtests measuring distinct motor and perceptual constructs that underpin FPS aim performance. Trial data is captured locally in Unreal, scored statistically against a normative seed and the player's own history, and sent to an LLM-backed analysis service that returns a short written report plus a scoped follow-up chat. Repeat sessions populate a longitudinal dashboard.

This module is the v1 product. Realistic training drills are a separate downstream PRD; this module's data schema and outputs must support them, but no drills ship in v1.

## 2. Goals

- Deliver a 15–20 minute fixed assessment battery covering 6 independent aim constructs.
- Produce statistically defensible per-subtest scores (z-scores, percentile bands, confidence intervals) using a comparable design across all users.
- Generate concise, actionable LLM-written analysis and a follow-up chat scoped to the player's results.
- Track progress across periodic re-tests (weekly/monthly cadence) via a dashboard.
- Architect the LLM provider as a swappable interface and the normative dataset as a replaceable layer, so both can mature without rewrites.

## 3. Non-goals (v1)

- Drill / training scenarios — separate PRD.
- Multiplayer, leaderboards, or social features.
- In-game movement, weapon recoil, or any game-specific mechanics — assessment is pure aim, static stance.
- Cloud accounts, login, or remote sync — local profiles only.
- General-purpose chat unrelated to the player's results.
- A validated normative dataset. v1 uses a placeholder seed (~20–30 self-collected sessions) to demonstrate the pipeline; true norms are a later phase.

## 4. Target user

Mixed-skill FPS players, hobbyist through competitive. The product is game-agnostic — neither assessment nor future drills emulate a specific title's mechanics. Players supply their own DPI and in-game sens; the assessment normalises angular measurements rather than enforcing a fixed sensitivity.

## 5. End-to-end user flow

1. **First-time onboarding:** create local profile, enter DPI + in-game sens + FOV (cm/360° calculated and shown), run a short free-aim warmup, complete a per-subtest tutorial (3–5 practice trials each).
2. **Assessment session:** 6 subtests run back-to-back with brief inter-subtest rest.
3. **Local scoring:** Unreal computes per-trial raw metrics and per-subtest aggregates; scores are derived against the baked-in normative seed and the player's own history.
4. **Analysis (online):** results JSON is sent to FastAPI; the LLM returns the short headline report and a chat session is opened.
5. **Dashboard:** player views composite + per-subtest scores, percentile bands, deltas vs prior session and personal best, and historical trends.

## 6. Functional requirements

### 6.1 Onboarding & calibration
- Sensitivity input: DPI, in-game sens, FOV. Display computed cm/360°.
- Free-aim warmup: ~60 seconds against passive targets.
- Per-subtest tutorial: 3–5 practice trials with on-screen guidance before each scored block.
- Sens / FOV stored per profile; re-prompt if the player changes setup.
- Optional latency probe (frame-time + monitor refresh) recorded for context, not for scoring.
- Crosshair selection: player picks a preset or customises a reticle (see 6.6) before the first subtest; the choice persists per profile.

### 6.2 Subtests (v1)

All subtests use **fixed difficulty parameters** identical for every player, every session, so scores are comparable across users and across re-tests. Trial count per scored block: ~30 trials, tuned to fit within the 15–20 min total budget.

1. **Simple Reaction Time** — single target appears at fixed location after a random foreperiod (1–3s). Click to dismiss. Measures baseline response latency.
2. **Flick Accuracy** — target appears at random angular distance (8°–45°) and angular size (1.5°–5°). Snap and click. Captures: time-to-first-hit, angular error at first shot, overshoot magnitude.
3. **Smooth Tracking** — target follows a complex, seed-generated quasi-sinusoidal path; the player holds the crosshair on it. Run as **4–6 trials of ~10s** (fewer, longer trials than the ~30-trial default — suited to sustained pursuit). No shooting is required, but fire state is recorded per sample so trigger discipline (firing only while on target) can be analysed. Captures angular error over time and time-on-target ratio.
4. **Target Switching** — two targets alternate active state at 0.5–1.5s intervals; flick, confirm hit, flick to next. Captures compound flick + micro-correction efficiency.
5. **Precision** — target appears at a **fixed distance (no distance variation)** with **randomised angular size** (small, e.g. ~0.3–1.5°). Holding distance constant isolates fine crosshair placement from ballistic travel, leaving target size — recorded per trial — as the difficulty variable, so an accuracy-vs-size curve can be derived downstream. Captures: time-to-first-hit, angular error at first shot, shots-to-hit, and per-trial target size.
6. **Reactive Tracking** — target follows an **erratic, unpredictable, seed-generated path** (random direction/speed changes at random intervals), in contrast to the smooth predictable path of #3. Same continuous-control capture as Smooth Tracking — angular error over time, time-on-target ratio, per-sample fire state — run as **4–6 trials of ~10s**. Isolates reactivity / correction speed (how fast error recovers after each direction change) rather than smooth pursuit.

### 6.3 Scoring
- Per-trial raw metrics captured client-side using frame-accurate timing (`FPlatformTime::Seconds()`).
- Per-subtest aggregates: mean, SD, plus a domain-appropriate composite (e.g. flick: weighted blend of speed and error).
- Z-scores computed against the baked-in normative seed; percentile bands at 10/25/50/75/90.
- Composite score: weighted blend of subtest z-scores. Weights documented and tunable.
- Confidence intervals reported alongside scores; outliers flagged but not auto-removed.

### 6.4 LLM analysis
- **Report (default output):** short headline, 2–3 key strengths, 2–3 key weaknesses, one-line improvement direction. Roughly 100–200 words.
- **Chat:** persists the analysis session. Player can ask "why is my flick worse than my tracking?", "how do I improve overshoot?", etc. System prompt scopes the conversation to the player's results plus general FPS technique. Out-of-scope queries politely redirected.
- LLM provider sits behind an abstract `AnalysisProvider` interface; concrete implementations for the chosen provider in v1, with at least one alternative stubbed.
- Provider receives **structured JSON only** (scores, percentiles, deltas, session metadata) — never raw per-trial data.

### 6.5 Progress dashboard
- Top-level: composite score over time (line chart) with percentile band overlay.
- Per-subtest drill-down: subtest score history plus its raw metric breakdown.
- Deltas vs prior session and vs personal best, shown numerically.
- Session list with date, composite score, and links into that session's report and chat.

### 6.6 Crosshair customization
- The player can fully customise their reticle, on a par with the crosshair editors in Overwatch and Valorant. The crosshair is part of the player's aim setup, alongside sens and FOV, and applies consistently across every subtest, the warmup, the dashboard, and (later) all drills.
- **Adjustable fields:**
  - Shape: Dot (filled disc), Cross (4 arms with configurable centre gap), Plus (solid, no gap), Circle (hollow ring), T-Shape (no top arm).
  - Colour and master opacity.
  - Inner-line tier: length, thickness, per-element opacity, centre gap.
  - Optional outer-line tier: length, thickness, opacity, offset beyond the inner arms.
  - Optional centre dot: size, opacity (independent of shape choice — can be added on top of any cross/plus/circle).
  - Optional outline: colour, thickness, opacity (visibility on bright/dark scenes).
  - Circle radius (used when Shape = Circle).
- **Presets:** the project ships at least three authored profiles ("Default Cross", "Pro Dot", "Centre + Outer") that the player can pick from. Custom edits are saved as a new named profile on the player's local profile.
- **Persistence:** stored per profile, alongside sens/FOV. Re-loaded on game start; live-updates while the settings UI is open (multicast change delegate already in place, so the HUD reticle repaints the next frame as values change).
- **Settings UI:** v1 ships data-asset-driven configuration (engineer-authored profiles only). A player-facing settings UI with sliders, colour picker, and shape dropdown follows in a near-term iteration. The underlying data shape (USTRUCT with all fields `BlueprintReadWrite`, multicast `OnCrosshairChanged` delegate, runtime `ApplyProfile` swap) is already designed so the UI can be added without refactor.
- **Non-goals (this feature):** firing/movement-driven crosshair spread (this is an aim trainer with no recoil model), per-weapon crosshair sets, sharing crosshair codes between players.

## 7. Technical architecture

- **Unreal (client):** runs all subtests, captures trial data, computes raw + aggregate metrics, persists locally (SQLite or equivalent), renders dashboard. The assessment itself is fully offline.
- **HUD layer (UMG):** a C++ `UCrosshairWidget` overriding `NativePaint` renders the reticle from settings stored on the player's profile asset. Slate primitives only (no children, no UMG layout) for pixel-accurate, snap-to-grid rendering. The widget subscribes to the profile's `OnCrosshairChanged` multicast so any settings change repaints the next frame.
- **FastAPI (server):** stateless analysis gateway. Core endpoints: `POST /analysis` (accepts session results, returns report payload) and `POST /chat/{session_id}` (multi-turn chat tied to a stored analysis).
- **Provider abstraction:** `AnalysisProvider` interface exposing `generate_report(scores) → ReportPayload` and `chat_turn(history, message) → str`. New providers slot in without touching call sites.
- **Local data model:** profile, session, subtest, trial. Per-trial data retained for at least the last N sessions to support re-scoring when methodology evolves.

## 8. Data contract (sketch)

```
SessionResults {
  schema_version: "1.0",
  profile_id, session_id, started_at, completed_at,
  sens: { dpi, in_game_sens, cm_per_360, fov, polling_rate },
  hardware: { display_refresh, measured_input_latency_ms },
  subtests: [
    {
      name,
      raw_metrics: { ... },          // mean, SD, per-trial summary
      aggregate: { ... },            // domain-specific composite
      z_score, percentile, ci_95
    }
  ],
  composite: { score, z_score, percentile, ci_95 },
  deltas: { vs_previous: { ... }, vs_personal_best: { ... } }
}
```

`schema_version` is mandatory from day one — historical sessions must remain re-scorable as methodology evolves.

## 9. Success metrics

- **Validity demo (seeded with self-collected data):** at least 4 of 6 subtests show test-retest reliability r ≥ 0.7 across repeat sessions.
- **Coverage:** pairwise correlations between subtest scores < 0.7, indicating each captures a distinct dimension.
- **Engagement:** ≥ 80% of started assessments completed in one sitting.
- **Retention:** repeat-test rate within 14 days of first session (target TBD once baseline is observed).
- **Analysis usefulness:** qualitative — does the LLM report consistently surface weaknesses the player recognises?

## 10. Risks & open questions

- Normative data is the limiting factor for percentile validity; v1 explicitly frames percentiles as "demonstration of method," not validated population norms.
- Input latency varies by hardware; recorded but not corrected for. Worth a latency-probe step in onboarding to give context.
- LLM cost per analysis + chat — monitor and consider response caching for the report layer.
- Sens normalisation assumes the player's reported DPI is accurate; mouse polling rate captured for completeness but not used in scoring.
- Outlier handling policy (e.g. trials with sub-100ms times suggesting anticipation) needs an explicit rule before v1 ships.
