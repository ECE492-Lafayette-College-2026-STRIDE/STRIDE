/*
 * System_state.cpp
 *
 *  Created on: May 16, 2026
 *      Author: otienom
 */

#include "System_state.hpp"

namespace system_state {

MotorCommand SystemState::motorCommand_{};
MotorFeedback SystemState::motorFeedback_{};
BatteryState SystemState::batteryState_{};
SensorState SystemState::sensorState_{};
UserInputState SystemState::userInputState_{};
JetsonState SystemState::jetsonState_{};
ControlClusterState SystemState::controlClusterState_{};

void SystemState::init()
{
    motorCommand_ = MotorCommand{};
    motorFeedback_ = MotorFeedback{};
    batteryState_ = BatteryState{};
    sensorState_ = SensorState{};
    userInputState_ = UserInputState{};
    jetsonState_ = JetsonState{};
    controlClusterState_ = ControlClusterState{};
}

// ====================== Jetson command updates ======================

void SystemState::setMotorCommand(const MotorCommand& command)
{
    motorCommand_ = command;
}

MotorCommand SystemState::getMotorCommand()
{
    return motorCommand_;
}

void SystemState::setMotorCommandValid(bool valid)
{
    motorCommand_.commandValid = valid;
}

// ====================== Motor driver updates ======================

void SystemState::setMotorFeedback(const MotorFeedback& feedback)
{
    motorFeedback_ = feedback;
}

MotorFeedback SystemState::getMotorFeedback()
{
    return motorFeedback_;
}

void SystemState::setLeftMotorSpeed(float speedMph)
{
    motorFeedback_.speedLeft = speedMph;
}

void SystemState::setRightMotorSpeed(float speedMph)
{
    motorFeedback_.speedRight = speedMph;
}

void SystemState::setLeftMotorFault(bool fault)
{
    motorFeedback_.faultLeft = fault;
}

void SystemState::setRightMotorFault(bool fault)
{
    motorFeedback_.faultRight = fault;
}

// ====================== Battery monitor updates ======================

void SystemState::setBatteryState(const BatteryState& battery)
{
    batteryState_ = battery;
}

BatteryState SystemState::getBatteryState()
{
    return batteryState_;
}

void SystemState::requestResetCapacityToFull()
{
    batteryState_.resetCapacityToFullRequest = true;
}

void SystemState::clearResetCapacityToFullRequest()
{
    batteryState_.resetCapacityToFullRequest = false;
}

// ====================== Sensor driver updates ======================

void SystemState::setSensorState(const SensorState& sensors)
{
    sensorState_ = sensors;
}

SensorState SystemState::getSensorState()
{
    return sensorState_;
}

// ====================== User input updates ======================

void SystemState::setUserInputState(const UserInputState& input)
{
    userInputState_ = input;
}

UserInputState SystemState::getUserInputState()
{
    return userInputState_;
}

// ====================== Jetson link status ======================

void SystemState::setJetsonState(const JetsonState& jetson)
{
    jetsonState_ = jetson;
}

JetsonState SystemState::getJetsonState()
{
    return jetsonState_;
}

void SystemState::updateJetsonRxTick(std::uint32_t tick)
{
    jetsonState_.lastRxTick = tick;
}

void SystemState::updateJetsonTxTick(std::uint32_t tick)
{
    jetsonState_.lastTxTick = tick;
}

void SystemState::setJetsonConnected(bool connected)
{
    jetsonState_.connected = connected;
}

// ====================== Control cluster updates ======================

void SystemState::setControlClusterState(const ControlClusterState& control)
{
    controlClusterState_ = control;
}

ControlClusterState SystemState::getControlClusterState()
{
    return controlClusterState_;
}

} // namespace system_state
