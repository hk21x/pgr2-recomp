// PC Settings in PGR2's own menu: Options > General Options > Screen Center (which a PC monitor has no use for)
// becomes PC Settings, a page of the game's own option rows for the everyday picture settings, the last row opening
// the settings window (F1) for the rest.
//
// General Options is built by 0xA40A0: each item a button with a page builder (0x58970), a label (0x89200) and a help
// line (0x891E0), both pointers from the UTF-16 string table at 0x28A870 by StringList.ini id. Its first page
// (0xA1710) is a list of option rows, which this page copies:
//   the frame 0x2FEE08: +0x34 the title's string id, 1.0 at +0x38..+0x40, the byte +0x45 1, then thiscall 0x7A110
//     with 0xC and 0xB (the second after zeroing +0x38 and +0x3C when the byte 0x49E110 is set)
//   a row 0x58EE0 (cdecl, 17 arguments): x, y (floats, 0 and 144 + 24 a row, here 22 to fit ten), 1, label, help line, the int it
//     changes, the number of choices, 2, 0, eight choice texts; Left and Right step the int through the choices,
//     wrapping (0x4D130 on the row's actions 2 and 3), and the row shows the choice it holds
//   the button legend 0x28A488: 0x28A720 = 1, +0 and +0x28C = 0, then 0x89200 (A's text, id 21 "Continue", 0) and
//     0x891E0 (id 1393, as Game Options has); the heading above the rows ("Your Options") is the game's own
//   the page manager [0x2F4B78] (thiscall): 0x4D3A0 (rows, count, 1, 1) chains them up and down, 0x4D4C0 (rows,
//     count, action, page builder, 2) sends an action (4 A, 5 B) to a page, 0x4D270 (row, -1) gives one the focus
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>

#include "app/app.h"
#include "core/log.h"
#include "kernel/kernel.h"
#include "kernel/memory.h"
#include "xdk/hle.h"

namespace pgr2 {

extern uint32_t fov_percent, draw_distance_percent;

namespace {

constexpr uint32_t kStrings = 0x28A870;  // string table: a UTF-16 pointer per id
constexpr uint32_t kGeneralOptions = 0xA40A0, kPage = 0xA2E60;  // page builders
constexpr uint32_t kFrame = 0x2FEE08, kManager = 0x2F4B78, kLegend = 0x28A488;
constexpr uint32_t kRow = 0x58EE0, kFrameMode = 0x7A110, kLegendText = 0x89200, kLegendHelp = 0x891E0;
constexpr uint32_t kChain = 0x4D3A0, kAction = 0x4D4C0, kFocus = 0x4D270;
constexpr uint32_t kEmpty = 0x275C9C;  // the game's empty string

struct Text {
    uint32_t id;
    const char16_t* text;
};
constexpr Text kTexts[] = {
    {965, u"PC Settings"},                                 // IDS_OPTIONS_SCREEN_CENTRE, the item
    {1406, u"Picture settings for this PC, and more."},    // IDS_ON_FOCUS_HELP_OPTIONS_SCREEN
    {1044, u"PC SETTINGS"},                                // IDS_SCREEN_CENTRE, the page title
};
uint32_t g_guest[3];  // our copies in guest memory

// a UTF-16 copy in guest memory, made once per text
uint32_t Guest(const char16_t* text) {
    static std::vector<std::pair<const char16_t*, uint32_t>> made;
    for (auto& [t, va] : made)
        if (t == text) return va;
    size_t n = std::char_traits<char16_t>::length(text) + 1;
    uint32_t va = mem::AllocSystem(uint32_t(n * 2));
    std::memcpy(kernel::HostPtr(va), text, n * 2);
    made.push_back({text, va});
    return va;
}

uint32_t Nearest(const std::vector<uint32_t>& values, uint32_t v) {
    size_t best = 0;
    for (size_t i = 1; i < values.size(); i++)
        if (std::abs(int(values[i]) - int(v)) < std::abs(int(values[best]) - int(v))) best = i;
    return uint32_t(best);
}

// a row: its texts, and the setting it shows as a choice index
struct Row {
    const char16_t* label;
    const char16_t* help;
    std::vector<const char16_t*> choices;  // at most 8
    uint32_t (*get)();
    void (*set)(uint32_t);
};

const std::vector<uint32_t> kRender = {50, 75, 100, 125, 150, 200}, kAniso = {0, 2, 4, 8, 16},
                            kDraw = {50, 75, 100, 150, 200, 300, 400}, kFov = {75, 90, 100, 110, 125, 150}, kSharpen = {0, 25, 50, 75, 100};

const Row kRows[] = {
    {u"Render Scale", u"The 3D picture's size, percent of the screen. More is sharper and slower.",
     {u"50%", u"75%", u"100%", u"125%", u"150%", u"200%"},
     [] { return Nearest(kRender, app::Get().render_percent); },
     [](uint32_t i) { app::Get().render_percent = kRender[i]; }},
    {u"MSAA", u"Smooths the edges of the 3D scene. More is slower.",
     {u"Off", u"2x", u"4x", u"8x"},
     [] { return std::min<uint32_t>(app::Get().msaa, 3); },
     [](uint32_t i) { app::Get().msaa = i; }},
    {u"FXAA", u"Smooths the finished frame, HUD included. A little softer.",
     {u"Off", u"On"},
     [] { return uint32_t(app::Get().fxaa); },
     [](uint32_t i) { app::Get().fxaa = i != 0; }},
    {u"Sharpening", u"Crisper picture. Too much shows as grain.",
     {u"Off", u"25%", u"50%", u"75%", u"100%"},
     [] { return Nearest(kSharpen, app::Get().sharpen); },
     [](uint32_t i) { app::Get().sharpen = kSharpen[i]; }},
    {u"Texture Filtering", u"Sharper road and walls at a distance.",
     {u"Game", u"2x", u"4x", u"8x", u"16x"},
     [] { return Nearest(kAniso, app::Get().aniso); },
     [](uint32_t i) { app::Get().aniso = kAniso[i]; }},
    {u"Sharp Text and HUD", u"Menus, text and the HUD drawn crisp at high resolutions.",
     {u"Off", u"On"},
     [] { return uint32_t(app::Get().sharp_2d); },
     [](uint32_t i) { app::Get().sharp_2d = i != 0; }},
    {u"Draw Distance", u"How far the race cameras see, percent of the console's. Next race on.",
     {u"50%", u"75%", u"100%", u"150%", u"200%", u"300%", u"400%"},
     [] { return Nearest(kDraw, draw_distance_percent); },
     [](uint32_t i) { draw_distance_percent = kDraw[i]; }},
    {u"Field of View", u"The race cameras' view, percent of the console's.",
     {u"75%", u"90%", u"100%", u"110%", u"125%", u"150%"},
     [] { return Nearest(kFov, fov_percent); },
     [](uint32_t i) { fov_percent = kFov[i]; }},
    // the console's numbering from 1 (GameInfo::languages); a change makes the game fill its cache again at the next start
    {u"Language", u"At the next start, which copies the game's files again.",
     {u"English", u"Japanese", u"Deutsch", u"Fran\u00E7ais", u"Espa\u00F1ol", u"Italiano", u"Korean", u"Chinese"},
     [] { return std::clamp<uint32_t>(app::Get().language, 1, 8) - 1; },
     [](uint32_t i) { app::Get().language = i + 1; }},
};
constexpr int kSettingRows = int(std::size(kRows));
constexpr int kRows_ = kSettingRows + 1;  // and More Settings

uint32_t g_vars = 0;            // the rows' ints in guest memory
uint32_t g_shown[kSettingRows];  // what each held when last looked at
bool g_on_page = false;          // our page is the one shown (General Options' builder clears it)

uint32_t Text16(uint32_t id) {
    uint32_t p = kernel::R32(kStrings + id * 4);
    return p ? p : kEmpty;
}

uint32_t FloatBits(float f) {
    uint32_t u;
    std::memcpy(&u, &f, 4);
    return u;
}

void Build(X86Context& ctx, bool more) {
    using kernel::CallGuest;
    if (!g_vars) g_vars = mem::AllocSystem(4 * kSettingRows);
    for (int i = 0; i < kSettingRows; i++) {
        g_shown[i] = kRows[i].get();
        kernel::W32(g_vars + i * 4, g_shown[i]);
    }
    static uint32_t open_var = 0;
    if (!open_var) open_var = mem::AllocSystem(4);
    kernel::W32(open_var, 0);
    // the frame, as the game's option pages set it
    uint32_t frame = kernel::R32(kFrame);
    kernel::W32(frame + 0x34, 1044);
    uint32_t one = kernel::R32(0x275B98);
    kernel::W32(frame + 0x38, one);
    kernel::W32(frame + 0x3C, one);
    kernel::W32(frame + 0x40, one);
    kernel::G<uint8_t>(frame + 0x45) = 1;
    ctx.ecx = frame;
    CallGuest(ctx, kFrameMode, {0xC});
    if (kernel::G<uint8_t>(0x49E110)) {
        kernel::W32(frame + 0x3C, 0);
        kernel::W32(frame + 0x38, 0);
    }
    ctx.ecx = kernel::R32(kFrame);
    CallGuest(ctx, kFrameMode, {0xB});
    // the rows
    uint32_t rows = mem::AllocSystem(4 * kRows_);  // the page manager keeps the rows, not this list
    for (int i = 0; i < kRows_; i++) {
        bool setting = i < kSettingRows;
        const char16_t* label = setting ? kRows[i].label : u"More Settings";
        const char16_t* help = setting ? kRows[i].help : u"Display, controls, wheel and the rest: the settings window (F1).";
        uint32_t args[17] = {0, FloatBits(144.0f + 22.0f * float(i)), 1, Guest(label), Guest(help),
                             setting ? g_vars + i * 4 : open_var, 0, 2, 0};
        std::vector<const char16_t*> choices = setting ? kRows[i].choices : std::vector<const char16_t*>{u"Open"};
        args[6] = uint32_t(choices.size());
        for (int c = 0; c < 8; c++) args[9 + c] = c < int(choices.size()) ? Guest(choices[c]) : kEmpty;
        kernel::W32(rows + i * 4, CallGuest(ctx, kRow, args, 17));
    }
    // the button legend
    kernel::W32(0x28A720, 1);
    kernel::W32(kLegend, 0);
    kernel::W32(kLegend + 0x28C, 0);
    ctx.ecx = kLegend;
    CallGuest(ctx, kLegendText, {Text16(21), 0});
    ctx.ecx = kLegend;
    CallGuest(ctx, kLegendHelp, {Text16(1393)});
    // up and down, A and B back to General Options, A on More Settings to this page again with the window
    ctx.ecx = kernel::R32(kManager);
    CallGuest(ctx, kChain, {rows, uint32_t(kRows_), 1, 1});
    ctx.ecx = kernel::R32(kManager);
    CallGuest(ctx, kAction, {rows, uint32_t(kSettingRows), 4, kGeneralOptions, 2});
    ctx.ecx = kernel::R32(kManager);
    CallGuest(ctx, kAction, {rows + kSettingRows * 4, 1, 4, kPage, 2});
    ctx.ecx = kernel::R32(kManager);
    CallGuest(ctx, kAction, {rows, uint32_t(kRows_), 5, kGeneralOptions, 2});
    ctx.ecx = kernel::R32(kManager);
    CallGuest(ctx, kFocus, {kernel::R32(rows + (more ? kSettingRows : 0) * 4), uint32_t(-1)});
    mem::FreeSystem(rows);
}

}  // namespace

// each frame (src/game/hooks.cpp, before the frame step): the table entries point at our texts; the table is filled
// when the game loads its language's strings, so they are put back whenever the game wrote its own. Then the page's
// rows and the settings meet: a row changed in the menu sets its setting, a setting changed in the window its row
void ApplySettingsEntry() {
    if (!kernel::R32(kStrings + 965 * 4)) return;  // strings not loaded yet
    for (int i = 0; i < 3; i++) {
        const Text& t = kTexts[i];
        if (!g_guest[i]) g_guest[i] = Guest(t.text);
        uint32_t entry = kStrings + t.id * 4;
        if (kernel::R32(entry) != g_guest[i]) {
            static int logged = 0;
            if (logged++ < 3) LOGR("pgr2: string %u becomes %s", t.id, i == 0 ? "PC Settings" : i == 1 ? "its help" : "its title");
            kernel::W32(entry, g_guest[i]);
        }
    }
    if (!g_vars) return;
    bool changed = false;
    for (int i = 0; i < kSettingRows; i++) {
        uint32_t var = kernel::R32(g_vars + i * 4);
        if (var != g_shown[i] && var < kRows[i].choices.size()) {
            std::lock_guard lock(app::UiLock());
            kRows[i].set(var);
            g_shown[i] = var;
            changed = true;
            LOGR("pgr2: PC Settings row %d to choice %u", i, var);
        } else if (uint32_t want = kRows[i].get(); want != g_shown[i]) {
            kernel::W32(g_vars + i * 4, want);
            g_shown[i] = want;
        }
    }
    if (changed) {
        std::lock_guard lock(app::UiLock());
        app::Save();
    }
}

}  // namespace pgr2

// General Options' page builder: the page after it is a fresh visit
X86_EXTERN_FUNC(__imp__sub_000A40A0);
X86_FUNC(sub_000A40A0) {
    pgr2::g_on_page = false;
    __imp__sub_000A40A0(ctx, base);
}

// Screen Center's page builder (0xA2E60, no arguments, returns 1): our page instead. Built again from its own More
// Settings row, it also opens the settings window
X86_FUNC(sub_000A2E60) {
    bool more = pgr2::g_on_page;
    pgr2::Build(ctx, more);
    pgr2::g_on_page = true;
    LOGR("pgr2: PC Settings page%s", more ? ", the settings window opened" : "");
    if (more) {
        std::lock_guard lock(app::UiLock());
        if (!app::OverlayOpen()) app::ToggleOverlay();
    }
    ctx.eax = 1;
    ctx.esp += 4;
}
