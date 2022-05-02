#include "config.h"
#include "rx.h"
#include "hardware.h"
#include "microclock.h"

constexpr uint8_t SYNTHESIZER_REGISTER_A = 0x00;
constexpr uint8_t SYNTHESIZER_REGISTER_B = 0x01;
constexpr uint8_t SYNTHESIZER_REGISTER_C = 0x02;
constexpr uint8_t SYNTHESIZER_REGISTER_D = 0x03;
constexpr uint8_t VCO_SWITCHCAP_CONTROL_REGISTER = 0x04;
constexpr uint8_t DFC_CONTROL_REGISTER = 0x05;
constexpr uint8_t _6M_AUDIO_DEMODULATOR_CONTROL_REGISTER = 0x06;
constexpr uint8_t _6M5_AUDIO_DEMODULATOR_CONTROL_REGISTER = 0x07;
constexpr uint8_t RECEIVER_CONTROL_REGISTER_1 = 0x08;
constexpr uint8_t RECEIVER_CONTROL_REGISTER_2 = 0x09;
constexpr uint8_t POWER_DOWN_CONTROL_REGISTER = 0x0A;
constexpr uint8_t STATE_REGISTER = 0x0F;

#define SPI_CHILL_TIME 40

// Calculate rx5808 register hex value for given frequency in MHz
static uint16_t frequencyMHzToRegisterValue(const freq_t freqInMhz)
{
    const uint16_t tf = (freqInMhz - 479) / 2;
    const uint16_t N = tf >> 5;
    const uint16_t A = tf & 0x1F;
    return (N << 7) | A;
}

static freq_t registerValueToFrequencyMHz(uint16_t val)
{
    uint16_t N = val >> 7;
    uint16_t A = val & 0x7F;
    uint16_t tf = (N << 5) | (A & 0x1F);
    return 2 * tf + 479;
}

void RxModule::init(SPIProtocol* spi, const uint8_t selPin, const uint8_t rssiPin)
{
    this->spi = spi;
    this->selPin = selPin;
    this->rssiPin = rssiPin;
    // turn-off SPI chip select
    // set to high without glitching by using PULLUP
    pinMode(selPin, INPUT_PULLUP);
    pinMode(selPin, OUTPUT);
    digitalWrite(selPin, HIGH);
}

void RxModule::spiBegin()
{
    // wait for SPI to be ready
    mtime_t elapsed = usclock.millis() - lastSpiIOTimeMs;
    if (elapsed < SPI_CHILL_TIME) {
        usclock.delay(SPI_CHILL_TIME - elapsed);
    }
}

void RxModule::spiEnd()
{
    lastSpiIOTimeMs = usclock.millis();
}

// Reset rx5808 module to wake up from power down
void RxModule::reset()
{  
    spiBegin();
      spi->write(selPin, STATE_REGISTER, 0x00);
    spiEnd();
    powerUp();
}

// Set the frequency given on the rx5808 module
void RxModule::setFrequency(const freq_t frequency)
{
    // Get the hex value to send to the rx module
    uint16_t regVal = frequencyMHzToRegisterValue(frequency);

    spiBegin();
      //Channel data from the lookup table, 20 bytes of register data are sent, but the MSB 4 bits are zeros.
      // register address = 0x1, write, data0-15=regVal data15-19=0x0
      spi->write(selPin, SYNTHESIZER_REGISTER_B, regVal);
    spiEnd();
#ifndef USE_SPI_3WIRE_READ
    this->frequency = frequency;
#endif
}

freq_t RxModule::getFrequency()
{
#ifdef USE_SPI_3WIRE_READ
    spiBegin();
      uint32_t data = spi->read(selPin, SYNTHESIZER_REGISTER_B);
    spiEnd();
    return registerValueToFrequencyMHz((uint16_t)data);
#else
    return this->frequency;
#endif
}

// Set power options on the rx5808 module
void RxModule::setPower(const uint32_t options)
{
    spiBegin();
      spi->write(selPin, POWER_DOWN_CONTROL_REGISTER, options);
    spiEnd();
}

// Set up rx5808 module (disabling unused features to save some power)
void RxModule::powerUp()
{
    setPower(0b11010000110111110011);
    rxPoweredDown = false;
}

// Power down rx5808 module
void RxModule::powerDown()
{   
    setPower(0b11111111111111111111);
    rxPoweredDown = true;
}

// Read the RSSI value for the current channel
rssi_t RxModule::readRssi()
{
    const rssi_t value = hardware.readADC(rssiPin);
    LOG_DEBUG("ADC: ", value, DEC);
    return value;
}



/*
 * Bit-bang SPI implementation.
 */

// SPI clock speed should be about 100KHz (10us clock period)
#define SPI_DATA_CLOCK_DELAY 2
#define SPI_CLOCK_HIGH_PERIOD 5
#define SPI_CLOCK_DATA_DELAY 2

void BitBangSPI::init(uint8_t clkPin, uint8_t dataPin)
{
    SPIProtocol::init(clkPin, dataPin);
    pinMode(clkPin, OUTPUT);
    digitalWrite(clkPin, LOW);
    pinMode(dataPin, OUTPUT);
    digitalWrite(dataPin, LOW);
}

void BitBangSPI::write(const uint8_t selPin, const uint8_t addr, const uint32_t data)
{
    digitalWrite(selPin, LOW);  // Enable chip select
    writeBitBang(addr, 4);
    writeBit(true);  // Write to register
    writeBitBang(data, 20);
    digitalWrite(selPin, HIGH);  // Finished clocking data in
    digitalWrite(dataPin, LOW);  // Reset data to low
}

#ifdef USE_SPI_3WIRE_READ
uint32_t BitBangSPI::read(uint8_t selPin, uint8_t addr)
{
    digitalWrite(selPin, LOW);  // Enable chip select
    writeBitBang(addr, 4);
    writeBit(false);  // Read from register
    pinMode(dataPin, INPUT_PULLUP);
    uint32_t data = readBitBang<uint32_t>(20);
    digitalWrite(selPin, HIGH);  // Finished clocking data out
    pinMode(dataPin, OUTPUT);
    digitalWrite(dataPin, LOW);
    return data;
}
#endif

template <typename T> void BitBangSPI::writeBitBang(T bits, const uint_fast8_t size) const
{
    for (uint_fast8_t i = size; i > 0; i--)
    {
        writeBit(bits & 0x01);  // Is bit high or low?
        bits >>= 1;  // Shift bits along to check the next one
    }
}

inline void BitBangSPI::writeBit(const bool b) const
{
    digitalWrite(dataPin, b ? HIGH : LOW);
    delayMicroseconds(SPI_DATA_CLOCK_DELAY);
    digitalWrite(clkPin, HIGH);
    delayMicroseconds(SPI_CLOCK_HIGH_PERIOD);
    digitalWrite(clkPin, LOW);
    delayMicroseconds(SPI_CLOCK_DATA_DELAY);
}

#ifdef USE_SPI_3WIRE_READ
template <typename T> T BitBangSPI::readBitBang(const uint_fast8_t size) const
{
    T bits = 0;
    for (uint_fast8_t i = 0; i < size; i++)
    {
        if (readBit()) {
            bits |= (((T)1) << i);
        }
    }
    return bits;
}

inline bool BitBangSPI::readBit() const
{
    delayMicroseconds(SPI_DATA_CLOCK_DELAY);
    digitalWrite(clkPin, HIGH);
    delayMicroseconds(SPI_CLOCK_HIGH_PERIOD);
    bool bit = (digitalRead(dataPin) == HIGH);
    digitalWrite(clkPin, LOW);
    delayMicroseconds(SPI_CLOCK_DATA_DELAY);
    return bit;
}
#endif



/*
 * Native SPI implementation.
 */

#if TARGET == ESP32_TARGET
#include <SPI.h>

void NativeSPI::init(uint8_t clkPin, uint8_t dataPin)
{
    SPIProtocol::init(clkPin, dataPin);
    SPI.begin(clkPin, -1, dataPin);
}

void NativeSPI::write(const uint8_t selPin, const uint8_t addr, const uint32_t data)
{
    uint32_t payload = addr | (1 << 4) | (data << 5);
    // SPI clock speed should be 100KHz
    SPI.beginTransaction(SPISettings(100000, LSBFIRST, SPI_MODE0));
    digitalWrite(selPin, LOW);
    SPI.transferBits(payload, NULL, 25);
    digitalWrite(selPin, HIGH);
    SPI.endTransaction();
}
#endif
