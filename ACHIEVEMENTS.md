# PGR2 achievements - draft set

A draft achievement set for Project Gotham Racing 2, built so it can be shown in the port now and submitted to
[RetroAchievements](https://retroachievements.org) later, if it ever supports the original Xbox. No PGR2 set exists
there yet. Status: **draft list**. The memory conditions are still to be mapped (see "Mapping").

## Design rules (RetroAchievements conventions)

- Every achievement is a condition on the game's memory, never on our own code. That way the same logic works for
  any runtime that exposes the console's RAM: this port, or a future RA Xbox core. Conditions are written in
  rcheevos syntax (`0xX<address>` reads 32 bits, `0xH` 8 bits, and so on), against guest addresses. Static data
  in the XBE has the same address on the console and here; data on the heap is reached by pointer chains from
  a static global (`I:0xX<global>_0xX<offset>=...`), not by fixed heap addresses.
- Achievements fire on the moment something is earned (a medal goes from below to at-or-above a level, a rank
  rises past N), with a delta check (`d0x...`), never on simply loading a finished profile. They also check that
  a race or the frontend is actually running, not a demo, replay or attract race.
- Points follow RA's scale: 1, 2, 3, 4, 5, 10, 25, 50 and 100, weighted by effort.
- **Progression** achievements are each part of the career. **Win** marks the end of a career. **Missable** and
  **challenge** achievements are separate.
- Achievements that need Xbox Live are left out (the service is gone). So are those that need a second player
  (split screen, system link): RA treats multiplayer-only goals as optional at best.
- Difficulty is never unlocked by settings: every medal achievement counts the medal the game awarded.

## The game's structure (from its own data)

- **Kudos World Series**: 14 classes (Compact Sports, Sports Convertible, Coupe, Sport Utility, Pacific Muscle,
  Roadster, Classics, Sports Coupe, American Muscle, Super Car, Grand Touring, Track Specials, Extreme, Ultimate).
  - Each class has 3 to 13 series: 75 series and 301 races in all.
  - Every series can be won at five medal levels: Novice (Steel), Easy (Bronze), Medium (Silver), Hard (Gold),
    Expert (Platinum).
  - Finishing the whole Kudos World Series at each medal level unlocks a car: Steel the Feroce, Bronze the
    250 GTO, Silver the 911 GT1, Gold the CLK-GTR, Platinum the Cerbera Speed 12.
- **Kudos ranks**: 60. Rank 2 needs 4,000 Kudos, rank 60 needs 8,153,000. Ranks pay out Kudos Tokens.
- **Garage**: Kudos Tokens unlock cars. The game lists 102 car entries (how many a player can own is still to
  confirm).
- **Arcade Racing**: Street Racing, Timed Run and Cone Challenge, 5 stages each, with medals.
- **Time Attack**: Circuit Challenge (lap records per route) and Car Challenge.
- **Geometry Wars** on the arcade machine in the garage.
- Race types: Street Race, Hot Lap, Timed Run, One on One, Speed Camera, Overtake, Cone Challenge, Championship.

## The set (64 achievements, 780 points)

### Kudos World Series: whole-series medals (progression and win)
| # | Title | Description | Points | Type |
| --- | --- | --- | --- | --- |
| 1 | Steel Nerves | Complete the Kudos World Series at Novice (Steel) | 25 | progression |
| 2 | Bronze Age | Complete the Kudos World Series at Easy (Bronze) or better | 25 | progression |
| 3 | Silver Service | Complete the Kudos World Series at Medium (Silver) or better | 50 | progression |
| 4 | Golden Era | Complete the Kudos World Series at Hard (Gold) or better | 50 | progression |
| 5 | Platinum Driver | Complete the Kudos World Series at Expert (Platinum) | 100 | win |

### Kudos World Series: classes (28)
For each of the 14 classes, two achievements:
- **"<Class> Graduate"**: every series in the class won with any medal (Steel or better). Points: 3 for classes
  1 to 7, 4 for classes 8 to 14.
- **"<Class> Master"**: every series in the class won at Platinum. Points: 5 for classes 1 to 7, 10 for classes
  8 to 14.

Suggested titles, Graduate / Master:

| Class | Graduate / Master |
| --- | --- |
| Compact Sports | Pocket Rocket / Hot Hatch Hero |
| Sports Convertible | Top Down / Wind in Your Hair |
| Coupe | Two Doors / Coupe de Grace |
| Sport Utility | Utility Player / Off the Beaten Path |
| Pacific Muscle | Pacific Rim / Rising Sun |
| Roadster | Open Road / Roadster Royalty |
| Classics | Old School / Timeless |
| Sports Coupe | Sport Mode / Coupe d'Etat |
| American Muscle | V8 Thunder / Stars and Stripes |
| Super Car | Super Licence / Supremacy |
| Grand Touring | Grand Tour / Gentleman Driver |
| Track Specials | Track Day / Race Bred |
| Extreme | Living on the Edge / Extremist |
| Ultimate | Ultimate Driver / The Ultimate |

Points across all 28: 21 + 28 + 35 + 70 = 154.

### Kudos ranks (7)
| Title | Description | Points |
| --- | --- | --- |
| Making a Name | Reach Kudos Rank 5 | 2 |
| Rising Star | Reach Kudos Rank 10 | 3 |
| Crowd Pleaser | Reach Kudos Rank 20 | 5 |
| Showman | Reach Kudos Rank 30 | 10 |
| Kudos King | Reach Kudos Rank 40 | 10 |
| Living Legend | Reach Kudos Rank 45 | 25 |
| Gotham's Finest | Reach Kudos Rank 50 | 50 |

Rank 60 is out of reach: rank 50 already takes about every race at Platinum, and each rank after it needs far more
Kudos than the game has left to give (issue 38).

Points: 105.

### Garage (5)
| Title | Description | Points |
| --- | --- | --- |
| Token Gesture | Unlock a car with Kudos Tokens | 2 |
| Collector | Own 40 cars | 5 |
| Curator | Own 60 cars | 10 |
| Museum Piece | Own 80 cars | 25 |
| Full House | Own every car | 50 |

Points: 92.

### Arcade Racing (7)
| Title | Description | Points |
| --- | --- | --- |
| Street Smart | Gold or better on all 5 Street Racing stages | 5 |
| Against the Clock | Gold or better on all 5 Timed Run stages | 5 |
| Cone Zone | Gold or better on all 5 Cone Challenge stages | 5 |
| Street Legend | Platinum on all 5 Street Racing stages | 10 |
| Clockwork | Platinum on all 5 Timed Run stages | 10 |
| Cone Artist | Platinum on all 5 Cone Challenge stages | 10 |
| Arcade Perfect | Platinum on all 15 Arcade stages | 25 |

Points: 70.

### Single-event feats (challenge) (8)
| Title | Description | Points |
| --- | --- | --- |
| Style Points | Earn 2,000 Kudos in a single race | 3 |
| Showboat | Earn 10,000 Kudos in a single race | 10 |
| Ton Up | Reach 100 mph in a race | 2 |
| Flat Out | Reach 175 mph in a race | 10 |
| First Past the Post | Win a race | 5 |
| Traffic Control | Win an Expert Overtake event | 5 |
| Untouchable | Win a Kudos World Series race at Expert without touching a wall or a car | 25 |
| Lap of Honour | Set a lap record in Time Attack on 10 different routes | 10 |

Points: 70. The Kudos thresholds are guesses, to be tuned against real races.

### Geometry Wars (4)
| Title | Description | Points |
| --- | --- | --- |
| Insert Coin | Play Geometry Wars in the garage | 1 |
| Pacifist | Score 10,000 in Geometry Wars | 3 |
| Grid Warrior | Score 100,000 in Geometry Wars | 10 |
| Geometry Master | Score 1,000,000 in Geometry Wars | 25 |

Points: 39.

### Totals
5 + 28 + 7 + 5 + 7 + 8 + 4 = **64 achievements**. 250 + 154 + 105 + 92 + 70 + 70 + 39 = **780 points**. RA sets
for racing games typically run 400 to 1,000.

## Also part of an RA set

- **Leaderboards**:
  - Time Attack best lap per route.
  - Geometry Wars high score.
  - Most Kudos in a single race.
  - Arcade stage results.
- **Rich presence**, the line shown on a player's profile. For example: "Kudos World Series, Super Car, Hard |
  Edinburgh, Princes Street | Ferrari Enzo, lap 2 of 3 | Kudos Rank 34". Built from the mode, class, city, route,
  car, lap and rank in memory.

## Mapping: what must be found in memory

Each item needs a static address or a pointer chain, found by changing one thing in game and watching memory (the
harness, gdb watchpoints, and Harry's save files for late-game states):

1. **The profile in RAM.** Profile.Sav (30,100 bytes) is loaded into a buffer. Known so far: byte 0x844 is the
   MPH/KM/H option, and there are career fields at 0x8DC and 0xFEC to 0x1002. Needed: Kudos total, rank, tokens
   held and spent, the medal level per series (75), Arcade medals (15), cars owned (a bit or byte per car), and
   lap records.
2. **Race state.** Game mode, class, series, race type, difficulty, the race's Kudos, position, finish flag,
   speed camera speed, collision counts.
3. **Context flags.** In race, in replay, attract demo, in frontend, profile loaded. Each achievement checks these.
4. **Geometry Wars.** Score and "playing".

Late-game states cannot be reached by headless pad scripts (they cannot steer). A few of Harry's Profile.Sav files
at different stages of a career let the career fields be read and checked directly.

## Mapped so far (2026-10-20)

Found with the harness's `memdump` (guest memory against a saved Profile.Sav) and the status screens' code:
- **Live profile**: static, at 0x3665AC (the XBE's data, the same on a console). Profile.Sav's 30,100 bytes match it
  except the 20-byte signature and runtime-filled records. A second copy is on the heap at 0x6321DC.
- **Kudos Tokens held**: dword 0x366EBC (profile +0x910), read by the token line at 0x6F3E1.
- **Total Kudos**: the sum of the dwords at 0x35D090 + 16 x i, for i below the dword at 0x35D080 (8): the object at
  0x35BA78, method 0xA66C0. Kudos ranks follow from it (Frontend\Ranking.Ini: rank 2 at 4,000 ... rank 60 at
  8,153,000).
- **Cars owned**: car n (0 to 101; count 0x3109F0) is owned when bit p of the dword at 0x35BAA4 + n x 0x1C is set,
  p being the player index at 0x366800, and the car's entry in the car table (0x310A68 + n x 0x9C) is non-zero
  (0xA63C0; the byte 0x291969 unlocks every car).
- **Race state** (the race HUD, 0xDB460 and 0xE0840; checked live against the HUD in a headless race):
  - player n's car: dword 0x49E258 + n x 0x158 (0 for player 1)
  - each car's record: 0x2498 bytes from 0x39B000 (car c at + c x 0x2498): lap (int16, from 0; negative before the
    start) at 0x39B334, position (dword, from 0) at 0x39B33C, a flag byte at 0x39B30E that takes a car out of the
    field count
  - cars in the race: dword 0x383818; laps: dword 0x383830; race type: dword 0x383814 (1 in Instant Action)
  - finished: lap >= laps (the HUD switches to "Finished" there, 0xDB5CC)
  - the end-of-race Kudos breakdown reads floats at 0x39D42C, 0x39D44C, 0x39D46C, 0x39D48C (+ the car's offset)
  - in condition syntax, player 1 finishing first: `0xX39b33c=0_0x 39b334<h8000_0x 39b334>=0xX383830` (the lap below 0x8000: it is negative before the start; car 0, a pointer chain
    through 0x49E258 for any car)
- **Medals**: each race's record is 0x1C bytes from 0x35DD88 (the object 0x35BA78 + 0x2310); its first dword is the
  medal won, 1 Steel, 2 Bronze, 3 Silver, 4 Gold, 5 Platinum, 0 none (0xA66E0 counts them, called by 0xA6790 for the
  Single Player screen's medal rows with 1 to 5 in icon order). Levels (0x34A8C8, 0x2C each: flags, children,
  first child or race) and groups (0x34C284, 0x4C each) are built from Levels.Ini: Kudos World Series is group 3
  (classes = levels 23-36, races 1-172: Compact Sports 1-7, Sports Convertible 8-14, Coupe 15-22, Sport Utility
  23-30, Pacific Muscle 31-40, Roadster 41-51, Classics 52-62, Sports Coupe 63-75, American Muscle 76-89, Super Car
  90-103, Grand Touring 104-119, Track Specials 120-136, Extreme 137-153, Ultimate 154-172); Arcade is group 4 (Street
  Racing 173-192, Timed Run 193-212, Cone Challenge 213-232, 5 stages of 4 each); Time Attack group 5.
  To confirm with real saves: that a record keeps the best medal won.
- **Geometry Wars** (its text, 0x145990): score dword 0x45A308, hi-score 0x45A314, bombs 0x45A304, state 0x45A300
  (1 game over).
- **Harry's save (2026-10-20)**: the object 0x35BA78 is in Profile.Sav from file offset 0x1208 (its car table: 102
  at +0x24, then 0x1C-byte records keyed 2000 + car). Car ownership confirmed: bit 0 (player index 0) set for 29 cars.
  Past the car table the file is serialized differently from memory, so the save was loaded in the harness and
  memory read instead: races 213-216 (Cone Challenge stage 1) hold 1, Steel, matching his Single Player screen
  (Arcade: 4 Steel). Medal values confirmed. Total Kudos 5,010 (one entry), tokens 2, player index 0.
- **Cars**: a new profile owns the 29 ShowroomFree cars of Cars.Ini (Harry's save owns exactly those); 68 cost
  tokens, 5 are the Kudos World Series rewards. So the collection tiers are 40, 60, 80 and 102, and Token Gesture is
  owning any token car.
- **Speed**: the float at 0x298D50 + car x 0x6990, m/s (the speedometer 0xE5CF0 shows its magnitude, x 2.2374 for
  mph); 0x39B0D0 / 0x39B0D4 are the odometer. Compared by its bits (positive floats order as integers).
- **Player racing**: 0x38384C is 0 and race type 0x383814 above 0 (the attract demo drives car 0 with 0x38384C set
  and type 0; the HUD checks the same byte).
- **The race object** (from 0x383178): challenge type 0x383178 (the type table 0x30CEEC, 0x40 bytes each: 1 Time
  Attack, 4 Street Race, 7 One on One, 9 Timed Run, 11 Hot Lap, 16 Speed Camera, 17 Overtake, 21 Cone Challenge, ...),
  route 0x383180, group 0x383190 (3 Kudos World Series), level 0x383194, event 0x383198 (also the medal record's
  index), the win rule's target 0x38385C, difficulty 0x383868 (1 Novice to 5 Expert: the medal a success earns, the
  results screen 0xE2F10 names it). Each event's type: int16 at 0x33BEB4 + event x 0x68; the Kudos World Series'
  Overtakes are events 8, 20, 28, 45, 58, 64, 67, 91, 100, 105, 110, 114, 126, 130 and 138.
- **The race's end**: 0xF2650 decides the result into the success byte 0x383A58 (+ car) and sets 0x383984 (results
  done), which race setup 0xF14E0 clears. An Overtake is won by passing the target (0x39B340 >= 0x38385C), not by
  position. In the Kudos World Series and Arcade (group kind 2, 0x34C2A8 + group x 0x4C) a success only raises the
  medal (`if medal < difficulty`), which answers whether a record keeps the best medal.
- **Kudos in the race**: int32 0x39D3DC + car x 0x2498, the banked Kudos the race HUD shows (+0x78 of the Kudos
  object 0x39D364, read by 0xED0A0; 0xECC10 adds each banked combo; 0xEC900 zeroes it at race setup). Categories at
  +0x3C..+0x70 of the object (7 overtakes, 5 the clean section bonus). The race's final total, with the bonuses, is
  int32 0x3839A0 for car 0 (0xF2650, read by the results screen).
- **Contact**: 0xF0C50, reached only from the collision response 0x2EDE0 through 0xF3170 when the impact passes
  [0x291AA0] in a running race: it drops the combo, clears the byte 0x39D550 + car x 0x2498 (1 from race setup 0xF0520,
  the race is still clean), adds to the section's contact float 0x39D54C (zeroed each section; a section ending at 0
  earns the clean section bonus) and to the Kudos lost, 0x39D3E0.
- **Time Attack records**: route r's record is 0x50 bytes from 0x3609B4 (r below 146, 0x30DE90): +4 car, +8 best lap
  in ms (plus penalty seconds x 1000), +0xC a byte set once there is a record (0xA6830, from 0xEFD90 at a race's end
  after a new best lap, 0x383ACC). 92 routes are circuit challenges, the game's own "n / 92" count (0x62F36). Car
  Challenge records are dword 0x35BAB4 + car x 0x1C with the byte 0x35BAB8.
- Found (2026-10-21) from the code and the type and route tables, checked live in headless races (type, group,
  event, difficulty, the Kudos and contact values of the AI cars and the pad car); results, medals and records at a
  race's end are read from the code (the pad car cannot finish a race).
- To find: tokens spent; the player index (0x366800) that picks the car ownership bit, from a real save.

## In the port

Built (2026-10-20): `sdk/runtime/core/achievements.cpp` evaluates RetroAchievements-syntax conditions (memory sizes,
delta and prior, add source, sub source, add address, and/or next, reset and pause ifs, hit counts, alternative
groups) once a frame, waits for a condition to be unmet once before it can unlock, keeps unlocks in
`achievements.toml` beside the exe, shows a pop-up and lists the set in the settings window's Achievements tab.
`src/game/achievement_set.cpp` holds the whole set, 64 achievements (Kudos World Series 5, classes 28, Arcade 7,
Kudos ranks 7, Geometry Wars 4, garage 5, and the 8 single-event feats). Style Points and Showboat read the race's
Kudos while racing; Traffic Control is a Platinum medal on any Overtake event (Expert, so existing saves count);
Untouchable is the moment a Kudos World Series race at Expert is won with the clean byte still set; Lap of Honour adds
the 92 circuit routes' record bytes. Checked headless by writing the values into a race (the harness `poke` file): each
unlocked, Showboat stayed locked at 2,500 Kudos, and a plain race unlocks none. The draft's Speed Camera wording became any race, and Clean Sweep (Expert One on One) became First Past
the Post (any win), so they need no event type. Checked headless by writing Steel into races 1-7 mid-race: Pocket Rocket unlocked, nothing
else did.
`PGR2_ACH_TEST=1` adds a test one that unlocks at a race's start (checked headless).

The plan was:

- The SDK gets a small achievement runtime:
  - It evaluates the set's conditions against guest memory once a frame, as RA's rcheevos library does. The
    library itself (MIT licence) is a candidate to embed, which would make the format exactly RA's.
  - It shows a pop-up when an achievement unlocks.
  - It adds an Achievements tab to the settings window.
  - It saves unlocks to `achievements.json` beside the exe, separate from the game's saves.
- The set lives in a data file in this repository (titles, descriptions, points, conditions). Nothing from the
  disc is needed for it.
- A "hardcore" switch, as RA has, would turn off anything that changes the game (race_60fps, experiments) while
  achievements are on.
