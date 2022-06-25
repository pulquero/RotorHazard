#include "config.h"
#include "commands.h"
#include "hardware.h"

#ifdef USE_I2C
#include <Wire.h>

static void i2cOnReceive(int byteCount);
static void i2cOnRequest();

static volatile Message incomingMessage;

void i2cInit(uint8_t i2cAddress, bool reset) {
    hardware.setStatusLed(true);
    if (reset) {
        Wire.end();  // release I2C pins (SDA & SCL), in case they are "stuck"
        delay(250);  //  to help bus reset and show longer LED flash
    }
    hardware.setStatusLed(false);

    Wire.setWireTimeout();
    Wire.onReceive(i2cOnReceive);
    Wire.onRequest(i2cOnRequest);
    Wire.begin(i2cAddress);

    TWAR = (i2cAddress << 1) | 1;  // enable broadcasts to be received
}

/**
 * Processes payloads of write commands in the main loop
 */
void i2cEventRun() {
    if (incomingMessage.command != INVALID_COMMAND) {
        Message msg;
        // copy any incoming message for later processing
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            if (incomingMessage.isWriteCommand()) {
                msg.command = incomingMessage.command;
                const int size = incomingMessage.buffer.size;
                for (int_fast8_t i = size - 1; i >= 0; i--) {
                    msg.buffer.data[i] = incomingMessage.buffer.data[i];
                }
                msg.buffer.size = size;
                incomingMessage.command = INVALID_COMMAND;
            }
        }
        if (msg.isWriteCommand()) {
            const uint8_t expectedBufferSize = msg.getPayloadSize() + 1;
            if (msg.buffer.size == expectedBufferSize) {
                msg.buffer.index = expectedBufferSize;
                validateAndProcessWriteCommand(msg, I2C_SOURCE);
            }
        }
    }
}

/**
 * ISR for receiving I2C data.
 */
void i2cOnReceive(int bytesAvailable) {
    if (bytesAvailable > 0 && bytesAvailable <= incomingMessage.buffer.capacity()) {
        incomingMessage.command = Wire.read();
        const int size = bytesAvailable - 1;
        for (int_fast8_t i = 0; i < size; i++) {
            incomingMessage.buffer.data[i] = Wire.read();
        }
        incomingMessage.buffer.size = size;
    }
}

/**
 * ISR for sending I2C data.
 */
void i2cOnRequest() {
    const Command cmd = incomingMessage.command;
    if (isReadCommand(cmd)) {
        Message msg;
        msg.command = cmd;
        msg.handleReadCommand(I2C_SOURCE);
        sendReadCommandResponse(Wire, msg, I2C_SOURCE);
    }
}

#endif
