
#include <stdio.h>
#include <memory.h>

#include "./systick.h"
#include "./perf.h"

namespace lightkraken {

static const char *slotNames[] = {
	"ENET input      : ",
	"ArtNet dispatch : ",
	"Strip copy      : ",
	"Strip transfer  : ",
	"SPI Interrupt   : ",
	""
};

bool PerfMeasure::init = false;
PerfMeasure::Entry PerfMeasure::slots[SLOT_COUNT];

PerfMeasure::PerfMeasure(Slot slot_) {
	if (!init) {
		init = true;
		memset(slots, 0, sizeof(slots));
	}
	slot = slot_;
	start_time = Systick::instance().systemTick();
}

PerfMeasure::~PerfMeasure() {
	slots[slot].accumulated += Systick::instance().systemTick() - start_time;
	slots[slot].count ++;
}

void PerfMeasure::print() {
	for (int32_t c = 0; c < SLOT_COUNT ; c++) {
		uint64_t avg_time = slots[c].accumulated / slots[c].count;
		printf("%s: %d\n", slotNames[c], int(avg_time));
	}
}

}
