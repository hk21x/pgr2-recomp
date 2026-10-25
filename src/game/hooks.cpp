// Hooks for default.xbe. A strong definition replaces the recompiled function at that address, for example:
//
// #include "xdk/hle.h"
//
// X86_EXTERN_FUNC(__imp__sub_00012345);
// X86_FUNC(sub_00012345) {
//     __imp__sub_00012345(ctx, base);  // the original
// }
#include <algorithm>
#include <atomic>
#include <cctype>
#include <cmath>
#include <mutex>
#include <string>
#include <unordered_map>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "app/app.h"
#include "core/log.h"
#include "kernel/kernel.h"
#include "xdk/d3d8.h"
#include "xdk/hle.h"

namespace {

// PGR2_LIGHT set: each car's lighting inputs whenever its wheel flags, the sun colour or its brightness (in 1/20s)
// change, the first 400 changes; to find why the car's light colour (vertex constant c56) is 0 on Windows
float F(uint32_t va) {
    uint32_t v = kernel::R32(va);
    float f;
    std::memcpy(&f, &v, 4);
    return f;
}

void LogCarLight(const char* where, uint32_t car) {
    static const bool on = std::getenv("PGR2_LIGHT") != nullptr;
    static std::unordered_map<uint32_t, uint64_t> last;  // per car: flags, sun, brightness in 1/20s
    static std::mutex m;
    if (!on || !car) return;
    uint64_t now = uint64_t(kernel::R32(car + 0x5B0) & 0xFF) | uint64_t(kernel::G<uint8_t>(car + 0x754)) << 8 |
                   uint64_t(kernel::G<uint8_t>(car + 0x8F8)) << 16 | uint64_t(kernel::G<uint8_t>(car + 0xA9C)) << 24 |
                   uint64_t(kernel::R32(0x49EEA8)) << 32;
    now ^= uint64_t(int(F(car + 0x150C) * 20)) * 0x9E3779B97F4A7C15ull;
    {
        std::lock_guard lock(m);
        auto [it, added] = last.try_emplace(car, now);
        if (!added && it->second == now) return;
        it->second = now;
        static int n = 0;
        if (n++ >= 400) return;
    }
    // brightness +0x150C is 0.25 per wheel whose surface flag (+0x5B0, +0x754, +0x8F8, +0xA9C) is clear, eased in
    // (unless 0x3B1EC7 is set and the mode 0x368928 is not 1 or 6: then 1); the sun colour is the D3DCOLOR 0x49EEA8
    LOGR("pgr2 light %s car %08X brightness %g sun %08X wheel flags %u %u %u %u colours %08X %08X %08X %08X "
         "+cc %g +d0 %g 3B1EC7 %u 368928 %u 49E110 %u",
         where, car, F(car + 0x150C), kernel::R32(0x49EEA8), kernel::G<uint8_t>(car + 0x5B0), kernel::G<uint8_t>(car + 0x754),
         kernel::G<uint8_t>(car + 0x8F8), kernel::G<uint8_t>(car + 0xA9C), kernel::R32(car + 0x5B4), kernel::R32(car + 0x758),
         kernel::R32(car + 0x8FC), kernel::R32(car + 0xAA0), F(car + 0xCC), F(car + 0xD0), kernel::G<uint8_t>(0x3B1EC7),
         kernel::R32(0x368928), kernel::G<uint8_t>(0x49E110));
}

}  // namespace

// the car's lighting from its wheels' surfaces, at the start (0x1CDD0) and each frame (0x20690); ecx the car
X86_EXTERN_FUNC(__imp__sub_0001CDD0);
X86_FUNC(sub_0001CDD0) {
    uint32_t car = ctx.ecx;
    __imp__sub_0001CDD0(ctx, base);
    LogCarLight("start", car);
}

X86_EXTERN_FUNC(__imp__sub_00020690);
X86_FUNC(sub_00020690) {
    uint32_t car = ctx.ecx;
    __imp__sub_00020690(ctx, base);
    LogCarLight("frame", car);
}

// PGR2_LIGHT also tallies the flag byte (+0xB) of every distinct track triangle the ground query (0xC6210, ecx the
// grid cell: +8 count, records of 16 bytes from +0x18) looks at, logged every 2000 queries
X86_EXTERN_FUNC(__imp__sub_000C6210);
X86_FUNC(sub_000C6210) {
    static const bool on = std::getenv("PGR2_LIGHT") != nullptr;
    if (!on) return __imp__sub_000C6210(ctx, base);
    // which threads query, and whether two are inside at once
    static std::atomic<int> inside{0}, overlaps{0};
    {
        static std::mutex tm;
        static std::unordered_map<uint32_t, uint32_t> threads;  // guest stack page: calls
        std::lock_guard lock(tm);
        if (threads[ctx.esp >> 16]++ == 0) LOGR("pgr2 light: query from a new thread, stack %08X", ctx.esp);
    }
    if (inside++ > 0 && overlaps++ < 20) LOGR("pgr2 light: two ground queries at once (stack %08X)", ctx.esp);
    {
        static std::mutex m;
        static std::unordered_map<uint32_t, uint8_t> seen;
        static uint32_t calls = 0, hist[256] = {};
        uint32_t cell = ctx.ecx, n = kernel::R32(cell + 8);
        std::lock_guard lock(m);
        for (uint32_t i = 0; i < n && i < 4096; i++) {
            uint32_t e = cell + 0x18 + i * 0x10;
            uint8_t f = kernel::G<uint8_t>(e + 0xB);
            if (seen.emplace(e, f).second) hist[f]++;
        }
        if (++calls % 2000 == 0) {
            uint32_t bit7 = 0;
            std::string top;
            for (int b = 0; b < 256; b++) {
                if (b & 0x80) bit7 += hist[b];
                if (hist[b]) top += " " + std::to_string(b) + ":" + std::to_string(hist[b]);
            }
            LOGR("pgr2 light: %zu triangles queried, %u with bit 7; flag bytes%s", seen.size(), bit7, top.c_str());
        }
    }
    __imp__sub_000C6210(ctx, base);
    inside--;
}

// PGR2_LIGHT: each wheel's ground query (0x24DE0, ecx the wheel): its position (+0x40), the height found (+4), the
// flag (+0xD8) and colour (+0xDC), the first 200
X86_EXTERN_FUNC(__imp__sub_00024DE0);
X86_FUNC(sub_00024DE0) {
    uint32_t w = ctx.ecx;
    __imp__sub_00024DE0(ctx, base);
    static const bool clear = [] {  // experiment: every wheel sunlit
        bool on = std::getenv("PGR2_LIGHT_CLEAR") != nullptr;
        if (on) LOGR("pgr2: PGR2_LIGHT_CLEAR on, every wheel counts as sunlit");
        return on;
    }();
    if (clear) kernel::G<uint8_t>(w + 0xD8) = 0;
    static const bool on = std::getenv("PGR2_LIGHT") != nullptr;
    static std::atomic<int> n{0};
    if (on && n++ < 200)
        LOGR("pgr2 wheel %08X at %g %g %g ground %g flag %u colour %08X", w, F(w + 0x40), F(w + 0x44), F(w + 0x48), F(w + 4),
             kernel::G<uint8_t>(w + 0xD8), kernel::R32(w + 0xDC));
}

namespace pgr2 {
extern bool race_60fps, skip_intro, hide_hud;
extern uint32_t fov_percent, draw_distance_percent;
void ApplySettingsEntry();  // src/game/settings_entry.cpp
}

// skip_intro: the boot sequence 0x13DB90 plays Intro1, Intro2, the language's movie and Intro3 through the movie
// player 0x13C810 (stdcall: path, flag; true when played), waiting for the cache copy between them. Those four
// report "played" at once, so the waits stay. (Frontend.ini's IntroMovies, the byte 0x291AB6, is never read.)
X86_EXTERN_FUNC(__imp__sub_0013C810);
X86_FUNC(sub_0013C810) {
    static const char* kIntros[] = {"intro1.xmv", "intro2.xmv", "intro3.xmv", "english.xmv", "japanese.xmv",
                                    "german.xmv", "french.xmv", "spanish.xmv", "italian.xmv", "korean.xmv",
                                    "chinese.xmv"};
    if (pgr2::skip_intro) {
        std::string path;
        for (uint32_t p = kernel::R32(ctx.esp + 4); p && path.size() < 260; p++) {
            char c = char(kernel::G<uint8_t>(p));
            if (!c) break;
            path += char(std::tolower(static_cast<unsigned char>(c)));
        }
        std::string name = path.substr(path.find_last_of('\\') + 1);
        for (const char* intro : kIntros)
            if (name == intro) {
                LOGR("pgr2: intro movie %s skipped", path.c_str());
                ctx.eax = 1;
                ctx.esp += 12;  // ret 8
                return;
            }
    }
    __imp__sub_0013C810(ctx, base);
}

// fov_percent: the race cameras' fields of view (radians, 0.975 each: the camera table's fov entries, Frontend's
// camera_settings.ini keys fov0, fov1, inCar_fov0, inCar_fov1, read by 0x17B1D0 and 0x17C200), scaled. A value that
// is not the one written last is the game's own and becomes the one scaled.
void ApplyFov() {
    static const uint32_t kFov[] = {0x49DCF0, 0x49DD10, 0x49DD28, 0x49DD3C};
    static float original[4], written[4];
    static uint32_t logged = 100;
    uint32_t pct = std::clamp<uint32_t>(pgr2::fov_percent, 50, 200);
    for (int i = 0; i < 4; i++) {
        float cur;
        std::memcpy(&cur, &kernel::G<uint32_t>(kFov[i]), 4);
        if (cur != written[i]) original[i] = cur;
        float f = original[i] * float(pct) / 100;
        if (f != cur) std::memcpy(&kernel::G<uint32_t>(kFov[i]), &f, 4);
        written[i] = f;
    }
    if (pct != logged) LOGR("pgr2: race camera field of view %u%% (%g rad)", pct, original[0] * float(pct) / 100);
    logged = pct;
}

// draw_distance_percent: how far the race cameras see. Cities.Ini gives every city DrawDistance = 3000, kept in the
// city table (0x68 bytes a city from 0x30D670, the distance at +0x48; 17 cities); race setup (0x1808C0) copies the
// city's into 0x49DFDC (2000 in one mode) and the race camera (0x1859A0) makes it its far plane, times 1, 0.75 or 0.25
// by view; other cameras (0x1857F0) read the table. Both are scaled; a value that is not the one written last is the
// game's own.
struct Scaled {
    float original = 0, written = 0;
    void Apply(uint32_t va, uint32_t pct) {
        float cur;
        std::memcpy(&cur, &kernel::G<uint32_t>(va), 4);
        if (cur != written) original = cur;
        float f = original * float(pct) / 100;
        if (f != cur) std::memcpy(&kernel::G<uint32_t>(va), &f, 4);
        written = f;
    }
};
uint32_t DrawPercent() { return std::clamp<uint32_t>(pgr2::draw_distance_percent, 25, 400); }

void ApplyDrawDistance() {
    static Scaled cities[17];
    static uint32_t logged = 100;
    uint32_t pct = DrawPercent();
    for (int i = 0; i < 17; i++) cities[i].Apply(0x30D6B8 + i * 0x68, pct);
    if (pct != logged) LOGR("pgr2: draw distance %u%% (%g)", pct, cities[0].written);
    logged = pct;
}

X86_EXTERN_FUNC(__imp__sub_001859A0);
X86_FUNC(sub_001859A0) {
    static Scaled race;
    race.Apply(0x49DFDC, DrawPercent());
    __imp__sub_001859A0(ctx, base);
}

// ultrawide: the game's widescreen 3D views (race cameras 0x1857F0, garage, showroom: 0x88480, 0x16E960, 0x184530)
// take their aspect from the constant 0x2790F8 (16:9; 0x2790FC is 4:3) when the widescreen flag 0x48614C is set;
// it is set to the shape the frame is shown at, so the view widens instead of stretching
void ApplyAspect() {
    float want = std::max(app::FrameAspect(), 16.0f / 9.0f);
    float cur;
    std::memcpy(&cur, &kernel::G<uint32_t>(0x2790F8), 4);
    if (std::fabs(cur - want) < 1e-4f) return;
    std::memcpy(&kernel::G<uint32_t>(0x2790F8), &want, 4);
    LOGR("pgr2: widescreen 3D views at %.3f:1", want);
}

// ultrawide: a camera (0x1857F0, thiscall: x, y, w, h of its box on the 640 wide screen, ..., an aspect or 0 at +0x1C)
// keeps the box at +0x28..+0x34 and its aspect, 0x2790F8 times the box's shape, at +0x38. The rear view mirror in the
// bumper and hood views is one, 220,38 200x70: on a frame wider than 16:9 its box stretched with the screen and ran into
// the HUD. A box touching neither side is narrowed about the centre by 16:9 / the aspect used, as the HUD is
// (gpu::HudSqueeze), and given the aspect it has at 16:9: the box and its picture as at 16:9. Full width and split
// screen views reach a side and keep theirs.
X86_EXTERN_FUNC(__imp__sub_001857F0);
X86_FUNC(sub_001857F0) {
    uint32_t cam = ctx.ecx;
    int32_t x = int32_t(kernel::R32(ctx.esp + 4)), w = int32_t(kernel::R32(ctx.esp + 12));
    uint32_t given = kernel::R32(ctx.esp + 0x1C);
    __imp__sub_001857F0(ctx, base);
    float aspect;
    std::memcpy(&aspect, &kernel::G<uint32_t>(0x2790F8), 4);
    if (!kernel::G<uint8_t>(0x48614C) || given || aspect <= 16.0f / 9.0f + 1e-3f || x <= 0 || x + w >= 640) return;
    float k = 16.0f / 9.0f / aspect, own;
    int32_t nx = int32_t(std::lround(320 + (x - 320) * k)), nw = std::max(1, int32_t(std::lround(w * k)));
    kernel::W32(cam + 0x28, uint32_t(nx));
    kernel::W32(cam + 0x30, uint32_t(nw));
    std::memcpy(&own, &kernel::G<uint32_t>(cam + 0x38), 4);
    own *= k;
    std::memcpy(&kernel::G<uint32_t>(cam + 0x38), &own, 4);
    static int said = 0;
    if (said++ < 8) LOGR("pgr2: camera box %d,%u %dx%u narrowed to %d wide at %d for the %.3f:1 frame", x,
                         kernel::R32(cam + 0x2C), w, kernel::R32(cam + 0x34), nw, nx, aspect);
}

// profiles from a real console: the profile loader (0xA87B0) compares Profile.Sav's first 20 bytes with the roamable
// signature (XCalculateSignatureBegin 0x18BBB4 with 0, End 0x18BC16) of the rest, keyed with the title's signature key,
// which a real kernel takes from the disc's certificate and the host leaves at zero: a profile saved on an Xbox (an
// Action Replay copy) failed with 0x3EE, "damaged". The comparison is given the stored signature, so the loader's own
// checks decide: the version (5) and every section's length. Saving again signs it as this install's.
X86_EXTERN_FUNC(__imp__sub_0018BC16);
X86_FUNC(sub_0018BC16) {
    uint32_t ret = kernel::R32(ctx.esp), out = kernel::R32(ctx.esp + 8);
    __imp__sub_0018BC16(ctx, base);
    if (ret != 0xA8967) return;  // the profile loader's call; its stored signature 0x14 bytes before the digest
    uint8_t* digest = &kernel::G<uint8_t>(out);
    const uint8_t* stored = &kernel::G<uint8_t>(out - 0x14);
    if (std::memcmp(digest, stored, 20) == 0) return;
    std::memcpy(digest, stored, 20);
    LOGR("pgr2: a profile signed by another console (an Xbox's save): signature accepted, the profile's own checks follow");
}

// the music volume (Options > Sound / Music; 0x367528, 0 to 1, read by 0x5DD70 and others). In races the radio plays
// only through effect send 14 at full level and the DSP image puts it into the speakers at this volume (the menus'
// music has it in its mix bin levels instead: -577 mB full, -1179 half); the mixer stands in for the image
// (GameInfo::send_level), so the radio follows the setting again
namespace pgr2 {
float MusicLevel() {
    float f;
    std::memcpy(&f, &kernel::G<uint32_t>(0x367528), 4);
    return f == f ? f : 1.0f;
}
}  // namespace pgr2

// hide_hud: HideOverlays (a Frontend.ini switch, the byte 0x291A24), which the HUD code (0xDA000-0xE8000) reads each
// frame, set before every frame step (0xD90F0) so the option applies at once
X86_EXTERN_FUNC(__imp__sub_000D90F0);
X86_FUNC(sub_000D90F0) {
    static bool was = false;
    if (pgr2::hide_hud || was) kernel::G<uint8_t>(0x291A24) = pgr2::hide_hud ? 1 : 0;
    was = pgr2::hide_hud;
    pgr2::ApplySettingsEntry();
    ApplyDrawDistance();
    ApplyAspect();
    __imp__sub_000D90F0(ctx, base);
    ApplyFov();
}

// race_60fps (settings file) or PGR2_RACE60: races at 60 frames a second. The game sets its presentation interval
// through 0x177C70 (thiscall, the interval 0, 1 or 2): 1 in the movies and menus, 2 in races. Asked for 2, this
// passes 1: the swap paces to 60 and the frame step (0xD90F0, 1 / (60 / interval)) becomes 1/60, so physics takes
// one fixed 1/60 step a frame instead of two.
X86_EXTERN_FUNC(__imp__sub_00177C70);
X86_FUNC(sub_00177C70) {
    static const bool on = [] {
        bool v = pgr2::race_60fps || std::getenv("PGR2_RACE60") != nullptr;
        if (v) LOGR("pgr2: races at 60 frames a second");
        return v;
    }();
    if (on && kernel::R32(ctx.esp + 4) == 2) kernel::W32(ctx.esp + 4, 1);
    __imp__sub_00177C70(ctx, base);
}

// a city's area loads (0x116C20, the lighting set chosen inline from 0x383188 and 0x383184): the values it loads with
X86_EXTERN_FUNC(__imp__sub_00116C20);
X86_FUNC(sub_00116C20) {
    LOGR("pgr2: area loads with time of day %u (1 night), weather %u (2 overcast) from %08X", kernel::R32(0x383188),
         kernel::R32(0x383184), kernel::R32(ctx.esp));
    __imp__sub_00116C20(ctx, base);
}

// PGR2_NIGHT=1 (experiment): every race at night. The conditions are picked in 0x0F14E0 (it writes 0x383188 itself,
// found with a watchpoint); night is set right after, before the race copies it and loads the city's night set.
// PGR2_ROUTE=N (test): the race is on route N
// PGR2_WEATHER=N (test): the race's weather (0x383184: 0 clear, 2 overcast), set after the roll as night is
// PGR2_CAR=N (test): player 1 drives car N (the car table 0x3109F0, 0x270 bytes from 0x3109F4; the players' cars at
// 0x39B318 + player x 0x2498, which 0x17C50 reads when it builds the car)
X86_EXTERN_FUNC(__imp__sub_000F14E0);
X86_FUNC(sub_000F14E0) {
    static const long route = [] {
        const char* e = std::getenv("PGR2_ROUTE");
        long r = e && *e ? std::strtol(e, nullptr, 0) : -1;
        if (r >= 0) LOGR("pgr2: PGR2_ROUTE=%ld, every race on that route", r);
        return r;
    }();
    // race setup (ecx the race object 0x383178) works from its event, [ecx+0x20] into the event table 0x33BEAC (0x68
    // bytes each, the route at +0x10 as the frontend's 0x52262 sets it); the route 0x383180 and its city 0x38317C (the
    // route table's first dword, 0x30DEA4 + route x 0x20) as 0x522AB sets them
    // not the attract demo (+0x6D4, 0x38384C, set before setup): it picks its own city and loads it from that
    if (route >= 0 && !kernel::G<uint8_t>(ctx.ecx + 0x6D4)) {
        uint32_t event = kernel::R32(ctx.ecx + 0x20);
        kernel::W32(0x33BEAC + event * 0x68 + 0x10, uint32_t(route));
        kernel::W32(0x383180, uint32_t(route));
        kernel::W32(0x38317C, kernel::R32(0x30DEA4 + uint32_t(route) * 0x20));
    }
    static const long car = [] {
        const char* e = std::getenv("PGR2_CAR");
        long c = e && *e ? std::strtol(e, nullptr, 0) : -1;
        if (c >= 0) LOGR("pgr2: PGR2_CAR=%ld, player 1 drives that car", c);
        return c;
    }();
    if (car >= 0 && uint32_t(car) < kernel::R32(0x3109F0) && !kernel::G<uint8_t>(ctx.ecx + 0x6D4)) kernel::W32(0x39B318, uint32_t(car));
    __imp__sub_000F14E0(ctx, base);
    if (car >= 0) LOGR("pgr2: player 1's car %u", kernel::R32(0x39B318));
    if (route >= 0) LOGR("pgr2: race setup on route %u, city %u", kernel::R32(0x383180), kernel::R32(0x38317C));
    static const bool night = [] {
        const char* e = std::getenv("PGR2_NIGHT");
        bool on = e && *e == '1';
        if (on) LOGR("pgr2: PGR2_NIGHT=1, every race at night");
        return on;
    }();
    if (night) kernel::W32(0x383188, 1);
    static const long weather = [] {
        const char* e = std::getenv("PGR2_WEATHER");
        long w = e && *e ? std::strtol(e, nullptr, 0) : -1;
        if (w >= 0) LOGR("pgr2: PGR2_WEATHER=%ld, every race in that weather (0 clear, 2 overcast)", w);
        return w;
    }();
    if (weather >= 0) kernel::W32(0x383184, uint32_t(weather));
}

// PGR2's stage 0 combiner modes (0x105ED0(mode): 1 MODULATE(diffuse, texture), 5 SELECTARG1(texture), ...; it skips a
// mode equal to its cache 0x3F29A4): each call noted in the frame log with the cache and the caller, to see who picks
// full bright for a building for one frame
X86_EXTERN_FUNC(__imp__sub_00105ED0);
X86_FUNC(sub_00105ED0) {
    uint32_t mode = kernel::R32(ctx.esp + 4), cache = kernel::R32(0x3F29A4), ret = kernel::R32(ctx.esp);
    __imp__sub_00105ED0(ctx, base);
    char note[96];
    std::snprintf(note, sizeof note, "combiner mode %u (cache %u) from %08X colorop %u arg1 %u", mode, cache, ret,
                  kernel::R32(0x1D0570), kernel::R32(0x1D0578));
    d3d::FrameLogNote(note);
}

// a car's light flares (0x21CD0, ecx the car): drawn when its byte +0x1508 is set, in combiner mode 5, which the next
// draws inherit. Noted in the frame log when set; PGR2_FLARE_LOG=1 logs the first 40 cars it is set for
X86_EXTERN_FUNC(__imp__sub_00021CD0);
X86_FUNC(sub_00021CD0) {
    uint32_t car = ctx.ecx;
    uint8_t flag = kernel::G<uint8_t>(car + 0x1508);
    static const bool log_cars = std::getenv("PGR2_FLARE_LOG") != nullptr;
    static std::atomic<int> cars{0};
    if (log_cars && cars++ < 3) LOGR("pgr2: flares for car %08X, flag %u", car, flag);
    if (flag) {
        char note[64];
        std::snprintf(note, sizeof note, "flares car %08X flag %u", car, flag);
        d3d::FrameLogNote(note);
        static const bool log = std::getenv("PGR2_FLARE_LOG") != nullptr;
        static std::atomic<int> n{0};
        if (log && n++ < 40) LOGR("pgr2: %s", note);
    }
    __imp__sub_00021CD0(ctx, base);
}

// the gate before a car's flares (0x1C9C0, ecx the car): PGR2_FLARE_LOG=1 logs its inputs, the first 20 calls
X86_EXTERN_FUNC(__imp__sub_0001C9C0);
X86_FUNC(sub_0001C9C0) {
    uint32_t car = ctx.ecx;
    __imp__sub_0001C9C0(ctx, base);
    static const bool log = std::getenv("PGR2_FLARE_LOG") != nullptr;
    static std::atomic<int> n{0};
    if (!log || n++ >= 20) return;
    uint32_t p = kernel::R32(car + 0x94), cam = kernel::R32(0x3CD804);
    LOGR("pgr2: flare gate car %08X -> %u: 15d1 %02X, owner %08X, owner 19c8 %u 19cc %u 19d0 %u, camera 40 %u", car,
         ctx.eax & 0xFF, kernel::G<uint8_t>(car + 0x15D1), p, p ? kernel::R32(p + 0x19C8) : 0, p ? kernel::R32(p + 0x19CC) : 0,
         p ? kernel::G<uint8_t>(p + 0x19D0) : 0, cam ? kernel::R32(cam + 0x40) : 0);
}

// the cars' flare pass (0x314E0, ecx the car list 0x297390, +0xC the count): PGR2_FLARE_LOG=1 logs the count, 5 times
X86_EXTERN_FUNC(__imp__sub_000314E0);
X86_FUNC(sub_000314E0) {
    static const bool log = std::getenv("PGR2_FLARE_LOG") != nullptr;
    static std::atomic<int> n{0};
    if (log && n++ < 5) LOGR("pgr2: flare pass over %u cars", kernel::R32(ctx.ecx + 0xC));
    __imp__sub_000314E0(ctx, base);
}

// the race view's drawing (0x17FCC0, ecx the view, [4] its camera): flares and other effects are skipped when the
// camera's mode +0x128 is 12 and 0x291A27 is not -1. PGR2_FLARE_LOG=1 logs those when they change
X86_EXTERN_FUNC(__imp__sub_0017FCC0);
X86_FUNC(sub_0017FCC0) {
    static const bool log = std::getenv("PGR2_FLARE_LOG") != nullptr;
    if (log) {
        uint32_t cam = kernel::R32(ctx.ecx + 16);
        uint32_t now = (cam ? kernel::R32(cam + 0x128) : 0xFFFF) << 8 | kernel::G<uint8_t>(0x291A27);
        static uint32_t last = ~0u;
        if (now != last) LOGR("pgr2: race view camera mode %u, 291A27 %d", (last = now) >> 8, int8_t(now & 0xFF));
    }
    // PGR2_FLARES=1 (experiment): 0x291A27 -1, so the flare pass runs in camera mode 12 as in the other views
    static const bool flares = std::getenv("PGR2_FLARES") != nullptr;
    if (flares) kernel::G<uint8_t>(0x291A27) = 0xFF;
    __imp__sub_0017FCC0(ctx, base);
}

// the system link session's teardown (0x165AF0, ecx the session): XNET_TRACE=1 logs who ends it, with the guest stack
X86_EXTERN_FUNC(__imp__sub_00165AF0);
X86_FUNC(sub_00165AF0) {
    static const bool trace = std::getenv("XNET_TRACE") != nullptr;
    if (trace) {
        LOGR("pgr2: link session %08X ends, from %08X", ctx.ecx, kernel::R32(ctx.esp));
        // its players: [+0x3730, +0x3734) of 0x2DB0 bytes each, +0x76 their connected flag
        uint32_t b = kernel::R32(ctx.ecx + 0x3730), e = kernel::R32(ctx.ecx + 0x3734);
        LOGR("pgr2: players %08X..%08X", b, e);
        for (uint32_t p = b; p && p < e && p - b < 16 * 0x2DB0; p += 0x2DB0)
            LOGR("pgr2:   player %08X flag 76 %u, first words %08X %08X %08X %08X", p, kernel::G<uint8_t>(p + 0x76),
                 kernel::R32(p), kernel::R32(p + 4), kernel::R32(p + 8), kernel::R32(p + 12));
        kernel::LogGuestStack(ctx, 24);
    }
    __imp__sub_00165AF0(ctx, base);
}

// a link player's removal (0x1654D0, ecx the session, the player's id): XNET_TRACE=1 logs the timeout inputs
X86_EXTERN_FUNC(__imp__sub_001654D0);
X86_FUNC(sub_001654D0) {
    static const bool trace = std::getenv("XNET_TRACE") != nullptr;
    if (trace) {
        uint32_t s = ctx.ecx, id = kernel::R32(ctx.esp + 4), now = kernel::R32(kernel::R32(0x27562C));
        LOGR("pgr2: link player %08X removed (args %u %u), tick %u, timeout %u, from %08X", id, kernel::R32(ctx.esp + 8),
             kernel::R32(ctx.esp + 12), now, kernel::R32(s + 0x2F28), kernel::R32(ctx.esp));
        for (uint32_t p = kernel::R32(s + 0x3730), e = kernel::R32(s + 0x3734); p && p < e; p += 0x2DB0)
            LOGR("pgr2:   player %08X last heard %u, +74 %u +76 %u +2dac %u", kernel::R32(p + 0x24), kernel::R32(p + 0x60),
                 kernel::G<uint8_t>(p + 0x74), kernel::G<uint8_t>(p + 0x76), kernel::G<uint8_t>(p + 0x2DAC));
    }
    __imp__sub_001654D0(ctx, base);
}

// PGR2's fatal error (0x1388F0(message): formats the cache partition, then the "problem with the disc" screen): the
// message and the caller in the log
X86_EXTERN_FUNC(__imp__sub_001388F0);
X86_FUNC(sub_001388F0) {
    std::string msg;
    for (uint32_t p = kernel::R32(ctx.esp + 4); p && kernel::G<uint8_t>(p) && msg.size() < 200; p++) msg += char(kernel::G<uint8_t>(p));
    LOGE("pgr2: fatal error \"%s\" from %08X", msg.c_str(), kernel::R32(ctx.esp));
    logx::Flush();
    __imp__sub_001388F0(ctx, base);
}
