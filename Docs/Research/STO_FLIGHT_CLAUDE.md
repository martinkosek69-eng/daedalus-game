# STO normal-flight reference for the solar flight lab (task 0005)

- Author: Claude Code, independent research for Codex (task 0005)
- Base: `d492d94ea2e522c1651461f898ba1426bd58df01` (`codex/solar-flight`)
- Researched: 2026-10-03. Web sources change, so re-check before relying on details.
- Status: DRAFT checkpoint. Sections may still be refined before READY_FOR_REVIEW.

Scope: normal (sub-warp) flight of Star Trek Online (STO) on PC. Covers throttle,
reverse, braking, turn rate versus inertia, pitch limit, banking, mouse
steering and the chase camera. Hyperspace, warp, combat powers and pilot
maneuvers are out of scope and are only mentioned where they explain a source.
No STO code, data files, art or video were copied. STO was not launched.

## 1. Summary for the coordinator

1. Developers describe STO handling in an interview as ships that behave like
   "tall ships or submarines" at destroyer pace, not like fighters, with 3D
   movement (D1). This is a design intent statement, not a formula.
2. Official ship pages publish three handling stats per ship: **Base Turn Rate
   in degrees/second**, **Impulse Modifier** and **Inertia** (S1–S3). Large
   ships are published at 4–6 °/s and agile pilot escorts at 20 °/s. A 600 m
   capital ship sits naturally at the slow end.
3. The rest is community-derived, not developer-confirmed (C1–C5):
   - persistent stepped throttle with a small reverse range, R toggles 0/100 %
   - reverse at about 25 % of forward speed
   - reduced turn rate near zero throttle, with full turn rate from about 25 % throttle
   - pitch rate tied to turn rate
   - low inertia makes the ship slide through turns
   - no player roll, and pitch limited to roughly 75°
4. The prototype's controls already resemble STO defaults: QE throttle, R toggle,
   Home camera reset and mouse-drag camera. Two differences:
   - STO uses W/S for pitch, while our lab uses WASD heading.
   - STO has no dedicated brake key. Stopping is throttle to 0 plus inertia.
   Both are legitimate design choices; neither is required for an STO-like feel.
5. Recommended lab feel: slow yaw (default 6 °/s) with a short response lag,
   visible but modest slide, visual-only bank that returns level, pitch clamp
   75–80°, and a level-horizon chase camera. Tunable ranges are in section 4.

## 2. Sources and evidence levels

Evidence levels: **OFFICIAL** = Cryptic/Arc page; **DEV** = named developer or
studio statement in an interview; **COMMUNITY** = player wiki, guide or forum;
**TERTIARY** = trope or summary site. No direct gameplay video was measured.

| ID | Source | Level | Date | Used for |
| --- | --- | --- | --- | --- |
| S1 | [Star Trek Online: Federation Flagships](https://www.playstartrekonline.com/en/news/article/9782283) (signed by the Lead Systems Designer) | OFFICIAL | 2016-02-10 | Star Cruiser (Odyssey family): Base Turn Rate 6 degrees/second, Impulse Modifier 0.15, Inertia 30; page says stats are subject to change |
| S2 | [Star Trek Online: Fed Pilot Ship Stats](https://www.playstartrekonline.com/en/news/article/9226533) | OFFICIAL | 2015-05-06 | Pilot Escort: Base Turn Rate 20, Impulse Modifier 0.24, Inertia 75; pilot maneuvers are a separate double-tap mechanic for these ships |
| S3 | [Command the Aetherian Harmony](https://www.playstartrekonline.com/en/news/article/11579886) | OFFICIAL | 2026-02-17 | Carrier: Base Turn Rate 4, Impulse Modifier 0.15, Inertia 20 |
| D1 | [Star Trek Online: Q&A with Cryptic](https://www.rpgsite.net/interview/2916-star-trek-online-qa-with-cryptic), RPG Site | DEV | 2010-02-17 | Design intent: 3D movement at big-ship pace ("tall ships or submarines"), feel and tactical pacing over a full Newtonian model |
| D2 | [Star Trek Online Space Combat Q&A](https://www.tentonhammer.com/articles/star-trek-online-space-combat-q-a), Ten Ton Hammer, Executive Producer Craig Zinkievich | DEV | 2009-11-02 (pre-launch) | Throttle/speed changeable in combat; full impulse puts power into engines; positional play, standing still is detrimental |
| C1 | Key binds, STO community wiki, [archived snapshot](https://web.archive.org/web/20211127061307/https://sto.fandom.com/wiki/Key_binds) | COMMUNITY | snapshot 2021-11-27 | Default space keys (section 3) |
| C2 | List of console commands, [archived snapshot](https://web.archive.org/web/20211231201905/https://sto.fandom.com/wiki/List_of_console_commands) | COMMUNITY | snapshot 2021-12-31 | `ThrottleAdjust`, `ThrottleSet` (negative = reverse, 0 = stop), `ThrottleToggle`, camera commands |
| C3 | Inertia, [archived snapshot](https://web.archive.org/web/20220604032647/https://sto.fandom.com/wiki/Inertia) | COMMUNITY | snapshot 2022-06-04 | Inertia scale, sliding, "power slide" |
| C4 | [Star Trek Online Starship Mechanics Guide](https://www.leveling-guides.com/star-trek-online-starship-mechanics-guide/) (republished community guide) | COMMUNITY | 2010, updated 2017 | Linear throttle, reverse ≈ 25 % speed, turn rate versus throttle, pitch rate tied to turn rate, community-fitted speed/turn formulas |
| C5 | Steam discussions: [Ship movement](https://steamcommunity.com/app/9900/discussions/0/627457521122396558/) (2015-01), [Pitch, Roll, Yaw](https://steamcommunity.com/app/9900/discussions/0/618463446160408412/) (2015-05), [mouse movement](https://steamcommunity.com/app/9900/discussions/0/527273983049171241/) (2015-09), [controller configuration](https://steamcommunity.com/app/9900/discussions/0/2765630416824385181/) (2017-09) | COMMUNITY | 2015–2017 | No loops, roll not player controlled, mouse steering method, custom throttle bindings; no developer posts in these threads |
| T1 | Tropedia "Star Trek Online", [archived snapshot](https://web.archive.org/web/20230904224045/https://tropedia.fandom.com/wiki/Star_Trek_Online) | TERTIARY | snapshot 2023-09-04 | Pitch limit of about 75° relative to the ecliptic; ecliptic used as camera reference |

Not accessible during this research:
- stowiki.net and forum.arcgames.com: Cloudflare bot check, not bypassed.
- Live sto.fandom.com: HTTP 402.

The archived copies above were used instead and are identified by snapshot date.

## 3. Findings by topic

Legend: ✅ confirmed by OFFICIAL/DEV source · 🟡 community-reported
(consistent across sources but not developer-confirmed) · 🔶 inferred by me ·
❌ not available.

### 3.1 Normal flight and throttle

- ✅ Throttle and speed can be changed in combat. Full impulse diverts power to
  engines (D2).
- 🟡 Throttle is persistent: the key changes a setting and the ship holds that
  speed without the key being held (C1, C4).
- 🟡 Default keys:
  - E throttle forward, Q throttle back
  - R toggles between 0 % and 100 %
  - Shift+R toggles Full Impulse, which is unavailable in combat and sector space (C1).
- 🟡 Default step: the community key-bind table says one press = one bar
  = 20 % (C1).
  - A 2017 Steam controller script uses `throttleadjust .25`/`-.25`, but that
    is a player's custom binding, not a default.
  - A search-engine summary claimed default 25 % steps with −25 % reverse. I
    could not find that claim in any page I opened, so it is not used ❌.
- 🟡 Speed scales roughly linearly with throttle; 50 % throttle ≈ half speed (C4).
- 🟡 Speed and turn rate also depend on engine/auxiliary power and equipment
  (C4, consistent with D2). Our lab has no power system, so this is out of scope.
- ❌ STO speed units cannot be mapped to metres per second. Ship models are not
  to physical scale with their speeds, so there is no STO "m/s" value to copy.

### 3.2 Reverse and braking

- 🟡 `ThrottleSet` accepts negative values for reverse and 0 for stop (C2).
- 🟡 Reverse speed is about 25 % of maximum impulse speed (C4). This matches the
  lab's −0.25 throttle floor.
- 🟡 In reverse the ship keeps its full turn rate (C4). Community players use
  slight reverse to turn tighter (C5).
- 🟡 There is no dedicated brake key in the defaults. Stopping means setting
  throttle to 0 (R or Q), and the ship slows according to its inertia (C1, C3).
- 🔶 A held brake key (prototype Space/X) is therefore our own addition. It
  should feel like a stronger "all stop" order, not a physical retro-thruster
  that could stop a 600 m ship instantly.

### 3.3 Turn rate versus inertia

- ✅ Turn rate is a published per-ship statistic in degrees/second (S1–S3).
  Examples:

  | Ship (official page) | Base Turn Rate | Impulse Modifier | Inertia |
  | --- | --- | --- | --- |
  | Aetherian Harmony carrier (S3) | 4 | 0.15 | 20 |
  | Odyssey-family Star Cruiser (S1) | 6 °/s | 0.15 | 30 |
  | Pilot Escort (S2) | 20 | 0.24 | 75 |

- ✅ Inertia is an official stat, but no OFFICIAL source I found defines it in
  seconds or as a formula ❌.
- 🟡 Inertia is a reverse scale: a higher value changes speed faster and slides
  less (C3, C4). Low inertia combined with a high turn rate allows a "power
  slide", where the ship faces one way while still moving another (C3).
- 🟡 Turn rate depends on throttle:
  - full turn rate at about 25 % throttle and above
  - roughly 3–4 °/s at 0 % throttle, whatever the ship (C4)
  - players report turning poorly when sitting still (C5)
- 🟡 Pitch rate appears to follow the same turn-rate value (C4).
- 🔶 For game feel, the important pattern is:
  - a big ship turns slowly (single-digit °/s);
  - its velocity follows the nose with a lag, so it slides a little;
  - it turns best at moderate speed, not when stopped.

### 3.4 Pitch limits and banking

- ✅ Developers describe 3D movement but at big-ship pace (D1).
- 🟡 Ships cannot loop or fly fully vertical; players spiral to change altitude
  (C5).
- 🟡/TERTIARY: Pitch is limited to about 75° relative to the ecliptic (T1).
  This was not confirmed by an official source ❌.
- 🟡 Players do not control roll. The ship appears to bank automatically while
  turning (C5).
- ❌ No source gives a bank angle, a return-to-level time, or whether the pitch
  angle auto-levels after releasing W/S. These need direct observation (see 6.2).

### 3.5 Mouse steering and chase camera

- 🟡 The default camera sits behind the ship. The ecliptic is the camera's
  horizontal reference (T1), so the horizon does not roll with the ship.
- 🟡 Default camera controls (C1, C2):

  | Control | Default | Action |
  | --- | --- | --- |
  | Reset Camera | Home | Centres the camera on the direction of travel |
  | Mouse look | Right-drag | Looks around |
  | Camera rotation | Left-drag | Rotates the camera |
  | Camera turn to focus | Left+Right click | Points the camera towards the pointer |
  | Camera target look | X | Looks at the target |
  | Zoom | Mouse wheel, Page Down | Steps of 15, 30 or 50 units |
  | Camera distance | `camdist`, `camCycleDist` | Sets or cycles the distance |

- 🟡 Mouse steering is not the primary STO control. A community method is to
  use a free camera and hold both mouse buttons to steer (C5).
- 🔶 For our lab, the important STO pattern is:
  - movement keys steer the ship;
  - the mouse only orbits the camera;
  - Home returns the camera behind the ship.

  The prototype controls already follow this pattern.

## 4. Recommendations for the 600 m Daedalus lab

These values are **inspired tuning**, not STO numbers, except where marked. They
fit the 0006 model as briefed: throttle −0.25..1, acceleration/brake, lateral
drift, smoothed and bounded yaw/pitch, pitch ±80°, visual bank returning level.
Every value should stay an explicit config field.

| Parameter | Suggested default | Tunable range | Basis |
| --- | --- | --- | --- |
| Throttle step (Q/E) | 0.25 | 0.20–0.25 | 🟡 STO default reportedly 20 % (C1). 0.25 is my choice because it lands exactly on −0.25 / 0 / 0.25 … 1 |
| Throttle range | −0.25 … 1.0 | keep | 🟡 reverse ≈ 25 % (C4) |
| R | toggle 0 ↔ 1 | keep | 🟡 C1 |
| Max forward speed | set by target approach time (formula below) | — | ❌ no STO m/s exists |
| 0 → 95 % speed at full throttle | 8 s | 5–15 s | 🔶 capital ship pace (D1) |
| Coast 100 % → stop (throttle 0) | 8 s | 5–15 s | 🔶 inertia-based slowdown (C3) |
| Held brake (Space/X) 100 % → stop | 4 s | 2.5–6 s | 🔶 our "all stop" addition |
| Yaw rate at ≥ 25 % throttle | 6 °/s | 4–10 °/s | ✅ official large-ship range 4–6 °/s (S1, S3) |
| Yaw rate at 0 throttle | 50 % of full | 40–70 % | 🟡 STO ≈ 3–4 °/s at 0 % (C4); ramp to 100 % by 25 % throttle |
| Yaw rate in reverse | 100 % of full | 70–100 % | 🟡 C4 |
| Rotation response (reach 63 % of commanded rate) | 0.5 s | 0.3–1.0 s | 🔶 heavy but responsive |
| Pitch rate | 0.8 × yaw rate | 0.6–1.0 × | 🟡 pitch follows turn rate (C4) |
| Pitch limit | ±75° | 70–80° | 🟡/TERTIARY ≈ 75° (T1); the planned ±80° is acceptable; soften the last 10° |
| Pitch hold on release | keep current pitch | — | ❌ unknown in STO; holding avoids surprise motion. Re-check if Codex observes STO auto-leveling |
| Visual bank (max) | 15° | 8–25° | 🔶 proportional to yaw rate; visual only |
| Bank return to level | 1.5 s | 1–3 s | 🔶 |
| Velocity alignment (slide) time constant | 2.5 s | 1.5–4 s | 🔶 cruiser-like moderate slide (C3: lower inertia = more slide) |
| Max slip angle in a sustained full-rate turn | ≤ 10° | 5–15° | 🔶 visible but readable |
| Chase camera distance | 3 ship lengths (1.8 km) | 1.5–8 lengths | 🔶 |
| Chase camera height | 0.4 ship length | 0.2–0.6 | 🔶 |
| Camera follow lag | 0.3 s | 0.15–0.5 s | 🔶 |
| Camera roll | none (level horizon) | keep | 🟡 ecliptic reference (T1) |
| Home reset duration | 0.4 s | 0.2–0.6 s | 🔶 |

**Max speed is a level-design choice.** STO gives no physical value. Choose it
from the time a beginner should need to visibly approach Earth from the spawn
point: `v_max ≈ distance_to_target / target_time`. For example, 3 minutes at
full throttle to close a chosen approach distance. Earth itself gives no motion
cue until it is close, and distant stars give no parallax. A subtle near-ship
particle/dust cue is a common way to show speed. That is my suggestion, not
STO evidence.

**WASD versus STO's W/S pitch.** STO beginners pitch with W/S. The lab's WASD
"heading" is acceptable if W/S pitch the nose and A/D yaw. If W/S are meant as
forward/back thrust, the throttle meaning becomes ambiguous next to QE. Codex
should confirm which one 0006 implements.

## 5. Proposed acceptance checks (automatable)

Each check uses the fixed-step model with scripted input and the config above.

| ID | Check | Pass criterion |
| --- | --- | --- |
| A1 | Throttle steps and clamp | Each Q/E press changes throttle by exactly the step. The value never leaves [−0.25, 1]. R toggles 0 ↔ 1. Throttle persists with no key held. |
| A2 | Acceleration | Full throttle from rest reaches 95 % of v_max in the configured time ±10 %. |
| A3 | Coast and brake | Throttle 0 from v_max stops within the coast time ±10 %. Held brake stops within the brake time ±10 %. Neither overshoots into reverse. |
| A4 | Reverse | Steady reverse speed is 25 % of v_max ±2 %. |
| A5 | Yaw rate versus throttle | Steady yaw rate is within ±5 % of config at ≥25 % throttle, at 0 throttle and in reverse. |
| A6 | 90° turn | At 50 % throttle, a 90° heading change takes 90/rate + response lag (±10 %). |
| A7 | Pitch clamp | Holding pitch up or down for 60 s never exceeds the limit. There is no flip, gimbal jump or heading discontinuity. |
| A8 | Bank | Bank is proportional to yaw rate and never above the max. It returns to < 1° within the return time after input stops. It never changes the trajectory. |
| A9 | Slide | During a sustained full-rate turn the slip angle stays ≤ the max. It decays to < 1° within 3 time constants after the turn ends. |
| A10 | Time partition | The same input script at 30, 60 and 144 FPS (or any frame partition) gives the same final position and heading within tolerance. |
| A11 | Camera | Home returns the camera behind the ship within the reset time. Mouse orbit never changes ship heading. The camera never rolls with bank and never clips inside the 600 m hull. |
| A12 | Pause | No position, speed or rotation changes while paused. Input released during pause does not keep turning after resume. |

## 6. Short beginner playtest checklist (Czech)

Pro začátečníka, asi 10 minut. U každého bodu zapiš ANO / NE a krátkou poznámku.

1. Po startu vidím svou loď zezadu a vím, kterým směrem letí.
2. Klávesou E se loď postupně rozjede a jede dál, i když klávesu pustím.
3. Klávesou Q ji zpomalím až do zastavení. Dalším stiskem pomalu couvá.
4. Klávesa R loď rychle přepne mezi „stát“ a „plný plyn“.
5. Brzda (mezerník nebo X) loď zastaví znatelně rychleji než samotné ubrání plynu.
6. Zatáčení (A/D) je pomalé jako u velké lodi, ale ne otravné. Otočení
   o čtvrt kruhu trvá zhruba 15–20 sekund.
7. Při zatáčení se loď lehce nakloní a po puštění klávesy se sama vyrovná.
8. Nos lodi jde nahoru i dolů jen do rozumného úhlu. Loď se nikdy nepřetočí
   vzhůru nohama.
9. Myší si můžu kamerou rozhlédnout kolem lodi. Klávesa Home ji vrátí za loď.
10. Vím, jak rychle letím a kolik mám plynu. Nikdy se neztratím a umím doletět
    k Zemi.

Výsledek: pokud je 8 nebo více odpovědí ANO, je ovládání pro začátečníka v pořádku.
Každé NE je podnět k doladění.

## 7. Uncertainty and limitations

- All handling formulas found are community-fitted (C4). They are not Cryptic
  code and must not be presented as developer-confirmed. This document
  deliberately copies no formula into recommendations.
- Official stats (S1–S3) show the relative scale between ship types. They do
  not reveal the internal integration, response curves or time constants.
- Some community sources are from 2010–2017. STO changed over the years, for
  example consoles and controllers were added. Current behaviour may differ.
- No direct gameplay evidence with timestamps was collected. I did not run STO
  and could not measure video. To close this gap, record about 60 s of STO
  normal flight with a known ship at 60 FPS and log these timestamps:
  - 0→100 % throttle acceleration time
  - 90° turn time at 25 % and at 0 % throttle
  - maximum pitch angle and whether pitch auto-levels after release
  - bank angle during a full-rate turn and its return time
  - slide angle after a hard turn
- Access to stowiki.net and the official forum was blocked by bot checks. I did
  not bypass them.
