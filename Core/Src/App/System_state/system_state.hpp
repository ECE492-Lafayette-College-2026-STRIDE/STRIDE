/*
 * System_state.hpp
 *
 *  Created on: May 16, 2026
 *      Author: otienom
 */

#ifndef SRC_APP_SYSTEM_STATE_SYSTEM_STATE_HPP_
#define SRC_APP_SYSTEM_STATE_SYSTEM_STATE_HPP_

#include <cstdint>

namespace system_state {

enum class Direction : std::uint8_t {
    Forward = 0,
    Reverse = 1
};

enum class ControlMode : std::uint8_t {
    Standard = 0,
    Caregiver = 1,
    StairClimbing = 2
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
    float speedLeft = 2.0f;
    float speedRight = 2.0f;

    bool faultLeft = false;
    bool faultRight = false;
};

struct BatteryState {
    float voltage = 1.0f;
    float current = 1.0f;
    float power = 0.0f;

    float realtimeCapacity_mAh = 0.0f;
    float flashCapacity_mAh = 0.0f;

    float percent = 0.0f;

    bool resetCapacityToFullRequest = false;
    bool valid = false;
};

struct SensorState {
    float frontDistance = 0.0f;        // ultrasonic distance in mm
    std::uint8_t frontStatus = 0U;     // ultrasonic status from sensor MCU
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
    std::uint32_t lastRxTick = 0U;
    std::uint32_t lastTxTick = 0U;
};

struct ControlClusterState {
    ControlMode mode = ControlMode::Standard;

    bool stairConfirm = false;
    bool resetRequest = false;

    bool valid = false;
};

class SystemState {
public:
    /**
     * @brief Reset all stored system state values to default.
     */
    static void init();

    // ====================== Jetson command updates ======================

    /**
     * @brief Store the latest motor command received from Jetson or control logic.
     *
     * @param command Motor command structure.
     */
    static void setMotorCommand(const MotorCommand& command);

    /**
     * @brief Get the latest stored motor command.
     *
     * @return Current motor command structure.
     */
    static MotorCommand getMotorCommand();

    /**
     * @brief Update only the motor command valid flag.
     *
     * @param valid true if the stored command is valid.
     */
    static void setMotorCommandValid(bool valid);

    // ====================== Motor driver updates ======================

    /**
     * @brief Store the full motor feedback structure.
     *
     * @param feedback Motor feedback structure.
     */
    static void setMotorFeedback(const MotorFeedback& feedback);

    /**
     * @brief Get the latest stored motor feedback.
     *
     * @return Current motor feedback structure.
     */
    static MotorFeedback getMotorFeedback();

    /**
     * @brief Update only the left measured motor speed.
     *
     * @param speedMph Measured left motor speed in mph.
     */
    static void setLeftMotorSpeed(float speedMph);

    /**
     * @brief Update only the right measured motor speed.
     *
     * @param speedMph Measured right motor speed in mph.
     */
    static void setRightMotorSpeed(float speedMph);

    /**
     * @brief Update only the left motor fault flag.
     *
     * @param fault true if left motor driver is faulted.
     */
    static void setLeftMotorFault(bool fault);

    /**
     * @brief Update only the right motor fault flag.
     *
     * @param fault true if right motor driver is faulted.
     */
    static void setRightMotorFault(bool fault);

    // ====================== Battery monitor updates ======================

    /**
     * @brief Store the latest battery state.
     *
     * @param battery Battery state structure.
     */
    static void setBatteryState(const BatteryState& battery);

    /**
     * @brief Get the latest stored battery state.
     *
     * @return Current battery state structure.
     */
    static BatteryState getBatteryState();

    /**
     * @brief Request the battery monitor/flash logic to reset capacity to full.
     *
     * This only sets the request flag. The battery driver should handle the
     * actual capacity reset and clear the request afterward.
     */
    static void requestResetCapacityToFull();

    /**
     * @brief Clear the battery full-capacity reset request flag.
     */
    static void clearResetCapacityToFullRequest();

    // ====================== Sensor driver updates ======================

    /**
     * @brief Store the latest ultrasonic sensor state.
     *
     * @param sensors Sensor state structure.
     */
    static void setSensorState(const SensorState& sensors);

    /**
     * @brief Get the latest ultrasonic sensor state.
     *
     * @return Current sensor state structure.
     */
    static SensorState getSensorState();

    // ====================== User input updates ======================

    /**
     * @brief Store the latest user input state.
     *
     * @param input User input state structure.
     */
    static void setUserInputState(const UserInputState& input);

    /**
     * @brief Get the latest user input state.
     *
     * @return Current user input state structure.
     */
    static UserInputState getUserInputState();

    // ====================== Jetson link status ======================

    /**
     * @brief Store the complete Jetson state.
     *
     * @param jetson Jetson state structure.
     */
    static void setJetsonState(const JetsonState& jetson);

    /**
     * @brief Get the latest Jetson state.
     *
     * @return Current Jetson state structure.
     */
    static JetsonState getJetsonState();

    /**
     * @brief Update the last Jetson receive tick.
     *
     * @param tick HAL tick when a valid Jetson command was received.
     */
    static void updateJetsonRxTick(std::uint32_t tick);

    /**
     * @brief Update the last Jetson transmit tick.
     *
     * @param tick HAL tick when feedback was transmitted to Jetson.
     */
    static void updateJetsonTxTick(std::uint32_t tick);

    /**
     * @brief Update Jetson connection status.
     *
     * @param connected true if Jetson link is considered connected.
     */
    static void setJetsonConnected(bool connected);

    // ====================== Control cluster updates ======================

    /**
     * @brief Store the latest control cluster state.
     *
     * @param control Control cluster state structure.
     */
    static void setControlClusterState(const ControlClusterState& control);

    /**
     * @brief Get the latest control cluster state.
     *
     * @return Current control cluster state structure.
     */
    static ControlClusterState getControlClusterState();

private:
    static MotorFeedback motorFeedback_;
    static BatteryState batteryState_;
    static SensorState sensorState_;
    static UserInputState userInputState_;
    static JetsonState jetsonState_;
    static MotorCommand motorCommand_;
    static ControlClusterState controlClusterState_;
};

} // namespace system_state

#endif /* SRC_APP_SYSTEM_STATE_SYSTEM_STATE_HPP_ */
