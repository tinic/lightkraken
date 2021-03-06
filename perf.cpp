
#include <stdio.h>
#include <string.h>
#include <algorithm>

#include "./systick.h"
#include "./perf.h"

extern "C" uint32_t SystemCoreClock;

namespace lightkraken {

static const char *slotNames[] = {
    "ENET input      ",
    "ArtNet dispatch ",
    "sACN dispatch   ",
    "Strip copy      ",
    "Strip transfer  ",
    "SPI Interrupt   ",
    "Set Uni data    ",
    "REST GET        ",
    "REST POST       ",
    ""
};

bool PerfMeasure::init = false;
PerfMeasure::Entry PerfMeasure::slots[SLOT_COUNT];

PerfMeasure::PerfMeasure(Slot slot_) {
    if (!init) {
        init = true;
        memset(slots, 0, sizeof(slots));
        for (int32_t c = 0; c < SLOT_COUNT ; c++) {
            slots[c].min = ~uint64_t(0);
        }
    }
    slot = slot_;
    start_time = Systick::instance().systemTick();
}

PerfMeasure::~PerfMeasure() {
    uint64_t this_interval = Systick::instance().systemTick() - start_time;

    slots[slot].accumulated += this_interval;
    slots[slot].count ++;

    slots[slot].max = std::max(slots[slot].max, this_interval);
    slots[slot].min = std::min(slots[slot].min, this_interval);
    slots[slot].last = this_interval;
}

void PerfMeasure::print() {
    (void)slotNames;
#if 0
    static int32_t counter = 0;
    printf("== %04d ===============\n", int(counter++));
    for (int32_t c = 0; c < SLOT_COUNT ; c++) {
        uint64_t avg_time = slots[c].accumulated / slots[c].count;
        uint64_t min_time = (slots[c].min == (~uint64_t(0))) ? 0 : slots[c].min;
        uint64_t max_time = slots[c].max;
        uint64_t last_time = slots[c].last;
        uint64_t count = slots[c].count;
        uint64_t last_time_ns = (uint64_t(last_time) * 1000000) / uint64_t(SystemCoreClock);
        printf("%s: last(%09d, %09dus) avg(%09d) min(%09d) max(%09d) count(%09d)\n", slotNames[c], int(last_time), int(last_time_ns), int(avg_time), int(min_time), int(max_time), int(count));
    }
#endif  // #if 0
}

}
