# PGR2 recomp - progress log

Newest first. Each entry: date, what changed, what was learned, what's next.

## Status
- Milestone: 4 - races run under pad control; Windows: boots, movies, menus, races, saves, audio, reflections
- XDK 5659: 191 of 250 functions, 13 of 13 data
- Speed: paced by the game's present interval (menus 60, races 30); race_60fps option (experimental)
- Rendering, confirmed on Windows: car body, car shadow (sun and night), fences/posts/signs, car selection, street
  light glows behind buildings, night building flicker (Edinburgh) gone, ultrawide with HUD, draw distance, MSAA,
  render scale. To confirm: trees (point sprites, 2026-10-21)
- PC: F1 / Back+Start settings window (display, monitor, graphics, FXAA, fps counter, pause on focus loss, game
  options: skip intro, hide HUD, field of view, draw distance, race_60fps, DLC cities; controls), keyboard play,
  remapping; confirmed on Windows except keyboard play and remapping across a restart
- PC Settings entry in PGR2's menu (Options > General Options, replacing Screen Center): confirmed on Windows
- Achievements: draft set in ACHIEVEMENTS.md (64, 780 points); runtime built (RA-syntax conditions, pop-up, tab,
  achievements.toml); all 64 live (2026-10-21); the pop-up confirmed on Windows
- System link: works headless (scripts/linktest.sh); Windows to try
- CPU renderer: near-camera band fixed (2026-10-20)
- Checks: scripts/smoke.sh before main; scripts/linktest.sh, savetest.sh, soak.sh
- Docs: FINDINGS.md (what is established, by topic), WINDOWS.md (testing)
- Releases: PAL and NTSC (same code; NTSC smoke OK from its disc image)
- Direct3D 11 renderer: done and confirmed on Windows (drawing, MSAA; F1 > Graphics > Renderer, next start). Not
  under D3D11: F3 captures, the 8 bit target swizzle (logged if PGR2 needs it)
- DLC cities (experimental, F1 > Game, `dlc`): Paris and Long Beach race on Windows (2026-10-21); a few objects left
  out, day sets at night; boats converted from the title update's pak layout (2026-10-21, Windows to confirm)
- DLC cars: the title update's 16 cars as a car pack built on first start (2026-10-21): load, count (118) and race
  headless; 34 of their 48 own colours (the table holds 600); confirmed on Windows (2026-10-22), DLC off clean
- Stutter (2026-10-22): wheel scan and the visibility report fence stalls fixed; slow frames 0-3 per 10 s on
  Harry's PC (was up to 272); render thread 10-14 ms a frame of 16.7
- Steering wheel (DirectInput, force feedback, F1 > Wheel): Harry's rig (Simucube, BU0836 pedals, shifter, button box)
  detected and bound on Windows (2026-10-22); pedals on Z/Rz/slider confirmed (2026-10-25)
- Ultrawide: menus, HUD, replays confirmed on 32:9; the rear view mirror confirmed (2026-10-25)
- Launcher (PGR2Launcher.exe, launcher.bat): builds and launches the game on Windows (2026-10-22)
- Release 1.0 (2026-10-22): the launcher built by .github/workflows/release.yml on a tag v*; 1.0.1 the same day; 1.0.2 on 2026-10-23 (radio, replays); 1.0.3 on 2026-10-25 (Paris, radio volume, pedals, mirror, profiles, media patched discs); 1.1.0 on 2026-10-26 (your music, sharp text and HUD, Nurburgring sky, shader cache, Quit game); 1.2.0 on 2026-10-29 (launcher updates, PC Settings page, wheels, mirror); 1.3.0 on 2026-10-31 (languages); 1.4.0 on 2026-10-31 (sharpening, MSAA and FXAA separate)
- Upstream: draft PR description in .local/upstream-pr.md, not sent

## Log

### 2026-10-31 - release 1.4.0: sharpening, MSAA and FXAA separate
- Harry asked for the release after checking the settings on Windows. Notes in docs/releases/v1.4.0.md, tag v1.4.0.

### 2026-10-31 - README: graphics settings compared
- Harry's Windows screenshots of one paused frame (3440x1440): MSAA 4x cleans edges, FXAA softens, render scale 200% is
  smoothest; sharpening 50% is crisp, 100% gritty on walls and roads, so the README recommends about 50%. A wipe and
  three panels in docs/images, from his captures in .local/win. This also confirms MSAA, FXAA and sharpening on a real GPU.

### 2026-10-31 - MSAA and FXAA separate, and sharpening (issue #25)
- A player found the image soft at the highest render scale. SDK (fork main): Settings::msaa, fxaa and sharpen replace
  antialiasing (the old key carries over); psfinal runs FXAA, then contrast adaptive sharpening, then gamma, in the one
  final pass on D3D12 and D3D11; both act on the finished frame, HUD included. Sharpening is off by default, its step
  a source or output pixel, whichever is larger, and its strength linear from zero (CAS's scale was grainy on the road
  at 50%, seen by running the formula on a capture).
- PC Settings: MSAA, FXAA and Sharpening rows; Race HUD and Frame Counter left for F1 (ten rows fit).
- Checked: DXC compiles psfinal, Windows syntax check, page headless, PGR2 and PGR1 smoke, EoN builds. Not seen on a
  GPU: the harness has no D3D.
- Next: Harry on Windows: sharpening at 25, 50, 100% at render scale 100 and 200, FXAA with MSAA, an old pgr2.toml.

### 2026-10-31 - release 1.3.0: the game's languages
- Harry confirmed the language setting on Windows ("Works"). Notes in docs/releases/v1.3.0.md, tag v1.3.0. Players on
  1.2.0 are offered it by the launcher, the first release to go out that way.

### 2026-10-30 - the game's languages
- The disc holds English, Japanese, German, French, Spanish, Italian, Korean and Chinese; on a console the dashboard's
  language chooses, and the host always answered English. SDK: Settings::language, GameInfo::languages, F1 > Game >
  Language, XC_LANGUAGE answers it (XBE_LANGUAGE to test); PGR2 lists the eight, and PC Settings has a Language row.
- Headless, each language to a race on a kept disk, no traps: the HUD in German, French, Spanish, Italian, Korean,
  Chinese (their own fonts); Japanese has its menus translated and the disc's own English HUD text ("Lap Time").
  Portuguese has text files but the game runs it as English (Z:\English.Ind), so is not offered. A switch finds
  no Z:\<language>.Ind, so the game formats the cache partition and copies again, about 100 s of movies; the same
  language again does not. Smoke OK for PGR2 and PGR1; EoN's eon-soak tree fails with or without this commit (a stale
  tree), and without a language list the host answers English as before.
- Next: Harry on Windows: switch language, restart, saves still there.

### 2026-10-29 - release 1.2.0: launcher updates, PC Settings page, wheels
- The shared launcher's updates (fork launcher-update 0b5ff49, from the EoN session): a release launcher offers a
  newer release's launcher at start (SHA-256 checked, swapped in, restarted) and builds the game from its own tag,
  offering a rebuild for another version. Fork main fast-forwarded to it: PGR2 and PGR1 smoke OK, PGR1 repro a live
  race, EoN smoke OK (run by the EoN session on the same launcher commit).
  game.toml rescan unchanged.
- Notes in docs/releases/v1.2.0.md: PC Settings page, wheels (no gamepad dead zone, d-pad from buttons), the
  ultrawide mirror, the launcher's long-path warning, shared fixes, Discord; this launcher downloaded by hand once.

### 2026-10-28 - wheels: the d-pad bound to buttons
- A player's wheel has no hat, so no d-pad. The Wheel tab's Buttons table listed Start to White only, though the
  wheel code reads a binding for every console input and the settings file keeps them all.
- SDK (fork main): the table also lists D-pad Up, Down, Left and Right; the steering device's hat still works beside
  them. Windows syntax check OK (the window is not built headless). SDK at the fork's main, which also brings the
  other sessions' xbe_recomp fix (an instruction ending in a section's zero filled tail): game.toml rescan unchanged,
  regenerated, smoke OK.
- Next: the player binds the d-pad on Windows.

### 2026-10-27 - PC Settings as a page of the game's own options
- Harry: the everyday picture settings in the game's menus rather than only the F1 window (the hybrid: F1 keeps
  display, controls, wheel and the rest).
- Options > General Options > PC Settings (Screen Center's slot, 0xA2E60) now builds a page of the game's option rows,
  copied from Game Options (0xA1710): render scale, anti-aliasing, texture filtering, sharp text and HUD, draw
  distance, field of view, race HUD, frame counter, and More Settings, which builds the page again and opens the
  window. Each row steps an int in guest memory; each frame a changed row sets its setting (and saves pgr2.toml), a
  setting changed in F1 moves its row. A and B go back to General Options. The page's widgets: FINDINGS.
- Headless: the page drawn as the game's own (heading, legend, help lines), Right changes render scale and
  anti-aliasing (logged), More Settings asks for the window, B returns to General Options. Smoke OK.
- Next: Harry on Windows (that each row's setting shows at once, the window from More Settings, gamepad and keyboard).

### 2026-10-27 - wheels: the game told controller 1 is a steering wheel
- A player (wheel, separate USB pedals and buttons): a huge dead zone, then a non-linear response, whatever the wheel
  software and the Wheel tab said. PGR2 reads each port's XInputGetCapabilities subtype (0x26D8E2 into +0xB96 of the
  port in 0x1375D0) and only for a wheel (0x10, test at 0x13787A) skips its gamepad stick handling: an adaptive dead
  zone and a learned stick gate (0x137460) on both sticks. The SDK never answered XInputGetCapabilities (the library
  code ran on the HLE's handle and failed), so a wheel was a pad.
- SDK: XInputGetCapabilities signed for 5659 (game.toml rescan reproduces it) and HLE'd: SubType 0x10 when a bound
  steering device steers the port (input::Pad::wheel, from WheelRead), else 0x01; a port whose wheel state changes is
  reported removed and inserted in one XGetDeviceChanges so the game reopens it. Harness PAD_WHEEL=N.
- Headless: PAD_WHEEL=8600 mid-race, `xapi: controller 1 is a steering wheel` and the race carries on (no
  disconnection pause). Smoke OK.
- Next: the player and Harry on Windows: the Wheel tab's dead zone and lock now the whole response.

### 2026-10-27 - the ultrawide mirror's picture wider than its frame
- Harry, 21:9 and 32:9 screenshots: the mirror's picture ran past its frame on both sides. Measured: the picture was
  the box narrowed once by 16:9 / aspect (k), as intended, the frame narrowed by k twice. The 2D frame is placed from
  the camera's box (+0x28..+0x34), which the 2026-10-23 fix narrowed, and the HUD squeeze narrowed it again.
- src/game/hooks.cpp: the box stays as the game set it; only the viewport 0x185640 sets from it is narrowed (the box
  swapped in for that call), the aspect at +0x38 as before. Headless 32:9 hood view (Black twice in a race): picture
  and frame both 270..370 of 640. Smoke OK.
- Next: Harry checks the mirror on 21:9 and 32:9.

### 2026-10-27 - launcher: too long a folder path stops the Windows SDK unpacking
- A player's launcher.log stopped after 7 of the Windows SDK's 10 MSIs with no error. Their launcher sat in
  `F:\Project Gotham Racing 2 (USA, Japan) (En,Ja,...)\Project Gotham Racing 2 (...)\PGR2\` (about 135
  characters); `msiexec /a` fails past 260 characters and the SDK's deepest files lie about 130 below `tools\`.
- SDK (fork main): Builder::PathTooLong (more than 80 characters, only before a first build) stops the build with
  what to do, the window shows it in red and disables Build; BuildAll logs "build stopped: <reason>" to
  launcher.log, so later failures are in the log too. README troubleshooting entry.
- Next: the player's confirmation from a short folder.

### 2026-10-26 - release 1.1.0

Harry confirmed Your music on Windows ("works perfectly"). Since 1.0.3: Your music (launcher section 4, FFmpeg), sharp
text and HUD (Sharpen2D), Nurburgring's sky (W-buffered draws not far-clipped), the shader cache on disk, Quit game
in the settings window, best-fit contiguous memory and the shared rendering fixes. Notes in docs/releases/v1.1.0.md,
tag v1.1.0; a new launcher is needed for the music.

### 2026-10-26 - your music as PGR2's custom soundtracks (launcher)

PGR2 plays Xbox custom soundtracks: XAPI (compiled into the game, 0x190014 to 0x19020C) reads
E:\TDATA\FFFE0000\MUSIC\ST.DB and the game opens MUSIC\<soundtrack>\<song>.WMA (0x190434) with its own WMA decoder.
The ST.DB layout was read from that code (sdk launcher/core/music.h). Headless: a database and two ffmpeg-made WMA 2
files appear under Options > Sound / Music > PGR 2 Custom Soundtrack after the game's own soundtracks, and Play
streams them: the recording holds 440 Hz on the left and 660 Hz on the right alone, as encoded. sdk launcher: a
"Your music" section (game.json "music": true) picks a folder, converts each MP3/FLAC/OGG/Opus/M4A/AAC/WAV/WMA/AIFF to
WMA 2 with FFmpeg 7.1.1 (Gyan's essentials build, an optional toolchain.json tool fetched the first time, 88 MB, its
wmav2 encoder checked), writes ST.DB, and on Update converts only what changed. launcher_cli music on a test library
(loose files, an album with a CD2 folder, tags and none, accents): 4 songs in 3 soundtracks, re-runs keep what is
unchanged; in the game the soundtracks and accented names show. The SDK now includes the other sessions' shader cache
on disk, multichannel DSOUND voices and best-fit contiguous memory: recompiled, smoke OK, Windows sources check.
Next: Harry tries Your music in the launcher (a new launcher build).

### 2026-10-25 - sharp text and HUD (the SDK's Sharpen2D, from Everything or Nothing)

GameInfo::sharpen_2d on: the textures 2D draws use are enlarged on a background thread (Catmull-Rom colour, mask-like
alpha rebuilt as a sharp anti-aliased edge, soft alpha interpolated; A8 text and pictures up to 128 texels 4x, larger
2x) and drawn as replacements; F1 > Graphics "Sharp text and HUD" turns it off. SDK moved to the fork's main for it
(697c30b). Headless with the reference renderer (XBE_SHARPEN_DUMP): 15 menu and HUD textures enlarged (the font sheet,
the speedometer, the gear digits): staircase edges become smooth curves; title and race frames otherwise unchanged.
Smoke OK, every Windows source checks. Next: Harry looks at the menus and HUD at 3440x1440.

### 2026-10-25 - Nurburgring's flashing sky: W-buffered draws no longer far-clipped

Harry: Time Attack > Car Challenge > Paris > TVR Cerbera Speed 12 goes to Nurburgring (the title update's cars.ini puts
its challenge on "Nurburgring 1", so that is right); the sky is black in the flyover and white with brown bars that
flash while driving, D3D12 and D3D11, MSAA on or off. Headless (PGR2_ROUTE=50, new PGR2_WEATHER=0) the reference
renderer draws a blue sky with clouds. Harry's captures (XBE_CAPTURE_DRAWS=vs:B0101C01): the frame clear is depth
only, the sky dome draw changes nothing on screen, and a hillside is missing as well: geometry past the far plane,
which the console's W-buffer keeps and D3D clips. sdk: for W-buffered draws the vertex shaders keep z within w (both
the programs' epilogue and fixed function), near clipping and the shader's depth unchanged. All 83 generated shaders
compile (shadercheck). Smoke OK. Next: Harry checks Nurburgring.

### 2026-10-25 - Quit game in the settings window (issue 16)

Issue 16 (rizaumami): no way to quit without Alt+Tab and the window's close button. The settings window has a Quit game
button at its top right (sdk), which asks first and then closes the window as its close button does; Alt+F4 already
did, now written down in README and WINDOWS.md. The SDK is now the fork's main with the PGR1 and EoN work (3948
present parameters, mouse look, MultiSampleMode): recompiled, smoke OK, every Windows source checks.

### 2026-10-25 - release 1.0.3

Harry confirmed on Windows 11: everything since 1.0.2 works (Paris races, its floor, the radio volume, pedals on any
axes, the ultrawide mirror, Xbox profiles). Notes in docs/releases/v1.0.3.md, tag v1.0.3. The licence (local branch
`licence`) is still not on main.

### 2026-10-24 - Paris's crash after the start line: the city slots kept aside completely

Harry: L'Arc de Triomphe crashes 2 s after the line, at 100% draw distance too, on PGR2's fatal "m_CompressionStream.
msg". Reproduced headless (PGR2_ROUTE=151 once the menus have registered Paris, scripts/pad/paris.txt): 52 s into the
race. A log of each streamed block unpacked (0x1003E0) showed the last asking for block 7742 of a table of 2000, its
entry garbage: an animated texture's frame (0x1075E0) read from 0x407754, where the city material object's kind bytes
for slots 100 to 112 had landed. The 2026-10-24 fix covered one loader and one reader; the object has two loaders, a
load-time and a per-frame constant pass that walk every slot, and two resets (FINDINGS). Now each loader runs with the
real count, the slots past 100 are copied out and what they overwrote restored (before 0x107820 writes the animated
textures), and the object's count stays at 100; the slots past 100 get their pixel constants from the game's own
passes run on a scratch copy of the object, and a draw naming one gets a stand-in slot with its colour and constant
for the call. Headless: route 151 five minutes, past the line and the Arc, no fault; "city slots: 113". Smoke OK.

### 2026-10-24 - Paris at 200% draw distance: m_CompressionStream after five minutes

Harry: Paris much better, then PGR2's fatal error "m_CompressionStream.msg" near the end of a race (draw distance
200%, 310 s in). The F3 stacks: a streamed city block (table 0x3F2D38) failing to unpack in 0x1003E0. The title
update's streaming code and constants are the disc build's, and the only arrays it enlarged besides the city ones are
paint colours, two car lists and 0x20 bytes of the audio manager (FINDINGS). Not reproduced: headless the car cannot
drive a lap (holding the throttle stops at the first corner; the auto start's flag not found among five candidates).
Suspected: 200% draw distance streams more of Paris than the streamer's buffers hold. Next: Harry's A/B at 100%.

### 2026-10-24 - Paris's missing floor; the radio at the music volume

Harry's video (Paris, Pont d'Iena): the road surface missing; the radio ignoring the Music volume. Only Paris.
- Floor: the draws the crash guard left out all used material type 5, whose table entry was zero: the city material
  arrays are 100 long on the disc build and Paris fills 113 of one, the extra 13 dwords overwriting material types 0
  to 6 (memory dumps: the base game's types 0 to 9 intact, Paris's not; the title update's XBE, imported into Ghidra
  as pgr2tu, has the same code with the arrays moved and 150 long). src/game/city_tables.cpp keeps entries past 100
  aside (the loader 0x16C450, the reader 0x118B90, the reset 0x1BEFE0). Headless Paris: "113 entries, 13 kept aside",
  the guard no longer fires, the road is drawn.
- Radio: Music volume is the float 0x367528 (found by diffing memory at half and full); the race radio's samples are
  full scale and its only send is at 0 mB at any setting, so the console's DSP applies it. sdk 97a51d2: GameInfo::
  send_level scales sends played into the speakers; PGR2 gives the music volume. Headless: radio gain 0.501 at full,
  0.251 at half (the player's profile), 0.351 in Instant Action (default 0.7).
- Also: a scripts/pad/paris.txt run fails when the harness disk has no DLC (smoke wipes it): copy the packs again.
Smoke OK. Next: Harry checks Paris and the radio volume.

### 2026-10-24 - Paris crash: draws with garbage strides left out; an uncompiled menu callback

Harry's log (Paris, draw distance 200%): every DLC file matches, and the game stops 1.3 s after Paris loads, with no
fatal error. Correction to the entry below: PGR2_ROUTE=147 is not Paris any more (the DLC cities get their routes in
the menus; 147 was empty, so Edinburgh loaded against it, and at 200% its stream read went to a garbage offset:
"BizFile::ReadDecompress"). scripts/pad/paris.txt now drives a Paris Time Attack headless. With the reference renderer
on, Paris draws a few meshes whose stream stride is garbage (about 0xFFFFFFFC): their vertices lie gigabytes outside
the guest's memory, and the GPU renderers' copy of them is the memcpy access violation in VCRUNTIME140 that Windows
reports. sdk: such draws are left out and logged; headless the guard fires in Paris 2 s after it loads and never in a
base game race (REF=60). XBE_FILES_LOG=1 also logs short reads. The SDK merge brought the recompiler's list of code
addresses never made functions: 0x7F9F0, a menu item's callback (0x80F70), is now in [functions]; the other three are
data. Smoke OK. Open: the other player's "m_CompressionStream.msg" (100% draw distance), not reproduced.

### 2026-10-24 - DLC packs' files checked against their ContentMeta.xbx

Report (Windows, a player's own DLC copy): Long Beach and the Paris cars work, Paris tracks stop: before drawing, at the
start, or with "dirty disc" and broken graphics; the log ends on PGR2's fatal error "m_CompressionStream.msg" (0x136E70:
zlib 1.1.4 inflate of a compressed chunk failed), Event Viewer an access violation in memcpy. Headless with Harry's DLC
copy, route 147 (Paris), three runs: no fault. A downloaded pack's ContentMeta.xbx holds each file's plain SHA-1 (every
Paris and Long Beach file matches), so with DLC on each pack's files are now hashed once at start and the log names
any that do not match ("... does not match its ContentMeta.xbx: a damaged or incomplete copy"); tested with one byte
of Paris.pak_stream changed. Smoke OK. Next: the player's log with this build, to know if their copy is damaged.

### 2026-10-24 - profiles from a real Xbox load

Report: an Action Replay profile (Jay "FNG") copied into hdd\E\UDATA\4d53004b was "damaged". The loader (0xA87B0)
compares the file's signature with one keyed by the title signature key, which the host leaves at zero (a real kernel
derives it from the certificate; Microsoft's keys are not shipped), so no real console's profile can match. An override
of XCalculateSignatureEnd (0x18BC16), for the loader's call alone (return 0xA8967), gives the comparison the stored
signature when it differs; the version and section-length checks still run. Headless: a profile with its signature
changed loads (log line "signature accepted"), Game Options show its KM/H. Smoke OK.
Also reported, next: Paris (DLC) stops on the game's fatal error "m_CompressionStream.msg" (a compressed file failed
to unpack); an Xbox controller adapter (XGUNNER, USB 1209:0001, a DirectInput gamepad) is not seen by SDL.

### 2026-10-23 - a media patched disc image

Harry's /home/harry/pgr2-variant.iso: a rebuilt XISO (volume at sector 0) of the PAL disc. Read in place (a read-only
XDVDFS lister, no extract): the same 11233 files, all byte for byte the same as the PAL disc's except the seven .xbe
files, each changed in one byte, 7D to EB: the jge after XapiVerifyMediaInDrive (0x193964 in default.xbe) became a
jmp, the usual media patch for running from a modded console's hard disk. The signature was not redone, so its SHA-1
differs (acd77633...), and the launcher refused it as unsupported. The patched code is XAPI start-up the host
replaces: both media patched default.xbe files (PAL, and NTSC computed with the same byte) are now in
GameInfo::same_code, launcher/game.json and the README. `GAME=/home/harry/pgr2-variant.iso scripts/smoke.sh`: SMOKE OK.
The released launcher (1.0.2) takes it from the next release.

### 2026-10-23 - the rear view mirror in ultrawide

Harry, 32:9: the mirror (hood and bumper views) stretched past its frame into the HUD. Headless with
FRAME_ASPECT_X100=356, Black (CAMERA_NEXT, from the controller layouts at 0x34CC1C) to the hood view, CAP draw lists
(the harness now lists each draw's viewport, sdk 6e84311): about 1000 draws in the viewport 220,38 200x70 on the frame,
their projection made for that box stretched to 32:9 (y/x 7.63), so the picture was right but the box too wide, while
the frame around it (2D) is squeezed. The box comes from the camera setup 0x1857F0, which src/game now narrows about the
centre for a box touching neither side, with the 16:9 aspect: 270,38 100x70, x scale 1.134 (2 x 0.566), clear of the
HUD in the capture. Smoke OK. Next: Harry checks the mirror and the pedals; release 1.0.3.

### 2026-10-23 - pedals on Z, Rz and a slider bind

Harry's Leo Bodnar BU0836-LC pedals report 3 axes, Z (clutch), Rz (gas) and a slider (brake) (joy.cpl), but only the
clutch moved in F1 > Wheel: Detect and Raw input looked at the first N of the eight axes (X Y Z for N = 3). sdk 9825bb8:
each device's axes are found by type (EnumObjects), Detect and Raw input look at those, and the log names them
(`3 axes (Z Rz slider 1)`). Smoke OK. Next: Harry binds the pedals; the mirror in 32:9 hood view.

### 2026-10-23 - release 1.0.2

Harry confirmed on Windows: the radio plays in races, and replays (the depth of field) are right. Since 1.0.1: the
radio, the real replay fix (1.0.1's never reached the GPU renderers), the PGR1 session's wheel review fixes. Notes in
docs/releases/v1.0.2.md, tag v1.0.2.

### 2026-10-23 - the radio was silent in races: sent to an effect alone

Harry: no radio while driving, in every build. Headless (XBE_DSOUND_LOG=2, new: every stream with packets): the radio
track opens (Radio\tracks\DJPerpetrator.wma) and its decoded PCM plays on stream B006E4F0 (2 ch, 44.1 kHz, packets
queued), mix bins 0 1 4 5 3 10 at -10000 and 24 (FXSEND 14) at 0: the DSP effects image is what plays it into the
speakers. The mixer dropped effect sends whenever a voice listed a speaker bin, audible or not: gain 0. sdk 6eb3947:
speaker bins count only when audible; with none, the sends play into the speakers (stereo kept). Now gain 0.50 per
side, as the menu music; voices with an audible speaker route (engine, ambience) unchanged. SMOKE OK. Windows to
confirm: the radio plays in races (dry: no in-car filtering, the effects still not modelled).

### 2026-10-22 - the replay depth of field fix never ran on Windows (corrected)

The PGR1 session's review of 48aa102: both renderers find a sampled texture's target by the read format's depth flag,
so the depth buffer read through LIN_A8B8G8R8 (a colour format) missed its target (kept under the depth key), fell
to guest memory (where the GPU renderers never write depth) and kTexDepthBytes never applied: 1.0.1 shipped with the
replay blur unchanged on Windows (its notes say fixed). sdk 34f1fd8: the lookup tries the depth key for the colour
formats DepthBytesOrder names; X8R8G8B8 read from depth gets alpha 1; the programmable path converts each stage right
after its fetch (a later texbem/texm3x stage reads the converted value); the reference renderer squeezes depth
tested 2D at the near plane as the renderers do. SMOKE OK, wincheck clean, shaders compile. Windows to confirm: a
replay or the attract demo with the car sharp; then 1.0.2.

### 2026-10-22 - release 1.0.1

Since 1.0: rigs of several DirectInput devices and H-pattern shifters, the ultrawide title menus at 16:9, the replay
depth of field, the MSAA render scale crash (sdk ab282ad), more diagnostics. Notes in docs/releases/v1.0.1.md; the tag
v1.0.1 builds and publishes the launcher (.github/workflows/release.yml). Harry confirmed the ultrawide menus.

### 2026-10-22 - replays blurred all over: the depth buffer read as colour

Reports (Harry's screenshot of the attract replay): the whole picture blurred, the car too. Headless (CPU renderer:
sharp) the frame ends with PGR2's depth of field: draw 10910 copies the back buffer 03D78000 (read as LIN_X8R8G8B8)
into 03595000, then draw 10917 blends over the frame with ONE / INVSRCALPHA reading the depth buffer 03B20000 as
LIN_A8B8G8R8 (0x3F). The harness draw lists now give each draw's depth target and each texture's data address,
which showed it. Both Windows renderers sampled any depth target as a shadow map (a depth comparison, alpha 1): the
blur saw every pixel as far. sdk: a depth target read through a 32 bit colour format (A8R8G8B8, X8R8G8B8, A8B8G8R8,
swizzled or linear) gives the console's bytes (depth's top byte in alpha, the stencil byte 0); depth formats still
compare. The HLSL checked with DXC (-HV 2018), wincheck clean, SMOKE OK. Windows to confirm: a replay or the attract
demo with the background soft and the car sharp.

### 2026-10-22 - ultrawide menus stretched: the title screens draw their 2D with depth on

Harry's log (the squeeze notes): every frame has 2D on the back buffers W-buffered (ZENABLE 2) with LESSEQUAL, which
the squeeze took for depth tested glows and left wide. Headless: the title screen and its menu draw all their 2D that
way (vs 144, z=2 zf=203), Kudos World Series and deeper with depth off: hence stretched at the start, right deeper in,
stretched again back at the start. Their vertices sit at rhw 0.5 to 1 (w 1 to 2), light glows at rhw about 0.01.
sdk: a depth tested pretransformed draw is squeezed where its vertices have rhw 0.25 or more (the vertex shader, hud.z);
the squeeze log says each target and outcome once. Generated vertex shaders checked with DXC, SMOKE OK. Windows to
confirm: the title and its menus at 16:9 in ultrawide, glows still on their lamps.

### 2026-10-22 - ultrawide menus stretched after going back (investigating)

Harry: in ultrawide the menus are stretched at first, kept at 16:9 deeper in, and stretched again after going back to
the start (the same Kudos World Series screen both ways). Headless, the screen's draws are the same both visits: 31
pretransformed draws, depth off, on the back buffers (03D78000 and 03EA4000 alternating), then PGR2's own two-draw copy
to 03C4C000; the squeeze's conditions (pretransformed, not depth tested, a 480 line target) hold both times. So the
difference is in the Windows renderers' targets. sdk: the release log notes each target's squeeze decision when it
changes (`ultrawide: 2D on target ...`: size, host width, stretch, depth state, squeezed or why not); the harness draw
lists give the depth comparison. Next: Harry's log from going in, back and in again.

### 2026-10-22 - wheels: any DirectInput devices, several at once; H-pattern shifters

Harry's rig was not found: a Simucube OSW base, Leo Bodnar BU0836-LC pedals, an SLI-PRO button box and a Thrustmaster
TH8A shifter, four USB devices, none a Logitech. The wheel code took the first "driving" device (or one named wheel or
racing) and read everything from it. sdk core/wheel.cpp now opens every DirectInput game controller but Xbox pads
(their device path names IG_; SDL keeps them), logs each, and reads each control from the device its binding names
(settings: { device, guid, axis or button, rest, full, invert }, older files still read); bindings are made in F1 >
Wheel by turning, pressing or moving the control on any device. The forces go to the steering's device and axis
(exclusive there, the others read in the background). The game threads never read the settings' strings: the
settings window hands the wheel thread a copy (WheelSetup). An H-pattern shifter (Reverse, 1 to 7) presses the game's
shift inputs until the car's gear matches, when the game reports the gear (GameInfo::wheel: WheelFeel.gear,
shift_up/shift_down). PGR2's gear and its manual shifts found headless (FINDINGS, the gearbox): the byte 0x29DACA +
car x 0x6990, B up and X down, only in manual. The harness pad scripts gained BLACK and WHITE.
wincheck clean, SMOKE OK; nothing of the wheel code runs headless: Harry's rig to try.

### 2026-10-22 - shared scripts in the SDK; the SDK on the fork's main

The fork's main fast-forwarded to pgr2-xdk5659 (now frozen) and the 11 merged task branches deleted; this repo's
submodule tracks main (.gitmodules branch). The generic scripts moved into sdk/scripts (harness, smoke, soak,
linktest, wincheck, shadercheck, recompile, linux-tools, Ghidra helpers, framelog, wavstats); scripts/ here are
one-line wrappers and scripts/game.env holds PGR2's values (log, target, Ghidra project, smoke's menu window, the
soak pad generator scripts/soak_pad.py, the link pads). savetest.sh and the pads stay here. The SDK's game template
gained the launcher files, launcher.bat, the release workflow, the scripts and CLAUDE/PROGRESS/FINDINGS skeletons.
Checks through the shared scripts: SMOKE OK, LINKTEST OK, SAVETEST OK, a one minute soak with no traps, wincheck clean.
Then all PGR1 material left this repository (PGR1.md, the plan, references): it is its own project.

### 2026-10-22 - release 1.0

The repository is hk21x/pgr2-recomp now (the old name redirects): game.json, README and WINDOWS.md use it. README:
Getting Started leads with the launcher (download from the release, choose the disc, Build, Play, Update, where its
files go), building from source second; the wheel, launcher and DLC lines moved to their sections; 64 achievements.
.github/workflows/release.yml builds PGR2Launcher.exe on GitHub's Windows machines (Visual Studio's clang): an
artifact on pushes to main that touch the launcher, and the release with the exe on a tag v*, its notes from
docs/releases/<tag>.md. Both runs green; release v1.0 published with PGR2Launcher.exe (2 MB):
https://github.com/hk21x/pgr2-recomp/releases/tag/v1.0

### 2026-10-22 - the launcher works end to end on Windows; the icon

Harry: the launcher built the game with the portable kit and launched it ("Launched and working"). An icon for both:
launcher/icon.ico (the launcher's CMake already embeds it); the game's CMakeLists now embeds it too as resource 1,
which the window class loads. Harry's drawing (.local/win/pgr2-launcher.icon, an SVG of his own) is launcher/icon.svg;
launcher/icon.ico is rendered from it with cairosvg at 256 (PNG), 128, 64, 48, 32, 24 and 16 px (ImageMagick's own
SVG renderer dropped its gradients and clipping).

### 2026-10-22 - launcher, first run on Windows: the kit works, two SDK header installers were missing

Harry's run: every tool downloaded, verified and unpacked, the SDK's 8 installers extracted, the source cloned,
CMake configured with the portable clang (19.1.7, the MSVC 14.44 libraries, SDK 10.0.26100), and the build ran to
step 153 of 425 before SDL's hidapi wanted devpropdef.h (and SDL's checks had not found d3d11.h, dinput.h, dsound.h).
The SDK's architecture neutral headers are in the "x86" installers (Desktop Headers x86: dinput.h, dsound.h; OnecoreUap
Headers x86: devpropdef.h; found by searching each installer's tables for the names), the x64 ones hold only the few
x64 specific files. Added both to toolchain.json (fetched by the launcher each build); the SDK's stamp now names its
installers, and the launcher rebuilds CMake's cache when the toolchain changes (SDL's failed header checks were
cached). Libraries were complete (Store Apps Libs: kernel32, d3d12, dxgi, xaudio2 ...; Desktop Libs x64: dinput8 ...).

### 2026-10-22 - a launcher for players: choose the disc, Build, Play

Harry asked for one exe a non-technical player runs: choose their ISO, it does the rest (portable tools, source from
GitHub). sdk launcher/ (generic, a game supplies launcher/game.json and toolchain.json; PGR2's are here):
- Tools into PGR2\tools beside the exe, nothing installed, no admin: LLVM 19.1.7's Windows release (tar.xz, the same
  clang as Visual Studio 17.14's; xz-embedded decodes it as it streams, only the compiler, linker and their DLLs kept,
  about 680 MB), CMake 3.31.12, Ninja 1.13.2, MinGit 2.56.0, Microsoft's C++ libraries 14.44 (four vsix packages from
  the Visual Studio manifest) and the Windows SDK 10.0.26100 (eight .msi files and the .cab files they name,
  extracted with msiexec /a). Every file checked by SHA-256 (pinned, or the manifest's). About 1.4 GB to download.
- The game: an .iso (read with the runtime's own disc reader) or a folder, accepted by the xbe's SHA-1 (PAL or US;
  the two differ only in the xbe header's first 0x370 bytes, the code is the same). play\assets is a junction to the
  folder or a hard link to the image (a copy across drives); the xbe goes into source\assets for the build.
- Source: git clone (MinGit) of this repo's main with the sdk; Update when origin moves on.
- Build: the developer prompt's INCLUDE/LIB/PATH set by hand, cmake --preset release -DCMAKE_LINKER_TYPE=LLD, ninja's
  [n/m] as the progress bar; the exe and DLLs copied to play\. "Remove the build files" deletes source\out.
- Window: Dear ImGui on Direct3D 11 (WARP without a GPU driver), three steps (game, build, play), a details log.
  Microsoft's license (Build Tools) must be accepted before the first build.
Tested on Linux with launcher_cli: the plan resolves from the live manifests (6 tools), ninja/cmake/git/msvc/llvm
download, verify and unpack (llvm 806 MB in 76 s, the MSVC libraries complete: msvcrt/libcmt/msvcprt/libcpmt), both
ISOs and the assets folder are recognised and linked, the source clones with the sdk and the xbe is put in place.
Windows sources syntax-checked against mingw headers. Not tested: anything only Windows runs (msiexec, WinHTTP, the
window, the compile with the portable kit). launcher.bat builds PGR2Launcher.exe with Visual Studio.
Next: Harry builds it with launcher.bat and runs it from an empty folder; launcher.log back.

### 2026-10-22 - "damaged content package" with DLC off

Harry: "The following additional content package(s) are damaged ... PGR2 Long Beach Booster Pack". The built car pack
(T:\$C\4d53004b0000FF01) had category 1 and a ContentMeta.xbx taken from the Long Beach pack (its name too). The
disc game lists category 1 packs whatever the setting, so with DLC off it checked the pack's signature (not
bypassed then) and called it damaged. Reproduced headless (category 1, DLC off, START: the screen); fixed: the pack
is category 0x20, which the car pack listing takes only with DLC on (mask 1 | 0x20), and is named "PGR2 DLC Cars
(title update)". The stamp changed, so the pack is made again at the next start with DLC on. Headless: DLC on, the
pack loads; DLC off, straight to the title menu. Smoke OK. Confirmed on Windows by Harry the same day.

### 2026-10-22 - stutter, second pass: the fence after a report no longer waits

Harry's log with the last-count answer: slow frames down from 75-272 to 1-7 per 10 s, but the title still waited
about 9 ms a frame in "flush" for 20 s spans. Headless, PGR2's reports walk a ring of 4096 slots (0x80FF0000 up, about
three a frame after the first second), so a cache by address missed until the ring wrapped (and then answered with
another test's count). And the flush came from Swap (0x1C55D0 calls D3D_SetFence every frame), not from a read:
the emulation flushed on any fence after a report was asked for. PGR2 reads its tests later. sdk 1d264f6: such a
fence kicks the render thread (Backend::Kick: its Flush op, the title not waiting; reports written once the GPU is
past them); the address cache is gone. XBE_REPORTS_EXACT=1 waits as before. Smoke OK. Next: Harry's XBE_PERF log:
"flush" near 0 throughout; the sun flare and light glows still hide behind buildings.
Result (Harry, XBE_PERF, same race): "flush" 0.00 ms in every second; slow frames 0-3 per 10 s (the race load, a
93 ms fence wait right after it, odd single frames). The render thread's host time is 10-14 ms of 16.7 (bind about
4 ms): the next limit, as steady "next frame" waits, if a heavier scene goes past it. Flares to confirm.

### 2026-10-22 - the remaining micro-stutter: a visibility report read waited for the render thread

Harry's XBE_PERF log (race, D3D12, 3440x1440, draw distance 200%): 60 fps on average, but in most 10 s spans 75-272
frames over 25 ms; the title thread waited 4-9 ms a frame on average in "flush" (0.5-0.9 waits a frame); host
renderer 9-12 ms a frame on the render thread, no shader or pipeline compiles. Headless (no render thread) PGR2 asks
for one visibility report a frame (0x80FFF000: it resets it to pending, time 0 count 0 status FFFFFFFF, then
EndVisibilityTest) and reads it straight after; the null backend writes it at once. On Windows the report sits
behind the render thread's queue, so the read's D3D_SetFence flushed: the title waited for the frame so far, about
9 ms, every other frame: 26-30 ms frames, then a short one. sdk: the backends note each report's count as they write
it (gpu::NoteReport) and xdk/d3d8.cpp answers a report at once with the last count for that address (one frame late:
the sun and light occlusion), the GPU's own value written over it later; the first report still waits.
XBE_REPORTS_EXACT=1 keeps the old wait. Smoke OK, wincheck OK. Next: Harry's XBE_PERF log again: flush waits near 0,
the slow-frame lines near 0; the sun flare and light glows still fade behind buildings.

### 2026-10-22 - micro-stutter since the wheel support: DirectInput off the game's threads

Harry: trees, DLC setting, DLC cars, boats and system link work on Windows; no flicker; but the game micro-stutters
badly since the 2026-10-21 changes. Of those, only the wheel support put new work on a path the game waits on:
controller 1's read (input::Poll, from the game's thread) called WheelRead, which with no wheel plugged in ran
DirectInput's EnumDevices every two seconds (tens to hundreds of milliseconds on a PC with many USB devices), and with
one re-acquired it and set its forces over USB (up to 60 calls a second). The point sprites only add constants (no new
pipelines). sdk 7c0bccd: a wheel thread does every DirectInput call (looking for a wheel, reading it every 2 ms,
forces); the game's read copies the last state. Also new: a release log line every 10 seconds that had slow frames
(over 1.5 frame periods), "frames: N of M in the last 10 s over X ms ..., the longest Y ms", to measure stutter.
wincheck OK, smoke OK (the harness has no wheel code). Next: Harry to rebuild and send pgr2.log after a race.

### 2026-10-21 - the title update's 16 cars, as a disc car pack

The disc game loads car packs (category 1, 0x157180: cars.ini, CarColours, Classes, Manufacturers from the pack).
src/game/dlc.cpp builds one, T:\$C\4d53004b0000FF01, from the title update's folder (uPAL or uNTSC copied into $C) on
the first start with DLC on: the 18 models converted from the update's pak layout to the disc's (ConvertPak: same
chunks, an INDX table instead of inline headers; byte-identical chunks checked), every car ShowroomFree, a city pack's
ContentMeta.xbx with offer 0xFF01 and category 1, a stamp so it is made once. After the loader, 0x157180's hook
copies the models into Z:\Cars and links the handling (D:\CarDynamics) and engine sounds (D:\data\audio).
The first load crashed in CarColours.ini: the colour table (0x32F1F0) holds 600, the disc has 566 and the update 48;
past the cap 0x14A8C0 returns null and 0x14E07C writes through it. The builder trims the update's own colours to the
free slots (34), taken in turn across the cars; colours a DLC car shares with the disc cost nothing (67 of 84 colour
choices kept). The other tables fit (classes 16 of 20, cars 118 of 200).
Evidence (headless, KEEP_HDD, PGR2_SKIP_INTRO=1): "car pack T:\$C\4d53004b0000FF01 loaded"; memory: 118 cars, 600
colours; Single Player shows "Car Challenges 0 / 118"; PGR2_CAR=102 (M3 CSL), 105 (356A), 111 (Atom 2), 117 (SR3)
each race Florence with the right model, no faults. The Paris boats are now converted too, not Stockholm's.
Time Attack > Car Challenge lists the classes "Paris Cars" and "Long Beach Cars" above Ultimate, 8 cars each, none
locked; the car screen opens for them (Subaru 22B STi on Hong Kong's Cotton Tree Drive). In the M3 CSL's race the
game opens DContentCar13.ini/.pak_hrd from the pack and M3CSL.xwb / M3CSL_PCM.dat through the D:\data\audio links.
New test switch PGR2_CAR=N (player 1's car, 0x39B318).
Next: Harry to try the cars on Windows (WINDOWS.md): the showroom, engine sounds, the colours.

### 2026-10-21 - steering wheel support with force feedback

Base: Logitech G920/G923, force feedback wanted. SDL could not do it as set up: its DirectInput force feedback needs
its DirectInput joysticks on, which (XInput being off) take the Xbox pads from Windows.Gaming.Input and merge their
triggers; its HIDAPI Logitech driver covers G29/G27/G25 but not the Xbox wheels. So sdk core/wheel.cpp opens the first
DirectInput driving device (or one named wheel/racing) itself: exclusive in front, axes -32768 to 32767, the wheel's
autocentre off; spring, damper, constant force and sine effects on X. input.cpp merges it into controller 1 (its own
button map; Back + Start works), leaves SDL wheels alone, and feeds it the game's rumble; GameInfo::wheel gives the
feel (src/game/wheel_feel.cpp: racing from 0x38384C, 0x383814 and the lap, speed from 0x298D50 + car x 0x6990).
Forces: spring 0.25 in menus, 0.15 + 0.65 x speed/40 m/s racing; damper 0.10-0.22; rumble as a sine (60 ms low motor,
25 ms high), x road; all x strength. Settings (pgr2.toml, F1 > Wheel): steering axis and invert, rotation 900,
lock 360, dead zone, pedals [axis, rest, full] with Detect, ffb_strength 70, ffb_road 50, wheel_buttons (G920 order:
A B X Y RB LB Menu View RSB LSB). Defaults: steering X, throttle Y, brake Rz, clutch slider 1, pedals released at the
top. wincheck OK, smoke OK; no wheel here, so untested: Harry to try (WINDOWS.md). Next: self-aligning torque from the
car's slip once its physics values are found.

### 2026-10-21 - display modes and race_60fps confirmed; wheel support next

Harry: display modes and resolution changes mid-race work; races at 60 work, replays and ghosts included. Trees and the
DLC setting to check tomorrow. Next: steering wheel support, the Logitech G920/G923 as the base, force feedback wanted.

### 2026-10-21 - trees: point sprites

Harry: every tree is bare (the DLC aside): Sydney's terrace street has leafy trees on the Xbox, none here. The foliage is
point sprites: headless, every point list draw had D3DRS_POINTSPRITEENABLE 1, a vertex program writing oPts
(438.7 / w + 8, console pixels), size min 2, max 64, a texture on stage 0 and no texture coordinates from the program;
both renderers drew each point as one console pixel. sdk: the vertex shaders pass oPts on (TEXCOORD7; 0 under the
fixed function, which takes D3DRS_POINTSIZE), DrawConstants::pts carries sprite on, min, max and the render state's
size, and PointHlsl makes a sprite a square of that size, every stage's coordinates 0 to 1 across it. The CPU renderer
does the same (two triangles a point, never culled). Generated HLSL checked with DXC (-HV 2018) for the geometry, fixed
function, program and pixel shaders. Headless Sydney route 119 (PGR2_ROUTE): leafy trees by the terraces. Smoke OK.
Windows to confirm (D3D12 and D3D11).

### 2026-10-21 - DLC cities work on Windows; a setting

Harry: Paris and Long Beach both race, the left out objects are hardly noticeable. The DLC switch is now a game option
(`dlc` in pgr2.toml, F1 > Game "Xbox Live DLC cities", next start; PGR2_DLC=1 still sets it). WINDOWS.md has the steps
and the differences from the console; README lists it. The headless Paris streaming crash (3 of ~12 runs, before the
boat and bucket fixes) was likely the same garbage meshes; not seen since. Next: steering wheel support.

### 2026-10-21 - DLC: Paris turn 1 crash, the draw list's material buckets

Harry: the magenta is gone (four error material draws left out, vertex shader B0101C91), then a crash at Paris turn 1:
a write to guest 0 on the game's main thread in 0x11B210, called from the city's render (0x117C40). 0x11B210 sorts the
city's draw list (8 byte entries from +0x7750, count +0x1EF24) into 106 material buckets whose write pointers are the
table 0x457188-0x457330; an entry's bucket is the byte at +2 of its material. A title update material's bucket past
the table read a null pointer from beyond it. With PGR2_DLC=1 such entries are now taken out of the list first
(logged with their bucket number). Not reproduced headless (the pad car never reaches turn 1). Smoke OK.

### 2026-10-21 - DLC: Paris runs on Windows; PGR2's error material left out

Harry: with the boats swapped Paris no longer crashes, but a big magenta triangle remains (race capture 214426). Draws
3655 and 3656 (vertex shader B0101B41, pixel shader B01020A0, texture 81B57000, as 3781-3784 in the flyover capture)
have pixel constant c0 = 0x00FF00FF: PGR2's error material, which the engine uses for an object it cannot set up (here
more title update meshes inside the city paks); the following fixed function draws only inherit the constant. With
PGR2_DLC=1 draw_filter.cpp leaves out a draw that has a pixel shader and c0 exactly 0x00FF00FF (logged, first four).
Not exercised headless (the pad race never draws one; a Hong Kong race draws none). Windows to confirm.
The packs that vanished from the harness disk twice: scripts/smoke.sh (and any run without KEEP_HDD) starts from an
empty disk; the game deletes nothing (deletions are logged now and none was).

### 2026-10-21 - DLC: the magenta sky and the Windows crash were the packs' boats

Harry's D3D12 capture of the Paris flyover (16:9): the sky draw (2847, vs B0101BE1, its 1024x512 DXT1 panorama) is
fine; draws 3781-3793 have garbage vertices (-2.6e14, 1.5e-33, 197554 ...) and pixel constant c0 = (1, 0, 1), huge
triangles over half the screen (magenta, white under the letterbox's tint). The CPU renderer drops non-finite and huge
triangles, so headless looked right. The packs' boat01.pak and boat02.pak are not in the disc's object pak layout
(an INDX chunk index, then WMSH, MAT, GPUD; theirs open with a zlib compressed WMSH chunk, the title update's layout):
the disc build reads them as garbage meshes, and on Windows the process died 2 to 4 s into the flyover with no log
line (the render thread reading those vertices, likely). dlc.cpp now puts the disc's river boats in their place
(StockholmBoat1.pak, boatBig.pak from D:\Cache\Objects) when a pack's object pak does not open with INDX.
Headless: both swapped, the Paris flyover and race clean. Smoke OK. Windows to confirm.

### 2026-10-21 - DLC on Windows: Paris in the menus; night races; fatal errors logged

Harry (Windows, PGR2_DLC=1 from PowerShell with $env:): Paris shows in Time Attack and Instant Action; the flyover
shows Paris with the right two thirds of the sky magenta (3440x1440, aspect screen, D3D11, MSAA 8x, render 64%,
draw distance 200%), then the game stops.
- PGR2's fatal error 0x1388F0 is now logged with its message and caller (`pgr2: fatal error "..." from ...`); it
  formats the cache partition and shows "There's a problem with the disc you're using" (seen headless).
- A night race in Paris stopped the game: the packs hold the day set only (Paris.pak_day, .pvs_Day) and the area
  loader (0x116D7B) asks for .pak_night: missing night and overcast sets are now linked to the day set (races run in
  daylight). PGR2_ROUTE no longer touches the attract demo (+0x6D4), which picks its own city (forcing it loaded
  Edinburgh's paks against a Paris route: "BizFile::SkipBytes SetFilePointer").
- File deletions are logged (`files: ... deleted`): the packs vanished from TDATA once headless, cause not seen again.
- Magenta sky: the CPU renderer draws the flyover's sky right at 4:3 and 2.39 with 200% draw distance; no magenta in
  our GPU code, so a draw the GPU renderers treat differently (alpha test or blend of a pink keyed texture is a guess).
  Needs Harry's A/B (D3D12, MSAA off, 16:9) and a D3D12 F3 capture.

### 2026-10-21 - DLC: Paris and Long Beach race on the disc build (PGR2_DLC=1)

With Harry's go-ahead to pass over the content signature checks, src/game/dlc.cpp (all behind PGR2_DLC=1):
- XFindFirstContent (0x192134): the city pack listing (mask 2) also takes category 0x10, the booster packs'.
- 0x191366 (XAPI: a content header's keyed signature) answers with the header's own, and PGR2's per-file check
  0x156F00 passes: the packs are unsigned until a console's installer signs them.
- After a city pack loads (0x1572F0): its plain .pak files (Paris's boat01/boat02, river boats the city loads as objects
  from Z:\Cars, where the disc's are copied from D:\Cache\Objects at boot) are copied to Z:\Cars; its .ini and .xwb
  files the disc has no file of that name for are linked into D:\data\audio (ParisRefs, ParisMono, Paris_dry...); its
  stereo.ini (the city's ambience lines) is added to the disc's in T:\$PGR2\stereo.ini, linked over
  D:\data\audio\stereo.ini (Audio::Manager::Setup stopped the game, 0x12C1DF, with no line for the city).
- sdk: symbolic links match the longest prefix (a link to one file wins over its drive's); kernel::GameFileExists;
  XBE_FILES_LOG=1 logs every open and miss; the harness logs a crash's address and backtrace (run.out).
- PGR2_ROUTE=N (test, hooks.cpp): every race on route N, written into the event (+0x10) before race setup 0xF14E0,
  with the route 0x383180 and its city 0x38317C. New routes: Paris 146-153 (city 17), Long Beach 154-162 (city 18).
Headless (second boot with the packs on the disk): 19 cities and 163 routes; Paris raced (snap: its boulevards, the
start gantry, the Paris radio stations and river ambience), the attract demo too; Long Beach raced twice, clean.
Paris crashed 3 times in about 12 runs, about 30 s into the race, always the same place: the streaming thread's
read completion (0x1362F0 -> 0x1004A0 -> 0x1003E0 -> zlib inflate 0x136E70 / 0x1D33C9) with a block source
far past physical memory; not seen in 4 runs with a diagnostic logging the slots, so timing dependent. Not found yet.
Not done: the cities in the game's menus (only reached here with PGR2_ROUTE), the DLC cars (title update).

### 2026-10-21 - DLC city packs on the disc build: why they are refused

Traced headless ([xbe] trace, not committed) with both packs in hdd/E/TDATA/4d53004b/$C:
- PGR2 lists packs with XAPI's XFindFirstContent (0x192134) three times: category 0x80000000, 1 (car packs, loaded by
  0x157180) and 2 (city packs, 0x1572F0: cities.ini and Routes.ini from the pack, 0x1574F0 lists both kinds). Its
  metadata reader 0x191F83 needs ContentMeta.xbx's XCSF magic, flag bit 0 and the category (header +0x30) in the mask:
  both booster packs are category 0x10, which the disc build never asks for, so it returns 0 for each.
- With the copies' category set to 2, both packs are listed and the city loader runs, but 0x191B03 (XAPI: open the
  pack's metadata, 0x19199B) returns null: 0x19144B compares a keyed signature of the header (0x191366) with the
  header's first 20 bytes, and these packs have zeros there (the community installer signs them on the console at
  install time). Past that, 0x156F00 checks every file's keyed signature (XCalculateSignature, 0x155510) against the
  metadata's file table (0x191BA2).
So the disc build would load the two cities only if the packs were signed as a console's installer signs them, or
those checks were passed over; stopped here for Harry to decide (the permission check refused a hook that passes over
the signature checks).

### 2026-10-21 - Xbox Live DLC: what Harry's packs are and what the disc game does with them

Harry's DLC is a community installer (a CDX menu XBE, not needed) around `content/`: two packs for T:\$C
(4d53004b00000001 Paris, 4d53004b00000002 Long Beach: city paks, Cities.Ini, Routes.Ini, CarKeys.Ini, ContentMeta.xbx)
and a title update for T:\$U in PAL and NTSC builds (uPAL, uNTSC): default.xbe version 0x202 (PAL; NTSC 0x205),
built 2004-05-21, 16 KB larger than the disc's version 2, with the 16 new cars (dcontentcar*.pak_*, .dat, .xwb), a
new cars.ini, races.ini, classes.ini, manufacturers.ini, credits and downloader.xbe. Nothing is committed; the files
stay in /home/harry/dlc.
- The disc XBE has content pack support (T:\$C\%08X%08X, each pack's Content.txt, cars.ini, CarColours.ini,
  Manufacturers.ini, Classes.ini, Routes.ini, cities.ini) and checks t:\$u\contentmeta.xbx at boot (on an Xbox a
  title update there is launched in place of the disc's XBE). Only the update reads T:\$U (cars, races, credits) and
  CarKeys.Ini.
- Headless with both packs in hdd/E/TDATA/4d53004b/$C: the disc game opens each pack's ContentMeta.xbx three times
  and reads nothing else from it (a failed signature check, or a pack that asks for the update).
- The update XBE: XDK 5659 for every library, but its D3D section is 0x50 bytes larger (a patched D3D8); the scan finds
  144 of 250 functions and 8 of 13 data (the disc: 191 and 13). Every game address moves.
Options for Harry: (a) the disc build with the city packs (why they are refused, then Paris and Long Beach without the
new cars); (b) the update recompiled as a second image, run as the Xbox runs it (the cars, the cities, the update's
fixes), which means mapping its XDK functions and moving every game hook and the achievement map to its addresses.

### 2026-10-21 - achievement pop-up confirmed on Windows

Harry: the pop-up shows (the test achievement at a race's start). So the earlier unseen pop-ups were unlocks already
in achievements.toml. Still to see in real play: the five new ones (Style Points the easiest).

### 2026-10-21 - achievements: the last five mapped, all 64 live

An agent traced them (Ghidra and headless races; ACHIEVEMENTS.md has the map): the race object from 0x383178 (type,
group, difficulty 0x383868), the race's Kudos 0x39D3DC (what the HUD shows), the result at a race's end (success
0x383A58, results done 0x383984, both from 0xF2650), the clean-race byte 0x39D550 (cleared by the contact handler
0xF0C50), the Overtake events (type 17) and the 92 Time Attack circuit records (0x3609C0 + route x 0x50). Added Style
Points, Showboat, Traffic Control, Untouchable, Lap of Honour. sdk harness: a `poke` file in the run folder writes
guest memory at the next frame (gdb cannot attach here, ptrace_scope 1). Headless: writing 2,500 Kudos, a Platinum
on Overtake event 8, ten route records, then an Expert Kudos World Series win with the clean byte, unlocked Style
Points, Traffic Control, Lap of Honour and Untouchable; Showboat stayed locked; the smoke race unlocks nothing.
wincheck OK, smoke OK. Not seen live: a real finish (the pad car cannot win), so Harry's play confirms them.

### 2026-10-21 - Windows: under-car flicker fixed, PC Settings entry confirmed; pop-up unseen

Harry: the flicker under the car is fixed (with the shadow nudge off since W-buffering), the PC Settings entry works.
The achievement pop-up has not been seen. The unlock path reads right (Frame after each Present, Recent read by
DrawHud, the same ImGui path as the fps counter, which shows); likely nothing new was unlocked: unlocks already in
achievements.toml never pop again. To check on Windows: achievements.toml and the F1 Achievements tab, and
PGR2_ACH_TEST=1 (a test achievement at a race's start). Mapping the last five achievements is under way.

### 2026-10-21 - Direct3D 11 complete; Renderer in the settings window

Harry: MSAA 2x/4x/8x under D3D11 works, with render scale changes too. The renderer is now a choice in F1 > Graphics
(Direct3D 12 or 11, next start) and in the README; WINDOWS.md lists it as confirmed. A PC whose GPU cannot start
Direct3D 12 could not reach F1 to switch: when the chosen renderer's Init fails, d3d8.cpp now drops it and starts
Direct3D 11 (`CreateFallbackBackend`; the log says so). Smoke OK.

### 2026-10-21 - Direct3D 11 step 3: MSAA

MSAA under D3D11 as under D3D12: an eligible colour target (at least 320x240) and the depth drawn with it get
multisampled twins that draws go to. Settle resolves a dirty twin before anything reads res (colour:
ResolveSubresource; depth: a pass writing each pixel's first sample from a Texture2DMS view), Refresh refills a stale
twin from res (colour: blit; depth: the depth copy pass), Wrote marks the twin behind when res is written by an
upload, a copy or a partial clear; whole clears clear both. Changing antialiasing settles and drops every twin; so does
a render scale or shape change. Occlusion counts divide by the samples. wincheck and -Wall clean, smoke OK. Next:
Harry tries MSAA 2x/4x/8x under D3D11, with a render scale change while it is on.

### 2026-10-21 - Direct3D 11 step 2 confirmed

Harry: every check matches D3D12 (night street lights, car selection, car shadow, mini map, ultrawide HUD bars,
render scale mid-race), the Hong Kong turn 3 spot is fine on D3D11, and D3D12 works as before after the d3d_common
move. Next: step 3, MSAA under D3D11.

### 2026-10-21 - Direct3D 11 draws on Windows

Harry's first step 2 run was black: build.bat never updated the SDK submodule, so `git pull` built the step 1
renderer. build.bat now runs `git submodule update --init sdk`, and the D3D11 renderer logs `d3d11: drawing renderer`.
After updating, Harry: "D3D11 working". Next: his checks (night lights, car selection, mini map, render scale,
ultrawide), D3D12 still the same after the d3d_common move, and the speed at Hong Kong turn 3 on both; then step 3
(MSAA, captures).

### 2026-10-21 - Direct3D 11 renderer, step 2: drawing

First the renderer-independent half of d3d12.cpp moved to `sdk/runtime/gpu/d3d_common.{h,cpp}` (a move: state
translation, PsoKey and BuildPipelineKey, shader classification and keys, every Fill*Constants, StageSampler, the
blit/depth HLSL, guest rect lists, XStretch, HudSqueeze; the D3D12 members that read backend state now pass it in and
stay as one-line wrappers). Every line removed from d3d12.cpp was checked to reappear unchanged apart from those
signatures. Then d3d11.cpp got the console path on top of it: targets (colour BGRA, depth R24G8 with DSV and SRV,
smaller depth targets, CPU write reloads), guest textures (2D, cube, volume, P8 palettes, BC padding, cube faces from
targets), the generated shaders with input layouts, blend/depth/rasterizer state per PsoKey, the two constant buffers
updated only on change, dynamic vertex and index rings, viewport and scissor, points through the geometry shader,
W-buffer depth, clears (whole: Clear*View; rects: a full screen pass per rect), CopyRects (region copy or drawn),
depth copies, Sync read-backs, NOOVERWRITE locks (staging copies collected without waiting, delivered from the CPU),
occlusion queries for visibility tests, gamma, FXAA, render scale and aspect changes, ultrawide stretch and HUD squeeze.
Not yet: MSAA (logged), F3 captures, the 8 bit target swizzle (logged when PGR2 samples one), the perf report.
wincheck clean, d3d11.cpp and d3d_common.cpp clean with -Wall -Wextra, smoke OK. Nothing here has drawn a pixel: Harry
tests on Windows (menus, a race, render scale, ultrawide), then compares speed at Hong Kong turn 3 with D3D12.

### 2026-10-21 - Direct3D 11 step 1 confirmed on Windows

Harry: F1 works, black screen, as expected. Log: `D3D11 adapter: AMD Radeon RX 7900 XT, feature level B100`,
settings window opened and closed, clean exit. Next: step 2, drawing.

### 2026-10-21 - North American release: same code; Windows build fix for D3D11

Harry's NTSC disc (PGR2_USA.iso, a whole disc dump: game partition at 0x18300000). A full extract-xiso filled /tmp
("write error: Success"), so only default.xbe was read out (to assets/us/). It is the PAL file's size and differs in
531 bytes, all in the signature and certificate (region 3 vs 4, version 5 vs 2, keys, time stamp): no code or data
byte differs, so no per-release address table is needed. `GameInfo::same_code` lists its SHA-1, which skips the
"different release" question; the harness now boots from a disc image (`GAME=path.iso scripts/harness.sh`), and
`GAME=/home/harry/PGR2_USA.iso scripts/smoke.sh` printed SMOKE OK (menu, Instant Action, race), as did the PAL smoke.
Open: saves are signed with the certificate's key, which differs, so a PAL save may not load in NTSC and vice versa.

Harry's Windows build of D3D11 step 1 failed: d3d11.cpp included windows.h without NOMINMAX and the Windows SDK's
min/max macros broke x86_ops.h. wincheck missed it (it defined NOMINMAX itself, and mingw's headers leave min/max
alone in C++); it now defines only what the real build does and fails any file that includes windows.h without
NOMINMAX before it (memory.cpp and threads.cpp got it too). Fixed, with the overlay's `100%` format warning.

### 2026-10-21 - Direct3D 11 renderer, step 1: plumbing

Harry asked for DX11. Plan in three steps: (1) the setting, the device, the swap chain, presenting and the F1 window;
(2) drawing: targets, textures, the generated shaders, input layouts, constants, blend/depth/stencil, render scale,
W-buffer; (3) read-backs, copies, cube reflections, visibility tests, gamma, ultrawide, MSAA. Step 1 done:
`renderer = "d3d11"` in pgr2.toml (settings file only until it draws; applies at the next start) makes
`CreateBackend` return `sdk/runtime/gpu/d3d11.cpp`: a D3D11 device on the high performance adapter (11_1 or 11_0),
a flip-model swap chain with tearing when vsync is off, resizing, and ImGui's DX11 backend for the counter and the
settings window. Draws and clears are dropped; copies run on guest memory and visibility tests answer at once with no
pixels, as the headless renderer does, so the game runs but the screen is black apart from the overlay. The log says
`D3D11 adapter: ...`. Syntax-checked against the Windows headers (wincheck), smoke OK (the headless path is
unchanged). Next: Harry checks it starts, F1 works and a race runs blind; then step 2, sharing the renderer-independent
parts of d3d12.cpp (constants, shader classes, HUD squeeze, stretch) rather than copying them.

### 2026-10-20 - repository public

Harry made hk21x/pgr2-decomp public. Checked the whole history: 56 paths ever committed, all code, configs, docs and
pad scripts; the only binaries are the two README images (screenshots); no blob over 500 KB; no game files, generated
code, captures or tokens. WINDOWS.md no longer says the repository is private.

### 2026-10-20 - ultrawide HUD bars: corners on the edge stay there

Harry: performance much better; in ultrawide a street race's text sits outside its grey bars. The bars are drawn in one
draw with the right hand bar, so the draw spanned the frame and was left unsqueezed as a full screen pass, while the
text (its own draws) was squeezed. Now each corner decides: one on the frame's left or right edge stays, the rest are
squeezed (D3D12 vertex shader, the CPU renderer the same), so bars reach from the edge to past their text and full
screen passes keep their width without a per draw extent check. Headless preview at 2.39: the POSITION / LAP TIME,
Kudos and LAP bars run from the edges to just past their text. Smoke OK.

### 2026-10-20 - race_60fps: trimming the render thread's per draw cost

Harry's step times at the spot (5800 draws): assemble 3.1, bind 4.0, consts 1.9, tables 1.6, geometry 1.8, shaders
1.0, pso 0.5, setup 0.2, untimed ~3.7 ms: spread, ~3 us a draw; 60 needs ~15 ms. Trimmed (sdk):
- the perf clock reads (about 8 a draw, always on) only with XBE_PERF;
- a guest texture's binding remembered per stage for the rest of the frame (keyed by its header words; palettised
  ones and an extension excluded), the colour and depth target lookup remembered for the same surfaces; both start
  over each frame and whenever a target is made, dropped or rescaled (targets_gen_);
- Assemble copied the whole State (tens of KB) per draw while SetVertexShaderInput is active; now only the four
  fields it reads.
Smoke OK; a Hong Kong race snap draws rails, posts and buildings. Windows to measure.

### 2026-10-20 - race_60fps slowdown: reuse helped uploads, not time; per step timers

Harry's second log at the spot: uploads 41 -> 21-23 MB a frame, reuse 77% of vertex constants, 37% of draw
constants, 88% of tables; but host time unchanged (draw ~16.7 ms at 5500-6000 draws: assemble 2.8, bind 4.0, ~10
unaccounted). Vertex data (about 20 MB a frame) is copied per draw; caching it on the GPU would need knowing when the
title rewrites a buffer (it is marked dirty only on Lock), so not done blind. sdk: the draw path skips setting the
pipeline, topology, viewport, scissor, blend factor and stencil reference when they equal the last draw's (every
other setter forgets them), and the first perf line times each step of a draw (shaders, pso, consts, tables, setup,
geometry). wincheck OK, smoke OK. Next: his log with the step times.

### 2026-10-20 - race_60fps slowdown: the render thread's per draw cost

Harry's perf lines (Hong Kong arcade race 1, races at 60, draw distance 200, scale 100, MSAA off): at the slow spot
5000-6000 draws a frame, host (render thread) 17-19 ms a frame (draw 16-18: assemble 3.0, bind 4.0, the rest ~10),
title thread recording 4 ms and waiting 14-17 ms for it; 41 MB uploaded a frame (each draw copied all 192 vertex
constants and a 2.1 KB constant block). Before the spot: 1850 draws, host 8 ms. So the render thread's per draw
cost (~3 us) caps it near 5500 draws at 60. sdk: a draw reuses the last vertex constant and draw constant uploads
when their bytes match (same ring generation), a descriptor table already made for the same four views, and sets
root arguments only when they change; the third perf line counts the reuses. wincheck OK, smoke OK.
Next: Harry's perf lines at the spot again.

### 2026-10-20 - race_60fps slowdown in Hong Kong (Arcade race 1, after turn 3)

Harry: drops to about 40 fps there with race_60fps on, fine without; MSAA, render scale and aspect make no
difference (so not GPU fill). His frame logs (F4 before and after turn 3): 3395 draws a frame before, 5703 after (world
geometry B0101B41 2367 -> 3906), at 60 a second 340k draws: CPU bound. His settings had draw distance 200%, which
likely adds much of it. XBE_PERF's lines were info level and dropped from release logs: now report lines (sdk).
Next: his perf lines at the spot (draw distance 100 and 200) to see whether the title thread or the draw recording
holds it up.

### 2026-10-20 - ultrawide

Aspect setting 3, "Fill the screen" (aspect = 'screen'): app::FrameAspect() is the screen's shape (at least 16:9).
- PGR2 (src/game/hooks.cpp): the widescreen 3D views (race cameras 0x1857F0; 0x88480, 0x16E960, 0x184530) take
  their aspect from 0x2790F8 (16:9) when 0x48614C is set; it is set to the frame's shape each frame. Headless at
  2.39: the car 0.76 as wide in the 640x480 frame, as (16/9)/2.39 predicts.
- sdk d3d12: the frame shown at that shape; screen shaped targets (the front buffer's height, wider than tall) get
  XStretch more pixels across (640x480 at 2.39 and scale 3: 3440x1440), so viewport, scissor, clear rects, copies
  (direct only at equal scales), slopes and point sizes take separate x and y scales; a shape change remakes the
  targets as a scale change does. 2D (pretransformed, no depth test, not spanning the frame) is squeezed by
  (16/9)/shape into the centred 16:9 part (DrawConstants::hud, HudSqueeze); light glows and full screen passes stay.
- Harness: FRAME_ASPECT_X100 forces a shape; the CPU renderer squeezes 2D the same way. Previews (stretched snaps):
  a Hong Kong race with a round speedometer and a centred HUD, the title screen centred with its line full width.
wincheck OK, smoke OK. Windows to check.

### 2026-10-20 - car shadow nudge off: it caused the sunlit patches

Harry's A/B at the Florence spot: XBE_DRAW_SKIP=B0101931 removes the car's shadow (so the patches came from that
pass), PGR2_NO_FILTER=1 shows the shadow right with no patches. The 4096-unit nudge was measured with a Z buffer; under
W-buffering it is about 0.24 m of w, enough to win against the car's lower body. The draw filter is now off by default
(PGR2_SHADOW_NEARER=1 turns it on). To watch: the under-car shadow flicker it was made for (likely gone with
W-buffering). The D16 W-depth fix of the previous entry stays (correct on its own).

### 2026-10-20 - sunlit shadow patches on the car (D3D12 W-buffer on D16); render scale with MSAA

Harry: patches of shadow on the player's car in sunlit daytime races, MSAA or not; and with MSAA on the render scale
cannot be changed (the game freezes).
- The CPU renderer draws the Florence grid car clean with and without the car shadow filter (PGR2_NO_FILTER), so
  neither the filter nor shared code. Daytime only points at the sun shadow map: a 1024x1024 LIN_D16 target the car
  draws with ZENABLE 2 (W-buffered) and polygon offset 128, which the car body's shader compares against. D3D12 wrote
  W depth as floor(w) / 2^24 whatever the format (D16 values 256 times too small) and added the offset converted to
  24 bit units (256 times too big for D16). sdk: both in the guest format's own steps (1 / (zscale + 1), offset x
  zscale / 2^24-1). The CPU renderer already kept depth in the format's units.
- Render scale: a new value now applies once it has held for half a second; dragging the slider remade every
  target, and with MSAA every twin, each frame.
wincheck OK, smoke OK. Windows to check.

### 2026-10-20 - MSAA (Windows to test)

sdk d3d12: antialiasing 2/3/4 is MSAA 2x/4x/8x. Colour targets at least 320x240 (the scene; not reflections, the
mini map or shadow maps) and the depth targets drawn with them get a multisampled twin (EnsureMs, checked against the
device's sample support). Draws go to the twins (pipelines keyed by sample count); res stays what everything else
reads: Settle resolves a twin into it before sampling, copies, read-backs, captures and presenting (ResolveSubresource
for colour, a pass loading sample 0 for depth), Refresh fills a twin from res after a CPU upload, a copy or a draw
without the twin, Clear clears both. Visibility counts divide by the samples. Changing the level or the render scale
settles and drops the twins. Off by default. wincheck OK, the depth resolve shader compiles with DXC, smoke OK; not
run on a GPU.

### 2026-10-20 - achievements: garage, speed and a win (59 live)

Harry's save owns exactly Cars.Ini's 29 ShowroomFree cars: the collection tiers became 40/60/80/102 and Token Gesture
any of the 68 token cars. Speed traced from the speedometer (0xE5CF0): |float 0x298D50 + car x 0x6990| m/s
(0x39B0D0 is the odometer, 0x29DAB0 the engine for the rev counter); checked at 73 mph against the HUD. The attract
demo drives car 0 too: player racing is 0x38384C = 0 and race type 0x383814 > 0 (dumps of the demo and a race).
Added Token Gesture, Collector, Curator, Museum Piece, Full House, Ton Up, Flat Out, First Past the Post. Headless:
a plain race and the demo unlock nothing; writing 50 m/s then a first-place finish mid-race unlocked Ton Up and First
Past the Post, not Flat Out. Smoke OK.

### 2026-10-20 - mini map lost when the render scale changes

Harry: changing the render scale makes the race's mini map transparent. The harness's CAP lists show the map drawn
once, at the race's start, into the 128x128 target 018EB000 (1864 translucent quads), then sampled by a HUD quad
(FVF 0x20144, alpha kill) every frame. D3D12's ApplySettings applied a new scale at once by dropping every target, so
a target drawn once came back empty. sdk: RescaleTargets keeps each colour target's picture, blitted (with alpha)
into a new resource at the new scale on the same view slot; depth targets start again; with a renderer extension
the old behaviour stays. The settings window now says render scale applies at once (it always did). wincheck OK,
smoke OK. Windows to check.

### 2026-10-20 - draw distance option (experimental)

Harry: street lights fixed, the new options work, the night building flicker is gone. Draw distance traced:
Frontend\Cities.Ini's DrawDistance (a UTF-16 key at 0x28099C, read into the city table: 0x68 bytes a city from
0x30D670, +0x48; every one of the 17 is 3000). Race setup 0x1808C0 copies the city's into 0x49DFDC (2000 in one
mode), and the race camera 0x1859A0 makes it its far plane, times 1, 0.75 or 0.25 by view; other cameras 0x1857F0
read the table (500, 0x291A08, for scratch ones). draw_distance_percent (25-400, from the next race; PGR2_DRAW
headless) scales both. Hong Kong (race.txt, same moment, HUD hidden): 25% cuts the street's far end; 400% adds
nothing over 100% (the pixel difference is edges from a sub-pixel position change): the city's own geometry ends
before 3000 and the skyline is painted on the sky. Kept as an experimental option. Smoke OK.
Next: whether objects also have their own cull or detail distances (more buildings, trees, traffic farther out).

### 2026-10-20 - achievements: race state and medals mapped, 47 live

- Race state from the HUD code (0xDB460, 0xE0840), checked against the HUD in a headless race: car records of 0x2498
  bytes (lap int16 at 0x39B334, position at 0x39B33C, both from 0), player 1's car at 0x49E258, laps 0x383830, cars
  0x383818, finished when lap >= laps.
- Medals from the Single Player screen's medal rows (0x627B5 -> 0xA6790 -> 0xA66E0): a dword per race from
  0x35DD88 (stride 0x1C), 1 Steel to 5 Platinum; the level and group tables give each class's and Arcade type's
  consecutive races (ACHIEVEMENTS.md).
- src/game/achievement_set.cpp: the Kudos World Series (5), class (28) and Arcade (7) achievements added to the ranks
  (47 in all). Headless: Steel written into races 1-7 mid-race unlocked Pocket Rocket alone. wincheck OK, smoke OK.
Then Geometry Wars (score 0x45A308, hi-score 0x45A314, from its text at 0x145990): 4 more, 51 live.
Next: Harry's saves to confirm medal values and the player index; garage (tokens spent), single-race feats.

### 2026-10-20 - CPU renderer: the black band near the camera fixed

The reference renderer left the road nearest the camera black (about the bottom third in a Hong Kong race, larger
with a wider field of view). A probe at 100,440 showed no road draw reaching the pixel; a new drawlog count
(`nonfinite`) found 141-154 draws a frame losing triangles: those crossing the near plane get a clipped corner whose
w, interpolated in float between -4139 and 32817, rounds to 0, so x / w is infinite. sdk: the corner keeps at least
the near plane's w, and triangle setup and edge functions run in double (corners near the plane land around 1e25).
Now 0 such draws and the road is whole to the camera. D3D12 clips on the GPU and never had this. Smoke OK.

### 2026-10-20 - sound effects: measured, not built

A race (Hong Kong, race.txt) traced: PGR2 sets the I3DL2 listener every frame (room 0 mB, room HF 0, decay 2.1 s, HF
ratio 1.0, reflections 0 mB after one call at +1000, reverb 0 mB after one at +2000, delays 0.02 / 0.012 s), never
calls SetI3DL2Source, and routes every voice to speaker bins 0-5 only (no FX sends, so nothing reaches the custom
effects image 6WayDistortion.bin's 15 effects). Filters: bypass on streams, mode 3 (multi) on some 3D buffers with
0x8000 coefficients. Whether the console's custom image runs an I3DL2 reverb at all is unknown, so a reverb here
could take PGR2 further from the console. Not built; wanted: a console recording of a street and a tunnel.

### 2026-10-20 - achievements: memory mapped in part, runtime built

- Tools: Ghidra helper scripts committed (scripts/ghidra, scripts/ghidra.sh); harness `memdump` file writes guest
  memory (sdk).
- Mapped (ACHIEVEMENTS.md): the live profile is static at 0x3665AC (Profile.Sav matches it but for the signature and
  runtime-filled records); Kudos Tokens 0x366EBC; Total Kudos the sum of 8 dwords at 0x35D090 (stride 16, the
  object 0x35BA78, 0xA66C0); cars owned by a bit per player at 0x35BAA4 + car x 0x1C (0xA63C0). Medals not yet: a
  fresh profile has none to find, and the status page reading them was not found yet.
- sdk core/achievements.cpp: RetroAchievements condition syntax (subset), checked each frame at Swap, armed only
  after being unmet once, unlocks in achievements.toml, pop-up and settings tab (Windows). Headless: the test
  achievement (PGR2_ACH_TEST=1, present interval at race start) unlocked at the race's start, not before; the rank
  conditions parse. wincheck OK, smoke OK.
Next: medals and race state (a saved profile with progress from Harry), then the rest of the set.

### 2026-10-20 - PC Settings in PGR2's own Options menu

src/game/settings_entry.cpp: Screen Center becomes PC Settings. Each frame the string table entries 965 (label), 1406
(help) and 1044 (page title) point at our UTF-16 texts (mem::AllocSystem; put back if the game reloads its strings).
The page builder 0xA2E60 builds its page as before, then opens the settings window; once the window closes and 0.7 s
have passed (a B pressed while the page slides in is lost: tried first, the page stayed), GameInfo::pad holds B on
port 0 for 0.2 s and the menu is back on General Options. Headless (no window there: it reports itself closed at
once): label, help and title read PC Settings, choosing it returns to General Options, which answers the D-pad.
wincheck OK, smoke OK. Next: Harry checks it with the real window.

### 2026-10-20 - scope: a PC Settings entry in PGR2's own Options menu

Traced headless (an experiment, not committed):
- Options > General Options is built by 0xA4100: each item is a button with a page builder (0x58970: x, y, ...,
  builder, type 2), a label (0x89200) and a help line (0x891E0), both pointers from the string table at 0x28A870
  (UTF-16, StringList.ini ids). Items: Game Options, Controller Options, Screen Center, Screen Brightness, ...
- Screen Center: label id 965 (pointer 0x28B784), help 1406 (0x28BE68), page title 1044 (0x28B8C0 by the same
  table), builder 0xA2E60 (no arguments, returns 1). Its page has arrows, Continue and Back; B returns.
- A builder that builds nothing leaves an empty page that B cannot leave (soft lock): the page must be built.
Plan: relabel Screen Center as "PC Settings" (pointers to our own UTF-16 strings), let 0xA2E60 build its page,
open the settings window over it, and on closing press B for the player (GameInfo::pad) to return. About half a day
with headless checks; Windows check for the window itself. A native page drawn with PGR2's own widgets: several
days (widget API, value selectors), not recommended.

### 2026-10-20 - PC options from the SCDA port's list; achievements draft

Harry asked for the graphics and settings options of TGP482's Splinter Cell Double Agent port (same SDK), the
settings window reachable in game, and an achievement set for RetroAchievements.
- Achievements: ACHIEVEMENTS.md, 64 achievements (780 points) built from the game's own data (Frontend\*.Ini: 14
  Kudos World Series classes, 75 series, 301 races, 5 medal levels, 60 Kudos ranks, 3 Arcade types x 5 stages,
  Geometry Wars), written as RA memory conditions to be mapped (profile in RAM, race state, context flags).
- SCDA's options are game code in its own repository, not in the SDK. Taken over, written here (sdk, generic): a
  monitor choice, FXAA (final blit, psfxaa; compiles with DXC), a frames-a-second counter, pause on focus loss
  (Pace waits, the XAudio2 thread stops pulling the mixer, so the title's sample clock stops too), option labels and
  help lines, controller navigation of the window through SDL (ImGui's XInput polling off) and Back + Start to open
  it. Not taken: shader model 3, shadow filter, AO (SCDA's engine); MSAA, SMAA, HDR output and shadow map
  resolution are candidates for later.
- PGR2 (src/game): Frontend\Frontend.ini and camera_settings.ini are developer switches the retail game never
  reads; their globals are live. skip_intro: the boot sequence 0x13DB90 plays Intro1, Intro2, the language movie
  and Intro3 through the movie player 0x13C810 (stdcall path, flag), which now reports those four played at once
  (IntroMovies, 0x291AB6, is dead). First boot reaches the title at 45 s (the cache copy waits stay), a normal boot
  in 14 s. hide_hud: HideOverlays (0x291A24, read by the HUD code 0xDA000-0xE8000) each frame; a race without HUD
  checked. fov_percent: the four race camera fields of view (0.975 rad at 0x49DCF0, 0x49DD10, 0x49DD28, 0x49DD3C),
  scaled each frame; 140% checked (wider view; the CPU renderer's black band near the camera grows with it, also
  there at 100%). PGR2_SKIP_INTRO, PGR2_HIDE_HUD and PGR2_FOV set them headless.
- An entry in PGR2's own Options menu would need its frontend menu code; Back + Start does the job for now.
- wincheck OK on every changed Windows file; smoke OK.
Next: Harry's Windows check of the options and the street lights; then the night building flicker.

### 2026-10-20 - documentation pass; PGR1 assessment

Harry asked for the docs to cover the last days' findings and for a view on Project Gotham Racing 1.
- FINDINGS.md (new): everything established so far by topic, each marked console (generic, SDK) or PGR2.
- PGR1.md (new): the SDK, PC features, harness and scripts carry over as they are. The main new work is PGR1's
  older XDK: a new xdk/<build>, its layout, and, the biggest risk, render state numbering that may differ from
  5659/5849. PGR2's addresses do not carry over; the methods that found them do. A first-session plan is included.
- README.md: features and status. WINDOWS.md: system link, game switches, the current test list. sdk README and
  harness README: the runtime features and switches added in this work.

### 2026-10-20 - street light glows through buildings: W-buffer scale of pretransformed w

Harry: under-car flicker much better, car selection fixed, F1 works; new: night street lights show through buildings.
The glows are tested for visibility: 0x114FA0 / 0x114D20 draw a small XYZRHW quad per light between Begin and
EndVisibilityTest (0x1C42F0), with rhw = 1 / (1/rhw - 0.1) and z = 1, so the console takes their depth from w.
The XDK's SetTransform (0x1C34A0) keeps 1 / the projection's far w (device +0x940); W-buffered, 0x1C0FC0 folds
ZScale / far w into the fixed function matrix and 0x1C0300 into the pass-through program's c0.w. SetTransform is HLE
here, so the probes and the glows had w ~6 against the buildings' ~200000 (vertex programs carry the scale in their
own matrices): always in front. sdk: State::inv_wfar from SetTransform (PGR2's far w 999.934); both renderers scale
fixed function and pretransformed w by zscale * inv_wfar. CPU renderer: hidden probes now fail the depth test
(zfail 196 of a 14x14 quad), before none did. Visibility tests are noted in the frame log. Smoke OK.

### 2026-10-20 - car selection: the wheel through the body was a depth buffer bigger than the target

Harry: the CPU renderer, like Windows, shows the MINI's front right wheel through the body. The drawlog showed the
showroom car rendered into a 500x200 texture (02DB7000, pasted onto the screen by a 2D quad) with the 640x480 depth
buffer 03B40000 bound; both renderers dropped depth when the sizes differ, so the car's 71 draws ran without a depth
test. The console uses the top left of the bigger buffer. sdk 0f1b57e: the CPU renderer keeps a depth buffer at least
as big as the target; D3D12 binds a depth target of the colour target's size for that surface (Clear too). The
CPU snap of the car selection is solid. Smoke OK. Next: Harry to confirm on Windows; night under-car and building
flicker.

### 2026-10-20 - overnight checks

- 20 minute soak with everything above: 9 races, no traps, frames to the end, RSS 102 -> 105 MB.
- Code review of the settings window and input: the keyboard's controller 1 now stays plugged in while keyboard
  play is on (alt-tabbing away would have unplugged it, and the game pauses on a removed controller).

### 2026-10-20 - W-buffer depth: perspective correct w

With w interpolated linearly on the screen (as xemu does) the CPU renderer drew long triangles that reach the camera
as streaks in front of everything (1 frame in 8 runs); the pixel's own w (1 / interpolated 1/w; in D3D12 a
perspective interpolated TEXCOORD6) has no streaks and keeps the car selection whole. The headless one-frame pop
count is too noisy to rank depth modes (the CPU renderer also leaves random spike patterns in single snaps).

### 2026-10-20 - car selection's missing panels: the Z buffer; W-buffering on by default

The CPU renderer shows the car selection MINI whole with W-buffering and broken with z / w (panels missing, the
background through the body), matching Harry's Windows report: the showroom's depth range leaves a Z buffer no
precision at the car. sdk: the D3D12 W-buffer is now on by default (XBE_WBUFFER=0 turns it off). Shaders of the
menus, car selection and a race compile with DXC (71). Smoke OK.

### 2026-10-20 - System link works: XNetGetConnectStatus

Fourth attempt, with new tools (XNET_TRACE, gdb watchpoints with the joiner started under gdb, guest stack logs).
Chain: the joiner showed "lost connection" (FUN_00099780 from FUN_00094FF0) because FUN_00160560 found no connected
player; the host's entry was added at the join and removed by FUN_001654D0 because its +0x74 "leaving" flag was set
by FUN_00160C70 when FUN_0025A4B1(the host's address) returned 3. FUN_0025A4B1 is XNetGetConnectStatus (XTLID
0x40032): the library's version answers XNET_CONNECT_STATUS_LOST when its network object is null.
- sdk: XNetGetConnectStatus returns CONNECTED; XNetUnregisterInAddr (0x40030) hosted; both signed by the round trip
  (game.toml +2, rescan stable). XNET_TRACE=1 logs every network call.
- Two instances: lobby with both players, the host starts, both race (positions 1/2, 2/2), VDP keepalives on port
  1000 throughout. scripts/linktest.sh (two instances, link_host.txt / link_join.txt) checks it: LINKTEST OK.
- src/game/hooks.cpp: XNET_TRACE also logs the link session teardown (0x165AF0) with the guest stack and player
  removals (0x1654D0).
Next (Windows): two PCs on a LAN, or two instances with --instance 1 / 2.

### 2026-10-19 - D3D12 W-buffer (opt-in), render target reload logging

- sdk 9f82dd1: XBE_WBUFFER=1: draws with ZENABLE 2 (D3DZB_USEW, nearly all of PGR2's) write SV_Depth = w
  interpolated linearly on the screen (noperspective TEXCOORD6 from the vertex shader) plus the polygon offset in its
  units and slope; the rasterizer bias is zeroed for them. Off by default until tried. Headless: AUDIT with
  XBE_WBUFFER=1 makes the same keys; all 80 shaders of a race (40 W-buffered) compile with DXC.
- Under the car at night the road (B0101BB1, B0101D01) samples a 256x256 light pool target in stage 3, redrawn each
  frame. A target whose guest memory looks CPU-written is reloaded from memory over what the GPU drew; such reloads
  are now logged in release builds (three per target) and XBE_NO_TARGET_UPLOAD=1 skips them for sampled targets.
Next (Windows): XBE_WBUFFER=1, XBE_NO_TARGET_UPLOAD=1, and the log's reload lines during a night race.

### 2026-10-19 - PC settings window (F1), keyboard controls, remapping; under-car A/B negative; car select

- sdk 1274a46: F1 settings window (ImGui): display mode, resolution (applied at once), vsync, aspect ratio (auto,
  4:3, 16:9: Widescreen() honours it; next start), render scale (next start), anisotropic filtering, frame rate cap,
  the game's options (race_60fps), and controls: two keys per console input (keyboard = controller 1 while the window
  has focus) and a physical-to-console map for controllers. Saved to pgr2.toml ([keys], [gamepad] by name, only
  what differs). Neutral input to the game while it is open. Not run yet: Windows only (wincheck passes; it now has
  SDL's headers, so input.cpp is checked too). Headless: PGR2's menus take the left stick, so arrows steer and
  navigate.
- Harry: XBE_DRAW_SKIP=B0101A61 and XBE_DRAW_TEST=B0101A61:depth do not stop the under-car flicker: not that pass.
- Car selection with missing panels (Harry): the CPU renderer draws the car complete; the audit finds no raw vs
  converted vertex difference on that screen; needs a Windows capture.

### 2026-10-19 - under-car flicker at night; PGR2 uses W-buffering

Harry's night clips (.local/win): the dark patch under the car is missing in single frames every 4-5 game frames
while moving. His 4 frame logs: no stage blips at all (so the flame leak was not the visible flicker). At night
there is no sun shadow pass (B0101931 absent); the patch is vs B0101A61 (n110/n59 pairs and n37 under the car,
blend src ONE dst CONSTANT_COLOR, depth test, no write, t0 16x16, t3 a 256x256 target), drawn every frame on
Windows with identical inputs.
- PGR2 renders with ZENABLE 2 (D3DZB_USEW, W-buffering) on nearly every draw; the runtime treats it as a Z buffer.
  xemu's rule (psh.c, Z_PERSPECTIVE): depth = vertex w interpolated linearly in screen space, plus the polygon
  offset units, plus factor x slope, clamped to the clip range (SetDepthClipPlanes, which the runtime ignores),
  stored as floor(value) in 24 bit units.
- sdk: the CPU reference renderer implements it (REF_NO_WBUFFER=1 for the old way). Day and night frames look
  right. But measured without the shadow patch (PGR2_NO_FILTER=1), the car shadow mesh still fails against the road
  by up to 4k-16k units in W mode as in Z mode: W-buffering is correct but not the fix for these ground passes.
  Not yet in D3D12 (needs SV_Depth from a noperspective w).
- Headless the under-car decal is steady frame to frame in both modes.
Next: Harry's A/B: XBE_DRAW_SKIP=B0101A61 and XBE_DRAW_TEST=B0101A61:depth.

### 2026-10-19 - flame leak may not be the visible flicker

Harry: flicker also without gear changes or visible flames; night windows are dim on the console; flicker under
the car throughout. All screen blips in his logs follow the flame code (0x216FD), but every vs B0101B21 draw in his
captures (and headless) has a pixel shader bound, which overrides the stage 0 combiner: the mode 5 leak only shows
if those draws run without one. The frame log now records the pixel shader per draw (sdk).
Next: Harry's log with pixel shaders; a clip of the under-car flicker at night.

### 2026-10-19 - night building flicker: caught; exhaust backfire flames leave combiner mode 5

Harry's repeated frame logs (PGR2_NIGHT, 5 x 30 frames, flickering throughout): framelog_2 blips every 3-4 frames
(5, 9, 12, 15, 18), framelog_3 at 10, 11, 16. In each blip frame the combiner notes show mode 5 (SELECTARG1(texture),
full bright) set from 0x216FD right before the flickering draws (vs B0101B21, t0 827D1400), which then draw with it.
- 0x216FD is in FUN_00021620, called by FUN_00021CD0 for a car when its byte +0x1508 is set; the positions it uses
  (+0x1484, +0x1490) are tagged 'EXHL'/'EXHR' (FUN_0001CC90): exhaust backfire flames. It draws a quad with
  Begin/End in mode 5 and leaves mode 5 set; the next draws inherit it.
- Headless: the flame pass (FUN_000314E0) is skipped in camera mode 12 unless 0x291A27 is -1 (PGR2_FLARES=1 forces
  it); with it, all cars' +0x1508 stay 0 (gdb watchpoint: only the init clear): the scripts hold the throttle on
  automatic, no backfire. Harry's driving backfires.
- src/game/hooks.cpp: PGR2_FLARE_LOG=1 logs the race view camera mode, the flame pass and gate inputs; flames noted
  in the frame log; PGR2_FLARES=1 experiment.
Open: why the console does not show it (same game code). Questions for Harry: does the flicker follow gear changes
(flames)? On the console at night, are those windows bright or dim?

### 2026-10-19 - night frame log with combiner notes: no blip caught

Harry's log (PGR2_NIGHT=1, Edinburgh, 90 frames, 5681 combiner notes): no on-screen draw changed stage states for
one frame; blips only in an off-screen target (034E8000, reflections). His two earlier logs caught blips (frames
3/29, 11-14). Recording slows the game (90 frames in 18 s), which may hide a timing dependent fault.
- sdk: F4 records again each press, into framelog.txt, framelog_2.txt, ...
Next: several recordings in one session at the building.

### 2026-10-19 - night flicker: not reproduced headless; combiner calls noted in the frame log

Harry: on the original Xbox lit windows never flicker (daytime clip in .local/win for reference).
- src/game/hooks.cpp: PGR2_NIGHT=1 forces every race to night (0x383188 after FUN_000F14E0) so night is
  reproducible headless; night Florence and Edinburgh render.
- Headless night Edinburgh frame logs (90 frames, plain and XBE_VISIBLE_ALL=1): no one-frame stage blips. Harry's
  Windows log had them (frames 11-14).
- sdk: d3d::FrameLogNote; the game hook on FUN_00105ED0 (stage 0 combiner modes, cache 0x3F29A4) notes each call,
  the cache, the caller and the resulting COLOROP/ARG1 in the frame log. Headless: ~47 calls a frame; mode 5
  (full bright) once a frame from 0x8C025.
Next: Harry's frame log at the flickering building with the notes, to see which caller sets the building's mode
in a blip frame.

### 2026-10-19 - night and rain are normal in Instant Action; the flicker is night lighting

Harry: night is possible in Instant Action on the original Xbox too. His log: a day race and a night race with the
same inputs to FUN_000F14E0 (slot 3, selection 0): a random roll, as on the console. The time of day search was a
detour; the earlier "night where the console shows day" comparisons were different rolls.
- src/game/hooks.cpp keeps "pgr2: area loads with time of day T, weather W" (0x116C20); the race-start, setup,
  0x116990, 0x59EC0 and conditions diagnostics are removed (the conditions result fields were never read right).
Open: night lighting. The Edinburgh building blip (stage 0 MODULATE(diffuse, texture) to SELECTARG1(texture) for a
frame, PGR2's combiner mode 1 vs 5 via FUN_00105ED0) is the game's own per frame choice; to compare with console
night footage of the same city.

### 2026-10-19 - who sets night: FUN_000F14E0 (gdb watchpoint)

Harry's log: Sydney loaded with time of day 1 (night), Hong Kong with weather 2 (rain); the race-start line said 0
both times: set in between. Under gdb (the harness started inside gdb: this VM refuses ptrace attach; handle all
nostop, hardware watchpoints on 0x100383188/0x100383184) the writes come from FUN_000F14E0, called by race setup
(FUN_000F4910, FUN_000F4B00). It picks the conditions from per-city tables (0x33BECC, 0x33BEE4, default 0x33BEE8,
0x68 bytes a city) by the game mode's flags (0x30CEFC.., 0x40 a mode) and an index 0x3667D4 = 0x34EB20 + 1 (a menu
selection 0 to 4, likely difficulty; 0x366E58 keeps the last). It also calls rand() first.
- src/game/hooks.cpp logs "pgr2: conditions: mode M city C slot S (selection, kept) -> ..." per race (the two result
  fields are not yet read right: headless says 3/0 where the area loads 0/0 or 0/2). Headless: slot 3, selection 0.
Next: Harry's log of a night race to compare the inputs.

### 2026-10-19 - the night cone challenge: the game loaded the night lighting set

Harry's capture 191837 (Florence cone challenge, night; the console shows it by day): the sky texture itself
(2048x512, vs B0101BE1) is a night sky, so the game loaded the night set: not the renderer. Cities have three sets
(Game\Areas\<city>.pak_day, pak_overcast, pak_night); FUN_00116990 and FUN_00116C20 (inline) choose "night" when
0x383188 is 1, else "overcast" when 0x383184 is 1 or 2, else "day". FUN_001808C0 (race init) copies both into the
race and picks sky and fog colours (night 0xFF101020). FUN_00059EC0 only resets them (garage).
The race-start log line comes too early: headless the values change before the area loads (weather 0 at race start,
2 at load). src/game/hooks.cpp now logs "pgr2: area loads with time of day N, weather W" (0x116C20) and writes to
0x383188 by 0x59EC0. Headless Instant Action: day, or overcast (weather 2).
Next: Harry's log of a night race with the area load line; then who sets 0x383188 to 1.

### 2026-10-19 - night races that are daytime on the console

Harry: every event we show at night (Edinburgh, Moscow) is daytime on the original Xbox (screenshot: Edinburgh, F40,
daylight). Much of the remaining flicker (lit windows, lamp glows, the building) is night-only lighting.
Traced: the race's time of day is 0x383188 (1 night; weather 0x383184), copied from the race request (+0x2D54 in the
setup, FUN_0015E280 / FUN_001648F0; the request's own +0xA24 is cycled 0/1 by a race-setup menu option at
0x976A0-0x97720 and 0x92EC0). Data: data\audio\<city>Night.ini picked when it is 1 (0x12B76F); Florence has no
night data. Not the clock: KeQuerySystemTime feeds date text and weekly counters only (FUN_00132780, FUN_00083A90);
the runtime's clock is right (UTC, time zone 0). Headless (fresh disk): attract and Instant Action start with time
of day 0. src/game/hooks.cpp now logs "pgr2: race starts: time of day N" at each race start.
Next: Harry's log; suspect a Day/Night option kept in his saved profile (a fresh disk would show day).

### 2026-10-19 - Edinburgh building flicker: the game picks a fullbright combiner for single frames

Third frame log (stage tables written out). One-frame blips in stage 0 for a block of draws, here the building's
draws (vs B0101B21, texture 827D1400) in frames 11 and 13: COLOROP and ALPHAOP 4 -> 2, COLORARG1 and ALPHAARG1 0 ->
2: MODULATE(diffuse, texture) becomes SELECTARG1(texture): unlit, full bright at night.
Traced: PGR2 sets stage 0 through its own mode switch FUN_00105ED0(mode) (cache 0x3F29A4, writes 0x1D0570.. and
dirty 0x800); mode 1 is MODULATE(diffuse, texture), mode 5 SELECTARG1(texture). FUN_0016E960/0016F050 write
stage 0 directly without updating that cache. The library applies stage 0's combiner on dirty 0x4000/0x800
(FUN_001C78E0: 0x100, 0x800, 0x4000, 0xF stage units, 0x2000 fog, 0x400, 0xFF1000 lighting, 0x200; it clears all
but 0xC0000070); the runtime copies the deferred tables at every draw instead. In the blip frames the draws before
the building are the same as in normal frames: the game itself chose mode 5 for that draw, at irregular intervals.
Open: a deliberate effect (a flickering sign or lamp, also on the console) or a decision fed by something emulated
differently. Asked Harry to compare that spot with console footage.

### 2026-10-19 - Edinburgh building: one-frame texture stage glitch found

Second frame log (split hashes). Vertex bytes, textures and render states of every static draw: identical in all
60 frames. Pixel constants change smoothly (lamps passing: c0 ramps up and down), no alternation. The texture stage
states of a block of draws late in the frame (vs B0101A21, A61, A71 and B21 pieces, about 50) are different in
frames 3 and 29 only, then back: once or twice a second a stage setting differs for part of the frame. No draw
exists only in those frames, so a state change, not a stray draw, carries it.
- sdk: the frame log writes each stage table the first time it appears (tssdef); scripts/framelog.py --tss A B
  names the stage states that differ.
Next: Harry's third log, to see which stage setting and value.

### 2026-10-19 - frame log of the Edinburgh building flicker (night)

Harry's 60 frame log (.local/win/framelog.txt; the building on the right flickers, only while moving). Every
static draw on screen read identical vertex bytes and texture contents in all 60 frames: the data is right. What
changed every frame for the night world draws (vs B0101B21, 538 a frame) was the state hash: within a frame all
538 share one of 9 values, and the set changes every frame: a frame-wide value (pixel constants or deferred render
states such as fog), not per object.
- sdk: the frame log now keeps stage states and render states apart, writes the 16 pixel constants as values and
  hashes the vertex constants per block of 16 registers; framelog.py reports each. Headless: only the car paint's
  light (pixel c0) and the camera blocks change. Logging 60 frames took Harry 11.5 s (slower than intended).
Next: Harry's second log at the same building.

### 2026-10-19 - flicker: A/B switches all unchanged; frame log at full speed

Harry: XBE_RENDER_THREAD=0, XBE_VISIBLE_ALL=1, XBE_RAW_VERTICES=0, XBE_STATE_WALK=0: no change, flicker on every
example. So not threading, visibility tests or the raw vertex path. The capture "kerb flip" in 141150 was the car
moving over white paint (65 mph is a metre a game frame); the night clip shows signs and rails changing from one
game frame to the next. Captures never caught a flicker frame.
- sdk: XBE_FRAMELOG=N, F4 (or a file named framelog) logs N frames of draws at full speed with hashes of the
  vertex bytes, indices, texture contents and states each reads. scripts/framelog.py groups draws by identity and
  shows which input changes and whether it returns to earlier values (A B A B: double buffered data read wrong).
  Headless baseline (KWS race, 30 frames): static scenery identical every frame, only the car body's states change.
Next: Harry's frame logs at the flickering spots (Moscow, Edinburgh).

### 2026-10-19 - flicker round 3: twelve 6-frame captures (Moscow, Edinburgh), nothing caught

Harry: fences and missing objects back; much less flicker; still: kerbs (red and white), signs, buildings and
lights flicker, best seen at full speed. 12 runs of 6 captured frames (.local/win/1409xx-1411xx) plus a night clip.
- Draw-list diff of consecutive frames (main screen, back buffers as one): only dynamic draws (cars, particles,
  shadow quads with changing counts) come and go; no static draw changes textures, blend, depth, cull or c0 for one
  frame in any run. In the captures the scenery is drawn the same way every frame; a capture flushes the GPU every
  frame, so a timing dependent fault would not show (none of the captures ever caught the flicker).
- Kerb overlay vs road headless: the overlay fails the road's depth by up to 4096 units (not rounding).
- Deferred fog states the game writes inline are synced each draw (checked). Headless REF black band with RAW=1 was
  a different moment, not the raw path (the CPU renderer never uses raw vertices).
- sdk: XBE_VISIBLE_ALL=1 (every visibility test visible at once). Headless (day) culls little by visibility.
Next: full speed A/B on Windows: XBE_RENDER_THREAD=0, XBE_RAW_VERTICES=0, XBE_VISIBLE_ALL=1, XBE_STATE_WALK=0.

### 2026-10-19 - missing fences, lamp posts and signs: SetVertexShaderInput

Harry's capture 124828 against the console (YouTube): the low railing along the street is missing. Its texture
(32x32 DXT1, dark rails and posts) is drawn as quads with the building shader, writing no pixels; headless the
same quads decoded positions like -1.5e38: the buffer holds packed shorts, read as float3. PGR2 calls
D3DDevice_SetVertexShaderInput (0x1C0560; SetVertexShaderInputDirect 0x1BFFD0 tail-calls it with a table at
0x1CF704): the library pushes its own vertex array addresses and formats (0x1720, 0x1760) and sets dirty bit 30 so
the draws skip theirs. The SDK did not know it, so those draws used SetStreamSource's streams and the shader's
declaration.
- sdk: both hosted; the attributes come from the shader's (or the given) layout plus each stream input's offset.
  Signatures by the round trip (game.toml +2 lines, rescan stable). Headless: street lamps, posts, a blue sign and
  the chevron barrier now appear at the KWS start (render thread on and off). Smoke OK, wincheck OK.
- Seen while testing, not new: with raw vertices headless sometimes leaves a black band where road near the camera
  should be (also in earlier runs). To look at next.
Next: Harry checks the fences and the car shadow (4096 offset).

### 2026-10-19 - car shadow: depth test kept, 4096 units nearer

Without the depth test the car shadow was steady but showed through the car's rear (Harry's screenshot). Measured
headless with the new REF_ZOWNER (for each pixel the pass failed: which shader wrote the depth that stopped it, and
by how much): road (B0001B11) and kerbs (B0001CC1) at most 4096 units of the 24 bit buffer, the car body (B00019E1)
4096 to 65536. The filter now keeps the test and draws the pass 4096 units nearer (gpu::DrawAdjust::depth_bias): the
road and kerb failures are gone, the car still hides it. Main depth buffer is LIN_D24S8 (0x2E), the shadow map
LIN_D16 (0x30): no float depth involved. Smoke OK.
Next: Harry checks the shadow under the car; the missing fence (capture 124828 against the YouTube frame).

### 2026-10-19 - car shadow flicker fixed on Windows

Harry: car shadow right and steady, cone shadows fine (08118bf). Still open, both older than this work: buildings
flicker, and some barriers do not show.
Next: a clip and a capture where a barrier is missing; bisect the building flicker with XBE_DRAW_SKIP (candidates:
B0101B41 blended with polygon offset -1, building decals).

### 2026-10-19 - car shadow fix did not run on Windows: static initializer

Harry's run of 56c3109: flicker as before, and no "car shadow pass" line in the log: the filter was never set. It
was registered by a static initializer in src/game/draw_filter.cpp, a file nothing else references; it ran in the
harness (it matched with and without the render thread) but not in the Windows exe. Now game_info.cpp's init calls
InstallDrawFilter(), which logs "pgr2: draw filter installed". Smoke OK.
Lesson: game code registers through an explicit call, never a static initializer.

### 2026-10-19 - flicker fixed (pending Windows): car shadow pass without the depth test

Harry's bisection: XBE_DRAW_SKIP=B0101931 (the car shadow projection) made the flicker almost vanish;
XBE_DRAW_TEST=B0101931:depth the same; :alphakill much worse. The pass draws a ground mesh around the car that the
game builds each frame (flat, about 10 x 11 m at road height, reaching behind the camera: negative w), with the
car's shadow map compared in stage 0, polygon offset, no depth writes, multiply blend. Headless, as many of its
pixels failed the depth test as passed, most by far more than any offset; with the test off one frame in three
drew a band across the screen in the CPU renderer (which clips differently from D3D12), not seen on Windows.
- sdk: gpu::g_draw_filter, a hook for a title's code to drop a draw's depth test or the draw (D3D12 and the CPU
  renderer).
- src/game/draw_filter.cpp: draws with a depth texture in stage 0, polygon offset, no depth writes and a multiply
  blend go without the depth test. Matches only that pass (logged once per shader). Smoke OK.
Next: Harry checks the flicker and that the car shadow is right (not drawn over the car or barriers).

### 2026-10-19 - flicker: the polygon offset scale was right; layer bisection switch

Harry: no flicker while stationary; less with XBE_ZBIAS=10; most visible in sunlit areas. Capture 105745 lists
the targets: every one is 3x (main depth 640x480 -> 1920x1440, D24S8 as on the console). The offsets match the
XDK at 3x exactly (ZBIAS n gives units -n, slope -n/4, times 3: -1/-0.75, -9/-6.75, the shadow map's 8 -> 24), so
the slope scale is not lost; the earlier reading was wrong. Sunlit, moving, helped by more offset: the layers on
the road that rely on a polygon offset (multiply passes 00000042 with -9 and B0101C41 with -1, the shadow map
projection B0101931, blended B0101B41 decals with -1) are the suspects.
- sdk: XBE_DRAW_SKIP="h1 h2": draws with those vertex shader handles left out, to find the flickering layer.
Next: Harry's runs skipping one layer at a time.

### 2026-10-19 - flicker: car shadow and road shadows; polygon offset experiment

Harry's clean run: car body right with the render thread on (the earlier break was the leftover XBE_DRAW_TEST),
flicker unchanged. Clip .local/win/flicker-example.mp4 and five 6-frame captures (102016-102051, none caught a
flicker frame). In the clip, at the green corner after the KWS start, the road's shadow pattern and a yellow kerb
wedge swap between states from one game frame to the next, the same spot where headless showed it.
Findings: the 256x256 target 83AA5000 is a light pool (sprite 83A2AB80 is a white blob), stable; the 1024x1024
D16 shadow map holds the car from the sun, identical in every frame; matched road draws keep their textures and
camera constants (only c54, a per piece shade flag, and c71-74, the light pool projection, change between them).
Headless REF_ZEPS (a depth tolerance) did not reduce pops, so it is not z-fighting between road and kerb there.
The shadow layers (car shadow multiply vs B0101931, building shadow decals vs B0101C41) rely on a polygon offset
(units -1, slope -0.75); on Windows the main depth buffer's offset slope printed unscaled (-0.75), which means its
host and guest sizes matched: either it is not upscaled, or the scale is lost. With a 3x render the slope should
be 3x. Added: XBE_ZBIAS=N (scales every polygon offset) and the target list (guest and host sizes) at the top of
each capture's frame.txt, to settle this on Windows. Harness: REF_DRAWLOG=2 logs constant changes, REF_ZEPS.
Next: Harry's run with XBE_ZBIAS=3 and 10, and one capture to read the target sizes.

### 2026-10-19 - Windows build fix: GetEnvironmentStringsA

Harry's build failed in system_info.cpp: the Windows SDK has no GetEnvironmentStringsA under UNICODE (its narrow
function is plain GetEnvironmentStrings), mingw has both, so wincheck passed. sdk: the wide block instead.
scripts/wincheck.sh now defines UNICODE, _UNICODE and _CRT_SECURE_NO_WARNINGS as the Windows build does (it then
reproduces the failure on the old code) and checks every runtime source that includes windows.h by default (13).

### 2026-10-19 - "car broken again" was a leftover XBE_DRAW_TEST

Harry's three 6-frame captures (.local/win/095552_*, 095709_*, 095805_*): in the first two the body draws (vs
B01019E1) ran with zen 0, zwrite 0, cull 0 and per-draw images were saved: XBE_DRAW_TEST and XBE_CAPTURE_DRAWS from
the 2026-10-17 experiments were still set in his environment. The body's shader handle is B01019E1 with the render
thread and B00019E1 without it (memory layout), so XBE_RENDER_THREAD=0 only seemed to fix the car: the experiment
stopped matching. No code fault. The captures caught no flicker event (a few hundred scattered pixels).
- sdk: the log lists every XBE_* and PGR2_* environment variable at startup.
Next: a clean run (no leftover variables) and a new clip of the flicker.

### 2026-10-19 - flicker: Harry's videos analysed; state order; multi-frame capture

Harry's 30 and 60 fps videos (.local/win/30fps.mp4, 60fps.mp4; car body right in both, also at 30). One-frame
pops in almost every frame; in the large ones the car's shadow on the road vanishes for a frame and a dark
translucent rectangle with a hard vertical edge darkens part of the screen (sky and far buildings, right half).
Headless (KWS Florence race, frame pairs and 6-frame runs with the new SNAP_FRAMES): similar one-frame artefacts
with and without the render thread, so not a threading race. The new REF_DRAWLOG lists each draw (pixels, box,
state, textures): the road's lightmap stage samples a 256x256 shadow texture (surface 03AA5000) filled each frame
by a full quad and two sprite quads; a 1024x1024 D16 shadow map is applied by a multiply pass (vs B0001931);
six 256x256 reflection faces are drawn each frame. Textures of matched draws do not swap between frames.
- sdk 72975c8: every state setter reads pending pushed commands first (XBE_STATE_WALK=0 to compare), as the
  constant setters did since the car body fix. Headless pops over 8 six-frame runs: 121,688 pixels with it,
  171,398 without (fewer, not gone). Smoke OK.
- XBE_CAPTURE_FRAMES=N: F3 captures N frames in a row, so a bad frame can be diffed with its neighbours on D3D12.
- XBE_RENDER_THREAD / XBE_RAW_VERTICES environment overrides for Windows A/B.
Next: Harry's 6-frame capture over a flicker; his A/B with XBE_STATE_WALK=0 and XBE_RENDER_THREAD=0.

### 2026-10-18 - checked: does a capture leave state that could whiten walls?

Read the D3D12 capture path: a capture's saves blit and then Flush (Submit + wait, command list reset, Begin).
Begin re-sets heaps and root signature and forgets the bound targets; every draw allocates its own descriptor
table and upload space and sets topology, blend factor and stencil ref, so nothing cached survives the reset.
No state leak found: the white walls Harry saw are probably not a capture artefact. To settle it on Windows:
look at the same spot with no XBE_CAPTURE_DIR set and no capture taken.

### 2026-10-18 - option: races at 60 frames a second (experimental, off)

`race_60fps = true` in pgr2.toml (or PGR2_RACE60=1 headless) runs races at 60. src/game/hooks.cpp overrides the
interval setter 0x177C70 (thiscall, interval 0/1/2; stores 0x80000000 + n in D3D's global and n in 0x4860F1):
asked for 2, it passes 1. The frame step 0xD90F0 then gives dt 1/60 and physics takes one 1/60 step a frame, so
the simulation is the one the console runs at 30, sampled twice as often.
- Evidence (headless): the race presents 60 a second, interval stays 0x80000001; the start countdown ran 19 s in
  22.2 s wall with two CPU snaps (the snaps cost more at 60). A replay recorded at 60 plays back (the result
  screen's background replay moves). Not checked: replay and ghost speed against a 30 fps recording (0x179F0
  compares a ghost sample step with dt), Kudos timing, system link with one side at 60.
- Smoke OK with the option off (the default).
Next: Windows checks (pacing, car body, stripes, race_60fps feel and replays).

### 2026-10-18 - frame pacing by the game's present interval

The runtime now paces each Swap by the presentation interval the game set: 60 / vblanks a second, within fps_cap.
PGR2 asks for one vblank (0x80000001) in the movies and menus and two (0x80000002) in races, so menus run at 60 and
races at 30, as on the console, and the forced fps_cap 30 default is gone (game_info.cpp has no init any more).
- sdk 3889abd: data key D3D_PresentInterval (5659 0x1D093C; the swap helper 0x1C5290 reads it at 0x1C534A),
  signed on an anchor; round trip reproduces game.toml. fps_cap 0 now means "only the interval". The log has
  `d3d: present interval XXXXXXXX` at each change.
- Harness pad scripts count vertical blanks (each presented frame adds the vblanks it asked for), not frames.
  All pad scripts remapped: unchanged before the movies (the first boot presents at most 30 a second there by
  itself, also unpaced), doubled after. The title now needs START twice (attract -> PRESS START -> menu); a second
  START 150 vblanks after the first. smoke.sh, savetest.sh and soak.sh read the vblank count.
- Evidence: smoke OK (menu 11 draws a frame, race 2009); savetest OK; race clock 18.7 s of lap time in 20.1 s wall
  with two snaps (0.93x, the snap cost, as 0.91x before). Not re-run: arcade, kws, timeattack, replay, soak.
- Windows: if pgr2.toml has `fps_cap = 30`, set it to 60 (or delete the line) or the menus stay at 30.
Next: an option for races at 60 (the interval clamped to 1; see the research entry for what to check).

### 2026-10-18 - player car body fixed: pushed constants applied out of order

The missing body (only wheels, driver and plate) reproduced headless with the car standing on the grid (earlier
headless frames had the car moving). The reference renderer's per draw counts showed the body draws' triangles
cover no pixel: off screen. An event trace before the first body draw: the game pushes c14-c25 as raw commands
(SET_TRANSFORM_CONSTANT_LOAD 110 and 118, the world view-projection), then sets the car's own matrices with
SetVertexShaderConstantNotInlineFast(110, 16). The HLE wrote those at once, the pushed packets were read by Walk at
the next draw and overwrote them: every body draw used the world matrix with object space vertices.
Fix (sdk 9688e51): the constant setters and SetVertexShader read pending pushed commands first, so order is as on
the console. The grid frame now shows the TVR Tamora in its dark teal paint, as Harry's console screenshot does,
without PGR2_LIGHT_CLEAR. The wheel shade logic was right; PGR2_LIGHT_CLEAR is no longer needed (kept as an
experiment switch).
Harness: RAW=1, RT_THREAD=1, GEN_OPT=-O2 build like the Windows exe (none of them reproduced it on their own).

### 2026-10-18 - car body on Windows: the body draws write no pixels

Capture 213135 (XBE_CAPTURE_DRAWS=vs:B01019E1, PGR2_LIGHT_CLEAR on): the 16 body draws (962-977, up to 5722
vertices) change no pixel of the target; the tyre draws right after (same vertex and pixel shaders) do. The body
texture's alpha (tex_81629000) looks right, the light constants are right. So on D3D12 the body is dropped by alpha
kill, culling or the depth test (headless it is drawn). XBE_DRAW_TEST=B01019E1:alphakill|cull|depth switches each
off for that shader to find which. The walls were white again in this capture.

### 2026-10-18 - car body: a second, Windows-only cause

With PGR2_LIGHT_CLEAR on Windows (capture 211350): the body's constants are right (c56 1 1 1 1, c54 0.698, paint
c51/c52 set, pc0 0.188) yet the body still does not appear; headless the same switch shows the body in its paint.
Back buffer snapshots: before the car draws (1572) the road, after (1902) wheels, driver and the white shadow quad
under the car, no body. The body draws are 1610-1617 (vs B01019E1, ps B0101D10, t1 the 1024 D16 shadow map,
t3 the environment cube). New: XBE_CAPTURE_DRAWS=a-b saves the colour target after each draw in that range, to see
whether the body draws land and what covers them.

### 2026-10-18 - black car: what the shade flag does, and an experiment switch

Harry's reference (a YouTube video of a 07-11-03 pre-release build, Instant Action in Florence): on the grid the
player car sits in building shade with its paint clearly visible (dimmer, reflection heavy); moving in sun it is
bright. Ours in shade shows no paint at all.
- The wheel flag means shade: FUN_00023620 gives a wheel's light as the triangle's baked colour when set, the sun
  colour times it when clear (used for the tyre smoke colour, FUN_00023670).
- Brightness eases (0.9/0.1) to 0.25 per unflagged wheel; c56 = sun x brightness; the paint (c51/c52) reaches the
  body only through c56, so at brightness 0 the body shows just its environment reflection (looks transparent:
  the street behind it is reflected) plus a tiny pc0 (the road's baked colour times a body tint).
- Headless A/B at the same frame (Florence Timed Run, Tamora): normal = no body visible; PGR2_LIGHT_CLEAR=1 (every
  wheel's flag forced 0) = body in its teal paint. The draw lists are identical; only the constants differ.
- Checked and ruled out: the track data (inflated by the game's zlib, 66% of triangles near the track flagged), the
  query's height (right triangle), the flag test sense, threads (one thread queries), 0x49E110 (race running).
So in the console's shade the sun term cannot be 0 the way it is here; where the difference comes from is open
(which triangle wins when several overlap, x87 precision in the hit test 0xC19F0/0xC11D0, or data per time of day).
PGR2_LIGHT_CLEAR=1 is an experiment switch only (src/game/hooks.cpp), not a fix.
Also: capture 194603 shows white walls and no body; 192449 (an earlier build, same spot) did not: needs a plain
screenshot to tell whether the capture changes (depth target saved mid-frame) affect that frame.

### 2026-10-18 - black car: the shade flag is real track data

Harry: on the original Xbox the car is lit (a normal car from the selection). Checked headless with more probes
(PGR2_LIGHT, src/game/hooks.cpp):
- Of 9633 distinct track triangles the ground query looked at in a Florence race, 6316 have bit 7 of the flag
  byte; the flag bytes are a small set of values (0..52 and 128..180).
- The triangle records are not stored raw in Florence.pak_*: a watchpoint on one shows it written once at load by
  0x1DC533, zlib's inflate window copy (D3DX section) with the final value; every other file goes through it too.
- The query finds the right triangle: a wheel at y 1.93 gets ground 1.59 (a wheel radius below) and that triangle's
  flag and baked colour.
- 0x24DE0 stores bit 7 of the hit byte as the wheel flag; 0x207CC skips the 0.25 for a set flag.
So data, query and test agree: in Florence the player's wheels mostly sit on flagged triangles, brightness eases to
0, c56 (sun) is 0 and the body keeps only pc0, which for the body draw is tiny (0, 0.02, 0.027): no paint shows. In
Edinburgh (Time Attack) the car takes the other branch (+0x19C8 == 5, c56 = +0x1520, 0.8) and is lit.
Next: what the console shows in a shaded Florence street (paint visible, how dark), and what the flag bit means;
whether pc0 for the body (paint times an ambient level) should be larger.

### 2026-10-18 - black player car: the game's own shadow logic, the same headless

Windows run with XBE_VSC and capture 194603 (Florence grid): c56 is written only as 0 0 0 0 by 0x1F036 and c54 as
0 0 0 0.4 by 0x1F15C, so the body has ambient light only (pc0 0.188); the car's shadow map renders properly
(1441_8327A000_z). The cause is the game's own state, not the renderer:
- 0x1F036 writes the sun colour (0x49EEA8, FFFFFFFF) scaled by the car's brightness (+0x150C).
- Each frame (0x20690) the brightness eases (x0.9 + 0.1 x target) to 0.25 x the wheels whose flag is 0
  (TEST/JNZ at 0x207CC: flag 1 = in shadow).
- A wheel's flag is bit 7 of the flag byte of the track triangle under it (FUN_00024de0 -> 0xC74D0 -> 0xC6210,
  byte +0xB of the 16 byte triangle record), and its baked colour comes back too.
- `PGR2_LIGHT=1` (src/game/hooks.cpp, game only) logs each car's flags, colours, brightness and the sun colour
  when they change. Headless, the Instant Action car's wheels are flagged most of the time as well, its brightness
  near 0, the same as Windows; early headless frames showed race cars as "wheels and driver" from the start.
Both renderers draw what the game asks for. Open: whether the console shows the same (the flag may mean shaded by
buildings, in which case a dark car in a shaded street is right), or a bit 7 the runtime gets wrong. Next:
compare with the console or xemu at the same spot (Florence, Duomo grid; Chicago).

### 2026-10-18 - Windows capture of the black car

Captures work from Harry's PC (PowerShell's `set` had not set the variables). Capture 192449 (Florence grid,
black car): the six environment faces now go to six targets on Windows (SetRenderTargetFast fix confirmed).
The body's constants were not listed (only small draws listed them) and the shadow pass's depth was not saved:
both now captured. XBE_VSC used its 200 lines on frontend writes (0x118BFA, 0x118CA2) before the race; it now logs
each distinct register, value and caller once (headless race: c56 from 0x1F036, the sun colour times the car's
brightness, c54 from 0x1F15C).

### 2026-10-18 - Windows: reflections confirmed, release logs

Harry confirms reflections on car paint and glass now work on Windows (SetRenderTargetFast plus the D3D12 cube
copy). F3 "did nothing": his log shows the F3 stack dump at 7 s of a 19 s run (during the movies), but release
builds keep only error and report lines, so XBE_VSC, the F3 note and the capture lines were never written. Those
are reports now. The release log is logs\pgr2.log, overwritten each start (WINDOWS.md said pgr2_*.log).

### 2026-10-18 - car bodies, environment map, Windows captures

Harry's Windows test: boots, movies, audio and saves (KM/H and Time Attack across a restart) work; the licence plate
screen is right on Windows (garbled headless only). The player's car in Instant Action is a black body with wheels
(other cars fine), and some walls show green/magenta stripes. F3 captures copied to .local/win (6 frames).

Environment map. PGR2 draws its car reflection cube's six faces with D3DDevice_SetRenderTargetFast (0x1C3C00),
which ran the library's code: it writes the surfaces to the hardware directly, so the host never switched targets
and all six faces were drawn into whatever was bound before (the Windows capture shows face 0 drawn six times).
Now on the host (sdk f6f4edd), signed into 5659 (187 of 246). Headless all six faces fill (AUDIT shows +0 to +A0000
drawn) and the dumped cube shows the street. With the D3D12 cube copy from the earlier entry, reflections should
now work on Windows.

Black player car, what is known:
- The car body's vertex shader (B01019E1) gives diffuse = c56 x (N.L - c1.w^2) x paint (c51/c52) and specular from
  c54. In Harry's capture (160442, the Camaro) paint is white but c56 is 0 and c54 is (0, 0, 0, 0.4): no light,
  so black. Headless the same constants are c56 0.8, c54 0.557 (set by SetVertexShaderConstant1Fast from 0x1F080
  and 0x1F15C, FUN_0001ef90).
- FUN_0001ef90 scales the sun colour (0x49EEA8) by the car's brightness (+0x150C); both zeros on Windows mean the
  brightness or the sun colour is 0 there. The brightness is 0.25 x the wheels whose ground query (FUN_00024de0 ->
  FUN_000c74d0, track collision) says the surface is not flagged (bit 7 of its surface byte): all four flagged
  gives a black car. Pure CPU code; why it differs on Windows is open. Visibility query counts do not matter
  (headless with a forced count: same picture).
- Headless also had a black body for another reason: the reference renderer ignored polygon offset, so the car's
  self-shadow map (drawn with offset 128, slope 24) shadowed the car itself. Fixed (sdk b5993fc); the MINI shows
  its paint now (dark green, c51 = 0 0.04 0).
- `XBE_VSC=56 54` (new) logs every write to those constants with the caller: one Windows run with it answers
  whether 0x1F080/0x1F15C write zeros there.

Stripes (capture 085551, car reversing into a wall): every 2D texture the frame bound decodes cleanly (montage of
397); the brown in that frame is the clear colour of the 256x256 reflection target 834CA000, which most of the
scene samples at stage 3; the striped wall uses bump-mapped reflection shaders (dot product reflect, normal map
82501200). Needs a capture of the target after each draw near the wall to go further.

### 2026-10-18 - coverage, soak, system link

Pad scripts (each from a first boot, making the profile as save1.txt does; checked with snaps at each step):
- `timeattack.txt`: Circuit Challenge, Edinburgh, Princes Street East, MINI Cooper S, Auto; rolling start, 60 s
  of throttle.
- `kws.txt`: Kudos World Series, Compact Sports Series, Street Race (Florence, Duomo 2), MINI, Auto, Novice;
  grid start, 60 s of throttle.
- `arcade.txt`: Arcade Racing, Timed Run 1 (Stockholm, Gamla Stan Loop, Vauxhall VX220), Novice; 60 s throttle.
- `replay.txt`: the same Timed Run left to time out (2:45), then View or Save Replay: the replay plays with its
  controls (Save, rewind, pause, fast forward, cameras).
No traps in any; quitting a race (pause > Quit > Yes) goes back to its menus or the title.

Soak (`scripts/soak.sh 60`): Instant Action races back to back for 60 minutes, each 60 s of throttle, then
pause, Quit, Instant Action again (a random city and car each time: Barcelona, Edinburgh, Florence, ...).
31 races scripted, 30 3D scenes drawn, 107,460 frames, 200 million draws; no traps, no errors, no hang (frames to
the end, 31 empty frames, one per load). Memory: RSS 100 MB at 5 minutes, 104 MB at 60 (+4.4 MB an hour), data
600 -> 619 MB: no leak worth chasing. 10 minute run before it: the same.

System link (two instances, `sdk/harness/run.sh -n 2`):
1. Binding failed: PGR2 binds ports 1000-1002, Linux keeps ports below 1024 for root (WSAEACCES). Fixed (sdk
   35f0f68): loopback instances on a non-Windows host shift those ports up 20000 and back. Both bind now.
2. Host: Multiplayer > System Link > Create New Game > Edinburgh > Princes Street East > options: the lobby
   ("Player (Host)", seven open, waiting). Joiner: System Link > Search for Games. Traced (not committed): the
   search broadcast (9 bytes) reaches the host, the host's 204 byte answer reaches the joiner, the joiner sends a
   186 byte join to the host's port 1002, the host answers 1183 bytes, the joiner acks 12 bytes. Then neither sends
   anything; the host lists no one and the joiner says "You have lost connection with the host".
3. A joiner with a different profile name: the same.
Three attempts, stopped. Ideas for next time: which XNet calls run after the ack (XNetRegisterKey with the host's
key, XNetXnAddrToInAddr on the host's XNADDR from the 1183 bytes; the runtime keeps one key), whether a socket
waits in select() or an overlapped receive that never completes, and whether the identical console identity of
two instances (same EEPROM, serial, profile id 12E9194916CD) matters. On Windows with two PCs the port shift does
not apply; worth trying there.

### 2026-10-18 - research: video settings, timing and 60 fps (no code)

Video settings (EEPROM). PGR2 reads them: XGetVideoFlags (0x191297) calls ExQueryNonVolatileSetting(8,
XC_VIDEO_FLAGS) and returns (value >> 16) & 0x5F (1 widescreen, 2 720p, 4 1080i, 8 480p, 0x10 letterbox, 0x40
PAL60), dropping 720p/1080i unless the AV pack (0x192f34) is HDTV. Its callers: the device setup 0x1783a0 and the
PAL60 check 0x143050. The device setup always asks for a 640x480 back buffer; widescreen sets
D3DPRESENTFLAG_WIDESCREEN and the flag at +0x37C, 480p sets D3DPRESENTFLAG_PROGRESSIVE. 720p and 1080i are never
used: PGR2 has no HD mode.
- The runtime already reports widescreen when the output is 3:2 or wider, plus 480p (Windows: always widescreen
  on Harry's 3440x1440). Headless A/B with the new `OUTPUT=WxH` (same Time Attack frame,
  captures/1018-ab43-race.png and 1018-ab169-race.png): widescreen renders anamorphic 16:9 into 640x480, a wider
  horizontal field of view (more of the street) and the HUD narrowed and moved out to the edges; the D3D12 backend
  shows it at 16:9, so the shapes come out right. The 2D menus are the same in both.
- So nothing to enable: widescreen is on, 480p changes nothing but a flag, and more pixels come from the runtime's
  render scale (render_percent), not from the game.

Timing. FUN_000d90f0 (from the main loop 0x179830) runs each frame:
- dt = interval / refresh, from the presentation interval byte 0x4860F1 and the refresh rate byte 0x48614B (60).
- game time += dt x timescale (timescale 1.0, set by FUN_000d9060); in some modes measured time is used instead:
  swap timestamps from the swap callback 0xD9010 (or the performance counter), capped at 0.15 s.
- Physics steps a fixed 1/60 s at a time (FUN_000f5d60) until it catches up with game time: two steps a frame at
  30 fps, one at 60.
The interval comes from FUN_00177c70(n), which sets the D3D presentation interval: n = 1 0x80000001 (every vblank),
2 0x80000002 (every second vblank), and stores it in D3D's interval global 0x1D093C and the byte 0x4860F1.
Measured headless: 1 through the movies and menus, 2 from the race on. So the console runs the frontend at 60 and
races at 30, by presentation interval; the runtime ignores the interval and paces only by fps_cap, which is why
races ran double speed at 60 and why the 30 fps default slows the menus to 30.

What 60 fps would take:
1. First, generic: the runtime honours the presentation interval in Swap (pace to refresh / interval; an XDK
   layout key for D3D's interval global, 0x1D093C in 5659). Menus at 60, races at 30 as on the console, and the
   fps_cap 30 default could go. Test that the game speed is right in both.
2. Then races at 60: keep the interval at 1 in races (an override of FUN_00177c70 clamping 2 to 1, in src/game, as
   an option). dt becomes 1/60 and physics takes one 1/60 step a frame: the same simulation, so handling is
   unchanged. To check: anything per frame outside the physics loop (FUN_000179f0 compares an interval-derived
   step in replay/ghost timing; ghosts saved at 30 replayed at 60; Kudos and HUD timers; particles; system link
   between a 30 and a 60 machine), and that 60 frames a second of the race fit the frame budget on PC.

### 2026-10-18 - saves

PGR2 saves profiles with XCreateSaveGame: E:\UDATA\4d53004b\<12 hex digits>\Profile.Sav (30100 bytes) and
SaveMeta.xbx; TitleMeta/TitleImage/SaveImage.xbx at the title's folder. A profile is saved when made, when leaving
Options, and when a race is quit (statistics and career fields). The first 20 bytes are a signature.
- Interactive (cmd presses and snaps), on one disk: Create New Profile (name, plate, plate text, helmet, all
  defaults) -> saved. Options > General > Game Options: Speedometer MPH -> KM/H; saved on backing out of Options;
  byte 0x844 of Profile.Sav is that option (1 MPH, 0 KM/H). Time Attack (Edinburgh, Princes Street East, MINI):
  started, quit, profile saved again. Kudos World Series: Compact Sports Series, Street Race Florence Duomo 2,
  Novice: started, quit; saved again (career fields at 0x8DC and 0xFEC..0x1002). Rebooted on the same disk: the
  title offers "Start Player's Game", Game Options shows KM/H, the race HUD shows KM/H.
- `scripts/savetest.sh` (pad scripts save1.txt, save2.txt, about 8 minutes): a first boot makes the profile and
  sets KM/H, checks the byte; a normal boot of that disk opens Game Options, checks the game read Profile.Sav and
  the byte is still KM/H, and snaps the screen (KM/H). Passes.
- Not done: a record. Every record PGR2 keeps (fastest lap, best Kudos) needs a finished lap, which needs
  steering; a fixed pad script drives blind, and steering by snapshots took a minute a step (a lap would take an
  hour). Time Attack laps start rolling on Princes Street; holding throttle hits the barrier at its end.
  The Time Attack attempt and the career start do save and reload.
- Headless only: licence plate pictures are garbled and some full screen blurs are dotted in the reference
  renderer; worth a look on Windows (plate select screen).

### 2026-10-18 - sound, checked headless

- sdk harness: `WAV=file` writes the mixed output (48 kHz 16 bit stereo, header kept current, the log's
  `audio: output started` is its first sample). The headless mixer is now pulled on a fixed 10 ms schedule (it
  slept 10 ms after each pull, so it ran slow). `scripts/wavstats.py` prints RMS, peak, clipped samples and the
  strongest frequency per interval.
- Race run (race.txt, 240 s, `.local/run/1/race.wav`): movies -18 to -25 dB RMS with silent gaps between logos;
  attract and title music -27 dB RMS, peak -17.8 dB; race -19 to -24 dB RMS, peaks to -2 dB. Clipped samples 0.
  Sample rate 48 kHz out; voices at 44.1 and 22.05 kHz resampled by their SetFrequency.
- Music in the menus: the title's music is a 44.1 kHz stream at gain 0.13 (-17.7 dB: stream headroom -6 dB and the
  game's volume); it fades to 0 once Instant Action is chosen and the race loads.
- Engine: a temporary per-voice trace (not committed) showed the engine buffer (44.1 kHz) driven by SetFrequency
  48 kHz -> 73 kHz -> 76 kHz under throttle, dropping at each gear change (to 55k, 47k, 44k) and climbing again.
  Eleven 8 kHz streams sit queued at full gain with silence (voice chat style); harmless.
- DirectSound exports still running the library's code (the runtime's .XTLID audit), named from their export ids
  (alphabetical within each interface) and Ghidra:
  - B003E IDirectSound_GetCaps (0x1ECB84): reads the library's never-initialised counters, reports 0 free voices;
    PGR2 calls it at audio init and never reads the result.
  - B00A3 XAudioCalculatePitch, B00A5 XAudioCreatePcmFormat: pure arithmetic.
  - B00AB (0x1ECFE5): a file media object for data/audio (XWaveFile style), plain file reads.
  - B0080 FlushEx, B0093 SetMaxDistance, B0094 SetMinDistance, B009A SetPosition, B008B SetDopplerFactor: the
    IDirectSoundStream thunks, each calling the CDirectSoundStream method.
  - CDirectSoundStream_SetDopplerFactor (0x1ED954) was unmatched and wrote through a null voice pointer near guest
    address 0 on each 3D stream update: now on the host (accepted), signed into 5659 (186 of 245).
- Called but only accepted, no effect in the mixer: DownloadEffectsImage and SetEffectData (PGR2 loads
  6WayDistortion.bin, 15 DSP effects), SetI3DL2Listener/Source (reverb), SetFilter (buffers and streams),
  SetVelocity and SetDopplerFactor (Doppler), SetMixBinHeadroom, EnableHeadphones. Sound is dry: no distortion,
  reverb, filters or Doppler. Not linked by PGR2: the other 26 DSOUND names the scan lists as missing.
- Needs Windows: whether it sounds right (XAudio2 path, same mixer), the music volume in menus, and the engine.

### 2026-10-18 - D3D12 groundwork without a capture

Tools (headless, nothing needs a GPU):
- `AUDIT=N` (sdk harness/audit.cpp): every Nth frame's draws as the D3D12 backend would get them. Writes each
  generated shader to `cap/hlsl/`, assembles each draw both ways (raw guest vertices as D3D12 reads them, and the
  CPU conversion the harness and reference renderer use, `XBE_RAW_VERTICES=0` in the harness build) and compares
  every corner, lists sampled textures with their mip levels checked against their parents, and the targets drawn
  to (`cap/textures.txt`, `cap/vertices.txt`). `AUDIT_TEX=<addr>` saves a texture as its guest memory decodes, which
  is what D3D12 reads (the null backend never draws into guest memory between REF frames).
- `scripts/shadercheck.sh`: compiles `cap/hlsl` with DXC (fetched into .local/dxc), HLSL 2018 rules as FXC.
- `scripts/wincheck.sh`: syntax-checks d3d12.cpp and xaudio2.cpp with clang against mingw-w64 headers (unpacked into
  .local/mingw, no root). D3D12 changes are now at least compiled before Harry gets them.

What the D3D12 path draws when something fails (read, d3d12.cpp): a shader that fails to compile logs
`d3d12: vs_5_0 compile failed` plus the source and the draw is skipped; a failed pipeline logs and skips; a missing
or uncreatable texture is a null view (samples 0, black); an unknown texture format decodes its first byte as grey.
No fallback is a flat colour, and Harry's logs have no compile or pipeline errors.

Findings, race (Instant Action, every 10th frame) and the menus plus attract races (every frame, 1.3M draws):
- Shaders: 77 vertex/pixel shaders in a race, 75 in menus and attract; all compile with DXC.
- Textures used: DXT5 (787), DXT1 (108), A8R8G8B8, YUY2 (movies), L8/AL8/LIN_L8, R5G6B5 cube, LIN_X8R8G8B8, LIN_D16.
  DXT goes to the GPU as BC1/BC3 unchanged; every mip level of every texture is within 0.04 of its parent shrunk
  (only 2x2 and 1x1 DXT levels differ more), so level offsets are right. Same decode code as the reference.
- Vertices: 334,600 raw draws, 11,646 differed from the CPU path. Cause: declarations longer than the stream stride
  (a 24 byte FLOAT3 D3DCOLOR NORMSHORT2 NORMPACKED3 layout on a 20 byte stride, decals and a second texture coordinate):
  the console reads the next vertex's bytes, D3D12 read zeros at the last vertex. Fixed (sdk a867407): the raw buffer
  includes the overhang. Now 0 of 1,303,670 differ. The two shaders involved never read the bad element, so this
  was not visible.
- Environment map: the game draws ~800 draws a frame into face 0 of a 256x256 R5G6B5 cube (0x83458000) and three
  pixel shaders add it as specular (t3.rgb * v1). D3D12 always read cubes from guest memory, which is all zeros
  (AUDIT_TEX dump): reflections were black on Windows. Fixed (sdk 8770388): faces drawn on the GPU are copied into
  the cube once a frame.
- An 8 bit L8 target (0x83AA5000, 256x256, 3 draws a frame) sampled at stage 3: the console's B8 surface keeps the
  blue byte, D3D12 keeps BGRA (alpha 0 from the clear). Now read as its blue byte. The shaders that read it use only
  rgb of grey content, so not visible. The reference renderer wrote 16 bit 565 texels into it (fixed).
- Sync/late copies wrote 32 bit linear rows over any surface: over the L8 or swizzled targets that would run past
  them. Only 32 bit linear surfaces are written back now.

Not found, needs Windows: the brown polygons and the green/magenta stripes. Every input D3D12 gets (vertices,
textures, mips, shaders) now matches the reference path headless, so what is left is the HLSL itself versus the
reference interpreter (same rules, separately written; only a GPU runs HLSL), the D3D12 state mapping (blend,
depth, stencil, samplers), upscaling, and timing (render thread). Wanted from Windows: an F3 capture with
`XBE_CAPTURE_DIR` while the fault is on screen; and whether reflections on cars and wet roads look right now.

### 2026-10-18 - smoke test, upstream draft

- `scripts/smoke.sh`: runs `scripts/pad/race.txt` headless (240 s) and fails on any trap, unimplemented export or
  guest trap in the log, when the title menu is not up before the menu choice (frames 3900-4200 under 100 draws a
  frame), when the last 300 frames average 100 draws a frame or fewer, or when frames stop. First boot: menu 11,
  race 1586 draws a frame; KEEP_HDD=1: race 2810; `PAD=scripts/pad/none.txt` fails as it should (11 draws a frame).
  CLAUDE.md: run it before every fast-forward of main.
- sdk: comments no longer name PGR2; harness README documents FPS. The branch (40e9c35..) is generic.
  `.local/upstream-pr.md` drafts the upstream PR; Harry decides when to send it.
- Rendering (brown polygons, green/magenta stripes): not started. `.local/win` has only pgr2.log, with F3 stack
  dumps and no capture: XBE_CAPTURE_DIR was not set, or its folder not copied. Every texture format used in a race
  (0x0, 0x1, 0x6, 0xC, 0xF, 0x1E, 0x24, 0x30) decodes in the headless renderer, so the fault is likely D3D12 only.
- Playtest: no notes and no CrashDumps folder; the log ends with Exit (0).

### 2026-10-18 - game speed: 30 frames a second

Harry finished a full Instant Action race on Windows; the game ran far too fast at fps_cap 60, and right at 30.
PGR2 steps races by a fixed time per frame (the frame time path at 0xD9317 takes measured swap times only in some
modes) and showed 30 frames a second on the console: it presents on the next vertical blank
(interval 0x80000001) and its frames took more than one. Measured headless: the race clock ran 1.84x at 60
(3:48 -> 3:07 in 22.3 s), 0.91x at 30 with snap overhead (4:35 -> 4:14 in 23 s).
- `src/common/game_info.cpp`: PGR2's init sets fps_cap 30 unless the settings file has an fps_cap line.
- sdk: the swap callback is called when a swap is shown (new layout key CDevice_SwapCallback, 5659 0x1DA4);
  PGR2 records frame timestamps there. Correct, but not what fixed the speed.
- harness: paces to the settings' fps_cap when FPS is unset, so it runs at 30 like Windows; pad scripts halved.
The settings file `pgr2.toml` only exists once written from the settings overlay; create it by hand to override.

Windows screenshots also show large flat brown polygons over track and sky, and green/magenta striped surfaces;
car bodies are fine on D3D12 (headless only). Needs F3 captures with XBE_CAPTURE_DIR set.

### 2026-10-18 - Windows prep, white quad, movies, menus, first race

**Windows build prep.** `.gitmodules` uses https://github.com/hk21x/XboxXBERecomp.git (SSH push URL only in the
VM's sdk config). PGR2 names: `PGR2.exe`, window "Project Gotham Racing 2", `pgr2.toml`, logs `pgr2_*.log`
(the harness log is now `.local/run/1/logs/pgr2_001.log`). `build.bat` keeps the committed config (it runs
xbe_symbols only when `config/game.toml` has no `[symbols]`). WINDOWS.md has the steps and checklist.
D3D12 review found one real gap: titles poll GetVisibilityTestResult in a loop (PGR2 0x114CC0) and D3D12 wrote
reports only at Present; D3D_SetFence now flushes when a report was asked for and Flush delivers reports.

**Harry's first Windows run**: builds, boots, takes the pad. White quad flickering over the menus, movies solid.

**White quad (shared code, D3D12 and headless).** PGR2 calls Swap(2) (copy, no finish), draws its own copy of
the supersampled back buffer to the front buffer (a white quad sampling texture 0 = back buffer, u 0..1280, plus
pillarbox bars; 0xD983D..0xD9B59), then Swap(4). The host presented on every Swap, so every other frame showed
the copy pass: an untextured white quad. Swap now honours COPY/BYPASSCOPY/FINISH: the copy draws go to a target
that is never shown, FINISH shows the back buffer, the swap count moves once a frame.

**Movies.** The decoder writes correct YUY2 (checked in memory) and YUY2 conversion existed. PGR2's movie player
gives texture coordinates in register 9 under FVF XYZRHW | DIFFUSE; my immediate-mode FVF path packed only the
FVF's fields, so every pixel sampled texel (0,0). The FVF is now widened by the slots written.
`captures/1018-0722-ref_00600.png` (Bizarre Creations), `1018-0722-ref_01200.png` (Microsoft Game Studios).

**Car bodies / direct constant uploads.** 0x1852D0, 0x185330, 0x185450 run thousands of times in the attract
demo (0x1853B0 never); their constants reach `g_state.vsc` (114..117 hold a matrix). Not the cause. Bodies are
still partly missing; left for the Windows report.

**Menus.** Three bugs stood between the title and the menus:
- XAPI init failed: MODE SENSE page 0x3E (DVD authentication) through IOCTL_SCSI_PASS_THROUGH_DIRECT was answered
  with zeros, then the signed security sector check (0x19372C). Now the page is answered and the check passes on
  the host (`XapiVerifyMediaInDrive`, our name). T: and U: are mounted, no reboot to the dashboard, no "not enough
  free blocks" dialog.
- The harness ran unpaced (thousands of frames a second): presses were milliseconds long. Paced to 60 (FPS=N).
- DirectSoundGetSampleTime (0x1EB88B, .XTLID 0xB0011) read the APU sample counter 0xFE80200C, which never moved:
  the frontend clock (0xAD280, delta at 0x2FEE14) stood still, so menu text stopped at its first letter and items
  were drawn at near zero size. Now a host function counting 48 kHz samples. Menus animate and respond.
`captures/1018-s01.png` (title, no white quad), `1018-t01.png` (title menu), `1018-t03.png` (Instant Action).

**Race.** Title -> RIGHT, RIGHT -> Instant Action -> "Welcome to Yokohama" (`1018-t04.png`) -> race start
(`1018-t05.png`) -> accelerating, lap 1 of 2 at 80 mph (`1018-t06.png`), lap 2 (`1018-t08.png`).
`scripts/pad/race.txt` does it from a clean start: `1018-0748-ref_10800.png` (in the race, lap 1).

**START during Intro2** no longer hangs: it skips to the title (`1018-0750-ref_02400.png`). The frozen sample
clock was the likely cause; not separately root-caused.

**Next.** Windows retest; the stale band under the HUD at race start (`1018-t05.png`); car bodies; dotted
textures once the Windows report says whether D3D12 shows them.

### 2026-10-17 - fork, config split, cache partition, first real drawing: attract demo and title screen

**Housekeeping.** Submodule URL fixed to git@github.com:hk21x/XboxXBERecomp.git (`.gitmodules` and the sdk's own
remote), `pgr2-xdk5659` pushed, a fresh clone of `xdk-5659-bringup` builds both tools and its scan reproduces
the config, `main` fast-forwarded. gdb is system wide now; `.local/tools` removed.

**Config split.** The build, `--sign` and xbe2pe read `config/pgr2.toml`, which includes `config/game.toml`
(xbe_symbols output, never edited) and holds every hand entry. A rescan reproduces game.toml byte for byte.
Documented in CLAUDE.md.

**Cache partition (SDK kernel, generic).**
- `InitialiseFileCache - !XFormatUtilityDrive`: XAPI's formatter (0x193A83) asks IOCTL_DISK_GET_PARTITION_INFO,
  which returned zeros, so the size check failed before any write. It does not call IoQueryVolumeInformation.
  Now a partition opened whole (`\Device\Harddisk0\PartitionN`) is a raw volume: the retail partition layout
  (Z: 750 MB), raw reads give zeros, raw writes are accepted, and a FATX header written at offset 0 empties the
  partition's host folder (`files: partition 5 formatted`).
- IoQueryVolumeInformation (76) implemented anyway: its caller is XGetFilePhysicalSortKey (0x18C31F), which the
  cache copy (0x13B770) uses to sort files. File objects get a FILE_OBJECT body with a zeroed FsContext.
- The cache copy then found nothing: FindFirstFile (0x191139) sends `*.*` as an empty name, which the runtime
  matched against nothing. An empty name now matches everything. `D:\Cache\Resource\*.*` is copied to Z:.
- NtUserIoApcDispatcher (232) trapped: the APC routine of ReadFileEx; it now calls the completion routine.
- `KEEP_HDD=1 scripts/harness.sh ...` keeps the hard disk between runs. Both paths tested: first boot formats Z:
  and copies the cache, a kept disk boots straight through; both reach the title screen.

**Movies (XMV).** Intro1 (audio), Intro2 (3 audio tracks), English (no audio, paced by the vertical blank count
via GetDisplayFieldStatus 0x1C4350), Intro3, then the attract demo. They play through without presses (about
100 s headless). **START during Intro2 hangs the player** (0x13C810 polls XMVDecoder_GetNextFrame 0x2113E8,
which keeps returning "no frame"); not root-caused, `scripts/pad/boot.txt` waits until the attract demo.
No skip hook was needed because nothing stalls unless a movie is skipped.
The movies themselves render black in the reference captures (YUY2 640x480 texture, format 0x24).

**Crashes fixed on the way (SDK, generic):**
- D3DDevice_SetDepthClipPlanes (0x1C2050, flags 3) ran library code over host-less device state and crashed in
  0x1C19D0: now a host function that accepts it (the renderer keeps the viewport depth range).
- IDirectSoundBuffer_PlayEx (0x1ECCD9) and IDirectSoundBuffer_Pause (0x1ECD15, switch 0/1/2 in 0x1F39F8) started
  or paused hardware voices in library code: now host functions.
- Headless only: the game polls GetVisibilityTestResult (0x1C3610) until the GPU report is written; the null
  backend never wrote reports. It now writes them at once with 0 pixels. D3D12 already delivered them.

**Title screen.** The frontend draws with Begin/End and FVF 0x144 (XYZRHW, diffuse, one texture). EndPrimitive
turned FVF handles into an empty program ("pretransformed immediate mode vertices are not supported"). It now packs
the slots in the FVF's layout and draws through the fixed pipeline. Captures:
- `captures/1017-2339-ref_06600.png`, `1017-2339-ref_07800.png`, `1017-2346-ref_12000.png`,
  `1017-2346-ref_72000.png`: attract demo races (Hong Kong and a European city); car bodies partly missing.
- `captures/1017-2355-ref_24000.png` (kept disk) and `captures/1017-2358-ref_24000.png` (first boot): the
  Project Gotham Racing 2 title screen.
- `captures/1017-2355-ref_40000.png`: the screen after pressing A on the title (blurred city, button glyphs).
The reference renderer shows dotted textures and white bands across UI panels; not checked against D3D12 here.

**Signatures:** 183 of 242 functions (D3DDevice_SetDepthClipPlanes, IDirectSoundBuffer_PlayEx and _Pause added;
5849 knows the names without signatures). Fallback ids: Reset FFFFFE04, SetDepthClipPlanes FFFFFE05, PlayEx
FFFFFF0D, Pause FFFFFF0E.

**Next.** The menu after the title (what the A press leads to), then a race. Look at the missing car bodies
(possibly the game's own vertex shader constant uploads, 0x1852D0 and friends, now decoded but unverified), the
START-in-Intro2 hang, and the reference renderer's YUY2 and dotted textures.

### 2026-10-17 - 5659 SDK bring-up, first presented frame

**Housekeeping.** Scaffold committed to `main` (d47ce51), `main` fast-forwarded over `xdk-5659-survey`, branch-per-task
rule added to CLAUDE.md. `sdk` submodule URL set to https://github.com/hk21x/XboxXBE-Recomp.git (`.gitmodules` and the
sdk's `origin`, with an SSH push URL because HTTPS has no credentials here). **The push of `pgr2-xdk5659` fails:
GitHub answers "Repository not found" for hk21x/XboxXBE-Recomp** (SSH authenticates as hk21x). The SDK commits stay
local until the fork exists; this branch's gitlink points at them.

**Weak matches (all 7 correct).** SelectVertexShader 0x1C0100 writes 0x1EA0 with the address; LineWidth 0x1C1320
writes 0x380 and render state 0x9D; TexCoordIndex 0x1C1670 writes texgen 0x3C0 + stage * 0x10; InitDefaultState
0x1C9280 loops SetTransform/SetRenderStateNotInline/SetTextureStageStateNotInline; SetVertexShader 0x1C03F0 stores
the shader at device + 0x790; SetTexture 0x1C3FE0 stores [stage] at + 0xF78; SetPixelShader 0x1C25C0 copies the
shader's 0x39 render states to D3D__RenderState 0x1D0740.

**D3D misses: 43 of 60 found** (my earlier "63" counted wrong; it was 60 functions + 4 data). Found by what each
writes to the push buffer, the device fields it touches and its ret size, e.g. Swap 0x1C55D0 (returns +0x2468),
Clear 0x1C2C50 (0x1D98/0x1D8C, ret 0x18), SetRenderTarget 0x1C3980, SetViewport 0x1C3DB0, SetTransform 0x1C34A0,
MakeSpace 0x1C6140, D3D_MakeRequestedSpace 0x1C6000, D3D_SetFence 0x1C5DE0, D3D_BlockOnTime 0x1C5E90,
CDevice_KickOff 0x1C5CC0 (fastcall), the render states by the D3D__RenderState index each stores (ZEnable 0x8F ...
DoNotCullUncompressed 0xA5). The other **17 are not in the image**: no caller in game code, nothing in the D3D
section's undefined gaps (the only sizeable one, 0x1C47C0, is the state query): BeginPush, EndPush, CopyRects,
DeletePixelShader, DeleteVertexShader, EnableOverlay, FlushVertexCache, GetVertexShaderFunction, IsFencePending,
LightEnable, PersistDisplay, D3DDevice_Release, SetLight, SetMaterial, SetPalette, SetTile, SetVertexShaderConstant4.
Data: D3D_DirtyFlags 0x1D0538, D3D_IndexData 0x1D0534, D3D_DeferredTextureState 0x1D0540, D3D_RenderState 0x1D0740.
Also IDirectSound_DownloadEffectsImage 0x1EBA16 (calls CDirectSound::DownloadEffectsImage 0x1EB7DC).

**layout.toml re-derived**, every value cited to a PGR2 instruction; "unverified" header removed. The CDevice tail
moved -0x10 from 5849 (miniport at +0x1C18, so the vertical blank and gamma fields too), ZScale and SuperSample
moved -4, the front fields -4 (PixelShader 0x780, VertexShader 0x790, IndexBuffer 0x79C). DSOUND and XPP offsets are
unchanged from 5849 (constructor 0x1EDE60, effects image reader 0x1EFA36, XGetDevices 0x26E033).

**Signatures.** `xbe_symbols --sign config/game.toml --out sdk/xdk/5659/signatures.toml`, then a fresh
`xbe_symbols` run reproduces the hand-filled config exactly. Two things needed for that: `[functions]` names the
CDirectSoundStream constructor so the two stream vtables get separate anchors (otherwise both anchor 0x1EC26F and
the scan drops both), and that `[functions]` line is the one thing the scan does not write back. **Note: a scan
run rewrites the whole config, so `[[hook]]`/`[[function]]` entries added later are lost if xbe_symbols runs again.**

**Walk.** `sdk/runtime/xdk/d3d8.cpp` `Walk` and `EndPush` now load vertex shader constants: 0x1EA4 sets the load
slot, 0x0B80-0x0BFC write dwords from it, each method group moves the slot on by its constants (the XDK's own
SetVertexShaderConstantNotInlineFast 0x1BFEF0 sends one load and several 16 dword groups, which needs exactly this).

**First boot.** Crashed at once in guest D3D code 0x1CB2C1: the game calls D3DDevice_Reset 0x1C38D0 (from
0x177BF0), the library's own Reset rebuilds the frame buffers through display hardware the host never set up.
Added a host `D3DDevice_Reset` (name added to the 5849 and 5659 signatures): frame buffers for the parameters
(kept when unchanged), set as the target with the device references moved, cleared. With that the game runs: CPU
threads up, ~3,100 presents a second in the harness, 0 draws. `captures/1017-2235-ref_00060.png` shows the Xbox
**dirty disc error** ("There's a problem with the disc you're using..."). A breakpoint on the error routine
0x1388F0 (called from 0x13DD77 in 0x13DCE0) gives the message `InitialiseFileCache - !XFormatUtilityDrive`: with
no `Z:\Version.12` the game formats its cache partition and XFormatUtilityDrive returns FALSE. No trapping stubs
were hit. Boot log also warns that kernel imports 76 IoQueryVolumeInformation, 232 NtUserIoApcDispatcher and
343 XcPKGetKeyLen are unimplemented.

**Tooling notes.** `sdk/harness/*.sh` are mode 644 upstream, so `scripts/harness.sh` runs them through `bash`.
No gdb on the VM and no sudo: `.local/tools/gdb.sh` is gdb 15.1 extracted from the Ubuntu packages. REF=60 at
3,100 frames a second wrote 5.6 GB of TGAs in 120 s; use a much larger REF (or `snap`) until frames are paced.

**Next.** Find why XFormatUtilityDrive fails (it is guest XAPI code: trace its kernel calls, starting with the
unimplemented IoQueryVolumeInformation), or give the harness hard disk a formatted cache partition. Then the boot
sequence onward to the menu.

### 2026-10-17 - XDK survey, 5659 seed, push buffer check

**Tools.** `scripts/linux-tools.sh` builds `xbe_symbols` and `xbe_recomp` cleanly.

**Library versions** (`xbe_symbols assets/default.xbe sdk/xdk`), title id 0x4D53004B:
XAPILIB, D3D8, D3DX8, XGRAPHC, DSOUND, XBOXKRNL, XMV, XONLINES, XVOICE, LIBCMT, LIBCPMT, all 1.0.5659.
There is no separate XNET library entry; XNet is linked as part of XONLINES (sections XONLINE and XNET).
The XBE has no BINK section and no Bink strings: movies are XMV (42 `.xmv` files).

Sections: `.text 11000-1BF7FC`, `D3D 1BF800-1D2EDC`, `D3DX`, `XGRPH`, `DSOUND`, `WMADEC`, `XMV`, `XONLINE`,
`XNET`, `XPP 26C740-2755E8`, `.rdata`, `.data`, `DOLBY`, `XON_RD`, `.data1`, `$$XTIMAGE`, `$$XSIMAGE`, `.XTLID` (352 ids).

**5659 seed.** The first run stopped (only 5849 signatures). Created `sdk/xdk/5659/` from copies of
`sdk/xdk/5849/{layout,signatures}.toml` and re-ran. Result: **135 of 238 functions, 8 of 12 data**.
- 24 found only by `.XTLID` id (Direct3D_CreateDevice, 4 IDirectSound*, all the XNet/Winsock and XOnlineStartup/Cleanup ones).
- 12 found by constants. Low scores to double check in Ghidra: D3DDevice_SelectVertexShader 0x1C0100 (0.60),
  SetRenderState_LineWidth 0x1C1320 (0.60), SetTextureState_TexCoordIndex 0x1C1670 (0.65), D3D_InitDefaultState
  0x1C9280 (0.67), SetVertexShader 0x1C03F0 (0.75), SetTexture 0x1C3FE0 (0.76), SetPixelShader 0x1C25C0 (0.78).
  1.00: DrawIndexedVertices 0x1C09B0, DrawVertices 0x1C0910, SetRenderState_StencilFail 0x1C2210; 0.88: SetPixelShaderConstant 0x1C27C0, SetShaderConstantMode 0x1C01A0.

Misses (left at 0 in `config/game.toml`):
- Not applicable (no Bink in this game): BinkDoFrame BinkGetSummary BinkNextFrame BinkWait
- D3D (60, first counted as 63 by mistake): CDevice_KickOff D3D_BlockOnResource D3D_BlockOnTime D3D_KickOffAndWaitForIdle D3D_MakeRequestedSpace D3D_SetFence
  D3DDevice_BeginPush BlockOnFence BlockUntilVerticalBlank Clear CopyRects DeletePixelShader DeleteVertexShader DrawVerticesUP
  EnableOverlay End EndPush FlushVertexCache GetBackBuffer2 GetDepthStencilSurface2 GetRenderTarget2 GetVertexShaderFunction
  InsertFence IsFencePending LightEnable LoadVertexShader MakeSpace PersistDisplay Release SetFlickerFilter SetGammaRamp
  SetIndices SetLight SetMaterial SetPalette SetRenderState_{DoNotCullUncompressed Dxt1NoiseEnable MultiSampleAntiAlias
  MultiSampleMode MultiSampleRenderTargetMode OcclusionCullEnable PSTextureModes RopZCmpAlwaysRead RopZRead StencilCullEnable
  StencilEnable TextureFactor YuvEnable ZEnable} SetRenderTarget SetScissors SetScreenSpaceOffset SetSoftDisplayFilter
  SetTextureStageStateNotInline SetTextureState_BumpEnv SetTile SetTransform SetVertexShaderConstant4 SetViewport Swap
- DSOUND (29): CDirectSoundStream_SetFormat CDirectSoundStream_SetVelocity DirectSoundUseFullHRTF DirectSoundUseLightHRTF
  IDirectSoundBuffer_{GetCurrentPosition Lock SetAllParameters SetI3DL2Source SetMaxDistance SetMinDistance SetOutputBuffer
  SetRolloffCurve StopEx Unlock} IDirectSoundStream_SetEG IDirectSound_{CommitEffectData CreateSoundBuffer DownloadEffectsImage
  GetOutputLevels GetSpeakerConfig SetDistanceFactor SetDopplerFactor SetOrientation SetPosition SetRolloffFactor SetVelocity}
- XNet/Winsock (6): WSARecvFrom accept getsockname getsockopt listen send (may simply not be linked)
- CRT (7): crt_atan crt_cos crt_log crt_log10 crt_sin crt_sqrt crt_tan
- Data (4): D3D_DeferredTextureState D3D_DirtyFlags D3D_IndexData D3D_RenderState
  (DirtyFlags/RenderState are read from SelectVertexShader/SetPixelShader, whose bytes did not match)
Found data: D3D_Device 0x1D09E0, D3D_pDevice 0x1D09D8, D3D_Streams 0x1CF630, D3D_SimpleRenderStateEncode 0x2759D0,
DSound_StreamVtable 0x283F70 / 2 0x283F60, XPP_GamepadType 0x26C7F8, XPP_MemoryUnitType 0x26C778.

Two misses identified on the way (not yet put in the config):
- 0x1C6140 is the push buffer space request (MakeSpace / D3D_MakeRequestedSpace): every inline push compares
  `[D3D_Device]` with `[D3D_Device+4]` and calls it, e.g. 0x1852E3 (game) and in D3D 0x1C5290, 0x1C7390.
- 0x1C34A0 is D3DDevice_SetTransform: copies 16 dwords to `pDevice + 0xC50 + type*64`, ors a dirty bit into
  0x1D0538 (likely D3D_DirtyFlags). **5849 has CDevice_Transforms = 0xC60**, so the CDevice layout has moved
  in 5659 and `sdk/xdk/5659/layout.toml` must be re-derived entry by entry (header comment added saying so).

**Ghidra.** `xbe2pe.py` -> `.local/game.exe` (135 exports), `analyzeHeadless ~/ghidra-projects pgr2 -import` OK (~3 min).
Helper scripts in `.local/ghidra_scripts/` (PushBufferScan, DeviceRefs, Decomp; git-ignored).

**Push buffers.**
- No `D3DDevice_CreatePushBuffer`/`CreateFixup`: nothing anywhere builds a resource header with type
  push buffer (0x20001) or fixup (0x60001). The only push buffer type tests in D3D are generic resource code:
  0x1C3360 (Register: keeps a push buffer's address unmasked, called from 10 game sites for XPR data),
  0x1C3090 (resource teardown).
- No `RunPushBuffer` found: no D3D function reads a push buffer object and emits an NV2A jump/call to it.
- The 30 XPR0 bundles in `assets/` hold 222 resources, all textures (type 4); no push buffer resources.
  The `.pak*` archives were not inspected.
- **The game does write hardware commands itself.** Four `.text` functions push straight into the device
  push buffer, inlining MakeSpace: 0x1852D0, 0x185330, 0x1853B0, 0x185450 (only `.text` code touching
  `[0x1D09E0]`/`[0x1D09E4]`). Each writes `0x41EA4, slot` (SET_TRANSFORM_CONSTANT_LOAD) then `0x400B80`/`0x800B80`
  (16 or 32 dwords of SET_TRANSFORM_CONSTANT), i.e. hand-rolled vertex shader constant uploads (16/32 floats,
  e.g. matrices), and clear bits in a game global 0x49DFD8. Callers not yet traced.
  The runtime's `Walk` (`sdk/runtime/xdk/d3d8.cpp`) only decodes immediate-mode vertex methods and
  begin/end; it **ignores 0x1EA4/0x0B80**, so these constants would be silently lost and geometry using them
  would draw with stale constants. Needs a decision (Walk support or X86_FUNC overrides of the 4 functions).
- Other `.text` hits for method-header-shaped immediates were checked and are colours/flags, not commands.

**Repo notes.** `sdk/xdk/5659/` is committed in the `sdk` submodule only locally: the submodule's remote is
upstream TGP482/XboxXBE-Recomp, so the superproject's gitlink was left at 40e9c35 to avoid pointing at an
unpushed commit. Harry needs to decide where SDK commits are pushed (fork of the SDK, or vendor it).

**Next.** Verify the low-score constant matches, find the remaining D3D misses (starting with MakeSpace
0x1C6140, SetTransform 0x1C34A0, Swap, Clear, SetRenderTarget, SetViewport) and re-derive each `layout.toml`
offset from 5659 code; then decide how to handle the 4 direct constant pushers; then recompile and first boot.
