// City drawing tables past the disc build's 100 entries (the DLC cities, made for the title update, need more).
//
// The city material object at 0x3FE8C0 holds three fixed arrays one after another: 100 pixel constants of 16 bytes,
// 100 dwords at 0x3FEF00, then the material types (8 bytes each) at 0x3FF090. A city pak's chunk loader (0x16C450)
// appends its dwords at the count 0x40AA56 and one byte each at 0x4076F0 + the same index. Paris has 113: the 13
// past the end wrote over material types 0 to 6 (type 5 lost its shaders and vertex size: the floor's draws asked
// for stride -4 and were left out) and over the variables after the byte array (0x407754). The title update moved
// the arrays and made them 150 long.
//
// Here the entries past 100 are kept aside: the loader's overflow is copied out and what it overwrote put back; the
// one reader (0x118B90, an index byte at +6 of the draw record) sees the kept entry in its slot for that call alone;
// the reset (0x1BEFE0) clears them too.
#include <cstdint>
#include <cstring>

#include "core/log.h"
#include "kernel/kernel.h"
#include "xdk/hle.h"

namespace {

constexpr uint32_t kDwords = 0x3FEF00, kBytes = 0x4076F0, kTypes = 0x3FF090;
constexpr uint32_t kCount = 0x40AA56, kTypeCount = 0x40AA54;
constexpr uint32_t kSlots = 100, kMax = 256;  // the index is a byte

uint32_t g_dwords[kMax];
uint8_t g_bytes[kMax];

uint16_t Count() { return kernel::G<uint16_t>(kCount); }

}  // namespace

X86_EXTERN_FUNC(__imp__sub_0016C450);
X86_FUNC(sub_0016C450) {
    constexpr uint32_t kOver = kMax - kSlots;
    uint8_t types[kOver * 4], after[kOver];
    std::memcpy(types, kernel::HostPtr(kTypes), sizeof(types));  // where dwords 100 on land
    std::memcpy(after, kernel::HostPtr(kBytes + kSlots), sizeof(after));
    uint32_t from = Count(), types_before = kernel::G<uint16_t>(kTypeCount);
    __imp__sub_0016C450(ctx, base);
    uint32_t to = Count();
    if (to <= kSlots || to <= from) return;
    if (to > kMax) to = kMax;
    for (uint32_t i = from > kSlots ? from : kSlots; i < to; i++) {
        uint32_t k = i - kSlots;
        g_dwords[i] = kernel::R32(kDwords + i * 4);
        g_bytes[i] = kernel::G<uint8_t>(kBytes + i);
        // types this call added are newer than the copy; they lie far above the overflow in practice
        if (k * 4 >= types_before * 8) {
            LOG_FIRST(4, "pgr2: city tables: dword %u overlaps material types this pak added", i);
            continue;
        }
        std::memcpy(kernel::HostPtr(kTypes + k * 4), types + k * 4, 4);
        kernel::G<uint8_t>(kBytes + i) = after[k];
    }
    static uint32_t said = 0;
    if (to > said) {
        LOGR("pgr2: city tables: %u entries, %u past the disc build's %u kept aside", to, to - kSlots, kSlots);
        said = to;
    }
}

X86_EXTERN_FUNC(__imp__sub_00118B90);
X86_FUNC(sub_00118B90) {
    uint32_t i = kernel::G<uint8_t>(ctx.ecx + 6);
    uint8_t kind = kernel::G<uint8_t>(ctx.ecx + 4);
    bool reads = kind == 4 || kind == 6 || kind == 7 || kind == 0x11;  // the cases that read the dwords
    if (!reads || i < kSlots || i >= Count()) {
        __imp__sub_00118B90(ctx, base);
        return;
    }
    uint32_t slot = kDwords + i * 4, was = kernel::R32(slot);
    kernel::W32(slot, g_dwords[i]);
    __imp__sub_00118B90(ctx, base);
    kernel::W32(slot, was);
}

X86_EXTERN_FUNC(__imp__sub_001BEFE0);
X86_FUNC(sub_001BEFE0) {
    __imp__sub_001BEFE0(ctx, base);
    std::memset(g_dwords, 0, sizeof(g_dwords));
    std::memset(g_bytes, 0, sizeof(g_bytes));
}
