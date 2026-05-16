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

namespace jetson {

Jetson::Jetson()
    : m_huart(&huart4)
{
    std::memset(m_rxBuffer, 0, RX_BUFFER_SIZE);
    std::memset(m_txBuffer, 0, TX_BUFFER_SIZE);
}

bool Jetson::init()
{
    if (m_huart == nullptr) {
        return false;
    }

    std::memset(m_rxBuffer, 0, RX_BUFFER_SIZE);
    std::memset(m_txBuffer, 0, TX_BUFFER_SIZE);

    m_lastRxTick = HAL_GetTick();
    m_lastTxTick = HAL_GetTick();

    system_state::SystemState::setJetsonConnected(false);
    system_state::SystemState::updateJetsonRxTick(m_lastRxTick);
    system_state::SystemState::updateJetsonTxTick(m_lastTxTick);

    return true;
}

void Jetson::tick()
{
    receiveCommand();

    const std::uint32_t now = HAL_GetTick();

    if ((now - m_lastTxTick) >= FEEDBACK_PERIOD_MS) {
        sendFeedback();
    }

    system_state::SystemState::setJetsonConnected(isConnected());
}

bool Jetson::receiveCommand()
{
    if (m_huart == nullptr) {
        return false;
    }

    std::memset(m_rxBuffer, 0, RX_BUFFER_SIZE);

    HAL_StatusTypeDef status = HAL_UART_Receive(
        m_huart,
        reinterpret_cast<std::uint8_t*>(m_rxBuffer),
        RX_BUFFER_SIZE - 1U,
        1U
    );

    if (status != HAL_OK) {
        return false;
    }

    m_rxBuffer[RX_BUFFER_SIZE - 1U] = '\0';

    if (!parseCommand(m_rxBuffer)) {
        return false;
    }

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

    float speedLeft = 0.0f;
    float speedRight = 0.0f;

    int enableLeft = 0;
    int enableRight = 0;

    int brakeLeft = 1;
    int brakeRight = 1;

    int directionLeft = 0;
    int directionRight = 0;

    /*
     * Expected Jetson command:
     *
     * CMD,speedLeft,speedRight,enableLeft,enableRight,brakeLeft,brakeRight,directionLeft,directionRight
     *
     * Example:
     * CMD,1.0,1.0,1,1,0,0,0,0
     */
    if (std::strcmp(msg, "BAT_FULL") == 0) {
        auto battery = system_state::SystemState::getBatteryState();
        battery.resetCapacityToFullRequest = true;
        system_state::SystemState::setBatteryState(battery);
        return true;
    }
    const int parsed = std::sscanf(
        msg,
        "CMD,%f,%f,%d,%d,%d,%d,%d,%d",
        &speedLeft,
        &speedRight,
        &enableLeft,
        &enableRight,
        &brakeLeft,
        &brakeRight,
        &directionLeft,
        &directionRight
    );

    if (parsed != 8) {
        return false;
    }

    system_state::MotorCommand command{};

    command.speedLeft = speedLeft;
    command.speedRight = speedRight;

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

    system_state::SystemState::setMotorCommand(command);

    return true;
}

bool Jetson::sendFeedback()
{
    if (m_huart == nullptr) {
        return false;
    }

    const auto battery = system_state::SystemState::getBatteryState();
    const auto motorFeedback = system_state::SystemState::getMotorFeedback();
    const auto sensors = system_state::SystemState::getSensorState();
    const auto userInput = system_state::SystemState::getUserInputState();
    const auto jetsonState = system_state::SystemState::getJetsonState();

    std::memset(m_txBuffer, 0, TX_BUFFER_SIZE);

    /*
     * Feedback format:
     *
     * FB,battV,battI,battP,battPercent,
     * realtimeCapacity_mAh,flashCapacity_mAh,
     * leftSpeed,rightSpeed,leftFault,rightFault,
     * leftBrake,rightBrake,userBrake,
     * frontDist,rearDist,obstacle,jetsonConnected
     */

    const int len = std::snprintf(
        m_txBuffer,
        TX_BUFFER_SIZE,
        "FB,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d\r\n",
        battery.voltage,
        battery.current,
        battery.power,
        battery.percent,
        battery.realtimeCapacity_mAh,
        battery.flashCapacity_mAh,
        motorFeedback.speedLeft,
        motorFeedback.speedRight,
        motorFeedback.faultLeft ? 1 : 0,
        motorFeedback.faultRight ? 1 : 0,
        userInput.leftUserBrake,
        userInput.rightUserBrake,
        userInput.combinedUserBrake,
        sensors.frontDistance,
        sensors.rearDistance,
        sensors.obstacleDetected ? 1 : 0,
        jetsonState.connected ? 1 : 0
    );

    if (len <= 0 || len >= TX_BUFFER_SIZE) {
        return false;
    }

    const HAL_StatusTypeDef status = HAL_UART_Transmit(
        m_huart,
        reinterpret_cast<std::uint8_t*>(m_txBuffer),
        static_cast<std::uint16_t>(len),
        20U
    );

    if (status == HAL_OK) {
        m_lastTxTick = HAL_GetTick();
        system_state::SystemState::updateJetsonTxTick(m_lastTxTick);
        return true;
    }

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
