# MassCombat

A large-scale, crowd-combat sandbox built on **Unreal Engine 5.7** with the **Mass** framework.
Thousands of agents fight in melee using a data-oriented entity pipeline (Mass), per-entity AI driven by **StateTree**, GPU-skinned crowd rendering via **Vertex Animation Textures (VAT)**, and a lightweight, GAS-inspired **Action System** authored entirely as Mass-native data.

> Built with: Mass (Entity / Gameplay / AI / Crowd / Representation / LOD), StateTree + GameplayStateTree, Navigation/Avoidance, GameplayTags.

---

## Table of Contents

- [Highlights](#highlights)
- [Architecture Overview](#architecture-overview)
- [Crowd Combat & Attacker Slots](#crowd-combat--attacker-slots)
- [Mass Traits & Fragments](#mass-traits--fragments)
- [StateTree Tasks & Conditions](#statetree-tasks--conditions)
- [Simple Action System](#simple-action-system)
- [Creating VAT Assets](#creating-vat-assets)
- [Limitations & Scope](#limitations--scope)
- [Development Notes](#development-notes)
- [License](#license)

---

## Highlights

- **Crowd-scale melee** — targeting, slot assignment, and movement run as Mass processors over all entities, not per-actor ticks.
- **Slot-based engagement** — each target exposes a fixed number of *attacker slots* arranged around it, so crowds surround a victim instead of stacking on one point.
- **Player-priority targeting & retargeting** — NPCs can be biased toward the player within a radius, and re-evaluate targets on a cadence with stable slot reuse.
- **StateTree-driven AI** — combat behavior (chase → take slot → face → attack → react) is authored visually with a small set of reusable, Mass-aware Tasks and Conditions.
- **VAT crowd rendering** — agents render as ISM/Niagara-friendly meshes animated by texture, with CPU-accumulated frame data driving playback.
- **Data-only Action System** — attacks, hit-reacts, etc. are `UDataAsset`s composed of timed *steps* (montage, rotate-to-target, apply damage, …), gated by GameplayTags and cooldowns.

---

## Architecture Overview

```
Source/MassCombat/
├── Unit/            Faction, health, spawn/init         (MCUnitTrait, MCPlayerTrait)
├── Targeting/       Target selection + attacker slots    (MCTargetingProcessor)
├── Combat/          Engagement, damage queue, death      (subsystems + resolver)
├── Movement/        Facing / orientation intent          (MCOrientationIntentProcessor)
├── Behavior/        StateTree Tasks & Conditions         (the AI vocabulary)
├── Action/          Simple Action System                 (defs, steps, runtime)
├── Representation/  VAT anim state + ISM update          (rendering)
├── Actors/          Hybrid actor (hero NPC)
├── Core/            GameMode, project settings
└── Debug/           StateTree debug visualization
```

The combat domain is decomposed so that **each fragment has a single writing processor**: targeting is written only by `UMCTargetingProcessor`, engagement history only by the damage resolver, orientation only by the orientation-intent processor. This keeps Mass queries narrow and parallelizable, and gameplay processors stay separate from **representation** processors (anim state, ISM/VAT update) so visual concerns never leak into gameplay logic.

Damage never writes across entities directly — sources enqueue events into `UMCDamageSubsystem`, and a single `UMCDamageResolutionProcessor` applies them, preserving the single-writer rule for health and engagement.

> The **animation synchronization** and **montage handling** in the representation layer are adapted from Epic's **City Sample** crowd setup.

---

## Crowd Combat & Attacker Slots

The core problem in crowd melee is *"who attacks whom, and from where."* This is solved entirely in `UMCTargetingProcessor` as a per-frame pass over all entities:

1. **Gather candidates & attackers** — every targetable unit and every attacker is collected with its faction, location, and slot capacity.
2. **Player-priority pass** — attackers flagged `bPreferPlayerTarget` snap to the player when within `PlayerTargetRadius`.
3. **Returning targets** — during a combat window, attackers keep their previous target and re-evaluate only on `NextRetargetTime`, avoiding target thrash.
4. **Open assignment** — remaining attackers pick the nearest valid enemy that still has free slots, searching only within `TargetSearchRadius`. The lookup runs on **per-faction candidate grids** rebuilt each frame (counting-sort cell layout — two allocations, no per-cell arrays), queried in two stages: a near radius first, the full radius only if that finds nothing, with an empty-search backoff so units in empty terrain stop rescanning at combat cadence.
5. **Slot assignment** — attackers around a target are packed into slots `0..MaxAttackerCounts-1`:
   - **Stable mode** keeps each attacker's existing slot and only fills the gaps left by the fallen.
   - **Recalc mode** (`bRecalcSlotsOnAttackerLoss`) redistributes slots deterministically by entity index when an attacker is lost.
   - **Mutual-engagement handling** — when attackers target one another in a cycle (`A↔B`, or longer `A→B→C→A` rings), they close to the target's center instead of orbiting empty slots.

An entity's resolved data is split by domain: the target, slot location, and distances live in `FMCTargetingFragment`; combat history (last attacker, hit/attack timestamps) lives in `FMCEngagementFragment`. The StateTree Tasks and Conditions read these directly.

---

## Mass Traits & Fragments

Traits are the authoring surface (added to a Mass config in the editor); fragments are the per-entity / shared data they install.

### Traits

| Trait | Purpose |
|-------|---------|
| `UMCUnitTrait` (*MC Unit*) | Faction, max health, attacker-slot capacity & radius, slot-recalc policy. |
| `UMCTargetingTrait` (*MC Targeting*) | Installs targeting state; player-target preference, player/search radii. |
| `UMCCombatTrait` (*MC Combat*) | Installs engagement history and death state. |
| `UMCOrientationTrait` (*MC Orientation*) | Installs facing / orientation intent state. |
| `UMCAnimTrait` (*MC Anim*) | Default VAT anim data, walk-anim play-rate mapping, idle/walk state indices. |
| `UMCActionTrait` (*MC Action*) | Installs an entity's set of available `UMCActionDef`s. |
| `UMCPlayerTrait` (*MC Player*) | Tags an entity as the player so targeting can prioritize it. |

### Fragments

| Fragment | Kind | Contents |
|----------|------|----------|
| `FMCUnitFragment` | per-entity | Current `Health`. |
| `FMCUnitInfoFragment` | const shared | Faction, `MaxHealth`, `MaxAttackerCounts`, `AttackerSlotRadius`, recalc policy. |
| `FMCTargetingFragment` | per-entity | Target, slot index/location, distances, nearest-enemy, retarget timer, `bHasTarget`. |
| `FMCTargetingParams` | const shared | Player-target preference, `PlayerTargetRadius`, `TargetSearchRadius`. |
| `FMCEngagementFragment` | per-entity | Last attacker, last damaged / hit-react / attack timestamps. |
| `FMCOrientationFragment` | per-entity | Face-target window end, look-at turn rate, look-at-nearest flag. |
| `FMCDeathFragment` + `FMCDeadTag` | per-entity + tag | Delayed-destroy time; the `Dead` tag removes the entity from combat queries while its death plays out. |
| `FMCAnimStateFragment` | per-entity | Active VAT `AnimData`, current montage, global start time, play rate, current frame, state index. |
| `FMCAnimParams` | const shared | Walk-speed threshold & play-rate mapping, idle/walk anim + state indices, default VAT data. |
| `FMCActionFragment` | per-entity | Per-action runtime (cooldown/active windows) + currently granted `ActiveTags`. |
| `FMCActionSetParams` | const shared | The list of `UMCActionDef`s available to the entity. |
| `FMCPlayerTag` | tag | Marks the player entity. |

---

## StateTree Tasks & Conditions

A compact, reusable AI vocabulary. All Tasks/Conditions read Mass fragments directly via external-data handles and schedule their own ticks through the Mass signal subsystem (no per-frame polling).

### Tasks

| Task | Behavior |
|------|----------|
| `MC Move To Target Slot` | Path to the assigned attacker slot around the current target. |
| `MC Move To Nearest Enemy` | Loiter near the nearest enemy within a min/max radius (approach without crowding). |
| `MC Look At Nearest Enemy` | Rotate to face the nearest enemy at a configurable turn rate. |
| `MC Perform Action` | Execute a `UMCActionDef` from the Action System (attacks, hit-reacts — see below). |
| `MC Stand` | Hold position / idle until a StateTree transition stops it. |

> `MC Move To Target Slot` and `MC Move To Nearest Enemy` share a common `FMCMoveToTargetTask` base (path/repath/avoidance handling) and only override the goal source.
>
> There is no bespoke attack task — an attack is just `MC Perform Action` running an attack `UMCActionDef` (rotate → montage → apply damage), so attacks and reactions share one code path.

### Conditions

| Condition | Tests |
|-----------|-------|
| `MC Has Target` | Whether the entity has a valid target. |
| `MC Has Slot` | Whether the entity holds a valid attacker slot. |
| `MC Within Distance` | Distance to target / slot / nearest enemy against a threshold (selectable source). |
| `MC Damage Taken` | Whether the entity was recently damaged (drives hit-react transitions). |

*(Every condition supports `bInvert`.)*

---

## Simple Action System

A minimal, GAS-inspired action layer expressed entirely as **Mass-native data** — no `UGameplayAbility`, no actor-side ability component. It covers what crowd combat actually needs: timed effects, tag gating, and cooldowns.

### Concepts

- **`UMCActionDef`** — a `UDataAsset` describing one action:
  - `ActivationTag` — the GameplayTag used to start it.
  - `GrantsTags` / `BlockedTags` — tags granted while active, and tags that block activation.
  - `CooldownTime`, `bBlockMovementWhileActive`.
  - **`Tracks`** — one or more parallel timelines, each an ordered list of **steps**.
- **Steps** (`FMCActionStep` subtypes) — the building blocks, each with `OnStart` / `GetDuration` / `OnEnd`:
  - `Wait`, `Play Montage`, `Rotate To Target`, `Apply Damage`, `Mark Hit Reacted`, `Print (Debug)`.
- **Runtime** — `FMCActionLib` manages start/stop, cooldown windows, and the active-tag set on `FMCActionFragment`. Actions started by StateTree run until the owning task exits, then release their tags and begin cooldown.

### Usage

1. **Author** a `UMCActionDef` (e.g. `MCAD_Melee`): set its `ActivationTag`, add a track, and fill it with steps — e.g. `Rotate To Target → Play Montage → Apply Damage`.
2. **Register** it on the entity by adding the action to the **MC Action** trait's `ActionSet`.
3. **Drive** it from StateTree with the **MC Perform Action** task, setting `ActionTag` to the action's `ActivationTag`. The task advances each track's step cursor by duration and applies step effects in order.
4. **Gate** activation with `GrantsTags` / `BlockedTags` and `CooldownTime` to prevent overlap (e.g. an attack that blocks re-attack and hit-react while active).

Hit-react is just another action (`MCAD_HitReact`) triggered by the `MC Damage Taken` condition — actions and StateTree compose without special cases.

---

## Creating VAT Assets

Crowd agents are rendered with **Vertex Animation Textures**, baked with the
[AnimToTextureHelpers](https://github.com/kromond/AnimToTextureHelpers) toolset.

### Editor utility setup (5.7)

The helper's Editor Utility Widget targets an older engine version. To use it here:

- Import the **5.4.4** Editor Utility Widget from AnimToTextureHelpers.
- When converting the Skeletal Mesh **Soft Object Pointer** into a hard object reference, insert a **Load Asset (Blocking)** node — the widget then resolves the mesh correctly and bakes as expected.

Aside from that one node, the baking workflow is identical to the upstream tool.

### Project-specific requirements

Two settings must be set for VAT playback to work with this project's CPU-accumulated frame data:

1. In the **`AnimToTextureData`** asset, **`AutoPlay` must be OFF** — frame advancement is driven by the representation processors, not by the material's built-in autoplay.
2. After baking, in the generated **Material Instance**, **uncheck the `Frame` override** so the per-instance frame value supplied by the project takes effect.

---

## Limitations & Scope

This is a focused crowd-combat sandbox; several areas are intentionally out of scope.

| Limitation | Note |
|------------|------|
| **Manual anim ↔ state-index mapping** | VAT state indices and their animation sequences/montages are **not auto-mapped** — the baked state index and the matching montage must be assigned by hand and kept in sync. |
| **Melee only** | Targeting and actions are built around melee range; no ranged/projectile combat. |
| **Simple health model** | Health is a single float with direct damage — no armor, mitigation, or GAS-style attribute stacks. |
| **Single-player** | Mass replication is not implemented; the focus is local crowd simulation. |
| **Limited animation states** | Fixed idle/walk/attack state indices, no blend spaces (a constraint of the VAT pipeline). |
| **Bounded target search** | NPCs acquire targets only within `TargetSearchRadius` (via the per-faction candidate grids); there is no global aggro. |
| **Two-faction model** | Faction is a `uint8`; complex alliance relationships are not modeled. |

**Possible next steps:** auto-derive state indices from anim metadata, ranged actions, and Mass network replication.

---

## Development Notes

Much of the implementation was written with **Claude Code** as a pair-programming tool. The architecture, system design, data-oriented decisions, and code review were directed and owned by me; the AI accelerated boilerplate and iteration.

---

## License

This project mixes original code/assets with Epic Games content bundled with Unreal Engine (Third Person template, Mannequin meshes/animations, prototyping assets). The animation-sync and montage approach in the representation layer references Epic's **City Sample**. All Epic-provided content and Unreal Engine are governed by the [Unreal Engine EULA](https://www.unrealengine.com/eula).
