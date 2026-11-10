// PGR2's achievements (ACHIEVEMENTS.md has the whole draft set and the memory map). Only those whose memory is mapped
// are here; conditions are in RetroAchievements' syntax so the set can go there as it is.
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <string>
#include <vector>

#include "core/achievements.h"

namespace pgr2 {

// Total Kudos: the eight dwords at 0x35D090 + 16 i (the object at 0x35BA78, summed by 0xA66C0 for the status
// screen); the ranks' thresholds are Frontend\Ranking.Ini's
#define PGR2_TOTAL_KUDOS "A:0xX35d090_A:0xX35d0a0_A:0xX35d0b0_A:0xX35d0c0_A:0xX35d0d0_A:0xX35d0e0_A:0xX35d0f0_0xX35d100"

namespace {

// Medals: each race's record is 0x1C bytes from 0x35DD88 (the object 0x35BA78 + 0x2310), its first dword the medal won,
// 1 Steel to 5 Platinum, 0 none (0xA66E0 counts them for the status screens). The races of the Kudos World Series
// classes and the Arcade types are consecutive (the level tables at 0x34A8C8 and 0x34C284, from Frontend\Levels.Ini).
struct Range {
    int first, last;
};
constexpr Range kClasses[14] = {{1, 7},     {8, 14},    {15, 22},   {23, 30},   {31, 40},   {41, 51},   {52, 62},
                                {63, 75},   {76, 89},   {90, 103},  {104, 119}, {120, 136}, {137, 153}, {154, 172}};
constexpr Range kSeries = {1, 172};
constexpr Range kArcade[3] = {{173, 192}, {193, 212}, {213, 232}};  // street racing, timed run, cone challenge
constexpr Range kArcadeAll = {173, 232};

// every race in the range with at least that medal; the strings stay for the program's life
const char* Medals(Range r, int medal) {
    static std::deque<std::string> kept;
    std::string c;
    for (int i = r.first; i <= r.last; i++) {
        char b[32];
        std::snprintf(b, sizeof b, "%s0xX%x>=%d", c.empty() ? "" : "_", 0x35DD88 + i * 0x1C, medal);
        c += b;
    }
    return kept.emplace_back(std::move(c)).c_str();
}

// Cars: car c's record is 0x1C bytes from 0x35BAA4 (the object 0x35BA78 + 0x2C), owned by player p when bit p of its
// first dword is set (0xA63C0; p is 0x366800, 0 for the one profile). Frontend\Cars.Ini's order: the 29 ShowroomFree
// cars are owned from the start, these 68 cost Kudos Tokens, the other 5 are the Kudos World Series rewards.
constexpr int kTokenCars[68] = {3,  4,  5,  6,  7,  8,  11, 12, 13, 14, 17, 18, 19, 20, 23, 24, 25, 26, 28, 29, 31, 32, 34,
                                35, 36, 38, 40, 42, 43, 44, 45, 46, 47, 50, 51, 52, 54, 55, 56, 58, 59, 60, 61, 63, 65, 66,
                                67, 68, 70, 72, 73, 74, 75, 76, 81, 82, 83, 84, 85, 87, 88, 89, 91, 92, 95, 96, 97, 98};

// how many of the cars are owned, at least n
const char* Owned(const int* cars, int count, int n) {
    static std::deque<std::string> kept;
    std::string c;
    for (int i = 0; i < count; i++) {
        char b[32];
        std::snprintf(b, sizeof b, "%s0xM%x", i + 1 < count ? "A:" : "", 0x35BAA4 + cars[i] * 0x1C);
        c += b;
        if (i + 1 < count) c += "_";
    }
    return kept.emplace_back(c + ">=" + std::to_string(n)).c_str();
}
const int* AllCars() {
    static int all[102];
    for (int i = 0; i < 102; i++) all[i] = i;
    return all;
}

// Player 1 racing (car 0; the attract demo drives it too, with 0x38384C set and race type 0x383814 0). Speed: the
// float at 0x298D50 + car x 0x6990 in m/s (the speedometer, 0xE5CF0, shows its magnitude); compared by its bits, which
// order as the numbers do for positive floats (backwards is negative, above 0x80000000)
#define PGR2_RACING "0xH38384c=0_0xX383814>0_"
#define PGR2_SPEED "0xX298d50<h80000000_0xX298d50>="

// Kudos earned in the race: what the race HUD shows, the banked Kudos int32 at 0x39D3DC + car x 0x2498 (+0x78 of the
// Kudos object 0x39D364, read by 0xED0A0; 0xECC10 adds each banked combo, 0xEC900 zeroes it at race setup)
#define PGR2_RACE_KUDOS "0xX39d3dc>="

// The race object from 0x383178: challenge type 0x383178 (the type table 0x30CEEC: 4 Street Race, 17 Overtake ...),
// group 0x383190 (3 Kudos World Series), difficulty 0x383868 (1 Novice to 5 Expert, the medal a success earns). At a
// race's end 0xF2650 decides the result into the success byte 0x383A58 and sets 0x383984 (results done; race setup
// 0xF14E0 clears it): this matches the moment a Kudos World Series race at Expert is won.
#define PGR2_KWS_EXPERT_WON "0xX383190=3_0xX383868=5_0xH383a58=1_d0xH383984=0_0xH383984=1"
// 0x39D550 + car x 0x2498: 1 from race setup (0xF0520) until the car's first contact with a wall or a car (0xF0C50,
// from the collision response 0x2EDE0 through 0xF3170)
#define PGR2_CLEAN "0xH39d550=1_"

// The Kudos World Series' Overtake events (each event's type: int16 at 0x33BEB4 + event x 0x68); an Overtake is won by
// passing the target number of cars (0x39B340 >= 0x38385C), its medal then raised to the difficulty
constexpr int kOvertakes[15] = {8, 20, 28, 45, 58, 64, 67, 91, 100, 105, 110, 114, 126, 130, 138};

// any of the events with at least that medal: one alternative group each, the core always true
const char* AnyMedal(const int* events, int count, int medal) {
    static std::deque<std::string> kept;
    std::string c = "1=1";
    for (int i = 0; i < count; i++) {
        char b[32];
        std::snprintf(b, sizeof b, "S0xX%x>=%d", 0x35DD88 + events[i] * 0x1C, medal);
        c += b;
    }
    return kept.emplace_back(std::move(c)).c_str();
}

// Time Attack's circuit routes with a record: route r's record is 0x50 bytes from 0x3609B4 (+8 best lap in ms, +0xC a
// byte set once there is one; written by 0xA6830 at a race's end after a new best lap, 0xEFD90). These 92 are the
// circuit challenges (route info 0x30DEA4 + r x 0x20), the game's own "n / 92" count (0x62F36) adds the same bytes.
constexpr int kCircuits[92] = {1,   2,   3,   4,   5,   6,   7,   11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  23,  24,
                               25,  26,  27,  28,  29,  30,  31,  32,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,
                               50,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  75,  76,  77,  78,  79,  80,  84,  85,
                               86,  87,  88,  89,  90,  93,  94,  95,  96,  97,  98,  99,  100, 103, 104, 105, 106, 107, 108,
                               109, 110, 111, 112, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126};

// at least n routes with a record
const char* RouteRecords(int n) {
    static std::deque<std::string> kept;
    std::string c;
    for (int i = 0; i < 92; i++) {
        char b[32];
        std::snprintf(b, sizeof b, "%s0xH%x%s", i + 1 < 92 ? "A:" : "", 0x3609C0 + kCircuits[i] * 0x50, i + 1 < 92 ? "_" : "");
        c += b;
    }
    return kept.emplace_back(c + ">=" + std::to_string(n)).c_str();
}

}  // namespace

std::vector<achievements::Def> AchievementSet() {
    std::vector<achievements::Def> set = {
        {"kws_steel", "Steel Nerves", "Complete the Kudos World Series at Novice (Steel)", 25, Medals(kSeries, 1)},
        {"kws_bronze", "Bronze Age", "Complete the Kudos World Series at Easy (Bronze) or better", 25, Medals(kSeries, 2)},
        {"kws_silver", "Silver Service", "Complete the Kudos World Series at Medium (Silver) or better", 50, Medals(kSeries, 3)},
        {"kws_gold", "Golden Era", "Complete the Kudos World Series at Hard (Gold) or better", 50, Medals(kSeries, 4)},
        {"kws_platinum", "Platinum Driver", "Complete the Kudos World Series at Expert (Platinum)", 100, Medals(kSeries, 5)},
        {"rank5", "Making a Name", "Reach Kudos Rank 5", 2, PGR2_TOTAL_KUDOS ">=20250"},
        {"rank10", "Rising Star", "Reach Kudos Rank 10", 3, PGR2_TOTAL_KUDOS ">=61000"},
        {"rank20", "Crowd Pleaser", "Reach Kudos Rank 20", 5, PGR2_TOTAL_KUDOS ">=194250"},
        {"rank30", "Showman", "Reach Kudos Rank 30", 10, PGR2_TOTAL_KUDOS ">=382500"},
        {"rank40", "Kudos King", "Reach Kudos Rank 40", 10, PGR2_TOTAL_KUDOS ">=575000"},
        // issue 38: rank 50 is about the most a player can reach (every World Series and Arcade race at Platinum), so the
        // last two are ranks 45 and 50; the ids stay as they were so unlocks already earned are kept
        {"rank50", "Living Legend", "Reach Kudos Rank 45", 25, PGR2_TOTAL_KUDOS ">=953500"},
        {"rank60", "Gotham's Finest", "Reach Kudos Rank 50", 50, PGR2_TOTAL_KUDOS ">=1520000"},
    };
    // two for each class: every race with a medal, every race at Platinum
    static const char* const kClass[14][3] = {
        {"Compact Sports", "Pocket Rocket", "Hot Hatch Hero"},       {"Sports Convertible", "Top Down", "Wind in Your Hair"},
        {"Coupe", "Two Doors", "Coupe de Grace"},                    {"Sport Utility", "Utility Player", "Off the Beaten Path"},
        {"Pacific Muscle", "Pacific Rim", "Rising Sun"},              {"Roadster", "Open Road", "Roadster Royalty"},
        {"Classics", "Old School", "Timeless"},                       {"Sports Coupe", "Sport Mode", "Coupe d'Etat"},
        {"American Muscle", "V8 Thunder", "Stars and Stripes"},       {"Super Car", "Super Licence", "Supremacy"},
        {"Grand Touring", "Grand Tour", "Gentleman Driver"},          {"Track Specials", "Track Day", "Race Bred"},
        {"Extreme", "Living on the Edge", "Extremist"},               {"Ultimate", "Ultimate Driver", "The Ultimate"}};
    static std::deque<std::string> text;  // ids and descriptions made here
    for (int c = 0; c < 14; c++) {
        bool late = c >= 7;
        const char* id = text.emplace_back("class" + std::to_string(c + 1) + "_any").c_str();
        const char* desc = text.emplace_back("Win every " + std::string(kClass[c][0]) + " event with a medal").c_str();
        set.push_back({id, kClass[c][1], desc, late ? 4u : 3u, Medals(kClasses[c], 1)});
        id = text.emplace_back("class" + std::to_string(c + 1) + "_platinum").c_str();
        desc = text.emplace_back("Win every " + std::string(kClass[c][0]) + " event at Platinum").c_str();
        set.push_back({id, kClass[c][2], desc, late ? 10u : 5u, Medals(kClasses[c], 5)});
    }
    set.insert(set.end(), {
        {"arcade_street_gold", "Street Smart", "Gold or better on every Street Racing stage", 5, Medals(kArcade[0], 4)},
        {"arcade_timed_gold", "Against the Clock", "Gold or better on every Timed Run stage", 5, Medals(kArcade[1], 4)},
        {"arcade_cone_gold", "Cone Zone", "Gold or better on every Cone Challenge stage", 5, Medals(kArcade[2], 4)},
        {"arcade_street_platinum", "Street Legend", "Platinum on every Street Racing stage", 10, Medals(kArcade[0], 5)},
        {"arcade_timed_platinum", "Clockwork", "Platinum on every Timed Run stage", 10, Medals(kArcade[1], 5)},
        {"arcade_cone_platinum", "Cone Artist", "Platinum on every Cone Challenge stage", 10, Medals(kArcade[2], 5)},
        {"arcade_platinum", "Arcade Perfect", "Platinum on every Arcade stage", 25, Medals(kArcadeAll, 5)},
        // the garage
        {"token_gesture", "Token Gesture", "Unlock a car with Kudos Tokens", 2, Owned(kTokenCars, 68, 1)},
        {"cars40", "Collector", "Own 40 cars", 5, Owned(AllCars(), 102, 40)},
        {"cars60", "Curator", "Own 60 cars", 10, Owned(AllCars(), 102, 60)},
        {"cars80", "Museum Piece", "Own 80 cars", 25, Owned(AllCars(), 102, 80)},
        {"cars102", "Full House", "Own every car", 50, Owned(AllCars(), 102, 102)},
        // in a race: speed (100 mph is 44.7 m/s, 175 mph 78.2) and winning (position 0, the lap past the last one; the
        // lap is negative before the start)
        {"speed100", "Ton Up", "Reach 100 mph in a race", 2, PGR2_RACING PGR2_SPEED "h4232c71c"},
        {"speed175", "Flat Out", "Reach 175 mph in a race", 10, PGR2_RACING PGR2_SPEED "h429c6e39"},
        {"win", "First Past the Post", "Win a race", 5,
         PGR2_RACING "0xX39b33c=0_0x 39b334<h8000_0x 39b334>=0xX383830"},
        {"kudos2000", "Style Points", "Earn 2,000 Kudos in a single race", 3, PGR2_RACING PGR2_RACE_KUDOS "2000"},
        {"kudos10000", "Showboat", "Earn 10,000 Kudos in a single race", 10, PGR2_RACING PGR2_RACE_KUDOS "10000"},
        {"overtake_expert", "Traffic Control", "Win an Expert Overtake event", 5, AnyMedal(kOvertakes, 15, 5)},
        {"untouchable", "Untouchable", "Win a Kudos World Series race at Expert without touching a wall or a car", 25,
         PGR2_CLEAN PGR2_KWS_EXPERT_WON},
        {"lap_records", "Lap of Honour", "Set a lap record in Time Attack on 10 different routes", 10, RouteRecords(10)},
        // Geometry Wars in the garage (0x145990 draws its text): score 0x45A308, hi-score 0x45A314, bombs 0x45A304,
        // state 0x45A300 (1 game over)
        {"gw_play", "Insert Coin", "Score in Geometry Wars", 1, "0xX45a308>0"},
        {"gw_10k", "Pacifist", "Score 10,000 in Geometry Wars", 3, "0xX45a308>=10000"},
        {"gw_100k", "Grid Warrior", "Score 100,000 in Geometry Wars", 10, "0xX45a308>=100000"},
        {"gw_1m", "Geometry Master", "Score 1,000,000 in Geometry Wars", 25, "0xX45a308>=1000000"},
    });
    // PGR2_ACH_TEST=1: a test achievement that unlocks when a race starts (D3D's present interval, 0x1D093C, goes to
    // two vertical blanks a frame)
    if (const char* e = std::getenv("PGR2_ACH_TEST"); e && *e == '1')
        set.push_back({"test_race", "Test: green light", "Start a race", 1, "0xX1d093c=h80000002"});
    return set;
}

}  // namespace pgr2
