# Building and running PGR2 on Windows

Everything below runs on your PC; nothing here touches the Linux VM. FINDINGS.md explains the fixes referred to here.

## What you need

- Windows 10 or 11, 64 bit, a GPU with Direct3D 12 or 11.
- Visual Studio 2022 Community with **Desktop development with C++**. `build.bat` offers to add the two extra
  components it needs (**C++ Clang tools for Windows**, **C++ CMake tools for Windows**) through the Visual Studio
  Installer; say Y, or add them by hand under Modify, Individual components.
- Git for Windows: https://git-scm.com/download/win
- 7-Zip on the PATH, only for `release.bat`.
- Your PGR2 disc's files (PAL or NTSC, both run from the same build): `default.xbe` with the folders beside it (Cache, CarDynamics, data, Frontend, Game,
  media, ...), exactly as they are in the VM's `assets` folder. An .iso of the disc also works.
- About 15 GB free: the generated C++ alone is around 100 MB, the object files several GB.

## Steps

1. Clone with the SDK (both repositories are public, no sign-in needed).
   ```
   git clone --recurse-submodules https://github.com/hk21x/pgr2-recomp.git
   cd pgr2-recomp
   ```
   Already cloned before the SDK moved? Run `git pull`, then `git submodule sync`, then
   `git submodule update --init`.
2. Put the game in `assets\`: `assets\default.xbe` and its folders, or one `.iso` (then also copy
   `default.xbe` from the disc into `assets\`, the build recompiles it from there).
3. Run `build.bat` from Explorer or a plain command prompt.
   - [1/3] finds Visual Studio and clang.
   - [2/3] says `config\game.toml already written`. It must: the config is committed and must not be
     regenerated. If it ever runs `xbe_symbols` instead, stop and tell me.
   - [3/3] recompiles default.xbe (seconds) and compiles everything (30 to 90 minutes the first time).
   - Answer Y to "Play now?", or run `out\build\release\PGR2.exe` yourself.
4. The first start formats the emulated cache partition and copies the frontend files to it, then plays the four
   intro movies (about two minutes). Later starts reuse it. To repeat the first start, delete
   `out\build\release\hdd`.

Files the exe writes beside itself, in `out\build\release`:
- `logs\pgr2.log` the log of the last run (overwritten each start; send it with any report)
- `pgr2.toml` settings (window, vsync, graphics options). Speed: the game's own frame rate is honoured (menus
  60, races 30, as on the console); `fps_cap` only caps it further (60 by default, 0 for no cap of its own).
  `race_60fps = true` runs races at 60 instead (an experiment: same physics, one 1/60 step a frame).
- `hdd\` the emulated hard disk (E: saves, X/Y/Z: caches)

**Settings window: F1**, **Back + Start** on a controller, or in the game's menus **Options > General Options > PC
Settings** (it replaces Screen Center; closing the window returns to General Options by itself) (Start alone closes it; the D-pad or left stick moves,
A selects, B goes back). **Quit game** at its top right asks, then closes the game (as Alt+F4 or the window's close
button do). Tabs:
- Display: monitor (with more than one), display mode, resolution, vsync, aspect ratio (Fill the screen: ultrawide).
- Graphics: render scale, anisotropic filtering, frame rate cap, MSAA (2x/4x/8x), FXAA, sharpening (0 to 100%), frames-a-second
  counter, renderer (Direct3D 12 or 11, next start), **Sharp text and HUD** (on by default: the menus', HUD's and
  text's textures are enlarged two or four times with sharp edges on a background thread, so lettering and gauges stay
  crisp at high resolutions; a texture shows at its original size for a moment the first time it appears).
- Game: skip intro movies, hide the race HUD, race camera field of view (%), races at 60 (experimental).
- General: pause when the window is not in front.
- Controls: keys and controller remapping.
Everything is saved to `pgr2.toml` at once. Display mode, resolution, monitor, render scale, MSAA, FXAA, sharpening, the counter, hide HUD
and field of view apply at once; aspect ratio, skip intro and race_60fps at the next start. While the
window is open the game gets no input.

Keyboard (plays as controller 1 while the game window has the focus; remap in F1 > Controls, two keys per input):
- Steer / menus: arrow keys or A / D (left stick); accelerate W (right trigger), brake S (left trigger)
- A: Enter or Space; B: Backspace; X: Q; Y: E; Black: R; White: F; Start: Escape or P; Back: Tab
- D-pad: numpad 8 / 2 / 4 / 6; right stick: I / J / K / L; stick clicks: C and V
Controller remapping: F1 > Controls, the last column picks which physical input drives each console input.
- `CrashDumps\` a minidump when it crashes

Useful while testing:
- **F3** writes every game thread's stack to the log. With the environment variable `XBE_CAPTURE_DIR` set to a
  folder (`set XBE_CAPTURE_DIR=C:\pgr2cap` before starting the exe from the same prompt) it also captures the
  next frame there. The log says `F3: thread stacks, the next frame captured` when the key arrives and
  `d3d12: capturing a frame into ...` when it starts. Without the key: create an empty file named `capture` beside
  `PGR2.exe` (`type nul > out\build\release\capture` from a second prompt) and the next frame is captured.
- `XBE_CAPTURE_FRAMES=6` makes F3 (or the capture file) capture that many frames in a row, one folder each
  (`HHMMSS_01` ... `HHMMSS_06`): for a flicker, so a good and a bad frame can be compared.
- Experiments, each `0` or `1` (the log says when one is set): `XBE_RENDER_THREAD` (drawing on its own thread, on
  by default), `XBE_RAW_VERTICES` (vertex data straight to the GPU, on by default), `XBE_STATE_WALK` (state set in
  program order, on by default).
- `XBE_DRAW_SKIP=00000042 B0101C41` (experiment): draws with those vertex shader handles are left out, to find
  which layer of the scene flickers (the log names the first one skipped).
- `XBE_FRAMELOG=60`: F4 records the next 60 frames' draws into `logs\framelog.txt` at full speed (no images, no
  slowdown worth noticing), with hashes of what each draw reads; for flicker a capture does not catch.
- `XBE_WBUFFER=0`: back to the old depth (z / w). By default depth works as the console's W-buffer, which PGR2
  asks for on nearly every draw (car selection panels, the showroom car's wheels, street lights behind buildings).
- `XBE_NO_TARGET_UPLOAD=1` (experiment): render targets the game reads (light pools, reflections) are never reloaded
  from memory. The log now lists such reloads (`reloaded from guest memory`) without any switch.
- `XBE_VISIBLE_ALL=1` (experiment): every GPU visibility test answers "visible" at once (lamp glows and anything
  the game draws only when visible).
- `XBE_ZBIAS=3` (experiment): every polygon offset (the depth nudge that keeps shadows and road markings on top of
  the road) times that number. Captures now list each render target's guest and host size at the top of frame.txt.
- `XBE_PERF=1` logs frame timing; `XBE_D3D12_DEBUG=1` turns on the Direct3D 12 debug layer (slow).
- `XBE_VSC=56 54` logs who writes those vertex shader constants and the values (each distinct one once); used to
  find why the player's car body is black.
- `XBE_CAPTURE_DRAWS=1580-1660` makes a capture also save the image after each draw in that range (the numbers in
  the capture's frame.txt); `XBE_CAPTURE_DRAWS=vs:B01019E1` after every draw with that vertex shader (the car body),
  plus once before the first.
- `XBE_DRAW_TEST=B01019E1:alphakill` (or `:cull`, `:depth`, several joined by commas): an experiment, draws with that
  vertex shader without that test.
- Game switches (experiments, `1` to turn on): `PGR2_NIGHT` every race at night; `PGR2_SHADOW_NEARER` the car shadow
  pass drawn 4096 depth units nearer (the old flicker fix, off by default since W-buffering); `PGR2_FLARE_LOG` / `PGR2_FLARES` exhaust flame diagnostics; `PGR2_LIGHT` /
  `PGR2_LIGHT_CLEAR` car lighting diagnostics. `XNET_TRACE=1` logs every network call (system link).
- Leftover variables from an earlier experiment give false regressions: the log lists every `XBE_*` and `PGR2_*`
  variable it sees at the top. Clear them (`Remove-Item Env:XBE_DRAW_TEST` in PowerShell) when done.
- Setting these: in Command Prompt `set XBE_VSC=56 54`; in PowerShell `$env:XBE_VSC = "56 54"` (PowerShell's
  `set` does not set an environment variable). Start `PGR2.exe` from that same window.

`release.bat` zips `PGR2.exe` with an empty `assets` folder; not needed yet.

## Steering wheels and rigs (2026-10-22: any DirectInput devices, several at once)

Every DirectInput game controller is opened (wheel bases, pedal boxes, button boxes, shifters, as separate USB devices
or one), except Xbox pads, which stay with the controller support. They play together as controller 1, with the
keyboard and any pad. Each control is bound to one device: in F1 > **Wheel**, click a control, then turn, press or move
it on whichever device it is on.
- **Devices**: every device found, with its axes, buttons, hats and whether it offers force feedback ("in use": the
  forces go there). The log lists them at start: `wheel: <name> (USB vid:pid, type), N axes (their names), N buttons, N
  hats, force feedback offered/none`. Any of a device's axes binds (X Y Z Rx Ry Rz and two sliders), wherever they are:
  a pedal box on Z, Rz and a slider is fine.
- **Steering**: click, then turn the wheel to the right (it finds the device and the axis, and the direction). Wheel
  rotation as set in the wheel's own software (Simucube True Drive, G HUB ...; up to 2880), steering lock (degrees of
  wheel for the game's full lock, 360 by default), dead zone.
- **Pedals**: throttle, brake (and clutch, unused by the game): let the pedal rest, click, press it fully.
- **Force feedback**: on the device the steering is bound to, on the steering's axis (the pedal and button boxes are
  only read). Strength, road and impacts; Push left / Push right test it. The device's own centring spring is turned
  off while the game has it.
- **Buttons**: each console button (the d-pad's Up, Down, Left and Right, A B X Y, Black/White, Start/Back, the sticks'
  clicks) from a button on any device. The steering device's hat is the d-pad too, so a wheel without a hat binds
  the d-pad to buttons. Back + Start opens the settings window.
- **H-pattern shifter**: bind Reverse and gears 1 to 7 to the shifter's positions. With a manual gearbox (chosen at
  Select Transmission), moving the lever presses the game's own shift up (B) and down (X) until the car is in that gear;
  in neutral nothing happens. With an automatic gearbox the shifter does nothing. The tab shows the lever's gear and
  the car's.
- **Raw input**: each device's axes and held buttons, to see what Windows reports.
- With the steering bound to a device that is plugged in, controller 1 tells the game it is a steering wheel, as an
  Xbox wheel did: the game then takes the steering as it comes, without the pad's dead zone and stick shaping (before
  2026-10-27 it got those, a large dead zone then a bent response). The Wheel tab's rotation, lock and dead zone are
  then the only ones. The log says `xapi: controller 1 is a steering wheel`.
- Without any binding set up, the first device reporting itself as a driving device is used with the G920 layout
  (steering X, throttle Y, brake Rz, clutch slider 1). Anything else (a Simucube base, pedal and button boxes, a pad
  with rumble) is only used once bound in the Wheel tab.
To check (Harry's rig: Simucube OSW, Leo Bodnar BU0836-LC pedals, SLI-PRO buttons, Thrustmaster TH8A): all four listed
in the log and the tab; steering bound to the Simucube with force feedback on it; the three pedals from the BU0836-LC;
buttons from the SLI-PRO; the TH8A's gears in a manual car; nothing doubles up with a pad plugged in.

## The launcher (for players without Visual Studio)

`PGR2Launcher.exe` builds and plays the game for someone with no tools: choose the disc image or folder, accept
Microsoft's license for its C++ libraries, press Build, then Play. It downloads portable build tools (about 1.4 GB:
Clang, CMake, Ninja, MinGit, Microsoft's C++ libraries and Windows SDK) into a `PGR2` folder beside itself, installs
nothing in Windows and needs no admin rights. The first build takes 30 to 90 minutes and about 15 GB; "Remove the
build files" frees most of it afterwards. Later it offers Update when this repository's main branch moves on.

Make it: run `launcher.bat` (it needs what build.bat needs); it leaves `PGR2Launcher.exe` here. The exe holds no game
code, so it can be shared. To try it as a player would: copy it into an empty folder (ideally on a PC or account
without Visual Studio) and run it. Send `PGR2\launcher.log` with any report. The Linux side tested each step but
the build itself (`msiexec` for the SDK, then the compile) only runs on Windows.

## Your music (custom soundtracks)

PGR2 plays Xbox custom soundtracks (CDs the dashboard ripped to the hard disk). The launcher makes them from your
music: in its **4  Your music** section, *Choose a music folder...* and pick the folder.
- Each folder inside becomes a soundtrack named after it (with everything under it, so an artist's albums can sit in
  one); the songs directly in the chosen folder make one more. At most 100 soundtracks of 504 songs (the Xbox's).
- MP3, FLAC, OGG, Opus, M4A, AAC, WAV, WMA and AIFF files are converted to the Xbox's WMA with FFmpeg, which the
  launcher downloads the first time (about 90 MB, into its tools folder). Song names come from the tags
  ("Artist / Title"), else the file names; names are cut to 31 characters, as on the Xbox.
- *Update my music* after adding, changing or removing songs: only those are converted.
- In the game: Options > Sound / Music > **PGR 2 Custom Soundtrack** > Add/Remove Songs (the triggers change
  soundtrack: yours follow the game's own), then in a race choose the custom soundtrack in the pause menu's
  Radio/CD Options.
- The files go in `play\hdd\E\TDATA\FFFE0000\MUSIC` (`ST.DB` and numbered folders). A MUSIC folder copied from a
  real Xbox works as it is; the launcher then leaves it alone (move it away to use a folder of yours instead).

## Xbox Live DLC cities and cars (experimental)

Paris and Long Beach from the PGR2 booster packs, on the disc's game. Confirmed on Windows (2026-10-21): both race,
a few of their objects are left out. The 16 DLC cars (M3 CSL, Corvette C6, 250TR, ...) from the title update: headless
they load, show in the car counts (Time Attack: 118 car challenges) and race (M3 CSL, 356A, Atom 2, SR3 seen); Windows
to confirm.
1. From your DLC files, copy the folders `content\c\4d53004b00000001` (Paris) and `4d53004b00000002` (Long Beach) into
   `out\build\release\hdd\E\TDATA\4d53004b\$C\` (make the `$C` folder), so that
   `hdd\E\TDATA\4d53004b\$C\4d53004b00000001\Paris.pak_day` exists.
   For the cars, also copy the update folder `content\uPAL` (or `uNTSC`: the cars are the same) into that `$C`
   folder, so that `hdd\E\TDATA\4d53004b\$C\uPAL\cars.ini` exists. The first start with DLC on makes a car pack
   from it, `$C\4d53004b0000FF01` (the models converted, every DLC car free); the log says
   `the title update's cars from ... made into the car pack` and then `car pack T:\$C\4d53004b0000FF01 loaded`.
   With DLC on, the log checks each pack's files against the pack's own list: `content pack ...: all N files match`,
   or a line naming each damaged file (copy that pack again).
2. F1 > Game > **Xbox Live DLC cities**, then restart (or `dlc = true` in `pgr2.toml`; `PGR2_DLC=1` also works).
3. Paris and Long Beach are in the city lists (Time Attack, Instant Action, race setup); the DLC cars are in the
   classes "Paris Cars" and "Long Beach Cars".
What differs from the console: night and overcast races use the daytime scenery (the packs hold the day set only);
the river boats and the cars' models are converted from the title update's file layout; objects with title update
materials are left out (PGR2 would draw them magenta). The disc game holds 600 paint colours and has 566, so the DLC
cars keep 34 of their own 48 (each car keeps some; colours shared with disc cars are all kept). `PGR2_CAR=102` to `117`
puts player 1 in that DLC car, for testing. Headless, `scripts/pad/paris.txt` drives a Paris Time Attack from a normal
boot (the DLC cities get their route numbers in the menus, so `PGR2_ROUTE` cannot pick them).

## System link

On one LAN: start PGR2 on both PCs, then Multiplayer > System Link: one creates a game, the other searches.
Past the LAN: set `join_ip = 'a.b.c.d'` (the host's address) in the joiner's `pgr2.toml`; the search also asks that
host. On one PC: copy `out\build\release` to a second folder and start each copy with `PGR2.exe --instance 1` and
`--instance 2`. Headless this works end to end (lobby, race, results); it has not been tried on Windows yet.

## Language

Options > General Options > PC Settings > Language, or F1 > Game: English, Japanese, German, French, Spanish, Italian,
Korean or Chinese, at the next start. That start copies the game's files again (about two minutes of movies), as the
first one did. PC Settings' own rows stay in English; Japanese keeps the disc's English text in the race HUD.

## What to look at and report

Help and reports: the [Discord server](https://discord.gg/PQjkKsdtc). The files below help with any report.

Please note for each: works / wrong (how) / not reached, and send `logs\pgr2.log`. Screenshots and short clips help;
for a flicker, a frame log (F4 with `XBE_FRAMELOG=60`) at the spot.

Confirmed on Windows so far: boot, movies, menus, races, saves, sound, reflections, the player's car body, the car
shadow, fences and signs, car selection, the F1 window, the Direct3D 11 renderer (with MSAA), the PC Settings entry, the achievement pop-up.

Still to confirm:
1. **Trees** (2026-10-21): leafy trees in Sydney's terrace streets (point sprites), on D3D12 and D3D11.
2. **Flicker**: kerbs, signs, anything left (the Edinburgh buildings and the under-car patch are fixed).
3. **Keyboard and remapping**: drive a race on the keyboard; remap a key and a controller input, restart, check
   they were kept.
5. **System link**: two PCs, or two copies with `--instance`.
7. **Sound**: music volume in the menus, the engine, anything missing (reverb and filters are not modelled).
