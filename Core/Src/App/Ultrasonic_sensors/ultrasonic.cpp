/*
 * ultrasonic.cpp
 *
 *  Created on: May 18, 2026
 *      Author: otienom
 *
 *  Receives ultrasonic distance/status string from the sensor MCU.
 *
 *  Expected UART message:
 *  ultrasonic distance:%u,ultrasonic status:%u\r\n
 */

#include "ultrasonic.hpp"
#include "../System_State/system_state.hpp"
#include "../Jetson/run_jetson.hpp"

#include <cstdio>
#include <cstring>

using namespace system_state;

/*
 * Hard-coded UART handler for ultrasonic MCU.
 *
 * From your CubeMX/usart.c:
 *   USART1_RX = PB15 = ultrasonic_rx_Pin
 *   USART1_TX = PA9  = ultrasonic_Tx_Pin
 *
 * Therefore the ultrasonic receiver must use huart1.
 */
extern UART_HandleTypeDef huart1;

namespace {

constexpr std::uint16_t RX_LINE_BUFFER_SIZE = 128U;

/*
 * If no valid ultrasonic message arrives within this time,
 * SensorState.valid is set to false.
 */
constexpr std::uint32_t SENSOR_TIMEOUT_MS = 500U;

/*
 * One-byte interrupt receive variable.
 */
std::uint8_t rxByte = 0U;

/*
 * Buffer used to build one complete received line.
 */
char rxLineBuffer[RX_LINE_BUFFER_SIZE] = {0};
std::uint16_t rxIndex = 0U;

/*
 * Status tracking for timeout/stale detection.
 */
std::uint32_t lastRxTick = 0U;
bool hasReceivedAtLeastOnce = false;

} // anonymous namespace

void UltrasonicReceiver::init()
{
    rxByte = 0U;
    rxIndex = 0U;
    std::memset(rxLineBuffer, 0, sizeof(rxLineBuffer));

    lastRxTick = HAL_GetTick();
    hasReceivedAtLeastOnce = false;

    restartReceive();
}

void UltrasonicReceiver::restartReceive()
{
    /*
     * Receive one byte using interrupt mode.
     * When one byte arrives, HAL_UART_RxCpltCallback() will run.
     */
    HAL_UART_Receive_IT(&huart1, &rxByte, 1U);
}

void UltrasonicReceiver::rxCompleteCallback(UART_HandleTypeDef *huart)
{
    /*
     * Only process data from USART1, which is connected to the ultrasonic MCU.
     */
    if (huart->Instance != huart1.Instance)
    {
        return;
    }

    handleReceivedByte(rxByte);

    /*
     * Restart receive so the next byte can be captured.
     */
    restartReceive();
}

void UltrasonicReceiver::handleReceivedByte(std::uint8_t byte)
{
    /*
     * The sender transmits messages ending with \r\n.
     * Either \r or \n is treated as the end of the message.
     */
    if (byte == '\r' || byte == '\n')
    {
        /*
         * Ignore empty line caused by the second character in \r\n.
         */
        if (rxIndex == 0U)
        {
            return;
        }

        /*
         * Null-terminate the received line so sscanf can parse it.
         */
        rxLineBuffer[rxIndex] = '\0';

        std::uint16_t distance_mm = 0U;
        std::uint8_t status = 0U;

        if (parseLine(rxLineBuffer, &distance_mm, &status))
        {
            updateSystemState(distance_mm, status);

            lastRxTick = HAL_GetTick();
            hasReceivedAtLeastOnce = true;
        }

        /*
         * Clear buffer for next message.
         */
        rxIndex = 0U;
        std::memset(rxLineBuffer, 0, sizeof(rxLineBuffer));
        return;
    }

    /*
     * Store normal characters until the full line arrives.
     */
    if (rxIndex < (RX_LINE_BUFFER_SIZE - 1U))
    {
        rxLineBuffer[rxIndex++] = static_cast<char>(byte);
    }
    else
    {
        /*
         * Buffer overflow protection.
         * If a malformed/too-long line arrives, drop it and start over.
         */
        rxIndex = 0U;
        std::memset(rxLineBuffer, 0, sizeof(rxLineBuffer));
    }
}

bool UltrasonicReceiver::parseLine(const char *line,
                                   std::uint16_t *distance_mm,
                                   std::uint8_t *status)
{
    if (line == nullptr || distance_mm == nullptr || status == nullptr)
    {
        return false;
    }

    unsigned int parsedDistance = 0U;
    unsigned int parsedStatus = 0U;

    /*
     * Expected exact format from the sensor MCU:
     * ultrasonic distance:350,ultrasonic status:1
     */
    int fields = std::sscanf(line,
                             "ultrasonic distance:%u,ultrasonic status:%u",
                             &parsedDistance,
                             &parsedStatus);

    if (fields != 2)
    {
        return false;
    }

    if (parsedDistance > 65535U)
    {
        return false;
    }

    if (parsedStatus > 255U)
    {
        return false;
    }

    *distance_mm = static_cast<std::uint16_t>(parsedDistance);
    *status = static_cast<std::uint8_t>(parsedStatus);

    return true;
}

void UltrasonicReceiver::updateSystemState(std::uint16_t distance_mm,
                                           std::uint8_t status)
{
    SensorState sensorState;

    /*
     * Distance is stored in millimeters.
     */
    sensorState.frontDistance = static_cast<float>(distance_mm);

    /*
     * This requires SensorState to include:
     * std::uint8_t frontStatus = 0U;
     */
    sensorState.frontStatus = status;

    /*
     * A correctly parsed message means the sensor data is valid.
     */
    sensorState.valid = true;

    SystemState::setSensorState(sensorState);
}

void UltrasonicReceiver::process()
{
    /*
     * If we have never received data, do not mark invalid yet.
     */
    if (!hasReceivedAtLeastOnce)
    {
        return;
    }

    std::uint32_t now = HAL_GetTick();

    /*
     * If sensor messages stop arriving, mark the sensor state invalid.
     */
    if ((now - lastRxTick) > SENSOR_TIMEOUT_MS)
    {
        SensorState sensorState = SystemState::getSensorState();

        sensorState.valid = false;

        SystemState::setSensorState(sensorState);
    }
}

void UltrasonicReceiver::errorCallback(UART_HandleTypeDef *huart)
{
    /*
     * Only handle USART1 errors here.
     */
    if (huart->Instance != huart1.Instance)
    {
        return;
    }

    /*
     * Clear UART error flags.
     */
    __HAL_UART_CLEAR_OREFLAG(&huart1);
    __HAL_UART_CLEAR_NEFLAG(&huart1);
    __HAL_UART_CLEAR_FEFLAG(&huart1);
    __HAL_UART_CLEAR_PEFLAG(&huart1);

    /*
     * Drop any partial message after UART error.
     */
    rxIndex = 0U;
    std::memset(rxLineBuffer, 0, sizeof(rxLineBuffer));

    /*
     * Restart receive after the error is cleared.
     */
    restartReceive();
}
/*+
 * HAL UART callbacks.
 * These are called automatically by STM32 HAL.
 *
 * Put them here only if they do not already exist elsewhere in the project.
 */
extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
    if (huart->Instance == USART1) {
        UltrasonicReceiver::rxCompleteCallback(huart);
    }
    else if (huart->Instance == USART6) {
        jetson::runJetsonHandleRxComplete();
    }
}
extern "C" void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    UltrasonicReceiver::errorCallback(huart);
}
