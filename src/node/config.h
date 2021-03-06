#ifndef config_h
#define config_h

#if !defined(__TEST__) && (defined(_WIN32) || defined(__linux__))
#include "sil/arduino.h"
#else
#include <Arduino.h>
#endif
#include "util/rhtypes.h"

#define AVR_TARGET 1
#define STM32_TARGET 2
#define ESP32_TARGET 3
#define SIL_TARGET 5
#define TEST_TARGET 0

#if defined(STM32_CORE_VERSION)
#define TARGET STM32_TARGET
#define STM32F1_VARIANT 1
#define STM32F4_VARIANT 4
#ifdef STM32F4
#define VARIANT STM32F4_VARIANT
#else
#define VARIANT STM32F1_VARIANT
#endif
#elif defined(ESP_PLATFORM)
#define TARGET ESP32_TARGET
#elif defined(__TEST__)
#define TARGET TEST_TARGET
#elif defined(_WIN32) || defined(__linux__)
#define TARGET SIL_TARGET
#else
#define TARGET AVR_TARGET
#endif

#if TARGET == AVR_TARGET

// Set to 1-8 for manual selection of Arduino node ID/address.
// Set to 0 for automatic selection via hardware pin.
// See https://github.com/RotorHazard/RotorHazard/wiki/Specification:-Node-hardware-addressing
#define NODE_NUMBER 0

#endif

// ******************************************************************** //

#if TARGET == STM32_TARGET
    // currently, STM32F1 is not fast enough to support more than 4 nodes at 1ms loop time
    #define MULTI_RHNODE_MAX 4
    // ensure U(S)ART support is "Enabled (generic Serial)"
    // use USB support "None" (or "CDC (no generic Serial)") to use serial over UART pins
    // use USB support "CDC (generic Serial)" to use serial over USB port
    // i.e. -DUSBCON -DUSBD_USE_CDC
#elif TARGET == ESP32_TARGET
    #define MULTI_RHNODE_MAX 6
#else
    #ifndef MULTI_RHNODE_MAX
        // Set greater than 1 to support multiple freqs per node
        #define MULTI_RHNODE_MAX 1
    #endif
#endif  // STM32_MODE_FLAG

// multi-freq reads
#define READS_PER_FREQ 64

#if TARGET == AVR_TARGET
#include <util/atomic.h>
#else
#define ATOMIC_BLOCK(x)
#define ATOMIC_RESTORESTATE
#endif

#if TARGET == AVR_TARGET
    #define RH_WIRING 0
    #define ARDUVIDRX_WIRING 1
    #define CHORUS_WIRING 2

    /*
       Set to RH_WIRING for standard RotorHazard USB node wiring;
       set to ARDUVIDRX_WIRING for ArduVidRx USB node wiring
       See here for an ArduVidRx example: http://www.etheli.com/ArduVidRx/hw/index.html#promini
     */
    #define WIRING_TYPE RH_WIRING

    #if WIRING_TYPE == RH_WIRING
        #define RX5808_DATA_PIN 11             //DATA output line to RX5808 module
        #define RX5808_SEL_PIN 10              //CLK output line to RX5808 module
        #define RX5808_CLK_PIN 13              //SEL output line to RX5808 module
        #define RSSI_INPUT_PIN A0              //RSSI input from RX5808
        #define NODE_RESET_PIN 12              //Pin to reset paired Arduino via command for ISP
    #elif WIRING_TYPE == ARDUVIDRX_WIRING
        #define RX5808_DATA_PIN 10             //DATA output line to RX5808 module
        #define RX5808_SEL_PIN 11              //CLK output line to RX5808 module
        #define RX5808_CLK_PIN 12              //SEL output line to RX5808 module
        #define RSSI_INPUT_PIN A7              //RSSI input from RX5808
        #define NODE_RESET_PIN A1              //Pin to reset paired Arduino via command for ISP
    #elif WIRING_TYPE == CHORUS_WIRING
        #define RX5808_DATA_PIN 10             //DATA output line to RX5808 module
        #define RX5808_SEL_PIN 11              //CLK output line to RX5808 module
        #define RX5808_CLK_PIN 12              //SEL output line to RX5808 module
        #define RSSI_INPUT_PIN A3              //RSSI input from RX5808
        #define NODE_RESET_PIN A1              //Pin to reset paired Arduino via command for ISP
    #endif

    #define DISABLE_SERIAL_PIN 9  //pull pin low (to GND) to disable serial port
    #define HARDWARE_SELECT_PIN_1 2
    #define HARDWARE_SELECT_PIN_2 3
    #define HARDWARE_SELECT_PIN_3 4
    #define LEGACY_HARDWARE_SELECT_PIN_1 4
    #define LEGACY_HARDWARE_SELECT_PIN_2 5
    #define LEGACY_HARDWARE_SELECT_PIN_3 6
    #define LEGACY_HARDWARE_SELECT_PIN_4 7
    #define LEGACY_HARDWARE_SELECT_PIN_5 8

#endif

// use persistent homology detection
#define USE_PH

#if TARGET == ESP32_TARGET
#define RX_IMPL NativeRxModule
#else
#define RX_IMPL BitBangRxModule
#endif

#if TARGET != AVR_TARGET || MULTI_RHNODE_MAX == 1
#define SCAN_HISTORY
#else
// uncomment to activate scanner mode
//#define SCAN_HISTORY
#endif
// uncomment to activate raw mode
//#define RSSI_HISTORY

#ifdef __TEST__
#undef USE_PH
#define SCAN_HISTORY
#define RSSI_HISTORY
#endif

#if TARGET == ESP32_TARGET
    #define RX5808_SEL0_PIN 16
    #define RX5808_SEL1_PIN 5
    #define RX5808_SEL2_PIN 4
    #define RX5808_SEL3_PIN 15
    #define RX5808_SEL4_PIN 25
    #define RX5808_SEL5_PIN 26

    #define RSSI_INPUT0_PIN A0
    #define RSSI_INPUT1_PIN A3
    #define RSSI_INPUT2_PIN A6
    #define RSSI_INPUT3_PIN A7
    #define RSSI_INPUT4_PIN A4
    #define RSSI_INPUT5_PIN A5
#elif TARGET == STM32_TARGET
    #define RX5808_SEL0_PIN PB6
    #define RX5808_SEL1_PIN PB7
    #define RX5808_SEL2_PIN PB8
    #define RX5808_SEL3_PIN PB9
    #define RX5808_SEL4_PIN PB12
    #define RX5808_SEL5_PIN PB13
    #define RX5808_SEL6_PIN PB14
    #define RX5808_SEL7_PIN PB15

    #if VARIANT == STM32F1_VARIANT
    #define RSSI_INPUT0_PIN A0
    #define RSSI_INPUT1_PIN A1
    #define RSSI_INPUT2_PIN A2
    #define RSSI_INPUT3_PIN A3
    #define RSSI_INPUT4_PIN A4
    #define RSSI_INPUT5_PIN A5
    #define RSSI_INPUT6_PIN A6
    #define RSSI_INPUT7_PIN A7
    #elif VARIANT == STM32F4_VARIANT
    #define RSSI_INPUT0_PIN PB1
    #define RSSI_INPUT1_PIN A0
    #define RSSI_INPUT2_PIN A1
    #define RSSI_INPUT3_PIN A2
    #define RSSI_INPUT4_PIN A3
    #define RSSI_INPUT5_PIN A4
    #define RSSI_INPUT6_PIN A5
    #define RSSI_INPUT7_PIN A6
    #endif
#endif

#endif  // config_h
