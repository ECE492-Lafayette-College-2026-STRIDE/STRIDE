#pragma once

#include <cstdint>
#include "stm32h7xx_hal.h"

namespace motors {

/**
 * @class Motors
 * @brief High-level motor control driver for one ElectroCraft EZ-Drive motor channel.
 *
 * This class represents one motor object, either left or right.
 *
 * SystemState is the source of truth for the STRIDE system. This driver only:
 * - Pulls motor commands from SystemState
 * - Applies those commands to hardware using DAC/GPIO
 * - Reads hardware feedback/status
 * - Pushes measured feedback/status back to SystemState
 *
 * Command path:
 *
 * Jetson driver -> SystemState -> Motors driver -> DAC/GPIO -> EZ Drive
 *
 * Feedback path:
 *
 * EZ Drive tach/fault -> Motors driver -> SystemState -> Jetson driver
 *
 * @note Tach speed measurement uses timer input capture, not normal GPIO.
 *       The motor driver reads the captured timer value using
 *       HAL_TIM_ReadCapturedValue(), calculates tach frequency, converts it
 *       to measured speed in mph, and pushes the value to SystemState.
 */
class Motors {
public:

  /**
   * @enum Channel
   * @brief Identifies which physical motor channel this object controls.
   *
   * Left  -> DAC1 Channel 1 + left motor GPIO pins
   * Right -> DAC1 Channel 2 + right motor GPIO pins
   */
  enum class Channel : std::uint8_t {
    Left,
    Right
  };

  /**
   * @brief Construct a motor controller bound to one physical motor channel.
   *
   * @param channel Motor channel, either Left or Right.
   */
  explicit Motors(Channel channel);

  /**
   * @brief Initialize shared motor peripherals.
   *
   * Performs:
   * - Starts both DAC channels
   * - Forces both DAC outputs to 0 V for safe startup
   *
   * @pre MX_DAC1_Init() must already have been executed.
   *
   * @return true if initialization succeeded.
   * @return false if DAC startup or DAC zeroing failed.
   */
  static bool init();

  // ====================== SystemState Interface ======================

  /**
   * @brief Run one motor driver update cycle for this motor object.
   *
   * This function should be called periodically from the motor task, scheduler,
   * or superloop.
   *
   * It performs the normal command/status path:
   * - Pull commanded speed, enable, brake, and direction from SystemState
   * - Apply those commands to the motor drive hardware
   * - Read the fault GPIO
   * - Push fault/status information back to SystemState
   *
   * This function does not wait for tach edges. Tach speed should be updated
   * from the timer input-capture callback using readTachSpeedAndPushToSystemState().
   */
  void tick();

  // ====================== Speed Command Hardware Control ======================

  /**
   * @brief Set motor command voltage.
   *
   * This writes the analog speed command to the EZ Drive through the MCU DAC.
   * The voltage is converted to a 12-bit DAC code.
   *
   * @param volts Desired DAC output voltage from 0.0 V to 3.3 V.
   * @return true if the DAC write succeeded.
   * @return false if the DAC write failed.
   */
  bool writeMotor(float volts);

  /**
   * @brief Write raw 12-bit DAC value directly.
   *
   * @param code12 DAC code from 0 to 4095.
   * @return true if the DAC write succeeded.
   * @return false if the DAC write failed.
   */
  bool writeRaw(std::uint16_t code12);

  /**
   * @brief Convert commanded speed in mph to DAC command voltage.
   *
   * The STRIDE maximum commanded walking speed is 3 mph. This function maps:
   *
   * 0 mph -> 0.0 V
   * 3 mph -> 3.3 V
   *
   * @param commandedSpeedMph Desired commanded speed in mph.
   * @return Equivalent DAC voltage from 0.0 V to 3.3 V.
   */
  float speedToVolts(float commandedSpeedMph) const;

  // ====================== Digital Command Signals ======================

  /**
   * @brief Set motor enable signal.
   *
   * Active-low logic:
   * - true  -> drive LOW, motor enabled
   * - false -> release HIGH, motor disabled
   *
   * @param enableActive Desired enable state.
   * @return true after GPIO write is executed.
   */
  bool setEnable(bool enableActive);

  /**
   * @brief Set motor brake signal.
   *
   * Active-low logic:
   * - true  -> drive LOW, brake active
   * - false -> release HIGH, brake released
   *
   * @param brakeActive Desired brake state.
   * @return true after GPIO write is executed.
   */
  bool setBrake(bool brakeActive);

  /**
   * @brief Set motor direction signal.
   *
   * Active-low logic:
   * - true  -> drive LOW, reverse direction active
   * - false -> release HIGH, normal direction
   *
   * @param reverseActive Desired direction state.
   * @return true after GPIO write is executed.
   */
  bool setDirection(bool reverseActive);

  // ====================== Hardware Feedback ======================

  /**
   * @brief Read motor fault state from the EZ Drive fault GPIO.
   *
   * The EZ Drive fault output is active-low:
   * - LOW  -> fault present
   * - HIGH -> no fault
   *
   * @return true if the motor drive is faulted.
   * @return false if no fault is present.
   */
  bool readFault() const;

  /**
   * @brief Read tach speed using timer input capture and push measured speed to SystemState.
   *
   * This function reads the current timer input-capture value for this motor's
   * tach signal, calculates tach frequency, converts the result to measured
   * wheel speed in mph, and writes the measured speed to SystemState.
   *
   * This function is feedback only. It does not:
   * - Command the motor
   * - Write to the DAC
   * - Change enable, brake, or direction
   *
   * Tach Mode equation:
   *
   * motor_RPM = (60 * frequency_Hz) / tach_lines
   *
   * Wheel-speed conversion:
   *
   * wheel_RPM = motor_RPM / total_gear_ratio
   * mph = wheel_RPM * wheel_circumference_m * 60 / 1609.344
   *
   * STRIDE constants:
   * - tach_lines = 1000
   * - wheel_radius = 8 inches = 0.2032 m
   * - motor gearbox ratio = 21.33:1
   * - triwheel gear ratio = 3:1
   * - total gear ratio = 63.99:1
   *
   * @param htim Pointer to the timer used for input capture.
   * @param timerChannel HAL timer channel used for this tach input.
   * @param timerClockHz Timer counting clock in Hz after prescaler.
   * @return Calculated measured speed in mph.
   */
  float readTachSpeedAndPushToSystemState(TIM_HandleTypeDef* htim,
                                          std::uint32_t timerChannel,
                                          std::uint32_t timerClockHz);

private:

  // ====================== STRIDE Motor Constants ======================

  /// Maximum commanded STRIDE speed in miles per hour
  static constexpr float m_maximumSpeedMph = 3.0f;

  /// Number of tach/encoder lines configured for the EZ Drive tach feedback
  static constexpr std::uint32_t m_tachLines = 1000U;

  /// Wheel radius in meters: 8 inches * 0.0254 = 0.2032 m
  static constexpr float m_wheelRadiusM = 0.2032f;

  /// ElectroCraft motor gearbox ratio
  static constexpr float m_motorGearRatio = 21.33f;

  /// Additional triwheel gear ratio
  static constexpr float m_triwheelGearRatio = 3.0f;

  /// Total gear reduction between motor tach speed and wheel speed
  static constexpr float m_totalGearRatio = m_motorGearRatio * m_triwheelGearRatio;

  // ====================== Internal Helpers ======================

  /**
   * @brief Convert voltage to 12-bit DAC code.
   *
   * @param volts Voltage from 0.0 V to 3.3 V.
   * @return DAC code from 0 to 4095.
   */
  static std::uint16_t voltsToCode(float volts);

  /**
   * @brief Pull motor commands from SystemState and apply them to hardware.
   *
   * This reads commanded speed, enable, brake, and direction from SystemState
   * based on this object's motor channel.
   */
  void applyCommandsFromSystemState();

  /**
   * @brief Publish measured speed feedback to SystemState.
   *
   * This writes the measured motor speed in mph to the correct SystemState
   * variable depending on whether this object is the left or right motor.
   *
   * @param measuredSpeedMph Measured motor speed in mph.
   */
  void publishMeasuredSpeedToSystemState(float measuredSpeedMph) const;

  /**
   * @brief Publish motor fault status to SystemState.
   *
   * This writes the motor fault value to the correct SystemState variable
   * depending on whether this object is the left or right motor.
   *
   * @param faultActive true if fault is present, false otherwise.
   */
  void publishFaultToSystemState(bool faultActive) const;

  // ====================== Object State ======================

  /// Physical motor channel controlled by this object
  Channel m_channel;

  /// Previous timer input-capture count for tach speed measurement
  std::uint32_t m_previousTachCapture = 0U;

  /// True after the first tach capture has been stored
  bool m_hasPreviousTachCapture = false;
};

} // namespace motors
