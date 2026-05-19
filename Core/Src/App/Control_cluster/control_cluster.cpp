/*
 * control_cluster.cpp
 *
 *  Created on: May 18, 2026
 *      Author: otienom
 *
 *  Driver for STRIDE user-facing control cluster.
 */

#include "control_cluster.hpp"
#include "../System_State/system_state.hpp"

using namespace system_state;

namespace control_cluster {

namespace {

constexpr std::uint8_t PCA9685_MODE1     = 0x00U;
constexpr std::uint8_t PCA9685_MODE2     = 0x01U;
constexpr std::uint8_t PCA9685_LED0_ON_L = 0x06U;
constexpr std::uint8_t PCA9685_PRESCALE  = 0xFEU;

constexpr std::uint8_t MODE1_SLEEP   = 0x10U;
constexpr std::uint8_t MODE1_AI      = 0x20U;
constexpr std::uint8_t MODE1_RESTART = 0x80U;
constexpr std::uint8_t MODE2_OUTDRV  = 0x04U;

constexpr std::uint16_t LED_OFF = 0U;
constexpr std::uint16_t LED_DIM = 800U;
constexpr std::uint16_t LED_MED = 1800U;
constexpr std::uint16_t LED_ON  = 4095U;

constexpr float LOW_BATTERY_PERCENT = 20.0f;
constexpr std::uint32_t BLINK_PERIOD_MS = 500U;

struct ButtonRuntime {
    bool stablePressed = false;
    bool lastRawPressed = false;
    bool pressEvent = false;
    std::uint32_t lastChangeTick = 0U;
};

ControlClusterConfig cfg{};
bool initialized = false;

ButtonRuntime modeBtn{};
ButtonRuntime stairBtn{};
ButtonRuntime resetBtn{};

ControlClusterState state{};

std::uint32_t lastLedUpdateTick = 0U;
bool blinkOn = false;

std::uint16_t deviceAddress()
{
    return static_cast<std::uint16_t>(cfg.pca9685Address7bit << 1U);
}

ButtonRuntime& runtimeFor(ButtonId id)
{
    switch (id) {
    case ButtonId::Mode:
        return modeBtn;
    case ButtonId::Stair:
        return stairBtn;
    case ButtonId::Reset:
    default:
        return resetBtn;
    }
}

ControlMode nextMode(ControlMode current)
{
    switch (current) {
    case ControlMode::Standard:
        return ControlMode::Caregiver;
    case ControlMode::Caregiver:
        return ControlMode::StairClimbing;
    case ControlMode::StairClimbing:
    default:
        return ControlMode::Standard;
    }
}

} // anonymous namespace

bool ControlCluster::init(const ControlClusterConfig& config)
{
    cfg = config;

    modeBtn = ButtonRuntime{};
    stairBtn = ButtonRuntime{};
    resetBtn = ButtonRuntime{};

    state = ControlClusterState{};
    state.mode = ControlMode::Standard;
    state.stairConfirm = false;
    state.resetRequest = false;
    state.valid = false;

    lastLedUpdateTick = HAL_GetTick();
    blinkOn = false;

    if (cfg.hi2c == nullptr) {
        initialized = false;
        return false;
    }

    initialized = initPca9685();

    state.valid = initialized;
    SystemState::setControlClusterState(state);

    updateLeds();

    return initialized;
}

bool ControlCluster::isInitialized()
{
    return initialized;
}

void ControlCluster::tick()
{
    if (!initialized) {
        return;
    }

    const bool modePressed = updateButton(ButtonId::Mode, cfg.modeButton);
    const bool stairPressed = updateButton(ButtonId::Stair, cfg.stairButton);
    const bool resetPressed = updateButton(ButtonId::Reset, cfg.resetButton);

    if (modePressed) {
        handleModePress();
    }

    if (stairPressed) {
        handleStairPress();
    }

    if (resetPressed) {
        handleResetPress();
    }

    updateSystemState();
    updateLeds();

    if (state.resetRequest) {
        state.resetRequest = false;
        updateSystemState();
    }
}

bool ControlCluster::initPca9685()
{
    if (!writeRegister(PCA9685_MODE1, MODE1_SLEEP | MODE1_AI)) {
        return false;
    }

    /* Around 200 Hz PWM: prescale ~= round(25MHz / (4096 * 200)) - 1 = 30 */
    if (!writeRegister(PCA9685_PRESCALE, 30U)) {
        return false;
    }

    if (!writeRegister(PCA9685_MODE2, MODE2_OUTDRV)) {
        return false;
    }

    if (!writeRegister(PCA9685_MODE1, MODE1_AI)) {
        return false;
    }

    HAL_Delay(1U);

    if (!writeRegister(PCA9685_MODE1, MODE1_RESTART | MODE1_AI)) {
        return false;
    }

    return true;
}

bool ControlCluster::writeRegister(std::uint8_t reg, std::uint8_t value)
{
    return HAL_I2C_Mem_Write(cfg.hi2c,
                             deviceAddress(),
                             reg,
                             I2C_MEMADD_SIZE_8BIT,
                             &value,
                             1U,
                             10U) == HAL_OK;
}

bool ControlCluster::setPwm(std::uint8_t channel, std::uint16_t on, std::uint16_t off)
{
    if (channel > 15U) {
        return true;
    }

    const std::uint8_t reg = static_cast<std::uint8_t>(PCA9685_LED0_ON_L + (4U * channel));

    std::uint8_t data[4];
    data[0] = static_cast<std::uint8_t>(on & 0xFFU);
    data[1] = static_cast<std::uint8_t>((on >> 8U) & 0x0FU);
    data[2] = static_cast<std::uint8_t>(off & 0xFFU);
    data[3] = static_cast<std::uint8_t>((off >> 8U) & 0x0FU);

    return HAL_I2C_Mem_Write(cfg.hi2c,
                             deviceAddress(),
                             reg,
                             I2C_MEMADD_SIZE_8BIT,
                             data,
                             sizeof(data),
                             10U) == HAL_OK;
}

bool ControlCluster::setLedRaw(std::uint8_t channel, std::uint16_t value)
{
    if (channel == 0xFFU) {
        return true;
    }

    if (value == 0U) {
        return setPwm(channel, 0U, 0U);
    }

    if (value >= 4095U) {
        return setPwm(channel, 0U, 4095U);
    }

    return setPwm(channel, 0U, value);
}

bool ControlCluster::setRgb(std::uint8_t redChannel,
                            std::uint8_t greenChannel,
                            std::uint8_t blueChannel,
                            std::uint16_t red,
                            std::uint16_t green,
                            std::uint16_t blue)
{
    bool ok = true;
    ok = setLedRaw(redChannel, red) && ok;
    ok = setLedRaw(greenChannel, green) && ok;
    ok = setLedRaw(blueChannel, blue) && ok;
    return ok;
}

bool ControlCluster::readButtonRaw(const ButtonConfig& button)
{
    if (button.port == nullptr) {
        return false;
    }

    GPIO_PinState pinState = HAL_GPIO_ReadPin(button.port, button.pin);
    return pinState == button.activeState;
}

bool ControlCluster::updateButton(ButtonId id, const ButtonConfig& config)
{
    ButtonRuntime& rt = runtimeFor(id);

    const bool rawPressed = readButtonRaw(config);
    const std::uint32_t now = HAL_GetTick();

    rt.pressEvent = false;

    if (rawPressed != rt.lastRawPressed) {
        rt.lastRawPressed = rawPressed;
        rt.lastChangeTick = now;
    }

    if ((now - rt.lastChangeTick) >= cfg.debounceMs) {
        if (rawPressed != rt.stablePressed) {
            rt.stablePressed = rawPressed;

            if (rt.stablePressed) {
                rt.pressEvent = true;
            }
        }
    }

    return rt.pressEvent;
}

void ControlCluster::handleModePress()
{
    state.mode = nextMode(state.mode);
    state.stairConfirm = false;
    state.resetRequest = false;
    state.valid = true;
}

void ControlCluster::handleStairPress()
{
    if (state.mode == ControlMode::StairClimbing) {
        state.stairConfirm = true;
    }

    state.valid = true;
}

void ControlCluster::handleResetPress()
{
    state.mode = ControlMode::Standard;
    state.stairConfirm = false;
    state.resetRequest = true;
    state.valid = true;
}

void ControlCluster::updateSystemState()
{
    SystemState::setControlClusterState(state);
}

void ControlCluster::updateLeds()
{
    const std::uint32_t now = HAL_GetTick();

    if ((now - lastLedUpdateTick) >= BLINK_PERIOD_MS) {
        lastLedUpdateTick = now;
        blinkOn = !blinkOn;
    }

    const auto battery = SystemState::getBatteryState();

    if (battery.valid && battery.percent <= LOW_BATTERY_PERCENT) {
        setRgb(cfg.leds.modeRed, cfg.leds.modeGreen, cfg.leds.modeBlue,
               LED_MED, LED_MED, LED_OFF);     // amber
    } else {
        switch (state.mode) {
        case ControlMode::Standard:
            setRgb(cfg.leds.modeRed, cfg.leds.modeGreen, cfg.leds.modeBlue,
                   LED_OFF, LED_ON, LED_OFF);  // green
            break;

        case ControlMode::Caregiver:
            setRgb(cfg.leds.modeRed, cfg.leds.modeGreen, cfg.leds.modeBlue,
                   LED_OFF, LED_OFF, LED_ON);  // blue
            break;

        case ControlMode::StairClimbing:
        default:
            setRgb(cfg.leds.modeRed, cfg.leds.modeGreen, cfg.leds.modeBlue,
                   LED_ON, LED_ON, LED_OFF);   // yellow
            break;
        }
    }

    if (state.mode == ControlMode::StairClimbing) {
        const std::uint16_t green = state.stairConfirm ? (blinkOn ? LED_ON : LED_OFF) : LED_DIM;
        setRgb(cfg.leds.stairRed, cfg.leds.stairGreen, cfg.leds.stairBlue,
               LED_OFF, green, LED_OFF);
    } else {
        setRgb(cfg.leds.stairRed, cfg.leds.stairGreen, cfg.leds.stairBlue,
               LED_OFF, LED_OFF, LED_OFF);
    }

    if (state.resetRequest) {
        setRgb(cfg.leds.resetRed, cfg.leds.resetGreen, cfg.leds.resetBlue,
               blinkOn ? LED_ON : LED_OFF, LED_OFF, LED_OFF);
    } else {
        setRgb(cfg.leds.resetRed, cfg.leds.resetGreen, cfg.leds.resetBlue,
               LED_DIM, LED_OFF, LED_OFF);
    }
}

} // namespace control_cluster
