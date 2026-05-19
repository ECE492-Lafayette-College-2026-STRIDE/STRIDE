/*
 * run_jetson.cpp
 *
 *  Created on: May 19, 2026
 *      Author: otienom
 *
 *  Purpose:
 *  Runner layer for Jetson communication.
 *
 *  This file does not directly control motors, sensors, or battery.
 *  It only runs the Jetson driver so it can:
 *  - Receive UART commands from Jetson
 *  - Push motor commands and battery reset requests to SystemState
 *  - Pull feedback from SystemState
 *  - Send feedback to Jetson
 */

#include "run_jetson.hpp"
#include "Jetson.hpp"
#include <cstdio>
#include <cstdint>

namespace jetson {

static Jetson jetson_;
static bool initialized_ = false;

static volatile std::uint32_t usart6RxCompleteCount_ = 0U;
static std::uint32_t lastDebugPrintMs_ = 0U;

/*
 * Debug RX byte buffer.
 *
 * Bytes are pushed inside the UART RX complete callback path,
 * then printed later from runJetsonTick().
 *
 * This avoids calling printf() directly inside the UART interrupt.
 */
static constexpr std::uint16_t RX_DEBUG_BUFFER_SIZE = 256U;

static volatile std::uint8_t rxDebugBuffer_[RX_DEBUG_BUFFER_SIZE];
static volatile std::uint16_t rxDebugWriteIndex_ = 0U;
static volatile std::uint16_t rxDebugReadIndex_ = 0U;

static void pushRxDebugByte(std::uint8_t byte)
{
    const std::uint16_t nextWrite =
        static_cast<std::uint16_t>((rxDebugWriteIndex_ + 1U) % RX_DEBUG_BUFFER_SIZE);

    /*
     * If the debug buffer is full, drop the oldest byte.
     */
    if (nextWrite == rxDebugReadIndex_) {
        rxDebugReadIndex_ =
            static_cast<std::uint16_t>((rxDebugReadIndex_ + 1U) % RX_DEBUG_BUFFER_SIZE);
    }

    rxDebugBuffer_[rxDebugWriteIndex_] = byte;
    rxDebugWriteIndex_ = nextWrite;
}

static bool popRxDebugByte(std::uint8_t* byte)
{
    if (byte == nullptr) {
        return false;
    }

    if (rxDebugReadIndex_ == rxDebugWriteIndex_) {
        return false;
    }

    *byte = rxDebugBuffer_[rxDebugReadIndex_];

    rxDebugReadIndex_ =
        static_cast<std::uint16_t>((rxDebugReadIndex_ + 1U) % RX_DEBUG_BUFFER_SIZE);

    return true;
}

static void printRxDebugBytes()
{
    std::uint8_t byte = 0U;

    while (popRxDebugByte(&byte)) {
        if (byte == '\r') {
            printf("[Jetson RX Byte] 0x%02X '\\r'\r\n", byte);
        }
        else if (byte == '\n') {
            printf("[Jetson RX Byte] 0x%02X '\\n'\r\n", byte);
        }
        else if (byte >= 32U && byte <= 126U) {
            printf("[Jetson RX Byte] 0x%02X '%c'\r\n", byte, static_cast<char>(byte));
        }
        else {
            printf("[Jetson RX Byte] 0x%02X\r\n", byte);
        }
    }
}

void runJetsonInit()
{
    initialized_ = jetson_.init();
}

void runJetsonTick()
{
    if (!initialized_) {
        return;
    }

    /*
     * Print received USART6 bytes captured from the interrupt path.
     */
    printRxDebugBytes();

    /*
     * Process full command lines and send feedback.
     */
    jetson_.tick();

    /*
     * Temporary debug print.
     * This tells us whether USART6 RX interrupts are firing.
     */
    const std::uint32_t now = HAL_GetTick();

    if ((now - lastDebugPrintMs_) >= 1000U) {
        lastDebugPrintMs_ = now;

        printf("[Jetson Debug] USART6 RX complete count = %lu\r\n",
               static_cast<unsigned long>(usart6RxCompleteCount_));
    }
}

void runJetsonHandleRxComplete()
{
    if (!initialized_) {
        return;
    }

    usart6RxCompleteCount_++;

    const std::uint8_t byte = jetson_.receivedByte();

    /*
     * Store the raw received byte for debug printing.
     */
    pushRxDebugByte(byte);

    /*
     * Send the byte into the Jetson line parser.
     */
    jetson_.handleReceivedByte(byte);

    /*
     * Restart interrupt reception for the next byte.
     */
    jetson_.restartReceiveInterrupt();
}

} // namespace jetson
