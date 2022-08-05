#ifndef commands_h
#define commands_h

#if TARGET != SIL_TARGET
#include <Stream.h>
#endif
#include "io.h"
#include "rssirx.h"
#include "hardware.h"

// API level for node; increment when commands are modified
constexpr uint16_t NODE_API_LEVEL = 37;

#ifdef __TEST__
// unit tests dont supporting send 0/null
constexpr freq_t POWER_OFF_FREQ = 1111;
#else
constexpr freq_t POWER_OFF_FREQ = 0;  // frequency value to power down rx module
#endif
constexpr freq_t MIN_FREQ = 5645;
constexpr freq_t MAX_FREQ = 5945;
constexpr freq_t ERR_FREQ = 0xFFFF;

enum CommSource
{
    I2C_SOURCE = 1,
    SERIAL_SOURCE = 2,
    WIFI_SOURCE = 3
};

enum Command: uint8_t
{
    __READ_COMMANDS__ = 0x00,  // reserved
    READ_ADDRESS = 0x01,
    READ_MODE = 0x02,
    READ_FREQUENCY = 0x03,
    READ_RSSI = 0x04,
    READ_TIME_MILLIS = 0x05,     // read current 'millis()' value
    READ_MULTINODE_COUNT = 0x06, // read # of nodes handled by this processor
    READ_CURNODE_INDEX = 0x07,   // read index of current node for this processor
    READ_LAP_STATS = 0x08,
    READ_ENTER_STATS = 0x09,
    READ_EXIT_STATS = 0x10,
    READ_ENTER_AT_LEVEL = 0x11,
    READ_EXIT_AT_LEVEL = 0x12,
    READ_RHFEAT_FLAGS = 0x19,    // read feature flags value
    READ_REVISION_CODE = 0x1A,   // read NODE_API_LEVEL and verification value
    READ_RSSI_STATS = 0x20,
    READ_ANALYTICS = 0x21,
    READ_RSSI_HISTORY = 0x22,
    READ_SCAN_HISTORY = 0x23,
    READ_ADC = 0x34,
    READ_FW_VERSION = 0x3C,      // read firmware version string
    READ_FW_BUILDDATE = 0x3D,    // read firmware build date string
    READ_FW_BUILDTIME = 0x3E,    // read firmware build time string
    READ_FW_PROCTYPE = 0x3F,     // read node processor type

    __WRITE_COMMANDS__ = 0x40,  // reserved
    WRITE_MODE = 0x42,
    WRITE_FREQUENCY = 0x43,
    WRITE_CURNODE_INDEX = 0x47,  // write index of current node for this processor
    WRITE_ENTER_AT_LEVEL = 0x51,
    WRITE_EXIT_AT_LEVEL = 0x52,

    SEND_STATUS_MESSAGE = 0x75,  // send status message from server to node
    FORCE_END_CROSSING = 0x78,   // kill current crossing flag regardless of RSSI value
    RESET_PAIRED_NODE = 0x79,    // command to reset node for ISP
    JUMP_TO_BOOTLOADER = 0x7E,   // jump to bootloader for flash update
    INVALID_COMMAND = 0xFF
};

// maximum possible message size is limited to 32 bytes by I2C
constexpr uint8_t MESSAGE_BUFFER_SIZE = 18;
// maximum possible text size is MESSAGE_BUFFER_SIZE-2 (command byte + checksum byte)
// text is inclusive of a null-terminator
constexpr uint8_t TEXT_SIZE = 16;

constexpr bool isWriteCommand(uint8_t cmd) { return cmd > __WRITE_COMMANDS__ && cmd < INVALID_COMMAND; };
constexpr bool isReadCommand(uint8_t cmd)  { return cmd > __READ_COMMANDS__ && cmd < __WRITE_COMMANDS__; };

class Message
{
private:
    void handleReadRssiHistory(RssiNode& rssiNode);
    void handleReadScanHistory(RssiNode& rssiNode);
    void setMode(Mode mode) const;
public:
    Command command = INVALID_COMMAND;  // code to identify messages
    Buffer<MESSAGE_BUFFER_SIZE,TEXT_SIZE> buffer;  // request/response payload

    inline bool isWriteCommand() const { return ::isWriteCommand(command); };
    inline bool isReadCommand() const { return ::isReadCommand(command); };
    int_fast8_t getPayloadSize() const;
    void handleWriteCommand(CommSource src);
    void handleReadCommand(CommSource src);
};

void setFrequency(RssiNode& rssiNode, RxModule& rx, freq_t freq);
void handleStreamEvent(Stream& stream, Message& msg, CommSource src);
void sendReadCommandResponse(Stream& stream, Message& msg, CommSource src);
void validateAndProcessWriteCommand(Message& msg, CommSource src);

enum StatusFlag: uint8_t
{
    NO_STATUS       = 0x0,
    COMM_ACTIVITY   = 0x1,
    POLLING         = 0x2,
    SERIAL_CMD_MSG  = 0x4
};

extern uint8_t volatile cmdStatusFlags;
extern uint_fast8_t volatile cmdRssiNodeIndex;

#endif
