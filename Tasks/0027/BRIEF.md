# 0027 — Selectable Ancient ship computer / HUD

Owner: Codex coordinator. Base: 13c9d300571925e5711c3fc80d0e64804c394bcf.
Branch: codex/solar-flight, own A: checkout. Singleplayer presentation only.

User requests two Aurora HUD choices in P → Settings: preserve current Earth
interface and add a rich Ancient interface inspired by Aurora/Orion, Atlantis
and Puddle Jumper screen references. Ancient ships default to Ancient HUD;
Daedalus remains Earth. Sharp native 3840×2160, compact 32-inch-monitor layout.
No new weapons/shields/energy gameplay. Read canonical existing flight/nav/radar
state; never mutate simulation through drawing. Preserve assets and player saves.

Allowed: Solar HUD/shared draw primitives, presentation-only HUD settings and
menu commands, targeted package probe, Content/Data/Solar/hud.json, relevant
Tools launcher/probe, Docs and Tasks/0027 plus INDEX. No binary asset ownership
transfer required. Codex owns one sequential isolated build/probe slot after
process inspection; human apps and Claude checkout remain excluded.

Deliver: sourced visual reference notes; distinct vector Ancient graphics,
glyph-like original ornaments, authentic colour/shape vocabulary, readable Czech
labels and unchanged live measurements; two explicit settings choices, preference
retained across ship swaps and restart on A:. Common family mapping supports
future reviewed Ancient hulls. Earth mode remains available unchanged.

Checks: compilation/package; one native 4K targeted mouse-driven check of
selection, both appearances, Daedalus isolation, pause/flight invariance and
settings persistence across a separate process. Inspect representative renders;
most subjective design/flying evaluation belongs to user. Normal launcher updated
only after checks; old package preserved, normal commit/push and honest handoff.
