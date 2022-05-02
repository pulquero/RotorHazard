#include "hardware.h"

class TestHardware : public Hardware {
public:
    bool isInit;
    bool isRxInit[MULTI_RHNODE_MAX];
    bool isSettingsInit[MULTI_RHNODE_MAX];

    TestHardware() : Hardware(HIGH,LOW) {
    }
    void init()
    {
        Hardware::init();
        uint8_t dataPin = 31;
        uint8_t clkPin = 32;
        printf("Initializing SPI with pins: data=%u, clock=%u\n", dataPin, clkPin);
        spi.init(clkPin, dataPin);

        printf("Hardware initialized\n");
        isInit = true;
    }
    void initRxModule(uint_fast8_t nIdx, RxModule& rx)
    {
        uint8_t selPin = 41 + nIdx;
        uint8_t rssiPin = 61 + nIdx;
        printf("Initializing RX module %d with pins: chip-select=%u, rssi=%u\n", nIdx, selPin, rssiPin);
        rx.init(&spi, selPin, rssiPin);
        isRxInit[nIdx] = true;
    }
    void initSettings(uint_fast8_t nIdx, Settings& settings)
    {
        printf("Initializing settings for RX module %d\n", nIdx);
        isSettingsInit[nIdx] = true;
    }
    void setStatusLed(bool onFlag) {
        bool currentFlag = currentStatusLedFlag;
        Hardware::setStatusLed(onFlag);
        if (onFlag != currentFlag) {
            printf("Status LED changed to %d\n", onFlag);
        }
    }

    const char* getProcessorType() {
      return "Test";
    }
};

extern TestHardware testHardware;
