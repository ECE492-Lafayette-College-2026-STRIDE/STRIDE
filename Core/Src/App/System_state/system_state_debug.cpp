/*
 * System_state_debug.cpp
 *
 *  Created on: May 19, 2026
 *      Author: otienom
 *
 *  Purpose:
 *  Print SystemState values to the COM1 serial terminal.
 */

#include "System_state_debug.hpp"
#include "System_state.hpp"
#include "main.h"

#include <cstdio>

namespace system_state_debug {

static std::uint32_t lastPrintTick_ = 0U;

static const char* directionToString(system_state::Direction direction)
{
    return (direction == system_state::Direction::Reverse) ? "Reverse" : "Forward";
}

static const char* controlModeToString(system_state::ControlMode mode)
{
    switch (mode) {
        case system_state::ControlMode::Standard:
            return "Standard";

        case system_state::ControlMode::Caregiver:
            return "Caregiver";

        case system_state::ControlMode::StairClimbing:
            return "StairClimbing";

        default:
            return "Unknown";
    }
}

void init()
{
    lastPrintTick_ = HAL_GetTick();
}

void tick(std::uint32_t periodMs)
{
    const std::uint32_t now = HAL_GetTick();

    if ((now - lastPrintTick_) >= periodMs) {
        lastPrintTick_ = now;
        printOnce();
    }
}

void printOnce()
{
    const auto motorCommand = system_state::SystemState::getMotorCommand();
    const auto motorFeedback = system_state::SystemState::getMotorFeedback();
    const auto battery = system_state::SystemState::getBatteryState();
    const auto sensors = system_state::SystemState::getSensorState();
    const auto userInput = system_state::SystemState::getUserInputState();
    const auto jetson = system_state::SystemState::getJetsonState();
    const auto control = system_state::SystemState::getControlClusterState();

    printf("\r\n========== SYSTEM STATE ==========\r\n");

    printf("[Motor Command]\r\n");
    printf("Valid: %d\r\n", motorCommand.commandValid ? 1 : 0);
    printf("Left:  enable=%d brake=%d dir=%s speed=%.2f mph\r\n",
           motorCommand.enableLeft ? 1 : 0,
           motorCommand.brakeLeft ? 1 : 0,
           directionToString(motorCommand.directionLeft),
           motorCommand.speedLeft);

    printf("Right: enable=%d brake=%d dir=%s speed=%.2f mph\r\n",
           motorCommand.enableRight ? 1 : 0,
           motorCommand.brakeRight ? 1 : 0,
           directionToString(motorCommand.directionRight),
           motorCommand.speedRight);

    printf("\r\n[Motor Feedback]\r\n");
    printf("Left:  measuredSpeed=%.2f mph fault=%d\r\n",
           motorFeedback.speedLeft,
           motorFeedback.faultLeft ? 1 : 0);

    printf("Right: measuredSpeed=%.2f mph fault=%d\r\n",
           motorFeedback.speedRight,
           motorFeedback.faultRight ? 1 : 0);

    printf("\r\n[Battery]\r\n");
    printf("Voltage: %.2f V\r\n", battery.voltage);
    printf("Current: %.2f A\r\n", battery.current);
    printf("Power: %.2f W\r\n", battery.power);
    printf("Realtime Capacity: %.2f mAh\r\n", battery.realtimeCapacity_mAh);
    printf("Flash Capacity: %.2f mAh\r\n", battery.flashCapacity_mAh);
    printf("Percent: %.2f %%\r\n", battery.percent);
    printf("Reset To Full Request: %d\r\n", battery.resetCapacityToFullRequest ? 1 : 0);
    printf("Valid: %d\r\n", battery.valid ? 1 : 0);

    printf("\r\n[Sensors]\r\n");
    printf("Front Distance: %.2f mm\r\n", sensors.frontDistance);
    printf("Front Status: %u\r\n", static_cast<unsigned int>(sensors.frontStatus));
    printf("Valid: %d\r\n", sensors.valid ? 1 : 0);

    printf("\r\n[User Input]\r\n");
    printf("Left Brake: %.2f\r\n", userInput.leftUserBrake);
    printf("Right Brake: %.2f\r\n", userInput.rightUserBrake);
    printf("Combined Brake: %.2f\r\n", userInput.combinedUserBrake);
    printf("Emergency Stop: %d\r\n", userInput.emergencyStop ? 1 : 0);
    printf("Valid: %d\r\n", userInput.valid ? 1 : 0);

    printf("\r\n[Jetson]\r\n");
    printf("Connected: %d\r\n", jetson.connected ? 1 : 0);
    printf("Last RX Tick: %lu\r\n", static_cast<unsigned long>(jetson.lastRxTick));
    printf("Last TX Tick: %lu\r\n", static_cast<unsigned long>(jetson.lastTxTick));

    printf("\r\n[Control Cluster]\r\n");
    printf("Mode: %s\r\n", controlModeToString(control.mode));
    printf("Stair Confirm: %d\r\n", control.stairConfirm ? 1 : 0);
    printf("Reset Request: %d\r\n", control.resetRequest ? 1 : 0);
    printf("Valid: %d\r\n", control.valid ? 1 : 0);

    printf("==================================\r\n");
}

} // namespace system_state_debug
