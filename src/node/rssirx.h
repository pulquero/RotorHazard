#ifndef rssirx_h
#define rssirx_h

#include "config.h"
#include "rssi.h"
#include "rx.h"
#include "microclock.h"


class RssiReceivers {
public:
    RssiReceivers() = default;
    RssiReceivers(const RssiReceivers&) = delete;
    RssiReceivers(RssiReceivers&&) = delete;
    RssiReceivers& operator=(const RssiReceivers&) = delete;
    RssiReceivers& operator=(RssiReceivers&&) = delete;

    virtual RssiNode& getRssiNode(uint_fast8_t idx) = 0;
    virtual RxModule& getRxModule(uint_fast8_t idx) = 0;
    virtual Settings& getSettings(uint_fast8_t idx) = 0;
    virtual uint_fast8_t getCount() const = 0;
    virtual void start(mtime_t ms, MicroClock& uclock) = 0;
    virtual bool readRssi(mtime_t ms, MicroClock& uclock) = 0;
};


inline bool handleResult(RxModule& rx, const RssiResult result) {
    bool crossing = false;
    switch (result.mode) {
        case TIMER:
            crossing = result.crossing;
            break;
        case SCANNER:
            if (result.nextFreq > 0) {
                rx.setFrequency(result.nextFreq);
            }
            break;
    }
    return crossing;
}

template <typename RX> class SingleRssiReceiver final : public RssiReceivers {
private:
    RssiNode node;
    RX rx;

public:
    inline RssiNode& getRssiNode(uint_fast8_t idx) { return node; }
    inline RxModule& getRxModule(uint_fast8_t idx) { return rx; }
    inline Settings& getSettings(uint_fast8_t idx) { return node.getSettings(); }
    inline uint_fast8_t getCount() const { return 1; }
    inline void start(const mtime_t ms, MicroClock& uclock) {
        node.start(ms, uclock.tickMicros());
    }
    inline bool readRssi(const mtime_t ms, MicroClock& uclock) {
        bool crossing = false;
        if (node.active) {
            RssiResult result = node.process(rx.readRssi(), ms);
            crossing = handleResult(rx, result);
        }
        node.getState().updateLoopTime(uclock.tickMicros());
        return crossing;
    }
};


template <typename RX, uint_fast8_t N> class VirtualRssiReceivers final : public RssiReceivers {
private:
    uint_fast8_t nodeCount = N;
    RssiNode nodes[N];
    RX rx;
    int_fast8_t prevReadIndex = -1;
    uint_fast16_t readCounter = 0;

public:
    inline RssiNode& getRssiNode(uint_fast8_t idx) { return nodes[idx]; }
    inline RxModule& getRxModule(uint_fast8_t idx) { return rx; }
    inline Settings& getSettings(uint_fast8_t idx) { return nodes[idx].getSettings(); }
    inline uint_fast8_t getCount() const { return nodeCount; }
    inline void start(const mtime_t ms, MicroClock& uclock) {
        // preserve order on start
        for (uint_fast8_t i=0; i<nodeCount; i++) {
            nodes[i].start(ms, uclock.tickMicros());
        }
    }
    inline bool readRssi(const mtime_t ms, MicroClock& uclock) {
        if (nodeCount > 0) {
            uint_fast8_t idx;
            if (nodeCount > 1) {
                // alternate the single rx between different freqs
                idx = (readCounter++ % (READS_PER_FREQ*nodeCount))/READS_PER_FREQ;
                if (idx != prevReadIndex) {
                    rx.setFrequency(nodes[idx].getSettings().vtxFreq);
                    prevReadIndex = idx;
                }
            } else {
                idx = 0;
            }
            bool crossing = false;
            RssiNode& node = nodes[idx];
            if (node.active) {
                RssiResult result = node.process(rx.readRssi(), ms);
                crossing = handleResult(rx, result);
            }
            node.getState().updateLoopTime(uclock.tickMicros());
            return crossing;
        } else {
            return false;
        }
    }
};


template <typename RX, uint_fast8_t N> class PhysicalRssiReceivers final : public RssiReceivers {
private:
    uint_fast8_t nodeCount = N;
    RssiNode nodes[N];
    RX rxs[N];

public:
    inline RssiNode& getRssiNode(uint_fast8_t idx) { return nodes[idx]; }
    inline RxModule& getRxModule(uint_fast8_t idx) { return rxs[idx]; }
    inline Settings& getSettings(uint_fast8_t idx) { return nodes[idx].getSettings(); }
    inline uint_fast8_t getCount() const { return nodeCount; }
    inline void start(const mtime_t ms, MicroClock& uclock) {
        // preserve order on start
        for (uint_fast8_t i=0; i<nodeCount; i++) {
            nodes[i].start(ms, uclock.tickMicros());
        }
    }
    inline bool readRssi(const mtime_t ms, MicroClock& uclock) {
        bool anyCrossing = false;
        // preserve order on read
        for (uint_fast8_t i=0; i<nodeCount; i++) {
            RssiNode& node = nodes[i];
            if (node.active) {
                RxModule& rx = rxs[i];
                RssiResult result = node.process(rx.readRssi(), ms);
                if (handleResult(rx, result)) {
                    anyCrossing = true;
                }
            }
            node.getState().updateLoopTime(uclock.tickMicros());
        }
        return anyCrossing;
    }
};

extern RssiReceivers& rssiRxs;

#endif
