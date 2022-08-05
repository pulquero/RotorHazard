#include "config.h"
#include "hardware.h"
#include "commands.h"

#define SERIAL_BAUD_RATE 500000

#define SERIALCOM Serial

class Esp8266Hardware : public Hardware {
public:
    Esp8266Hardware() : Hardware(HIGH,LOW) {
    }
    void init()
    {
        Hardware::init();
        spi.init(SCK, MOSI);
        initSerial(SERIALCOM, SERIAL_BAUD_RATE);
    }

    void initRxModule(uint_fast8_t nIdx, RxModule& rx)
    {
        uint8_t selPin = RX5808_SEL_PIN;  //SEL (CH2) output line to RX5808 module
        uint8_t rssiPin = RSSI_INPUT_PIN; //RSSI input from RX5808
        rx.init(&spi, selPin, rssiPin);
    }

    const char* getProcessorType() {
      return "ESP8266";
    }

    uint8_t readADC(uint8_t pin) {
        int raw = analogRead(pin);
        // rescale to fit into a byte
        return raw >> 2;
    }
};
