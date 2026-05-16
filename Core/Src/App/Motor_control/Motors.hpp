#pragma once
#include <cstdint>

namespace motors {

/**
 * @class Motors
 * @brief High-level motor control driver for an ElectroCraft EZ-Drive channel.
 *
 * This class abstracts:
 * - DAC-based analog speed control (0–3.3 V output)
 * - Active-low digital control signals (Enable, Brake, Direction)
 * - Active-low Fault status input
 *
 * Hardware initialization (GPIO configuration, DAC peripheral init)
 * is handled externally by CubeMX.
 *
 * Responsibilities of this class:
 * - Start DAC channels at runtime
 * - Write command signals
 * - Read hardware status signals
 * - Cache software state for diagnostics
 *
 * @note Speed measurement is handled in a separate tachometer module.
 *       This class only reads externally updated speed variables.
 */
class Motors {
public:

  /**
   * @enum Channel
   * @brief Identifies which physical motor channel this object controls.
   *
   * Right -> DAC1 Channel 1 + Left GPIO pins
   * Left -> DAC1 Channel 2 + Right GPIO pins
   */
  enum class Channel : std::uint8_t {
    Left,
    Right
  };

  /**
   * @brief Construct a motor controller bound to a specific channel.
   *
   * @param channel Motor channel (Left or Right).
   */
  explicit Motors(Channel channel);

  /**
   * @brief Initialize shared motor peripherals.
   *
   * Performs:
   * - HAL_DAC_Start() on both DAC channels
   * - Forces DAC outputs to 0 V for safe startup
   *
   * @pre MX_DAC1_Init() must already have been executed.
   *
   * @return true if initialization succeeded, false otherwise.
   */
  static bool init();

  // ====================== Speed Control ======================

  /**
   * @brief Set motor command voltage.
   *
   * Voltage is clamped between 0.0 V and 3.3 V.
   * Internally converted to 12-bit DAC code (0–4095).
   *
   * @param volts Desired output voltage.
   * @return true if DAC write succeeded.
   */
  bool writeMotor(float volts);

  /**
   * @brief Write raw 12-bit DAC value directly.
   *
   * @param code12 DAC code (0–4095).
   * @return true if DAC write succeeded.
   */
  bool writeRaw(std::uint16_t code12);

  // ====================== Command Signals ======================

  /**
   * @brief Set motor enable signal.
   *
   * Active-low logic:
   * - true  → drive LOW (motor enabled)
   * - false → release HIGH (motor disabled)
   *
   * @param enable Enable state.
   * @return true if GPIO write executed.
   */
  bool setEnable(bool enable);

  /**
   * @brief Set motor brake signal.
   *
   * Active-low logic:
   * - true  → drive LOW (brake active)
   * - false → release HIGH (brake released)
   *
   * @param brake Brake state.
   * @return true if GPIO write executed.
   */
  bool setBrake(bool brake);

  /**
   * @brief Set motor direction signal.
   *
   * Active-low logic depending on hardware wiring.
   *
   * @param direction Direction bit (interpretation hardware-defined).
   * @return true if GPIO write executed.
   */
  bool setDirection(bool direction);

  // ====================== Status Reads ======================

  /**
   * @brief Read motor fault state.
   *
   * Motor drivers assert fault LOW.
   * @return true if motor driver asserts low.
   */
  bool readFault() const;


  // ====================== Helpers ======================

  /**
   * @brief Convert commanded speed value to equivalent voltage.
   *
   *
   * @param commandedSpeed Desired speed value.
   * @return Equivalent voltage (0.06–3.3 V).
   */
  float speedToVolts(std::uint32_t commandedSpeed) const;

  // ====================== Cached State Access ======================

  /**
   * @brief Retrieve cached commanded speed value from Jetson
   */
  std::uint32_t commandedSpeedCached() const;

  /**
   * @brief Retrieve cached enable state.
   */
  bool enableCached() const;

  /**
   * @brief Retrieve cached brake state.
   */
  bool brakeCached() const;

  /**
   * @brief Retrieve cached direction state.
   */
  bool directionCached() const;

  /**
   * @brief Retrieve cached fault state.
   */
  bool faultCached() const ;


  static void initRunner();
  static void tick_Motors();
  void setSpeed(std::uint32_t speed_mph);
private:
  //Maximum speed in Miles per hour
   static constexpr float m_maximumSpeedMph = 3.0f;
  /**
   * @brief Convert voltage to 12-bit DAC code.
   *
   * @param volts Voltage (0–3.3 V).
   * @return DAC code (0–4095).
   */
  static std::uint16_t voltsToCode(float volts);

  /// Physical motor channel
  Channel m_channel;

  /// Cached commanded speed value
  std::uint32_t m_commandedSpeed = 0;

  /// Cached last DAC voltage
  float m_speedInVolts = 0.0f;

  /// Cached enable state
  bool m_enable = false;

  /// Cached brake state
  bool m_brake = false;

  /// Cached direction state
 bool m_direction = false;

  /// Cached fault state (mutable to allow update in const read function)
  mutable bool m_fault = false;
};

} // namespace motors
