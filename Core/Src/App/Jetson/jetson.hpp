/*
 * Jetson.hpp
 *
 *  Created on: May 15, 2026
 *      Author: otienom
 */

#ifndef SRC_APP_JETSON_JETSON_HPP_
#define SRC_APP_JETSON_JETSON_HPP_

#include "main.h"
#include <cstdint>
#include <string>

namespace jetson {

class Jetson {
public:
    Jetson();

    bool init();
    void tick();

    /**
     * @brief Handle one byte received from the Jetson UART interrupt.
     *
     * This is called indirectly from HAL_UART_RxCpltCallback().
     *
     * @param byte Received UART byte.
     */
    void handleReceivedByte(std::uint8_t byte);

    /**
     * @brief Restart interrupt-based UART receive for the next byte.
     */
    void restartReceiveInterrupt();

    /**
     * @brief Return the latest byte received by HAL_UART_Receive_IT().
     *
     * @return Last received UART byte.
     */
    std::uint8_t receivedByte() const;

    bool sendFeedback();

    bool isConnected(std::uint32_t timeoutMs = 500U) const;
    std::uint32_t lastReceiveTime() const;

private:
    UART_HandleTypeDef* m_huart;

    static constexpr std::uint16_t RX_BUFFER_SIZE = 128U;
    static constexpr std::uint32_t FEEDBACK_PERIOD_MS = 100U;

    /*
     * UART receive uses a line buffer because the Jetson sends newline-
     * terminated CSV commands.
     *
     * Expected Jetson command:
     *
     * left_mph,right_mph,enable_left,enable_right,brake_left,brake_right,battery_full_flag,direction_left,direction_right\r\n
     *
     * Example:
     *
     * 1.200,1.200,1,1,0,0,0,0,0\r\n
     */
    char m_rxBuffer[RX_BUFFER_SIZE];
    std::uint16_t m_rxIndex = 0U;

    /*
     * Single-byte receive storage used by HAL_UART_Receive_IT().
     */
    std::uint8_t m_rxByte = 0U;

    /*
     * Set true when a complete newline-terminated command is ready.
     */
    volatile bool m_commandReady = false;

    std::uint32_t m_lastRxTick = 0U;
    std::uint32_t m_lastTxTick = 0U;

    bool processReceivedCommand();
    bool parseCommand(const char* msg);
    std::string buildFeedbackString() const;
};

} // namespace jetson

#endif /* SRC_APP_JETSON_JETSON_HPP_ */
