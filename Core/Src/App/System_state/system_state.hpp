/*
 * SystemState.hpp
 *
 *  Created on: May 16, 2026
 *      Author: otienom
 */

#ifndef SRC_APP_SYSTEM_STATE_SYSTEMSTATE_HPP_
#define SRC_APP_SYSTEM_STATE_SYSTEMSTATE_HPP_

#include <cstdint>

namespace system_state {

enum class Direction : std::uint8_t {
    Forward = 0,
    Reverse = 1
};

struct MotorCommand {
    bool enableLeft = false;
    bool enableRight = false;

    bool brakeLeft = true;
    bool brakeRight = true;

    Direction directionLeft = Direction::Forward;
    Direction directionRight = Direction::Forward;

    float speedLeft = 0.0f;
    float speedRight = 0.0f;

    bool commandValid = false;
};

struct MotorFeedback {
    float speedLeft = 0.0f;
    float speedRight = 0.0f;

    bool faultLeft = false;
    bool faultRight = false;
};

struct BatteryState {
    float voltage = 0.0f;
    float current = 0.0f;
    float power = 0.0f;

    float realtimeCapacity_mAh = 0.0f;
    float flashCapacity_mAh = 0.0f;

    float percent = 0.0f;

    bool resetCapacityToFullRequest = false;
    bool valid = false;
};
struct SensorState {
    float frontDistance = 0.0f;
    float rearDistance = 0.0f;

    bool obstacleDetected = false;
    bool valid = false;
};

struct UserInputState {
    float leftUserBrake = 0.0f;
    float rightUserBrake = 0.0f;
    float combinedUserBrake = 0.0f;

    bool emergencyStop = false;
    bool valid = false;
};

struct JetsonState {
    bool connected = false;
    std::uint32_t lastRxTick = 0;
    std::uint32_t lastTxTick = 0;
};

class SystemState {
public:
    static void init();

    // Jetson command updates
    static void setMotorCommand(const MotorCommand& command);
    static MotorCommand getMotorCommand();

    // Motor driver updates
    static void setMotorFeedback(const MotorFeedback& feedback);
    static MotorFeedback getMotorFeedback();

    // Battery monitor updates
    static void setBatteryState(const BatteryState& battery);
    static BatteryState getBatteryState();

    // Sensor driver updates
    static void setSensorState(const SensorState& sensors);
    static SensorState getSensorState();

    // Control cluster / user input updates
    static void setUserInputState(const UserInputState& input);
    static UserInputState getUserInputState();

    // Jetson link status
    static void setJetsonState(const JetsonState& jetson);
    static JetsonState getJetsonState();

    static void updateJetsonRxTick(std::uint32_t tick);
    static void updateJetsonTxTick(std::uint32_t tick);
    static void setJetsonConnected(bool connected);

private:
    static MotorCommand motorCommand_;
    static MotorFeedback motorFeedback_;
    static BatteryState batteryState_;
    static SensorState sensorState_;
    static UserInputState userInputState_;
    static JetsonState jetsonState_;
};

} // namespace system_state

#endif /* SRC_APP_SYSTEM_STATE_SYSTEMSTATE_HPP_ */
