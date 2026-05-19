/*
 * run_control_cluster.cpp
 *
 *  Created on: May 18, 2026
 *      Author: otienom
 *
 *  Runtime wrapper for control cluster driver.
 *
 *  IMPORTANT:
 *  Update the GPIO pin names and PCA9685 channel map in this file
 *  to match your CubeMX pin labels and PCB schematic.
 */

extern "C" {
#include "main.h"
#include "i2c.h"
}

#include "run_control_cluster.hpp"
#include "control_cluster.hpp"
#include "../System_State/system_state.hpp"

#include <cstdio>

using namespace control_cluster;
using namespace system_state;

extern I2C_HandleTypeDef hi2c2;

namespace {

ControlClusterConfig makeConfig()
{
    ControlClusterConfig cfg{};

    cfg.hi2c = &hi2c2;
    cfg.pca9685Address7bit = 0x40U;
    cfg.debounceMs = 30U;

    /*
     * Replace these with the real CubeMX names from main.h.
     * These blocks compile only if the symbols exist.
     */
#ifdef MODE_BUTTON_Pin
    cfg.modeButton.port = MODE_BUTTON_GPIO_Port;
    cfg.modeButton.pin = MODE_BUTTON_Pin;
    cfg.modeButton.activeState = GPIO_PIN_RESET;
#endif

#ifdef STAIR_BUTTON_Pin
    cfg.stairButton.port = STAIR_BUTTON_GPIO_Port;
    cfg.stairButton.pin = STAIR_BUTTON_Pin;
    cfg.stairButton.activeState = GPIO_PIN_RESET;
#endif

#ifdef RESET_BUTTON_Pin
    cfg.resetButton.port = RESET_BUTTON_GPIO_Port;
    cfg.resetButton.pin = RESET_BUTTON_Pin;
    cfg.resetButton.activeState = GPIO_PIN_RESET;
#endif

    /*
     * Replace this PCA9685 channel map with your schematic.
     * Defaults:
     * Mode RGB  = 0,1,2
     * Stair RGB = 3,4,5
     * Reset RGB = 6,7,8
     */
    cfg.leds.modeRed = 0U;
    cfg.leds.modeGreen = 1U;
    cfg.leds.modeBlue = 2U;

    cfg.leds.stairRed = 3U;
    cfg.leds.stairGreen = 4U;
    cfg.leds.stairBlue = 5U;

    cfg.leds.resetRed = 6U;
    cfg.leds.resetGreen = 7U;
    cfg.leds.resetBlue = 8U;

    return cfg;
}

static std::uint32_t lastPrintMs = 0U;
static constexpr std::uint32_t PRINT_PERIOD_MS = 500U;

} // anonymous namespace

void runControlClusterInit(void)
{
    ControlClusterConfig cfg = makeConfig();

    bool ok = ControlCluster::init(cfg);

    printf("Control cluster init: %s\r\n", ok ? "OK" : "FAILED");
}

void runControlClusterTick(void)
{
    ControlCluster::tick();

    const std::uint32_t now = HAL_GetTick();

    if ((now - lastPrintMs) >= PRINT_PERIOD_MS)
    {
        lastPrintMs = now;

        ControlClusterState control = SystemState::getControlClusterState();

        printf("CONTROL | mode: %u | stair: %u | reset: %u | valid: %u\r\n",
               static_cast<unsigned int>(static_cast<std::uint8_t>(control.mode)),
               control.stairConfirm ? 1U : 0U,
               control.resetRequest ? 1U : 0U,
               control.valid ? 1U : 0U);
    }
}
