// Motors.cpp  (SystemState-based motor driver)
#include "Motors.hpp"
#include "main.h"
#include "dac.h"
#include "../System_State/System_state.hpp"
#include <algorithm>

namespace motors {
constexpr float Motors::m_maximumSpeedMph;
constexpr std::uint32_t Motors::m_tachLines;
constexpr float Motors::m_wheelRadiusM;
constexpr float Motors::m_motorGearRatio;
constexpr float Motors::m_triwheelGearRatio;
constexpr float Motors::m_totalGearRatio;
// ----- pin + dac selection helpers -----
static inline std::uint32_t dacChannelFor(Motors::Channel channel) {
  return (channel == Motors::Channel::Left) ? DAC_CHANNEL_1 : DAC_CHANNEL_2;
}

static inline GPIO_TypeDef* enablePortFor(Motors::Channel channel) {
  return (channel == Motors::Channel::Left) ? ENABLE_LEFT_GPIO_Port : ENABLE_RIGHT_GPIO_Port;
}

static inline std::uint16_t enablePinFor(Motors::Channel channel) {
  return (channel == Motors::Channel::Left) ? ENABLE_LEFT_Pin : ENABLE_RIGHT_Pin;
}

static inline GPIO_TypeDef* brakePortFor(Motors::Channel channel) {
  return (channel == Motors::Channel::Left) ? BRAKE_LEFT_GPIO_Port : BRAKE_RIGHT_GPIO_Port;
}

static inline std::uint16_t brakePinFor(Motors::Channel channel) {
  return (channel == Motors::Channel::Left) ? BRAKE_LEFT_Pin : BRAKE_RIGHT_Pin;
}

static inline GPIO_TypeDef* directionPortFor(Motors::Channel channel) {
  return (channel == Motors::Channel::Left) ? DIRECTION_LEFT_GPIO_Port : DIRECTION_RIGHT_GPIO_Port;
}

static inline std::uint16_t directionPinFor(Motors::Channel channel) {
  return (channel == Motors::Channel::Left) ? DIRECTION_LEFT_Pin : DIRECTION_RIGHT_Pin;
}

static inline GPIO_TypeDef* faultPortFor(Motors::Channel channel) {
  return (channel == Motors::Channel::Left) ? FAULT_LEFT_GPIO_Port : FAULT_RIGHT_GPIO_Port;
}

static inline std::uint16_t faultPinFor(Motors::Channel channel) {
  return (channel == Motors::Channel::Left) ? FAULT_LEFT_Pin : FAULT_RIGHT_Pin;
}

// ----- class implementation -----
Motors::Motors(Channel channel) : m_channel(channel) {}

bool Motors::init() {
  if (HAL_DAC_Start(&hdac1, DAC_CHANNEL_1) != HAL_OK) return false;
  if (HAL_DAC_Start(&hdac1, DAC_CHANNEL_2) != HAL_OK) return false;

  if (HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0) != HAL_OK) return false;
  if (HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 0) != HAL_OK) return false;

  return true;
}

std::uint16_t Motors::voltsToCode(float volts) {
  const float clippedVolts = std::max(0.0f, std::min(volts, 3.3f));
  const float codeFloat = (clippedVolts / 3.3f) * 4095.0f;

  if (codeFloat <= 0.0f) return 0;
  if (codeFloat >= 4095.0f) return 4095;

  return static_cast<std::uint16_t>(codeFloat + 0.5f);
}

bool Motors::writeRaw(std::uint16_t code12) {
  if (code12 > 4095U) code12 = 4095U;

  const std::uint32_t dacChannel = dacChannelFor(m_channel);

  if (HAL_DAC_SetValue(&hdac1, dacChannel, DAC_ALIGN_12B_R, code12) != HAL_OK) {
    return false;
  }

  return true;
}

bool Motors::writeMotor(float volts) {
  return writeRaw(voltsToCode(volts));
}

float Motors::speedToVolts(float commandedSpeedMph) const {
  if (m_maximumSpeedMph <= 0.0f) {
    return 0.0f;
  }

  const float clippedSpeedMph =
      std::max(0.0f, std::min(commandedSpeedMph, m_maximumSpeedMph));

  return (clippedSpeedMph / m_maximumSpeedMph) * 3.3f;
}

// Active-low inputs (open-drain): active => pull LOW
bool Motors::setEnable(bool enableActive)
{
  HAL_GPIO_WritePin(enablePortFor(m_channel), enablePinFor(m_channel),
                    enableActive ? GPIO_PIN_RESET : GPIO_PIN_SET);

  return true;
}

bool Motors::setBrake(bool brakeActive) {
  HAL_GPIO_WritePin(brakePortFor(m_channel), brakePinFor(m_channel),
                    brakeActive ? GPIO_PIN_RESET : GPIO_PIN_SET);

  return true;
}

bool Motors::setDirection(bool reverseActive) {
  HAL_GPIO_WritePin(directionPortFor(m_channel), directionPinFor(m_channel),
                    reverseActive ? GPIO_PIN_RESET : GPIO_PIN_SET);

  return true;
}

bool Motors::readFault() const {
  // active-low fault output: LOW means faulted
  const bool faulted =
      (HAL_GPIO_ReadPin(faultPortFor(m_channel), faultPinFor(m_channel)) == GPIO_PIN_RESET);

  return faulted;
}

void Motors::tick()
{
  applyCommandsFromSystemState();

  const bool faultActive = readFault();
  publishFaultToSystemState(faultActive);
}

float Motors::readTachSpeedAndPushToSystemState(TIM_HandleTypeDef* htim,
                                                std::uint32_t timerChannel,
                                                std::uint32_t timerClockHz)
{
  /**
   * Feedback only:
   * - Reads timer input-capture value
   * - Calculates tach frequency
   * - Converts measured tach frequency to mph
   * - Pushes measured speed to SystemState for Jetson feedback
   */
  static constexpr float PI = 3.14159265359f;
  static constexpr float METERS_PER_MILE = 1609.344f;

  if (htim == nullptr || timerClockHz == 0U) {
    publishMeasuredSpeedToSystemState(0.0f);
    return 0.0f;
  }

  const std::uint32_t currentCapture =
      HAL_TIM_ReadCapturedValue(htim, timerChannel);

  if (!m_hasPreviousTachCapture) {
    m_previousTachCapture = currentCapture;
    m_hasPreviousTachCapture = true;

    publishMeasuredSpeedToSystemState(0.0f);
    return 0.0f;
  }

  const std::uint32_t timerPeriod = __HAL_TIM_GET_AUTORELOAD(htim);

  std::uint32_t captureDifference = 0U;

  if (currentCapture >= m_previousTachCapture) {
    captureDifference = currentCapture - m_previousTachCapture;
  } else {
    captureDifference = (timerPeriod - m_previousTachCapture) + currentCapture + 1U;
  }

  m_previousTachCapture = currentCapture;

  if (captureDifference == 0U) {
    publishMeasuredSpeedToSystemState(0.0f);
    return 0.0f;
  }

  const float frequencyHz =
      static_cast<float>(timerClockHz) / static_cast<float>(captureDifference);

  // Tach Mode equation: motor RPM = 60 * frequency / tach lines
  const float motorRpm =
      (60.0f * frequencyHz) / static_cast<float>(m_tachLines);

  // Convert motor RPM to wheel RPM using total gear reduction
  const float wheelRpm = motorRpm / m_totalGearRatio;

  // Convert wheel RPM to linear speed in mph
  const float wheelCircumferenceM = 2.0f * PI * m_wheelRadiusM;
  const float metersPerMinute = wheelRpm * wheelCircumferenceM;

  const float measuredSpeedMph =
      (metersPerMinute * 60.0f) / METERS_PER_MILE;

  publishMeasuredSpeedToSystemState(measuredSpeedMph);

  return measuredSpeedMph;
}

void Motors::applyCommandsFromSystemState()
{
  const system_state::MotorCommand command =
      system_state::SystemState::getMotorCommand();

  float commandedSpeedMph = 0.0f;
  bool enableActive = false;
  bool brakeActive = true;
  bool reverseActive = false;

  if (m_channel == Motors::Channel::Left) {
    commandedSpeedMph = command.speedLeft;
    enableActive = command.enableLeft;
    brakeActive = command.brakeLeft;
    reverseActive = (command.directionLeft == system_state::Direction::Reverse);
  } else {
    commandedSpeedMph = command.speedRight;
    enableActive = command.enableRight;
    brakeActive = command.brakeRight;
    reverseActive = (command.directionRight == system_state::Direction::Reverse);
  }

  if (!command.commandValid) {
    commandedSpeedMph = 0.0f;
    enableActive = false;
    brakeActive = true;
    reverseActive = false;
  }

  setEnable(enableActive);
  setBrake(brakeActive);
  setDirection(reverseActive);

  const float commandVolts = speedToVolts(commandedSpeedMph);
  writeMotor(commandVolts);
}

void Motors::publishMeasuredSpeedToSystemState(float measuredSpeedMph) const
{
  if (m_channel == Motors::Channel::Left) {
    system_state::SystemState::setLeftMotorSpeed(measuredSpeedMph);
  } else {
    system_state::SystemState::setRightMotorSpeed(measuredSpeedMph);
  }
}

void Motors::publishFaultToSystemState(bool faultActive) const
{
  if (m_channel == Motors::Channel::Left) {
    system_state::SystemState::setLeftMotorFault(faultActive);
  } else {
    system_state::SystemState::setRightMotorFault(faultActive);
  }
}

} // namespace motors
