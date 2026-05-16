/*
 * SystemState.cpp
 *
 *  Created on: May 16, 2026
 *      Author: otienom
 */

#include "system_state.hpp"

namespace system_state {

MotorCommand SystemState::motorCommand_{};
MotorFeedback SystemState::motorFeedback_{};
BatteryState SystemState::batteryState_{};
SensorState SystemState::sensorState_{};
UserInputState SystemState::userInputState_{};
JetsonState SystemState::jetsonState_{};

void SystemState::init()
{
    motorCommand_ = MotorCommand{};
    motorFeedback_ = MotorFeedback{};
    batteryState_ = BatteryState{};
    sensorState_ = SensorState{};
    userInputState_ = UserInputState{};
    jetsonState_ = JetsonState{};
}

void SystemState::setMotorCommand(const MotorCommand& command)
{
    motorCommand_ = command;
}

MotorCommand SystemState::getMotorCommand()
{
    return motorCommand_;
}

void SystemState::setMotorFeedback(const MotorFeedback& feedback)
{
    motorFeedback_ = feedback;
}

MotorFeedback SystemState::getMotorFeedback()
{
    return motorFeedback_;
}

void SystemState::setBatteryState(const BatteryState& battery)
{
    batteryState_ = battery;
}

BatteryState SystemState::getBatteryState()
{
    return batteryState_;
}

void SystemState::setSensorState(const SensorState& sensors)
{
    sensorState_ = sensors;
}

SensorState SystemState::getSensorState()
{
    return sensorState_;
}

void SystemState::setUserInputState(const UserInputState& input)
{
    userInputState_ = input;
}

UserInputState SystemState::getUserInputState()
{
    return userInputState_;
}

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

} // namespace system_state
