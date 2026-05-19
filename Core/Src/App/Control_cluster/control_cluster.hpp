/*
 * control_cluster.hpp
 *
 *  Created on: May 18, 2026
 *      Author: otienom
 *
 *  Driver for STRIDE user-facing control cluster.
 */

#ifndef SRC_APP_CONTROL_CLUSTER_CONTROL_CLUSTER_HPP_
#define SRC_APP_CONTROL_CLUSTER_CONTROL_CLUSTER_HPP_

#include "stm32h7xx_hal.h"
#include <cstdint>

namespace control_cluster {

enum class ButtonId : std::uint8_t {
    Mode = 0,
    Stair = 1,
    Reset = 2
};

struct ButtonConfig {
    GPIO_TypeDef* port = nullptr;
    std::uint16_t pin = 0U;
    GPIO_PinState activeState = GPIO_PIN_RESET;
};

struct LedChannelConfig {
    std::uint8_t modeRed = 0xFFU;
    std::uint8_t modeGreen = 0xFFU;
    std::uint8_t modeBlue = 0xFFU;

    std::uint8_t stairRed = 0xFFU;
    std::uint8_t stairGreen = 0xFFU;
    std::uint8_t stairBlue = 0xFFU;

    std::uint8_t resetRed = 0xFFU;
    std::uint8_t resetGreen = 0xFFU;
    std::uint8_t resetBlue = 0xFFU;
};

struct ControlClusterConfig {
    I2C_HandleTypeDef* hi2c = nullptr;
    std::uint8_t pca9685Address7bit = 0x40U;

    ButtonConfig modeButton;
    ButtonConfig stairButton;
    ButtonConfig resetButton;

    LedChannelConfig leds;
    std::uint32_t debounceMs = 30U;
};

class ControlCluster {
public:
    static bool init(const ControlClusterConfig& config);
    static void tick();
    static bool isInitialized();

private:
    static bool initPca9685();
    static bool writeRegister(std::uint8_t reg, std::uint8_t value);
    static bool setPwm(std::uint8_t channel, std::uint16_t on, std::uint16_t off);
    static bool setLedRaw(std::uint8_t channel, std::uint16_t value);
    static bool setRgb(std::uint8_t redChannel,
                       std::uint8_t greenChannel,
                       std::uint8_t blueChannel,
                       std::uint16_t red,
                       std::uint16_t green,
                       std::uint16_t blue);

    static bool readButtonRaw(const ButtonConfig& button);
    static bool updateButton(ButtonId id, const ButtonConfig& config);

    static void handleModePress();
    static void handleStairPress();
    static void handleResetPress();

    static void updateSystemState();
    static void updateLeds();
};

} // namespace control_cluster

#endif /* SRC_APP_CONTROL_CLUSTER_CONTROL_CLUSTER_HPP_ */
