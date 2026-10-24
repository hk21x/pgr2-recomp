// City material slots past the disc build's 100 (the DLC cities, made for the title update, need more).
//
// The city material object 0x3FE8C0 holds its slots in three arrays of 100, one after another, and then the material
// types and the animated textures:
//   +0x0000 pixel constants, 16 bytes a slot      (0x3FE8C0)
//   +0x0640 a colour dword a slot                 (0x3FEF00)
//   +0x07D0 material types, 8 bytes each          (0x3FF090)
//   +0x8E30 a kind byte a slot                    (0x4076F0)
//   +0x8E94 animated textures, 0xC4 each, 64      (0x407754): each frame's streamed block number
//   +0xC196 the slot count                        (0x40AA56)
// Slots are appended by the city pak's chunk loader (0x16CFA0, through 0x16C0D0, then 0x107820 appends the animated
// textures) and the streaming one (0x16C450); every frame 0x1186C0 (and 0x117F30 at load) rewrites each slot's pixel
// constant from its kind and colour; a draw (0x118B90) uses the slot its record names (byte +6). Paris has 113 slots:
// the 13 past the end overwrote material types 0 to 6 (the missing floor), the kind bytes landed on the animated
// textures (a frame then asked for stream block 7742: PGR2's fatal "m_CompressionStream.msg") and the per-frame pass
// wrote pixel constants over the colours. The title update made the arrays 150 long.
//
// Here the object never holds more than 100: each loader runs with the real count, the slots past 100 are copied out
// and what they overwrote put back (for the pak loader before 0x107820 writes the animated textures), and the count is
// left at 100. The slots past 100 get their pixel constants from the game's own passes (0x117F30 at load, 0x1186C0
// each frame, which read only the count, colours and kinds of their object) run on a scratch copy holding just them;
// a draw naming one is given a slot below 100 with that slot's colour and pixel constant swapped in for the call.
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "core/log.h"
#include "kernel/kernel.h"
#include "kernel/memory.h"
#include "xdk/hle.h"

X86_EXTERN_FUNC(__imp__sub_00117F30);  // the slots' pixel constants at city load (kinds 2 and 9)
X86_EXTERN_FUNC(__imp__sub_001186C0);  // and each frame

namespace {

constexpr uint32_t kObject = 0x3FE8C0, kColours = 0x3FEF00, kKinds = 0x4076F0, kTypes = 0x3FF090, kAnims = 0x407754;
constexpr uint32_t kCount = 0x40AA56, kTypeCount = 0x40AA54;
constexpr uint32_t kSlots = 100, kMax = 256, kOver = kMax - kSlots;  // a draw names its slot in a byte

uint32_t g_real = 0;                // the slots the city has, past 100 too
uint32_t g_colour[kMax];            // slots 100 on
uint8_t g_kind[kMax], g_map[kMax];  // their kind, and the slot below 100 a draw is given

// what the slots past 100 land on, kept before a loader writes them
struct Saved {
    bool pending = false;
    uint32_t from = 0, types = 0;
    uint8_t types_head[kOver * 4], anims_head[kOver];
} g_saved;

uint16_t& Count() { return kernel::G<uint16_t>(kCount); }

// the scratch object: the slots past 100 as its slots 0 on, for the constant passes
constexpr uint32_t kObjectSize = 0xC1A0;
uint32_t g_scratch = 0;

void FillScratch() {
    if (!g_scratch) g_scratch = mem::AllocSystem(kObjectSize, 16);
    if (!g_scratch) return;
    uint32_t n = g_real - kSlots;
    kernel::G<uint16_t>(g_scratch + 0xC196) = uint16_t(n);
    std::memcpy(kernel::HostPtr(g_scratch + 0x640), &g_colour[kSlots], n * 4);
    std::memcpy(kernel::HostPtr(g_scratch + 0x8E30), &g_kind[kSlots], n);
}

// a fastcall pass (ecx the object) of the game's own, run on the scratch object
void OnScratch(X86Context& ctx, uint8_t* base, void (*pass)(X86Context& __restrict, uint8_t*)) {
    uint32_t ecx = ctx.ecx, esp = ctx.esp;
    ctx.ecx = g_scratch;
    ctx.esp -= 4;
    kernel::W32(ctx.esp, 0xFFFFFFF0u);
    pass(ctx, base);
    ctx.ecx = ecx;
    ctx.esp = esp;
}

// before a loader appends: the object counts every slot again, and what an overflow would overwrite is kept
void Begin() {
    if (g_real > kSlots) Count() = uint16_t(g_real);
    g_saved.pending = true;
    g_saved.from = Count();
    g_saved.types = kernel::G<uint16_t>(kTypeCount);
    std::memcpy(g_saved.types_head, kernel::HostPtr(kTypes), sizeof(g_saved.types_head));
    std::memcpy(g_saved.anims_head, kernel::HostPtr(kAnims), sizeof(g_saved.anims_head));
}

// the closest slot below 100 to slot i: the same kind and colour gives the same pixel constant
uint8_t Closest(uint32_t i) {
    int best = 0, best_d = 1 << 30;
    for (uint32_t j = 0; j < kSlots; j++) {
        if (kernel::G<uint8_t>(kKinds + j) != g_kind[i]) continue;
        uint32_t a = kernel::R32(kColours + j * 4), b = g_colour[i];
        int d = 0;
        for (int s = 0; s < 32; s += 8) d += std::abs(int(a >> s & 0xFF) - int(b >> s & 0xFF));
        if (d < best_d) best = int(j), best_d = d;
    }
    return uint8_t(best);
}

// after it appended: the slots past 100 copied out, what they overwrote put back, the object left at 100
void End(X86Context& ctx, uint8_t* base) {
    if (!g_saved.pending) return;
    g_saved.pending = false;
    uint32_t to = Count();
    if (to <= kSlots) {
        g_real = to;
        return;
    }
    if (to > kMax) {
        LOG_FIRST(4, "pgr2: city slots: %u, past the %u a draw can name", to, kMax);
        to = kMax;
    }
    uint32_t from = g_saved.from > kSlots ? g_saved.from : kSlots;
    for (uint32_t i = from; i < to; i++) {
        uint32_t k = i - kSlots;
        g_colour[i] = kernel::R32(kColours + i * 4);
        g_kind[i] = kernel::G<uint8_t>(kKinds + i);
        if (k * 4 + 4 <= g_saved.types * 8)  // material types older than this pak's (its own lie far above)
            std::memcpy(kernel::HostPtr(kTypes + k * 4), g_saved.types_head + k * 4, 4);
        else
            LOG_FIRST(4, "pgr2: city slots: slot %u overlaps material types this pak added", i);
        kernel::G<uint8_t>(kAnims + k) = g_saved.anims_head[k];
    }
    g_real = to;
    Count() = uint16_t(kSlots);
    int exact = 0;
    for (uint32_t i = kSlots; i < g_real; i++) {
        g_map[i] = Closest(i);
        exact += kernel::R32(kColours + g_map[i] * 4) == g_colour[i];
    }
    FillScratch();
    if (g_scratch) OnScratch(ctx, base, __imp__sub_00117F30);
    static uint32_t said = 0;
    if (g_real != said) {
        LOGR("pgr2: city slots: %u, the %u past the disc build's %u kept aside (%d stand-ins of the same colour)", g_real,
             g_real - kSlots, kSlots, exact);
        said = g_real;
    }
}

void Reset() {
    g_real = 0;
    g_saved.pending = false;
}

}  // namespace

// the streaming chunk loader: material types, colours and kinds of a streamed pak part
X86_EXTERN_FUNC(__imp__sub_0016C450);
X86_FUNC(sub_0016C450) {
    Begin();
    __imp__sub_0016C450(ctx, base);
    End(ctx, base);
}

// the city pak's chunk loader: its MAT chunk takes slots through 0x16C0D0 (this, count, &first, &kinds; the colours
// returned), reads them in, then 0x107820 appends the animated textures where the kinds overflow
X86_EXTERN_FUNC(__imp__sub_0016C0D0);
X86_FUNC(sub_0016C0D0) {
    Begin();
    __imp__sub_0016C0D0(ctx, base);
}

X86_EXTERN_FUNC(__imp__sub_00107820);
X86_FUNC(sub_00107820) {
    End(ctx, base);
    __imp__sub_00107820(ctx, base);
}

X86_EXTERN_FUNC(__imp__sub_0016CFA0);
X86_FUNC(sub_0016CFA0) {
    __imp__sub_0016CFA0(ctx, base);
    End(ctx, base);  // a MAT chunk that returned before 0x107820
}

// a draw's material state (ecx the draw record: kind at +4, slot at +6)
X86_EXTERN_FUNC(__imp__sub_00118B90);
X86_FUNC(sub_00118B90) {
    uint32_t rec = ctx.ecx, i = kernel::G<uint8_t>(rec + 6);
    if (i < kSlots || i >= g_real) {
        __imp__sub_00118B90(ctx, base);
        return;
    }
    // slot j stands in: its colour and pixel constant are slot i's for the call; the last slot set (0x28F6E1, which
    // spares setting the same constants again) is forgotten before and after, as j stands for other slots too
    uint32_t j = g_map[i], colour = kColours + j * 4, pc = kObject + j * 16, was = kernel::R32(colour);
    uint8_t pc_was[16];
    std::memcpy(pc_was, kernel::HostPtr(pc), 16);
    if (g_scratch) std::memcpy(kernel::HostPtr(pc), kernel::HostPtr(g_scratch + (i - kSlots) * 16), 16);
    kernel::G<uint8_t>(rec + 6) = uint8_t(j);
    kernel::W32(colour, g_colour[i]);
    kernel::G<uint8_t>(0x28F6E1) = 0xFF;
    __imp__sub_00118B90(ctx, base);
    kernel::G<uint8_t>(0x28F6E1) = 0xFF;
    kernel::W32(colour, was);
    std::memcpy(kernel::HostPtr(pc), pc_was, 16);
    kernel::G<uint8_t>(rec + 6) = uint8_t(i);
}

// each frame's pixel constants: the object's 100, then the slots past 100 on the scratch object
X86_FUNC(sub_001186C0) {
    __imp__sub_001186C0(ctx, base);
    if (g_real > kSlots && g_scratch) OnScratch(ctx, base, __imp__sub_001186C0);
}

// the object's resets (0x1BEFE0 clears the colours too)
X86_EXTERN_FUNC(__imp__sub_001BEFE0);
X86_FUNC(sub_001BEFE0) {
    __imp__sub_001BEFE0(ctx, base);
    Reset();
}

X86_EXTERN_FUNC(__imp__sub_001073C0);
X86_FUNC(sub_001073C0) {
    __imp__sub_001073C0(ctx, base);
    Reset();
}
