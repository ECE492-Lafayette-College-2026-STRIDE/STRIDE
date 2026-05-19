/*
 * Jetson.cpp
 *
 *  Created on: May 15, 2026
 *      Author: otienom
 */

#include "Jetson.hpp"
#include "../System_State/System_state.hpp"

extern "C" {
#include "usart.h"
}

#include <cstdio>
#include <cstring>
#include <string>

namespace jetson {

Jetson::Jetson()
    : m_huart(&huart6),
      m_rxIndex(0U),
      m_rxByte(0U),
      m_commandReady(false)
{
    std::memset(m_rxBuffer, 0, RX_BUFFER_SIZE);
}

bool Jetson::init()
{
    if (m_huart == nullptr) {
        return false;
    }

    std::memset(m_rxBuffer, 0, RX_BUFFER_SIZE);
    m_rxIndex = 0U;
    m_rxByte = 0U;
    m_commandReady = false;

    m_lastRxTick = HAL_GetTick();
    m_lastTxTick = HAL_GetTick();

    system_state::SystemState::setJetsonConnected(false);
    system_state::SystemState::updateJetsonRxTick(m_lastRxTick);
    system_state::SystemState::updateJetsonTxTick(m_lastTxTick);

    /*
     * Start interrupt-based receive for the first byte.
     * After every received byte, run_jetson.cpp restarts this again.
     */
    restartReceiveInterrupt();

    return true;
}

void Jetson::tick()
{
    /*
     * Non-blocking receive path:
     * UART interrupt fills the receive buffer.
     * tick() only parses when a full command line is ready.
     */
    processReceivedCommand();

    const std::uint32_t now = HAL_GetTick();

    if ((now - m_lastTxTick) >= FEEDBACK_PERIOD_MS) {
        sendFeedback();
    }

    system_state::SystemState::setJetsonConnected(isConnected());
}

void Jetson::restartReceiveInterrupt()
{
    if (m_huart == nullptr) {
        return;
    }

    /*
     * Receive exactly one byte using interrupt mode.
     * Completion calls HAL_UART_RxCpltCallback().
     */
    (void)HAL_UART_Receive_IT(m_huart, &m_rxByte, 1U);
}

std::uint8_t Jetson::receivedByte() const
{
    return m_rxByte;
}

void Jetson::handleReceivedByte(std::uint8_t byte)
{
    const char c = static_cast<char>(byte);

    /*
     * Jetson sends "\r\n".
     * Ignore '\r' and process the command when '\n' arrives.
     */
    if (c == '\r') {
        return;
    }

    if (c == '\n') {
        m_rxBuffer[m_rxIndex] = '\0';
        m_commandReady = true;
        return;
    }

    if (m_rxIndex < (RX_BUFFER_SIZE - 1U)) {
        m_rxBuffer[m_rxIndex++] = c;
    } else {
        /*
         * Overflow protection.
         */
        m_rxIndex = 0U;
        std::memset(m_rxBuffer, 0, RX_BUFFER_SIZE);
        m_commandReady = false;
    }
}

bool Jetson::processReceivedCommand()
{
    if (!m_commandReady) {
        return false;
    }

    /*
     * Copy the command out of the shared receive buffer before parsing.
     */
    char localBuffer[RX_BUFFER_SIZE];
    std::memset(localBuffer, 0, RX_BUFFER_SIZE);
    std::strncpy(localBuffer, m_rxBuffer, RX_BUFFER_SIZE - 1U);

    m_rxIndex = 0U;
    std::memset(m_rxBuffer, 0, RX_BUFFER_SIZE);
    m_commandReady = false;

    /*
     * Debug print: full command line received from Jetson.
     */
    printf("\r\n[Jetson RX Line] %s\r\n", localBuffer);

    if (!parseCommand(localBuffer)) {
        printf("[Jetson RX Parse] FAILED\r\n");
        return false;
    }

    printf("[Jetson RX Parse] OK\r\n");

    m_lastRxTick = HAL_GetTick();

    system_state::SystemState::updateJetsonRxTick(m_lastRxTick);
    system_state::SystemState::setJetsonConnected(true);

    return true;
}

bool Jetson::parseCommand(const char* msg)
{
    if (msg == nullptr) {
        return false;
    }

    float speedLeftMph = 0.0f;
    float speedRightMph = 0.0f;

    int enableLeft = 0;
    int enableRight = 0;

    int brakeLeft = 1;
    int brakeRight = 1;

    int batteryFullFlag = 0;

    int directionLeft = 0;
    int directionRight = 0;

    /*
     * Optional manual battery full command.
     */
    if (std::strcmp(msg, "BAT_FULL") == 0) {
        system_state::SystemState::requestResetCapacityToFull();
        return true;
    }

    /*
     * Format from Jetson:
     *
     * left_mph,right_mph,enable_left,enable_right,brake_left,brake_right,battery_full_flag,direction_left,direction_right
     *
     * Example:
     *
     * 1.200,1.200,1,1,0,0,0,0,0
     */
    const int parsed = std::sscanf(
        msg,
        "%f,%f,%d,%d,%d,%d,%d,%d,%d",
        &speedLeftMph,
        &speedRightMph,
        &enableLeft,
        &enableRight,
        &brakeLeft,
        &brakeRight,
        &batteryFullFlag,
        &directionLeft,
        &directionRight
    );

    if (parsed != 9) {
        return false;
    }

    system_state::MotorCommand command{};

    command.speedLeft = speedLeftMph;
    command.speedRight = speedRightMph;

    command.enableLeft = enableLeft != 0;
    command.enableRight = enableRight != 0;

    command.brakeLeft = brakeLeft != 0;
    command.brakeRight = brakeRight != 0;

    command.directionLeft = directionLeft != 0
        ? system_state::Direction::Reverse
        : system_state::Direction::Forward;

    command.directionRight = directionRight != 0
        ? system_state::Direction::Reverse
        : system_state::Direction::Forward;

    command.commandValid = true;

    /*
     * Jetson writes commands into SystemState.
     * Motors later pull these commands from SystemState.
     */
    system_state::SystemState::setMotorCommand(command);

    /*
     * Battery full flag from Jetson.
     * This only sets the request flag. The battery/flash driver handles the reset.
     */
    if (batteryFullFlag != 0) {
        system_state::SystemState::requestResetCapacityToFull();
    }

    return true;
}

std::string Jetson::buildFeedbackString() const
{
    const auto battery = system_state::SystemState::getBatteryState();
    const auto motorFeedback = system_state::SystemState::getMotorFeedback();
    const auto sensors = system_state::SystemState::getSensorState();
    const auto userInput = system_state::SystemState::getUserInputState();
    const auto controlCluster = system_state::SystemState::getControlClusterState();

    const int controlMode = static_cast<int>(controlCluster.mode);
    const int controlValid = controlCluster.valid ? 1 : 0;

    char local[192];

    /*
     * Format expected by Jetson feedback parser:
     *
     * FB,
     * batteryPercent,
     * remainingCapacity_mAh,
     * leftMotorSpeed,
     * rightMotorSpeed,
     * leftMotorFault,
     * rightMotorFault,
     * leftUserBrake,
     * rightUserBrake,
     * ultrasonicFrontDistance,
     * ultrasonicFrontStatus,
     * controlClusterMode,
     * controlClusterValid
     *
     * Example:
     *
     * FB,85.00,25500.00,0.50,0.48,0,0,0.00,0.00,350.00,1,0,1
     */
    const int len = std::snprintf(
        local,
        sizeof(local),
        "FB,%.2f,%.2f,%.2f,%.2f,%d,%d,%.2f,%.2f,%.2f,%d,%d,%d\r\n",
        battery.percent,
        battery.realtimeCapacity_mAh,
        motorFeedback.speedLeft,
        motorFeedback.speedRight,
        motorFeedback.faultLeft ? 1 : 0,
        motorFeedback.faultRight ? 1 : 0,
        userInput.leftUserBrake,
        userInput.rightUserBrake,
        sensors.frontDistance,
        static_cast<int>(sensors.frontStatus),
        controlMode,
        controlValid
    );

    if (len <= 0 || len >= static_cast<int>(sizeof(local))) {
        return std::string{};
    }

    return std::string(local);
}

bool Jetson::sendFeedback()
{
    if (m_huart == nullptr) {
        return false;
    }

    const std::string feedback = buildFeedbackString();

    if (feedback.empty()) {
        printf("[Jetson TX] Feedback string empty\r\n");
        return false;
    }

    /*
     * Debug print: exact string being sent to Jetson.
     */
    printf("[Jetson TX] %s", feedback.c_str());

    const HAL_StatusTypeDef status = HAL_UART_Transmit(
        m_huart,
        reinterpret_cast<std::uint8_t*>(const_cast<char*>(feedback.c_str())),
        static_cast<std::uint16_t>(feedback.length()),
        20U
    );

    if (status == HAL_OK) {
        m_lastTxTick = HAL_GetTick();
        system_state::SystemState::updateJetsonTxTick(m_lastTxTick);

        printf("[Jetson TX Status] OK\r\n");

        return true;
    }

    printf("[Jetson TX Status] FAILED = %d\r\n", static_cast<int>(status));

    return false;
}

bool Jetson::isConnected(std::uint32_t timeoutMs) const
{
    return (HAL_GetTick() - m_lastRxTick) <= timeoutMs;
}

std::uint32_t Jetson::lastReceiveTime() const
{
    return m_lastRxTick;
}

} // namespace jetson
