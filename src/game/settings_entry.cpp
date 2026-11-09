// RECOMP in PGR2's own menu: Options > General Options > Screen Center (which a PC monitor has no use for) becomes a
// page of sections, each a page of the game's own option rows for the PC's settings. What rows cannot do (binding by
// moving a control, remapping, live readouts, achievements) stays in the settings window (F1), which a row opens.
//
// General Options is built by 0xA40A0: each item a button with a page builder (0x58970), a label (0x89200) and a help
// line (0x891E0), both pointers from the UTF-16 string table at 0x28A870 by StringList.ini id. Its first page
// (0xA1710) is a list of option rows, which this page copies:
//   the frame 0x2FEE08: +0x34 the title's string id, 1.0 at +0x38..+0x40, the byte +0x45 1, then thiscall 0x7A110
//     with 0xC and 0xB (the second after zeroing +0x38 and +0x3C when the byte 0x49E110 is set)
//   a row 0x58EE0 (cdecl, 17 arguments): x, y (floats, 0 and 144 + 24 a row, here 20 to fit eleven), 1, label, help line, the int it
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
#include <deque>
#include <mutex>
#include <string>
#include <vector>

#include "app/app.h"
#include "core/input.h"
#include "core/log.h"
#include "kernel/kernel.h"
#include "kernel/memory.h"
#include "xdk/hle.h"

namespace pgr2 {

extern bool skip_intro, dlc, hide_hud;
extern uint32_t race_fps;
void RaiseFrameCap();
extern uint32_t fov_percent, draw_distance_percent;

namespace {

constexpr uint32_t kStrings = 0x28A870;  // string table: a UTF-16 pointer per id
constexpr uint32_t kGeneralOptions = 0xA40A0, kPage = 0xA2E60, kPause = 0x8F820;  // page builders
constexpr uint32_t kResume = 0x8CE10, kButton = 0x58970;
constexpr uint32_t kFrame = 0x2FEE08, kManager = 0x2F4B78, kLegend = 0x28A488;
constexpr uint32_t kRow = 0x58EE0, kFrameMode = 0x7A110, kLegendText = 0x89200, kLegendHelp = 0x891E0;
constexpr uint32_t kChain = 0x4D3A0, kAction = 0x4D4C0, kFocus = 0x4D270;
constexpr uint32_t kEmpty = 0x275C9C;  // the game's empty string

struct Text {
    uint32_t id;
    const char16_t* text;
};
constexpr Text kTexts[] = {
    {965, u"Recomp"},                                      // IDS_OPTIONS_SCREEN_CENTRE, the item
    {1406, u"Every setting of this PC version."},          // IDS_ON_FOCUS_HELP_OPTIONS_SCREEN
    {1044, u"RECOMP"},                                     // IDS_SCREEN_CENTRE, the page title
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
    std::vector<const char16_t*> (*choices)();  // at most 8
    uint32_t (*get)();
    void (*set)(uint32_t);
};
using Choices = std::vector<const char16_t*>;

Choices OnOff() { return {u"Off", u"On"}; }

const std::vector<uint32_t> kRender = {25, 50, 75, 100, 125, 150, 175, 200}, kAniso = {0, 2, 4, 8, 16},
                            kDraw = {50, 75, 100, 150, 200, 300, 400}, kFov = {75, 90, 100, 110, 125, 150},
                            kSharpen = {0, 25, 50, 75, 100}, kRaceFps = {30, 60, 90, 120, 144},
                            kCaps = {30, 60, 120, 144, 240, 0}, kSens = {25, 50, 75, 100, 150, 200, 400, 800},
                            kWheelRange = {180, 270, 360, 540, 720, 900, 1080, 2880},
                            kWheelLock = {180, 270, 360, 450, 540, 720, 900, 1080},
                            kDead = {0, 1, 2, 3, 5, 10, 20}, kPercent = {0, 25, 50, 70, 85, 100};

// a UTF-16 copy of a name made at run time, kept for Guest
const char16_t* Wide(const std::string& text) {
    static std::deque<std::u16string> made;
    made.emplace_back(text.begin(), text.end());
    return made.back().c_str();
}

// the resolutions offered: the desktop's, then the widest the rows have room for
std::vector<app::Mode> ResolutionList() {
    std::vector<app::Mode> list = app::ListModes();
    if (list.size() > 7) list.resize(7);
    return list;
}

uint32_t NearestCap(uint32_t cap) {
    for (size_t i = 0; i < kCaps.size(); i++)
        if (kCaps[i] == cap) return uint32_t(i);
    return 1;
}

void WheelChanged() { input::WheelSetup(); }

// Quit Game: a second press to the right leaves
uint32_t g_quit = 0;

const std::vector<Row> kPicture = {
    {u"MSAA", u"Smooths the edges of the 3D scene. More is slower.",
     [] { return Choices{u"Off", u"2x", u"4x", u"8x"}; },
     [] { return std::min<uint32_t>(app::Get().msaa, 3); },
     [](uint32_t i) { app::Get().msaa = i; }},
    {u"Smooth Alpha Edges", u"With MSAA: softens fences and leaves too.", OnOff,
     [] { return uint32_t(app::Get().alpha_coverage); },
     [](uint32_t i) { app::Get().alpha_coverage = i != 0; }},
    {u"Edge Smoothing", u"The finished frame, HUD included. SMAA keeps more detail.",
     [] { return Choices{u"Off", u"FXAA", u"SMAA"}; },
     [] {
         const app::Settings& s = app::Get();
         return s.smaa ? uint32_t(2) : uint32_t(s.fxaa);
     },
     [](uint32_t i) {
         app::Get().fxaa = i == 1;
         app::Get().smaa = i == 2;
     }},
    {u"Sharpening", u"Crisper picture. Too much shows as grain.",
     [] { return Choices{u"Off", u"25%", u"50%", u"75%", u"100%"}; },
     [] { return Nearest(kSharpen, app::Get().sharpen); },
     [](uint32_t i) { app::Get().sharpen = kSharpen[i]; }},
    {u"Texture Filtering", u"Sharper road and walls at a distance.",
     [] { return Choices{u"Game", u"2x", u"4x", u"8x", u"16x"}; },
     [] { return Nearest(kAniso, app::Get().aniso); },
     [](uint32_t i) { app::Get().aniso = kAniso[i]; }},
    {u"World Textures", u"Doubled in the background. Can cause big hitches.", OnOff,
     [] { return uint32_t(app::Get().world_textures); },
     [](uint32_t i) { app::Get().world_textures = i != 0; }},
    {u"Sharp Text and HUD", u"Menus, text and the HUD drawn crisp at high resolutions.", OnOff,
     [] { return uint32_t(app::Get().sharp_2d); },
     [](uint32_t i) { app::Get().sharp_2d = i != 0; }},
};

const std::vector<Row> kPerformance = {
    {u"Render Scale", u"The 3D picture's size, percent of the screen. More is slower.",
     [] { return Choices{u"25%", u"50%", u"75%", u"100%", u"125%", u"150%", u"175%", u"200%"}; },
     [] { return Nearest(kRender, app::Get().render_percent); },
     [](uint32_t i) { app::Get().render_percent = kRender[i]; }},
    {u"Scaling Filter", u"How the frame reaches the screen's size.",
     [] { return Choices{u"Bilinear", u"Sharp"}; },
     [] { return app::Get().scale_filter % 2; },
     [](uint32_t i) { app::Get().scale_filter = i; }},
    {u"Frame Rate Cap", u"Limits the frame rate further than the game does.",
     [] { return Choices{u"30", u"60", u"120", u"144", u"240", u"None"}; },
     [] { return NearestCap(app::Get().fps_cap); },
     [](uint32_t i) { app::Get().fps_cap = kCaps[i]; }},
    {u"Low Latency", u"Next start. Less input lag; a slow frame shows more.", OnOff,
     [] { return uint32_t(app::Get().low_latency); },
     [](uint32_t i) { app::Get().low_latency = i != 0; }},
    {u"Frame Counter", u"Frames a second in a corner of the screen.", OnOff,
     [] { return uint32_t(app::Get().show_fps); },
     [](uint32_t i) { app::Get().show_fps = i != 0; }},
    {u"Renderer", u"At the next start. Direct3D 11 if 12 misbehaves.",
     [] { return Choices{u"Direct3D 12", u"Direct3D 11"}; },
     [] { return app::Get().renderer % 2; },
     [](uint32_t i) { app::Get().renderer = i; }},
};

const std::vector<Row> kDisplay = {
    {u"Monitor", u"The display the game runs on.",
     [] {
         Choices c{u"Primary"};
         std::vector<std::string> names = app::Monitors();
         for (size_t i = 0; i < names.size() && c.size() < 8; i++) c.push_back(Wide("Display " + std::to_string(i + 1)));
         return c;
     },
     [] { return std::min<uint32_t>(app::Get().monitor, 7); },
     [](uint32_t i) {
         app::Get().monitor = i;
         app::ApplyDisplay();
     }},
    {u"Display Mode", u"Applies at once.",
     [] { return Choices{u"Fullscreen", u"Borderless", u"Window"}; },
     [] { return uint32_t(app::Get().display_mode) % 3; },
     [](uint32_t i) {
         app::Get().display_mode = app::DisplayMode(i);
         app::ApplyDisplay();
     }},
    {u"Resolution", u"Applies at once. The settings window lists them all.",
     [] {
         Choices c{u"Desktop"};
         for (const app::Mode& m : ResolutionList()) c.push_back(Wide(std::to_string(m.width) + "x" + std::to_string(m.height)));
         return c;
     },
     [] {
         const app::Settings& s = app::Get();
         std::vector<app::Mode> list = ResolutionList();
         for (size_t i = 0; i < list.size(); i++)
             if (list[i].width == s.width && list[i].height == s.height) return uint32_t(i + 1);
         return uint32_t(0);
     },
     [](uint32_t i) {
         app::Settings& s = app::Get();
         std::vector<app::Mode> list = ResolutionList();
         s.width = i && i <= list.size() ? list[i - 1].width : 0;
         s.height = i && i <= list.size() ? list[i - 1].height : 0;
         app::ApplyDisplay();
     }},
    {u"Vertical Sync", u"Off can tear, and lowers delay.", OnOff,
     [] { return uint32_t(app::Get().vsync); },
     [](uint32_t i) { app::Get().vsync = i != 0; }},
    {u"Aspect Ratio", u"16:9 and Fill at once, 4:3 at the next start. From Screen fills ultrawide.",
     [] { return Choices{u"From Screen", u"4:3", u"16:9", u"Fill Screen"}; },
     [] { return app::Get().aspect % 4; },
     [](uint32_t i) { app::Get().aspect = i; }},
};

const std::vector<Row> kGame = {
    {u"Race Frame Rate", u"Next start. Same physics. Above 60, busy streets can dip.",
     [] { return Choices{u"30", u"60", u"90", u"120", u"144"}; },
     [] { return Nearest(kRaceFps, race_fps); },
     [](uint32_t i) {
         race_fps = kRaceFps[i];
         RaiseFrameCap();
     }},
    {u"Draw Distance", u"How far the race cameras see. Next race on.",
     [] { return Choices{u"50%", u"75%", u"100%", u"150%", u"200%", u"300%", u"400%"}; },
     [] { return Nearest(kDraw, draw_distance_percent); },
     [](uint32_t i) { draw_distance_percent = kDraw[i]; }},
    {u"Field of View", u"The race cameras' view, percent of the console's.",
     [] { return Choices{u"75%", u"90%", u"100%", u"110%", u"125%", u"150%"}; },
     [] { return Nearest(kFov, fov_percent); },
     [](uint32_t i) { fov_percent = kFov[i]; }},
    {u"Race HUD", u"Hide the race HUD for screenshots and replays.",
     [] { return Choices{u"Shown", u"Hidden"}; },
     [] { return uint32_t(hide_hud); },
     [](uint32_t i) { hide_hud = i != 0; }},
    {u"Skip Intro", u"No intro movies. At the next start.", OnOff,
     [] { return uint32_t(skip_intro); },
     [](uint32_t i) { skip_intro = i != 0; }},
    {u"Xbox Live DLC", u"Cities and cars from your packs. At the next start.", OnOff,
     [] { return uint32_t(dlc); },
     [](uint32_t i) { dlc = i != 0; }},
    {u"Language", u"At the next start, which copies the game's files again.",
     [] { return Choices{u"English", u"Japanese", u"Deutsch", u"Français", u"Español", u"Italiano", u"Korean", u"Chinese"}; },
     [] { return std::clamp<uint32_t>(app::Get().language, 1, 8) - 1; },
     [](uint32_t i) { app::Get().language = i + 1; }},
    {u"Pause in Background", u"The game stops while another window is in front.", OnOff,
     [] { return uint32_t(app::Get().pause_unfocused); },
     [](uint32_t i) { app::Get().pause_unfocused = i != 0; }},
};

const std::vector<Row> kControls = {
    {u"Keyboard", u"The keyboard plays as controller 1.", OnOff,
     [] { return uint32_t(app::Get().keyboard); },
     [](uint32_t i) { app::Get().keyboard = i != 0; }},
    {u"Mouse Look", u"The mouse moves the right stick.", OnOff,
     [] { return uint32_t(app::Get().mouse_look); },
     [](uint32_t i) { app::Get().mouse_look = i != 0; }},
    {u"Invert Mouse", u"Up and down swapped.", OnOff,
     [] { return uint32_t(app::Get().mouse_invert_y); },
     [](uint32_t i) { app::Get().mouse_invert_y = i != 0; }},
    {u"Mouse Sensitivity", u"The settings window goes up to 1000%.",
     [] { return Choices{u"25%", u"50%", u"75%", u"100%", u"150%", u"200%", u"400%", u"800%"}; },
     [] { return Nearest(kSens, app::Get().mouse_sensitivity); },
     [](uint32_t i) { app::Get().mouse_sensitivity = kSens[i]; }},
    {u"Default Keys", u"Right sets every key back to its default.",
     [] { return Choices{u"Keep", u"Reset"}; },
     [] { return uint32_t(0); },
     [](uint32_t i) {
         if (!i) return;
         app::Settings& s = app::Get();
         for (int c = 0; c < input::kControls && c < int(s.keys.size()); c++) s.keys[c] = app::DefaultKeys(c);
     }},
    {u"Default Controller", u"Right sets every controller input back.",
     [] { return Choices{u"Keep", u"Reset"}; },
     [] { return uint32_t(0); },
     [](uint32_t i) {
         if (!i) return;
         app::Settings& s = app::Get();
         for (int c = 0; c < input::kControls && c < int(s.pad_map.size()); c++) s.pad_map[c] = uint8_t(c);
     }},
};

const std::vector<Row> kWheel = {
    {u"Invert Steering", u"For a wheel that reads backwards.", OnOff,
     [] { return uint32_t(app::Get().wheel_steer.invert); },
     [](uint32_t i) {
         app::Get().wheel_steer.invert = i != 0;
         WheelChanged();
     }},
    {u"Wheel Rotation", u"As set in the wheel's own software.",
     [] { return Choices{u"180", u"270", u"360", u"540", u"720", u"900", u"1080", u"2880"}; },
     [] { return Nearest(kWheelRange, app::Get().wheel_range); },
     [](uint32_t i) {
         app::Get().wheel_range = kWheelRange[i];
         WheelChanged();
     }},
    {u"Steering Lock", u"Degrees for full lock. Smaller turns sooner.",
     [] { return Choices{u"180", u"270", u"360", u"450", u"540", u"720", u"900", u"1080"}; },
     [] { return Nearest(kWheelLock, app::Get().wheel_lock); },
     [](uint32_t i) {
         app::Get().wheel_lock = kWheelLock[i];
         WheelChanged();
     }},
    {u"Dead Zone", u"Steering around the centre that reads as straight.",
     [] { return Choices{u"0%", u"1%", u"2%", u"3%", u"5%", u"10%", u"20%"}; },
     [] { return Nearest(kDead, app::Get().wheel_deadzone); },
     [](uint32_t i) {
         app::Get().wheel_deadzone = kDead[i];
         WheelChanged();
     }},
    {u"Force Feedback", u"Overall strength. Off turns the forces off.",
     [] { return Choices{u"Off", u"25%", u"50%", u"70%", u"85%", u"100%"}; },
     [] { return Nearest(kPercent, app::Get().ffb_strength); },
     [](uint32_t i) {
         app::Get().ffb_strength = kPercent[i];
         WheelChanged();
     }},
    {u"Road and Impacts", u"Kerbs, gravel and impacts felt in the wheel.",
     [] { return Choices{u"Off", u"25%", u"50%", u"70%", u"85%", u"100%"}; },
     [] { return Nearest(kPercent, app::Get().ffb_road); },
     [](uint32_t i) {
         app::Get().ffb_road = kPercent[i];
         WheelChanged();
     }},
};

// the landing page's rows; a section's are its settings
struct Section {
    const char16_t* label;
    const char16_t* help;
    const std::vector<Row>* rows;
};
const Section kSections[] = {
    {u"Picture", u"Anti-aliasing, sharpening and textures.", &kPicture},
    {u"Performance", u"Render scale, frame cap, latency and renderer.", &kPerformance},
    {u"Display", u"Monitor, display mode, resolution and aspect ratio.", &kDisplay},
    {u"Game", u"Frame rate, draw distance, language and more.", &kGame},
    {u"Controls", u"Keyboard and mouse.", &kControls},
    {u"Wheel", u"Rotation, lock, dead zone and force feedback.", &kWheel},
};
constexpr int kSectionCount = int(std::size(kSections));
constexpr int kBindingsRow = kSectionCount, kQuitRow = kSectionCount + 1, kLandingRows = kSectionCount + 2;
constexpr int kMaxRows = 10;

uint32_t g_vars = 0;               // the rows' ints in guest memory
uint32_t g_shown[kMaxRows];        // what each held when last looked at
const std::vector<Row>* g_page = nullptr;  // the section shown, null on the landing page
bool g_on_page = false;            // one of ours is the page shown (General Options' builder clears it)
int g_section = -1;                // the section shown, -1 the landing page
uint32_t g_row_va[kMaxRows];       // the page's rows as the game made them, to tell which A was pressed on
int g_row_count = 0;
bool g_from_pause = false;         // reached from the pause menu, not General Options
bool g_in_pause_build = false;     // 0x8F820 is building its row
uint32_t g_pause_last = 0;         // the last button it chained

uint32_t Text16(uint32_t id) {
    uint32_t p = kernel::R32(kStrings + id * 4);
    return p ? p : kEmpty;
}

uint32_t FloatBits(float f) {
    uint32_t u;
    std::memcpy(&u, &f, 4);
    return u;
}

// the pause menu: a Recomp button after its last, in its row. Its buttons run one after another along action 3
void AddToPause(X86Context& ctx) {
    using kernel::CallGuest;
    if (!g_pause_last) return;
    uint32_t button = CallGuest(ctx, kButton, {0x42DC0000, 0, 3, kEmpty, kPage, 2, 4, 2, 0, 0, 0});
    ctx.ecx = kLegend;
    CallGuest(ctx, kLegendText, {Text16(965), button});
    ctx.ecx = kLegend;
    CallGuest(ctx, kLegendHelp, {Text16(1406)});
    uint32_t list = mem::AllocSystem(8);
    kernel::W32(list, g_pause_last);
    kernel::W32(list + 4, button);
    ctx.ecx = kernel::R32(kManager);
    CallGuest(ctx, kChain, {list, 2, 3, 0});
    ctx.ecx = kernel::R32(kManager);
    CallGuest(ctx, kAction, {list + 4, 1, 5, kResume, 2});
    mem::FreeSystem(list);
    LOGR("pgr2: Recomp added to the pause menu");
}

// the row the page manager has in focus, of those last built; -1 none. In the pause menu's mode (0x4C8C0 with 3, the
// manager's +0x28) each controller has its own focus, from +0x34; a page builder gets the controller as its second argument
int FocusedRow(uint32_t port) {
    uint32_t manager = kernel::R32(kManager);
    uint32_t focus = kernel::R32(manager + 0x28) == 3 ? kernel::R32(manager + 0x34 + (port & 3) * 4) : kernel::R32(manager + 0x2C);
    for (int i = 0; i < g_row_count; i++)
        if (g_row_va[i] == focus) return i;
    return -1;
}

void Build(X86Context& ctx, int focus, uint32_t port) {
    using kernel::CallGuest;
    if (!g_vars) g_vars = mem::AllocSystem(4 * kMaxRows);
    g_page = g_section >= 0 ? kSections[g_section].rows : nullptr;
    int count = g_page ? int(g_page->size()) : kLandingRows;
    for (int i = 0; i < count; i++) {
        g_shown[i] = g_page ? (*g_page)[size_t(i)].get() : 0;
        kernel::W32(g_vars + i * 4, g_shown[i]);
    }
    g_quit = 0;
    if (g_from_pause) {
        // as the pause menu's own Settings page (0x8ED10) starts
        kernel::G<uint8_t>(0x367524) = 0;
        CallGuest(ctx, 0x141560, {1});
    } else {
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
    }
    // the rows
    uint32_t rows = mem::AllocSystem(4 * kMaxRows);  // the page manager keeps the rows, not this list
    for (int i = 0; i < count; i++) {
        const char16_t* label;
        const char16_t* help;
        std::vector<const char16_t*> choices;
        if (g_page) {
            const Row& r = (*g_page)[size_t(i)];
            label = r.label, help = r.help, choices = r.choices();
        } else if (i < kSectionCount) {
            label = kSections[i].label, help = kSections[i].help, choices = {u"Open"};
        } else if (i == kBindingsRow) {
            label = u"Bindings and More", help = u"Wheel and key bindings, achievements: the settings window (F1).", choices = {u"Open"};
        } else {
            label = u"Quit Game", help = u"Right twice leaves. Progress since the last save is lost.", choices = {u"No", u"Sure?", u"Quit"};
        }
        uint32_t args[17] = {0, FloatBits(144.0f + 24.0f * float(i)), 1, Guest(label), Guest(help), g_vars + i * 4,
                             uint32_t(choices.size()), 2, 0};
        for (int c = 0; c < 8; c++) args[9 + c] = c < int(choices.size()) ? Guest(choices[size_t(c)]) : kEmpty;
        g_row_va[i] = CallGuest(ctx, kRow, args, 17);
        kernel::W32(rows + i * 4, g_row_va[i]);
    }
    g_row_count = count;
    // from the pause menu: the object each of its pages makes with a row (0x8F820, 0x8ED10, the Sound page 0x8DF70:
    // vtable 0x27953C from the page objects' allocator 0x4E5F0 on [0x2F4CC4], the row at +0x30). Without it the pause
    // frame went with the pause menu: the race behind showed undimmed, the title and its lines gone (issue 32)
    if (g_from_pause && count) {
        ctx.ecx = kernel::R32(0x2F4CC4);
        uint32_t obj = CallGuest(ctx, 0x4E5F0, {});
        if (obj) {
            kernel::G<uint8_t>(obj + 4) = 0;
            kernel::W32(obj + 0x28, 0);
            kernel::W32(obj + 0x2C, 0);
            kernel::W32(obj, 0x27953C);
            kernel::W32(obj + 0x30, g_row_va[count - 1]);
        }
    }
    // the button legend
    kernel::W32(0x28A720, 1);
    kernel::W32(kLegend, 0);
    kernel::W32(kLegend + 0x28C, 0);
    ctx.ecx = kLegend;
    CallGuest(ctx, kLegendText, {Text16(21), 0});
    ctx.ecx = kLegend;
    CallGuest(ctx, kLegendHelp, {Text16(1393)});
    // up and down; A to this builder (which looks at the row in focus); B back a page
    ctx.ecx = kernel::R32(kManager);
    CallGuest(ctx, kChain, {rows, uint32_t(count), 1, 1});
    ctx.ecx = kernel::R32(kManager);
    CallGuest(ctx, kAction, {rows, uint32_t(count), 4, kPage, 2});
    ctx.ecx = kernel::R32(kManager);
    CallGuest(ctx, kAction, {rows, uint32_t(count), 5, g_page ? kPage : g_from_pause ? kPause : kGeneralOptions, 2});
    ctx.ecx = kernel::R32(kManager);
    CallGuest(ctx, kFocus, {kernel::R32(rows + uint32_t(std::clamp(focus, 0, count - 1)) * 4), port});
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
    if (!g_vars || !g_on_page) return;
    bool changed = false;
    if (!g_page) {  // the landing page: Quit Game's row steps No, Sure?, Quit
        uint32_t var = kernel::R32(g_vars + kQuitRow * 4);
        if (var == 2 && g_quit == 1) {
            LOGR("pgr2: Quit Game chosen in the Recomp page");
            app::Quit();
        } else if (var != g_quit) {
            if (var == 2) var = 0, kernel::W32(g_vars + kQuitRow * 4, 0);  // from No the other way round: not a second press
            g_quit = var;
        }
        return;
    }
    for (size_t i = 0; i < g_page->size(); i++) {
        const Row& r = (*g_page)[i];
        uint32_t var = kernel::R32(g_vars + uint32_t(i) * 4);
        if (var != g_shown[i] && var < r.choices().size()) {
            std::lock_guard lock(app::UiLock());
            r.set(var);
            g_shown[i] = var;
            changed = true;
            LOGR("pgr2: Recomp section %d row %d to choice %u", g_section, int(i), var);
        } else if (uint32_t want = r.get(); want != g_shown[i]) {
            kernel::W32(g_vars + uint32_t(i) * 4, want);
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
    pgr2::g_section = -1;
    pgr2::g_from_pause = false;
    __imp__sub_000A40A0(ctx, base);
}

// the chaining of a page's buttons (thiscall: list, count, action, wrap): the pause menu's last one
X86_EXTERN_FUNC(__imp__sub_0004D3A0);
X86_FUNC(sub_0004D3A0) {
    if (pgr2::g_in_pause_build && kernel::R32(ctx.esp + 12) == 3) {
        uint32_t list = kernel::R32(ctx.esp + 4), count = kernel::R32(ctx.esp + 8);
        pgr2::g_pause_last = count ? kernel::R32(list + (count - 1) * 4) : 0;
    }
    __imp__sub_0004D3A0(ctx, base);
}

// the pause menu's builder: Continue, Retry, Quit, Settings, and Recomp. Not online (0x3B1EC3, whose menu has its own
// handlers) and not in a replay (a menu of its own, built from here, which does not chain along action 3 last)
X86_EXTERN_FUNC(__imp__sub_0008F820);
X86_FUNC(sub_0008F820) {
    pgr2::g_on_page = false;
    pgr2::g_section = -1;
    pgr2::g_from_pause = true;
    pgr2::g_in_pause_build = true;
    pgr2::g_pause_last = 0;
    int32_t replay_mode = int32_t(kernel::R32(0x383190));
    bool replay = replay_mode != -1 && (kernel::G<uint8_t>(0x34C2B0 + uint32_t(replay_mode) * 0x4C) & 4);
    __imp__sub_0008F820(ctx, base);
    pgr2::g_in_pause_build = false;
    if (!replay && !kernel::G<uint8_t>(0x3B1EC3)) {
        uint32_t eax = ctx.eax;
        pgr2::AddToPause(ctx);
        ctx.eax = eax;
    }
}

// Screen Center's page builder (0xA2E60, no arguments, returns 1): ours instead. From General Options the landing page;
// from the landing page A on a section opens it, on Bindings the settings window; from a section, back to the landing page
X86_FUNC(sub_000A2E60) {
    uint32_t port = pgr2::g_from_pause ? kernel::R32(ctx.esp + 8) : uint32_t(-1);
    int focus = 0;
    bool window = false;
    if (pgr2::g_on_page) {
        if (pgr2::g_section >= 0) {
            focus = pgr2::g_section;
            pgr2::g_section = -1;
        } else {
            int row = pgr2::FocusedRow(port);
            LOGR("pgr2: Recomp row %d chosen", row);
            if (row >= 0 && row < pgr2::kSectionCount) pgr2::g_section = row;
            else {
                focus = row < 0 ? 0 : row;
                window = row == pgr2::kBindingsRow;
            }
        }
    }
    pgr2::Build(ctx, focus, port);
    pgr2::g_on_page = true;
    if (window) {
        std::lock_guard lock(app::UiLock());
        if (!app::OverlayOpen()) app::ToggleOverlay();
    }
    ctx.eax = 1;
    ctx.esp += 4;
}
