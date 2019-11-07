#ifndef _PERF_H_
#define _PERF_H_

namespace lightkraken {

class PerfMeasure {
public:

	enum Slot {
		SLOT_ENET_INPUT,
		SLOT_ARNET_DISPATCH,
		SLOT_STRIP_COPY,
		SLOT_STRIP_TRANFER,
		SLOT_SPI_INTERRUPT,
		SLOT_COUNT
	};

	PerfMeasure(Slot slot);
	~PerfMeasure();

	static void print();

private:

	struct Entry {
		uint64_t accumulated;
		uint64_t count;
	};

	static bool init;
	static Entry slots[SLOT_COUNT];

	uint64_t start_time;
	Slot slot;
};

}

#endif  // #ifndef _PERF_H_
