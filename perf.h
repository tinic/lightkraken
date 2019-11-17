#ifndef _PERF_H_
#define _PERF_H_

namespace lightkraken {

class PerfMeasure {
public:

    enum Slot {
        SLOT_ENET_INPUT,
        SLOT_ARNET_DISPATCH,
        SLOT_SACN_DISPATCH,
        SLOT_STRIP_COPY,
        SLOT_STRIP_TRANFER,
        SLOT_SPI_INTERRUPT,
        SLOT_SET_DATA,
        SLOT_REST_GET,
        SLOT_REST_POST,
        SLOT_COUNT
    };

#ifndef BOOTLOADER
    PerfMeasure(Slot slot);
    ~PerfMeasure();
#else  // #ifndef BOOTLOADER
    PerfMeasure(Slot) {};
    ~PerfMeasure() {};
#endif  // #ifndef BOOTLOADER

    static void print();

private:

    struct Entry {
        uint64_t accumulated;
        uint64_t last;
        uint64_t max;
        uint64_t min;
        uint64_t count;
    };

    static bool init;
    static Entry slots[SLOT_COUNT];

    uint64_t start_time;
    Slot slot;
};

}

#endif  // #ifndef _PERF_H_
