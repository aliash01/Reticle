# Reticle

An FPS aim assessment and training tool built with Unreal Engine 5 and a FastAPI backend. Designed to measure distinct motor and perceptual constructs that underpin aim performance, score them statistically, and surface actionable analysis via an LLM-backed report and chat.

This is a portfolio project in active development. Built solo.

## Status

In early development. Currently building the assessment module (PRD #1). Drill scenarios (PRD #2) follow.

## What it does (planned)

- **Assessment battery** — 5 fixed subtests covering reaction time, flick accuracy, smooth tracking, target switching, and pre-aim. ~15–20 minutes end-to-end.
- **Statistical scoring** — z-scores, percentile bands, and confidence intervals per subtest, plus a composite score. User-vs-self tracking across periodic re-tests.
- **LLM analysis** — short written report highlighting strengths and weaknesses, plus a scoped chat for follow-up questions about results.
- **Realistic drills** *(later phase)* — short playlists of game-style scenarios targeting identified weaknesses, using mixed indoor/outdoor environments.

## Design docs

Full product requirements live in [`docs/`](./docs):

- [Assessment Module PRD](./docs/01_assessment_prd.md) — v1 deliverable.
- [Realistic Scenarios PRD](./docs/02_scenarios_prd.md) — follow-on training module.

These are versioned, living documents. They reflect pre-build decisions; revisions follow implementation.

## Tech stack

- **Client:** Unreal Engine 5.7, C++ (with Blueprints for UI glue).
- **Backend:** FastAPI, Python.
- **LLM:** Provider-abstracted; concrete implementation TBD.
- **Local storage:** UE SaveGame + SQLite for trial-level data.

## Architecture

The assessment runs fully offline in Unreal. Trial data is captured locally with frame-accurate timing, scored against a baked-in normative seed plus the player's own history, and persisted on-device. The LLM is only in the loop for the analysis report and follow-up chat, accessed via a stateless FastAPI gateway.

Sensitivity is normalised via player-supplied DPI and in-game sens (computed as cm/360°), so scores are comparable across hardware setups without forcing a fixed sensitivity.

## Repo layout

```
Source/Reticle/        # C++ source
Content/               # UE assets (via Git LFS)
Config/                # project config
docs/                  # PRDs and design notes
```

## Building

Requires Unreal Engine 5.7 and a C++ toolchain (Visual Studio 2022 on Windows). Clone, generate project files, open `Reticle.uproject`, build from the IDE or editor.

## Roadmap

- [x] Project setup, base pawn, sens-normalised input
- [ ] Simple Reaction Time subtest end-to-end
- [ ] Remaining four subtests
- [ ] Local scoring + dashboard
- [ ] FastAPI analysis gateway
- [ ] LLM report + chat
- [ ] Drill scenarios module
