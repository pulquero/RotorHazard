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
    virtual void start(MicroClock& uclock) {
        this->uclock = &uclock;
    }
    virtual bool readRssi() = 0;

protected:
    MicroClock* uclock = nullptr;

    void startNode(RssiNode& node) {
        utime_t us = uclock->tickMicros();  // update ticks
        node.start(uclock->millis(), us);
    }

    bool processNode(RssiNode& node, RxModule& rx) {
        bool crossing = false;
        if (node.active) {
            RssiResult result = node.process(rx.readRssi(), uclock->millis());
            Settings& settings = node.getSettings();
            switch (settings.mode) {
                case TIMER:
                    crossing = result.crossing;
                    break;
                case SCANNER:
                    if (result.nextFreq > 0) {
                        rx.setFrequency(result.nextFreq);
                        // get the actual frequency we will be scanning
                        settings.vtxFreq = rx.getFrequency();
                        uclock->tickMicros();  // update ticks
                        node.resetState(uclock->millis());
                    }
                    break;
            }
        }
        node.getState().updateLoopTime(uclock->tickMicros());
        return crossing;
    }
};


template <typename RX> class SingleRssiReceiver final : public RssiReceivers {
private:
    RssiNode node;
    RX rx;

public:
    RssiNode& getRssiNode(uint_fast8_t idx) { return node; }
    RxModule& getRxModule(uint_fast8_t idx) { return rx; }
    Settings& getSettings(uint_fast8_t idx) { return node.getSettings(); }
    uint_fast8_t getCount() const { return 1; }
    void start(MicroClock& uclock) {
        RssiReceivers::start(uclock);
        startNode(node);
    }
    bool readRssi() {
        bool crossing = processNode(node, rx);
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
    RssiNode& getRssiNode(uint_fast8_t idx) { return nodes[idx]; }
    RxModule& getRxModule(uint_fast8_t idx) { return rx; }
    Settings& getSettings(uint_fast8_t idx) { return nodes[idx].getSettings(); }
    uint_fast8_t getCount() const { return nodeCount; }
    void start(MicroClock& uclock) {
        RssiReceivers::start(uclock);
        // preserve order on start
        for (uint_fast8_t i=0; i<nodeCount; i++) {
            startNode(nodes[i]);
        }
    }
    bool readRssi() {
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
            bool crossing = processNode(nodes[idx], rx);
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
    RssiNode& getRssiNode(uint_fast8_t idx) { return nodes[idx]; }
    RxModule& getRxModule(uint_fast8_t idx) { return rxs[idx]; }
    Settings& getSettings(uint_fast8_t idx) { return nodes[idx].getSettings(); }
    uint_fast8_t getCount() const { return nodeCount; }
    void start(MicroClock& uclock) {
        RssiReceivers::start(uclock);
        // preserve order on start
        for (uint_fast8_t i=0; i<nodeCount; i++) {
            startNode(nodes[i]);
        }
    }
    bool readRssi() {
        bool anyCrossing = false;
        // preserve order on read
        for (uint_fast8_t i=0; i<nodeCount; i++) {
            bool crossing = processNode(nodes[i], rxs[i]);
            if (crossing) {
                anyCrossing = true;
            }
        }
        return anyCrossing;
    }
};

extern RssiReceivers& rssiRxs;

#endif
