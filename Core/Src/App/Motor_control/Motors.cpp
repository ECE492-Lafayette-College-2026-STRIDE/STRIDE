// Motors.cpp  (only the requested fixes)
#include "Motors.hpp"
#include "main.h"
#include "dac.h"
#include <algorithm>

namespace motors {
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
  const float codeFloat = (volts / 3.3f) * 4095.0f;
  if (codeFloat <= 0.0f) return 0;
  if (codeFloat >= 4095.0f) return 4095;
  return static_cast<std::uint16_t>(codeFloat + 0.5f);
}

bool Motors::writeRaw(std::uint16_t code12) {
  if (code12 > 4095) code12 = 4095;

  const std::uint32_t dacChannel = dacChannelFor(m_channel);
  if (HAL_DAC_SetValue(&hdac1, dacChannel, DAC_ALIGN_12B_R, code12) != HAL_OK) {
    return false;
  }

  m_speedInVolts = (static_cast<float>(code12) / 4095.0f) * 3.3f;
  return true;
}

bool Motors::writeMotor(float volts) {
  m_speedInVolts = volts;
  return writeRaw(voltsToCode(volts));
}

// Active-low inputs (open-drain): active => pull LOW
bool Motors::setEnable(bool enableActive)
{
  HAL_GPIO_WritePin(enablePortFor(m_channel), enablePinFor(m_channel),
                    enableActive ? GPIO_PIN_RESET : GPIO_PIN_SET);

  m_enable = enableActive;
  return true;
}

bool Motors::setBrake(bool brakeActive) {
  HAL_GPIO_WritePin(brakePortFor(m_channel), brakePinFor(m_channel),
                    brakeActive ? GPIO_PIN_RESET : GPIO_PIN_SET);
  m_brake = brakeActive;
  return true;
}

bool Motors::setDirection(bool reverseActive) {
  HAL_GPIO_WritePin(directionPortFor(m_channel), directionPinFor(m_channel),
                    reverseActive ? GPIO_PIN_RESET : GPIO_PIN_SET);
  m_direction = reverseActive;
  return true;
}

bool Motors::readFault() const {
  // active-low fault output: LOW means faulted
  const bool faulted =
      (HAL_GPIO_ReadPin(faultPortFor(m_channel), faultPinFor(m_channel)) == GPIO_PIN_RESET);
  m_fault = faulted; //
  return faulted;
}

float Motors::speedToVolts(std::uint32_t commandedSpeed) const {
  float volts = 0.0f;
  if (Motors::m_maximumSpeedMph > 0.0f) {
    volts = (static_cast<float>(commandedSpeed) /m_maximumSpeedMph) * 3.3f;
  }
  return volts;
}

void Motors::setSpeed(std::uint32_t speed_mph)
{
	m_commandedSpeed = speed_mph;
}
// ----- cached getters -----
std::uint32_t Motors::commandedSpeedCached() const { return m_commandedSpeed; }
bool Motors::enableCached() const { return m_enable; }
bool Motors::brakeCached() const { return m_brake; }
bool Motors::directionCached() const { return m_direction; }
bool Motors::faultCached() const { return m_fault; }

} // namespace motors
