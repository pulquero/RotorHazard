#ifndef rx_h
#define rx_h

#include "config.h"
#include "util/rhtypes.h"

inline freq_t canonicalFrequency(freq_t f) {
#ifdef USE_SPI_3WIRE_READ
    return (f & 0x01) == 0 ? f-1 : f;
#else
    return f;
#endif
}


class SPIProtocol {
protected:
    uint8_t dataPin = 0;
    uint8_t clkPin = 0;
public:
    virtual void init(uint8_t clkPin, uint8_t dataPin) {
        this->clkPin = clkPin;
        this->dataPin = dataPin;
    }
    virtual void write(uint8_t selPin, uint8_t addr, uint32_t data) = 0;
#ifdef USE_SPI_3WIRE_READ
    virtual uint32_t read(uint8_t selPin, uint8_t addr) = 0;
#endif
};


class BitBangSPI final : public SPIProtocol {
private:
    template <typename T> void writeBitBang(T bits, uint_fast8_t size) const;
    inline void writeBit(bool b) const;
#ifdef USE_SPI_3WIRE_READ
    template <typename T> T readBitBang(uint_fast8_t size) const;
    inline bool readBit() const;
#endif

public:
    void init(uint8_t clkPin, uint8_t dataPin);
    void write(uint8_t selPin, uint8_t addr, uint32_t data);
#ifdef USE_SPI_3WIRE_READ
    uint32_t read(uint8_t selPin, uint8_t addr);
#endif
};


#if TARGET == ESP32_TARGET
class NativeSPI final : public SPIProtocol {
public:
    void init(uint8_t clkPin, uint8_t dataPin);
    void write(uint8_t selPin, uint8_t addr, uint32_t data);
};
#endif


#if TARGET == TEST_TARGET
class TestSPI final : public SPIProtocol {
private:
    uint32_t rxRegisters[16];
public:
    void init(uint8_t clkPin, uint8_t dataPin) {
        SPIProtocol::init(clkPin, dataPin);
    }
    void write(uint8_t selPin, uint8_t addr, uint32_t data) {
        rxRegisters[addr] = data;
    }
#ifdef USE_SPI_3WIRE_READ
    uint32_t read(uint8_t selPin, uint8_t addr) {
        return rxRegisters[addr];
    }
#endif
};
#endif


/**
 * Functions for the rx5808 module.
 * See RTC6715 datasheet for spec.
 */
class RxModule final {
private:
    uint8_t selPin = 0;
    uint8_t rssiPin = 0;
    SPIProtocol* spi = nullptr;
    mtime_t lastSpiIOTimeMs = 0;
#ifndef USE_SPI_3WIRE_READ
    freq_t frequency;
    bool rxPoweredDown = false;
#endif
    void spiBegin();
    void spiEnd();

public:
    RxModule() = default;
    RxModule(const RxModule&) = delete;
    RxModule(RxModule&&) = delete;
    RxModule& operator=(const RxModule&) = delete;
    RxModule& operator=(RxModule&&) = delete;

    void init(SPIProtocol* spi, uint8_t selPin, uint8_t rssiPin);
    uint8_t getChipSelectPin() const { return selPin; }
    uint8_t getRssiPin() const { return rssiPin; }
    void setFrequency(freq_t frequency);
    freq_t getFrequency();
    void setPower(uint32_t options);
    rssi_t readRssi() const;
    void powerUp();
    void powerDown();
    bool isPoweredDown();
    void reset();
};
#endif
