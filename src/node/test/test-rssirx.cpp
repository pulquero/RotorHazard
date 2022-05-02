#include <ArduinoUnitTests.h>
#include <Godmode.h>
#include "../hardware.h"
#include "../rssirx.h"

#define assertRssiRx(rssirx, numRxs) \
{ \
    GodmodeState* nano = GODMODE(); \
    assertEqual(numRxs, rssirx.getCount()); \
    for (int i=0; i<rssirx.getCount(); i++) { \
        rssirx.getRssiNode(i); \
        RxModule& rx = rssirx.getRxModule(i); \
        assertEqual(HIGH, nano->digitalPin[rx.selPin]); \
        rssirx.getSettings(i); \
        assertEqual(i, rssirx.getSlotIndex(i)); \
        nano->analogPin[rx.rssiPin] = (MIN_RSSI_DETECT + 1) << 1; \
    } \
    rssirx.start(0, usclock); \
    assertEqual(numRxs, rssirx.getCount()); \
    assertFalse(rssirx.readRssi(0, usclock)); \
}

unittest(single_rx)
{
    GodmodeState* nano = GODMODE();
    nano->reset();
    SingleRssiReceiver<RX_IMPL> rssirx;
    hardware.initRxModule(0, rssirx.getRxModule(0));
    assertRssiRx(rssirx, 1);
}

unittest(physical_rx_0)
{
    GodmodeState* nano = GODMODE();
    nano->reset();
    PhysicalRssiReceivers<RX_IMPL,0> rssirx;
    assertRssiRx(rssirx, 0);
}

unittest(physical_rx_2)
{
    GodmodeState* nano = GODMODE();
    nano->reset();
    PhysicalRssiReceivers<RX_IMPL,2> rssirx;
    hardware.initRxModule(0, rssirx.getRxModule(0));
    hardware.initRxModule(1, rssirx.getRxModule(1));
    assertRssiRx(rssirx, 2);
}

unittest(physical_rx_2_no_rssi)
{
    GodmodeState* nano = GODMODE();
    nano->reset();
    PhysicalRssiReceivers<RX_IMPL,2> rssirx;
    hardware.initRxModule(0, rssirx.getRxModule(0));
    hardware.initRxModule(1, rssirx.getRxModule(1));
    rssirx.start(0, usclock); \
    assertEqual(0, rssirx.getCount()); \
}

unittest(virtual_rx_0)
{
    GodmodeState* nano = GODMODE();
    nano->reset();
    VirtualRssiReceivers<RX_IMPL,0> rssirx;
    assertRssiRx(rssirx, 0);
}

unittest(virtual_rx_2)
{
    GodmodeState* nano = GODMODE();
    nano->reset();
    VirtualRssiReceivers<RX_IMPL,2> rssirx;
    hardware.initRxModule(0, rssirx.getRxModule(0));
    assertRssiRx(rssirx, 2);
}

unittest_main()
