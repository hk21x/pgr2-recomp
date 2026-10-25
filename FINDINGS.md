# PGR2 recomp - findings

What the work so far has established, by topic. PROGRESS.md has the dated log with the evidence for each item.
Each item is marked:
- **[console]**: how the Xbox, its XDK or the runtime behaves. It holds for any title and the fix is in the SDK.
- **[PGR2]**: the game's own code and data. Addresses are PGR2's (XDK 5659, title id 0x4D53004B).

## The image

- **[PGR2]** Releases: the North American `default.xbe` (region NA+Japan, version 5, SHA-1 ec6e902f...) differs from
  the PAL one (region rest of world, version 2, SHA-1 676895b8...) in 531 bytes, all in the signature (0x4-0x103)
  and the certificate (time stamp, region, ratings, version, signing keys). Every section is byte for byte the same,
  so every address in this project holds for both; the NTSC disc runs the smoke test unchanged. The signing keys
  differ, so a save signed by one release may not be accepted by the other.
- **[PGR2]** Media patched copies (pgr2-variant.iso, 2026-10-23: a rebuilt XISO of the PAL disc): every data file the
  same, all seven .xbe files (default, update, dashupdate, downloader, three demos) changed in one byte, the jge after
  XapiVerifyMediaInDrive (default.xbe 0x193964, file 0x183964) made a jmp, signature not redone: the patch that lets a
  modded console run the game from its hard disk. That code is XAPI start-up the host replaces, so the PAL and NTSC
  patched files (SHA-1 acd77633..., 702b79fd...; the NTSC one computed, the same byte) are listed as the same code.
- **[PGR2]** XDK 1.0.5659 for every library (XAPILIB, D3D8, D3DX8, XGRAPHC, DSOUND, XBOXKRNL, XMV, XONLINES, XVOICE,
  LIBCMT, LIBCPMT). XNet is linked inside XONLINES. The movies are XMV (42 files); there is no Bink.
- **[PGR2]** Push buffers: there is no CreatePushBuffer or RunPushBuffer, and the XPR bundles hold only textures.
  Four game functions push vertex shader constants by hand (0x1852D0, 0x185330, 0x1853B0, 0x185450). The runtime's
  `Walk` decodes those commands (0x1EA4 load slot, 0x0B80 constants), so this is handled.
- **[console]** 5659 needed its own `layout.toml`. The CDevice tail moved -0x10 from 5849, ZScale and SuperSample
  moved -4, and the front fields moved -4. Every offset is cited to a PGR2 instruction. Signatures:
  191 of 250 functions, 13 of 13 data.
- **[console]** A function the scan cannot match is left at 0 and becomes a trapping stub. A library function that
  runs its own code against device state the host never set up crashes or silently does the wrong thing. Every
  one of these was found when the game called it, then moved to the host and signed in:
  Reset, SetDepthClipPlanes, SetRenderTargetFast, SetVertexShaderInput(Direct), PresentInterval,
  XapiVerifyMediaInDrive, PlayEx/Pause, SetDopplerFactor, XNetGetConnectStatus, XNetUnregisterInAddr.

## Boot, files, saves

- **[console]** Cache partition. XAPI's formatter asks IOCTL_DISK_GET_PARTITION_INFO; whole partitions are now raw
  volumes. `FindFirstFile("*.*")` sends an empty name, which must match everything. NtUserIoApcDispatcher (the
  ReadFileEx completion) and IoQueryVolumeInformation (file sort keys) are now implemented.
- **[console]** The DVD authentication mode page (MODE SENSE 0x3E) must be answered, or XAPI init fails.
- **[console]** DirectSoundGetSampleTime reads an APU counter that never moves on the host. PGR2's frontend clock
  ran from it: menus froze at their first letter. It is now a host count of 48 kHz samples.
- **[PGR2]** Profiles are at `E:\UDATA\4d53004b\<id>\Profile.Sav` (30100 bytes, a 20-byte signature first). They
  are saved on creation, on leaving Options and on quitting a race. Byte 0x844 holds MPH/KM/H.
  `scripts/savetest.sh` checks this.
- **[PGR2]** The profile loader (0xA87B0) checks Profile.Sav's 20-byte signature as a roamable XCalculateSignature
  (Begin 0x18BBB4 with 0, End 0x18BC16) over the rest, keyed with the title signature key a real kernel takes from
  the certificate (XboxSignatureKey, zero on the host), then the version (5) and each section's length; a mismatch
  is 0x3EE, "damaged". A profile from a real Xbox (an Action Replay copy) can never match, so src/game hands the
  loader its stored signature and its own checks decide; saving signs it as this install's.

## Frame pacing and speed

- **[console]** Titles pace themselves with the present interval (0x80000000 + vblanks), and the runtime now paces
  every Swap by it. PGR2 runs its menus at 60 (interval 1) and its races at 30 (interval 2), as on the console.
  Pacing by `fps_cap` alone ran races at double speed.
- **[console]** Swap honours COPY / BYPASSCOPY / FINISH. PGR2 draws its own copy of the supersampled back buffer
  between Swap(2) and Swap(4); presenting at every Swap showed that copy pass as a flickering white quad.
- **[PGR2]** The frame step is FUN_000D90F0: dt = interval / refresh, and physics takes fixed 1/60 steps
  (FUN_000F5D60). The interval setter is 0x177C70. `race_60fps` clamps 2 to 1, so a race at 60 runs the same
  simulation as at 30 (an experiment, off by default).

## Rendering: console behaviours the runtime now matches

- **[console] Pushed commands keep their order.** Raw commands a title pushes (constants, visibility tests) are
  read at the next draw. Every state and constant setter must first read what is pending, or a later direct set
  is overwritten by an earlier push. That bug put the player's car body off screen.
- **[console] W-buffering (D3DZB_USEW, ZENABLE 2).** PGR2 uses it on nearly every draw. Depth is the pixel's own w,
  interpolated perspective correct, plus the polygon offset in w units, stored as floor(w) in 24-bit units. xemu
  interpolates w linearly on the screen, which streaks long triangles. A Z buffer lost the showroom car's panels.
  - Vertex programs carry w already scaled to depth units, in their own matrices.
  - Fixed-function and pre-transformed (XYZRHW) vertices are scaled by the XDK by ZScale / far w:
    - SetTransform keeps 1 / far w of the projection.
    - 0x1C0FC0 folds the scale into the fixed-function matrix.
    - 0x1C0300 puts it in the pass-through program's c0.w.
  - SetTransform is on the host, so the runtime now computes this itself. Without it, lamp glows and their
    visibility probes showed through buildings.
- **[console] A depth buffer bigger than the colour target serves it.** The console uses the depth buffer's top-left
  part. PGR2 renders the showroom car into a 500x200 texture with the 640x480 depth buffer bound; dropping depth
  for the size mismatch let the far wheel show through the body.
- **[console] SetVertexShaderInput(Direct)** replaces the stream sources and the declaration for the draws that
  follow. Without it, PGR2's fences, lamp posts and signs read packed shorts as floats and vanished.
- **[console] SetRenderTargetFast** must switch targets on the host. PGR2 draws its six reflection cube faces with
  it. The faces drawn on the GPU are copied into the cube once a frame.
- **[console] Visibility tests.** Titles poll GetVisibilityTestResult inside a frame, so D3D_SetFence flushes when a
  report was asked for. In PGR2 a light glow's brightness comes from a visibility probe: an XYZRHW quad drawn
  between Begin and EndVisibilityTest (0x114FA0 / 0x114D20, 0x1C42F0).
- **[console] Other fixes:**
  - Immediate mode under an FVF keeps every slot written. The movies' texture coordinates are in slot 9.
  - A declaration longer than the stride reads the next vertex's bytes.
  - 8-bit targets keep their blue byte.
  - Only 32-bit linear surfaces are written back.

## Rendering: PGR2's own techniques

- **Trees** are point sprites: a vertex program sets each point's size (oPts, console pixels, 2 to 64) and the sprite
  supplies the texture coordinates; drawn as single pixels until 2026-10-21, every tree looked bare.

- **Car shadow.** The car's 1024x1024 D16 shadow map is projected onto a ground mesh built each frame
  (vs B0101931) with a polygon offset and no depth writes. With a Z buffer it failed the road's depth test by up to
  4096 units and flickered; `src/game/draw_filter.cpp` drew it 4096 units nearer. Since W-buffering that nudge is a
  quarter of a metre and put the shadow on the car's lower body in the sun, while the plain pass looks right: the
  nudge is off (`PGR2_SHADOW_NEARER=1` brings it back).
- **Polygon offsets** match the XDK exactly at 3x upscale. The layers that rely on them are road shadows, kerbs and
  decals.
- **Light pool under the car.** This is a 256x256 target drawn each frame, sampled in stage 3 by the road. The
  under-car flicker at night was much improved by W-buffering.
- **Reflections.** A 256x256 R5G6B5 cube (six faces via SetRenderTargetFast) is added as specular. Bump-mapped
  reflection shaders also sample a 256x256 reflection target.
- **Combiner modes.** PGR2 sets stage 0 through its own mode switch FUN_00105ED0 (cache 0x3F29A4): mode 1 is
  MODULATE(diffuse, texture) and mode 5 is SELECTARG1(texture). The exhaust backfire flames (FUN_00021620 via
  FUN_00021CD0, car +0x1508, 'EXHL'/'EXHR') leave mode 5 set. Draws with a pixel shader are not affected.
- **Car lighting.** The body's sun term is c56 = sun colour x the car's brightness (0x1F036). The brightness eases
  to 0.25 per wheel standing on an unflagged track triangle (bit 7 of the triangle's flag byte means shade). The
  data, the query and the test all agree with the console's code.
- **Depth of field (replays, the attract demo).** PGR2 copies the frame into a 640x480 target (0x03595000, read as
  LIN_X8R8G8B8), then draws it back over the frame (ONE, INVSRCALPHA) reading its depth buffer 0x03B20000 as a colour
  texture, LIN_A8B8G8R8 (format 0x3F): the console gives the D24S8 word's bytes there, depth's top byte in alpha, and
  the blur follows them. The renderers sampled every depth target as a shadow map (a comparison), so every pixel read
  as far and the whole picture blurred (Harry, 2026-10-22); a depth target read through a 32 bit colour format now
  gives its bytes (kTexDepthBytes).
- **Widescreen.** PGR2 reads XGetVideoFlags: widescreen renders anamorphic 16:9 into 640x480, with a wider field of
  view and the HUD moved out. It has no HD mode. More pixels come from the runtime's render scale.
- **Cameras and the rear view mirror.** Every camera is set up by 0x1857F0 (thiscall: its box x, y, w, h on the 640
  wide screen; box at +0x28..+0x34, applied by 0x185640), aspect at +0x38 = 0x2790F8 (16:9; 0x2790FC 4:3 without the
  widescreen flag 0x48614C) times the box's shape. The mirror of the bumper and hood views is a camera with the box
  220,38 200x70 drawn on the frame itself. Ultrawide (src/game/hooks.cpp): 0x2790F8 is the frame's shape, and a box
  touching neither side is narrowed about the centre by 16:9 / that shape with its 16:9 aspect, as the HUD is
  squeezed. Controller layouts: 0x34CC1C, 0xFC each, two button ids per action from +8 (action names 0x28A1F8, button
  names 0x28F960); the default has CAMERA_NEXT on Black, CAMERA_PREV on White.

## The gearbox

- **[PGR2]** The player's gear is the signed byte at 0x29DACA + car x 0x6990 (the gearbox 0x29DAB0, +0x1A; -1 reverse,
  1 first; there is no neutral). The car record is 0x2973B0 + car x 0x6990 (speed at +0x19A0); the gearbox's
  +0x24 points to it. Shifts go through 0x19EB0 (up) and 0x19F60 (down) from 0x1A7B0: the automatic logic 0x1A580 when
  0x16520 says automatic, and the car's requests +0x1BC4 (up) / +0x1BC5 (down), which the pad's B and X set.
  Automatic: the byte 0x39B320 + player x 0x2498 (set at Select Transmission: Auto), or the car's mode +0x19C8 or the
  dword 0x39B31C + player x 0x2498 being 1 or 2. Measured headless with gdb watchpoints and shift logging, 2026-10-22.

## Time of day and weather

- **[PGR2]** Conditions are picked in FUN_000F14E0 (race setup) from per-city tables, after a rand() call. Night is
  0x383188 = 1 and weather is 0x383184 (2 = rain/overcast). Night in Instant Action is a normal random roll on the
  console too.
- Each city has three lighting sets (`Game\Areas\<city>.pak_day`, `pak_overcast`, `pak_night`), chosen at area load
  (0x116C20). The log line `pgr2: area loads with time of day T, weather W` shows the choice. `PGR2_NIGHT=1`
  forces night (an experiment).

## Xbox Live content

- **[PGR2]** Content packs are listed from T:\$C\<offer id> by XAPI's XFindFirstContent (0x192134) with a category
  mask: the disc build asks for 1 (car packs, 0x157180) and 2 (city packs, 0x1572F0: the pack's cities.ini and
  Routes.ini); the Paris and Long Beach booster packs are category 0x10 (ContentMeta.xbx +0x30), made for the title
  update (T:\$U, version 0x202, a different build). Each pack's header is checked against a keyed signature
  (0x19144B / 0x191366) and each file's against the metadata's table (0x156F00); community copies are unsigned
  until a console installs them. PGR2_DLC=1 lists them, passes the checks and puts the pack's object paks, audio
  files and ambience lines where the disc build reads them (src/game/dlc.cpp).
- **[PGR2]** A pack as downloaded lists each file's plain SHA-1 in its ContentMeta.xbx (all 20 of Paris's, all 18 of
  Long Beach's); a console's install re-signs them (0x155510, XCalculateSignature), so those cannot be checked off the
  console. With DLC on, the log says whether each pack's files match, and names any that do not. A damaged pak makes
  the city stop on PGR2's fatal error "m_CompressionStream.msg" (zlib 1.1.4 inflate failing in 0x136E70).
- **[PGR2]** Paris draws some of its garbage meshes with stream strides like 0xFFFFFFFC: their vertices lie gigabytes
  outside the guest's memory. The GPU renderers copied them and faulted in memcpy (the Windows crash a second after
  Paris loads); the SDK now leaves out any draw whose vertices lie outside committed guest memory (logged
  `gpu: a draw reads vertices outside the guest's memory`). The DLC cities and their routes are registered in the
  menus, after boot: route indexes past 145 are empty until then, and PGR2_ROUTE on one loads Edinburgh against it.
- **[PGR2]** City material object 0x3FE8C0, its slots in arrays of 100: pixel constants +0x0 (16 bytes), colours
  +0x640 (0x3FEF00), then material types +0x7D0 (0x3FF090, 8 bytes: shaders, vertex size in the low 6 bits of +7),
  kinds +0x8E30 (0x4076F0, a byte), animated textures +0x8E94 (0x407754, 0xC4 each, 64; each frame's stream block)
  and refs +0xBF94 (0x40A854, 128); counts +0xC194 on (0x40AA54 types, 0x40AA56 slots, 0x40AA5A animations,
  0x40AA5E refs). Slots are appended by the city pak loader 0x16CFA0 (through 0x16C0D0, then 0x107820 appends the
  animations) and the streaming loader 0x16C450; 0x117F30 (load) and 0x1186C0 (each frame) write each slot's pixel
  constant from its kind and colour (they read nothing else of the object); a draw (0x118B90) names its slot in byte
  +6 of its record; 0x1BEFE0 and 0x1073C0 reset. Paris has 113 slots: the extra overwrote material types 0 to 6 (the
  missing floor: type 5's stride 0 - 4 = -4), its kinds landed on the animated textures (a frame asked for stream
  block 7742: "m_CompressionStream.msg") and the per-frame pass wrote constants over the colours. The title update
  made the arrays 150 long. src/game/city_tables.cpp keeps the slots past 100 aside and the count at 100.
- **[PGR2]** The title update's default.xbe (in the DLC's uPAL folder) has the disc build's code with addresses moved;
  .local/tu has a Ghidra project of it (pgr2tu, .local/tu/g.sh runs the helper scripts on it). Aligning both builds'
  disassembly and pairing their globals shows what it enlarged: the city material arrays (100 to 150), the paint
  colours (600 to 700, 84 bytes each from 0x32F1F4), two lists beside the car table (0x310454 and 0x310888, +0x50
  each) and 0x20 bytes in the audio manager 0x465638 (its fields from +0x2CA0 move). The city streaming code (blocks
  0x3F2D38, unpacked in place by 0x1003E0) and its constants are the same.
- **[PGR2]** What the disc build cannot take from the booster packs (title update data): object paks in the newer layout
  (a zlib compressed WMSH chunk first, where the disc's open with an INDX index: read as garbage meshes), materials
  whose bucket is past the city draw list's 106 (0x11B210, table 0x457188: a null write), and night or overcast sets
  (the packs hold the day set). PGR2 draws an object it cannot set up with its error material, pixel constant 0 magenta.
- **[PGR2]** Paks: the disc's layout opens with INDX (tag, 0, offset of END - 12, count), then 16 byte entries (tag, 0,
  size, offset - 12), each chunk's data 16 byte aligned, the file ending with END's header (END, 0, 0). The title
  update's has each chunk inline (tag, 0, size with 0x80000000 compressed and 0x40000000, data; compressed data a 4 byte
  size and the game's own deflate stream). The chunks are byte for byte the same, so the layout converts losslessly
  (ConvertPak in src/game/dlc.cpp; the boats and the 18 DLC car models).
- **[PGR2]** The title update's 16 cars (its folder: cars.ini, CarColours.ini, Classes.ini, Manufacturers.ini, each
  car's dcontentcarN.pak_hrd/_opn/_sft, dcontentcarN.ini handling, <AudioData>.dat/.xwb) load as a disc car pack
  (category 1, 0x157180) once the models are converted. The INI loader's tables (handler 0x14DD80 by section type)
  are fixed size: cars 0x3109F0 cap 200 (0x270 bytes each from 0x3109F4), colours 0x32F1F0 cap 600 (0x14A8C0; past it
  a null write at 0x14E07C), windscreens 0x32F174 cap 10, classes (0x3103EC, 0x38 each) max 20, type 8 0x310884
  max 45 (45 with the pack loaded; what it lists not yet known). The disc has 102 cars and 566 colours; the update's 48 colours do not fit.
  Each player's car is 0x39B318 + player x 0x2498 (read by 0x17C50).
- **[PGR2]** A fatal error goes through 0x1388F0(message): it formats the cache partition and stops; look for its
  caller (Audio::Manager::Setup when a city has no stereo.ini line).

## System link

- **[console]** XNetGetConnectStatus must answer CONNECTED. The library's own version answers LOST when its network
  object is null, so PGR2 flagged the host as leaving and dropped it ("lost connection").
- **[console]** Loopback instances on Linux shift ports below 1024 up 20000 (PGR2 binds 1000 to 1002).
- **[PGR2]** The lobby, the race and the results work with two headless instances (`scripts/linktest.sh`).

- **[PGR2]** Clear-weather Nurburgring: the race's frame buffer clear is depth and stencil only (flags 3), so the sky
  dome (vertex shader with 1351-index strip, pixel shader blending a 256x128 gradient and a cloud texture) has to
  cover every pixel the scenery leaves. It and the far hills lie past the far plane; the console's W-buffer does not
  clip there, the GPU renderers did: old frames showed through (a white sky, brown bars that flashed while driving).
  Since sdk (W-buffered draws keep z within w) the vertex shaders clamp instead. The DLC TVR Cerbera Speed 12's Car
  Challenge is on "Nurburgring 1" by the title update's cars.ini.

## Sound

- **[PGR2]** The music volume is the float 0x367528 (0 to 1; Instant Action from the title runs at the default 0.7,
  a player's game at their setting). The menus' music carries it in its mix bin levels (-577 mB full, -1179 half);
  in races the radio goes to effect send 14 alone at 0 mB, its samples at full scale, and the DSP image applies the
  volume. The mixer plays such sends into the speakers itself, so GameInfo::send_level gives it the volume.

- **[console]** The software mixer plays everything PGR2 uses: streams, ADPCM, the engine via SetFrequency, and
  the music fades.
- Not modelled (accepted, no effect): the DSP effects image, I3DL2 reverb, filters and Doppler. Sound is dry.
- **[PGR2]** The in-race radio (WMA from `data\audio\Radio\tracks`, decoded by the game's own WMADEC code into two
  44.1 kHz stereo PCM streams) goes to the effects send FXSEND 14 (mix bin 24) alone, every speaker bin at -10000: the
  DSP image plays it into the speakers (the in-car sound). With the effects not modelled it was silent in every build
  (Harry, 2026-10-23). A voice with no audible speaker bin now plays its effect sends into the speakers (dry, stereo
  kept); `XBE_DSOUND_LOG=2` lists every stream holding packets with its bins.

## Lessons on working

- Game code registers through an explicit call from `GameInfo::init`, never a static initializer: the Windows exe
  dropped one.
- Leftover environment variables from earlier experiments cause false regressions. The log now lists every `XBE_*`
  and `PGR2_*` variable at startup.
- Captures flush the GPU every frame and never caught a timing-dependent flicker. The frame log (`XBE_FRAMELOG`, F4)
  records at full speed and found the one-frame state changes.
- The CPU reference renderer and the D3D12 backend are written separately. A fault in both (the showroom depth) is
  in shared state handling; a fault in one only is in that renderer.
- When a draw writes nothing, check the inputs before the shaders: vertex format (SetVertexShaderInput), constant
  order (pushed vs direct), target and depth sizes, and the depth mode.

## Still open

- Any remaining flicker on Windows (the Edinburgh night buildings and the under-car patch are fixed).
- The CPU reference renderer's random spike pattern in single snaps.
- START during the second intro movie hung the player earlier. It now skips, but this was not root-caused.
- Sound effects (reverb, filters) are not modelled.
