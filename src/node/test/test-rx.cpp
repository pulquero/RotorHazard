#include <ArduinoUnitTests.h>
#include <Godmode.h>
#include "../rx.h"

template <size_t N> class SPIDataObserver : public DataStreamObserver {
private:
    PinHistory<bool>& pin;
    bool bitsToSend[N];
    const int triggerCount;
    int recvCount = 0;

public:
    SPIDataObserver(PinHistory<bool>& pin, int writeCount) : DataStreamObserver(false, false), pin(pin), triggerCount(writeCount) {
        // size: 2 = pin initialisation, writeCount = addr + r/w bit, N = data, 1 = set data to LOW at end
        int size = pin.historySize();
        bool allDataBits[size];
        pin.toArray(allDataBits, size);
        memcpy(bitsToSend, allDataBits+2+writeCount, N);
        attach(&pin);
    }

    ~SPIDataObserver() {
        detach(&pin);
    }

protected:
    String observerName() const {
        return "SPI";
    }

    void onBit(bool aBit) {
        recvCount++;
        if (recvCount ==  triggerCount) {
            pin.fromArray(bitsToSend, N);
        }
    }
};


unittest(rx_setFrequency_even)
{
    uint8_t clkPin = 5, dataPin = 6;
    GodmodeState* nano = GODMODE();
    nano->reset();
    BitBangSPI spi;
    RxModule rx;
    rx.init(&spi, 0, 0);
    spi.init(clkPin, dataPin);
    freq_t expected = 5770;
    rx.setFrequency(expected);
    SPIDataObserver<20> spiResponder(nano->digitalPin[dataPin], 5);
    freq_t actual = rx.getFrequency();
    assertEqual(canonicalFrequency(expected), actual);
}

unittest(rx_setFrequency_odd)
{
    uint8_t clkPin = 5, dataPin = 6;
    GodmodeState* nano = GODMODE();
    nano->reset();
    BitBangSPI spi;
    RxModule rx;
    rx.init(&spi, 0, 0);
    spi.init(clkPin, dataPin);
    freq_t expected = 5845;
    rx.setFrequency(expected);
    SPIDataObserver<20> spiResponder(nano->digitalPin[dataPin], 5);
    freq_t actual = rx.getFrequency();
    assertEqual(canonicalFrequency(expected), actual);
}

unittest(rx_powerUp)
{
    uint8_t clkPin = 5, dataPin = 6;
    GodmodeState* nano = GODMODE();
    nano->reset();
    BitBangSPI spi;
    RxModule rx;
    rx.init(&spi, 0, 0);
    spi.init(clkPin, dataPin);
    rx.powerUp();
    SPIDataObserver<20> spiResponder(nano->digitalPin[dataPin], 5);
    assertFalse(rx.isPoweredDown());
}

unittest(rx_powerDown)
{
    uint8_t clkPin = 5, dataPin = 6;
    GodmodeState* nano = GODMODE();
    nano->reset();
    BitBangSPI spi;
    RxModule rx;
    rx.init(&spi, 0, 0);
    spi.init(clkPin, dataPin);
    rx.powerDown();
    SPIDataObserver<20> spiResponder(nano->digitalPin[dataPin], 5);
    assertTrue(rx.isPoweredDown());
}

unittest_main()
