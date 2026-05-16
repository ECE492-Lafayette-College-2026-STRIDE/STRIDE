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

namespace jetson {

class Jetson {
public:
    Jetson();

    bool init();
    void tick();

    bool receiveCommand();
    bool sendFeedback();

    bool isConnected(std::uint32_t timeoutMs = 500U) const;
    std::uint32_t lastReceiveTime() const;

private:
    UART_HandleTypeDef* m_huart;

    static constexpr std::uint16_t RX_BUFFER_SIZE = 128U;
    static constexpr std::uint16_t TX_BUFFER_SIZE = 256U;
    static constexpr std::uint32_t FEEDBACK_PERIOD_MS = 100U;

    char m_rxBuffer[RX_BUFFER_SIZE];
    char m_txBuffer[TX_BUFFER_SIZE];

    std::uint32_t m_lastRxTick = 0U;
    std::uint32_t m_lastTxTick = 0U;

    bool parseCommand(const char* msg);
};

} // namespace jetson

#endif /* SRC_APP_JETSON_JETSON_HPP_ */
