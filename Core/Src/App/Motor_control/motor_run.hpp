#pragma once

#include "stm32h7xx_hal.h"
#include <cstdint>

namespace motors {

/**
 * @brief Initialize motor runner.
 *
 * This initializes motor hardware only. It does not create commands.
 * Commands must already exist in SystemState.
 */
void RunMotors_Init();

/**
 * @brief Run motor update loop.
 *
 * This only calls each motor object's tick() function.
 * Each motor object pulls commands from SystemState and pushes fault feedback
 * back to SystemState.
 */
void RunMotors_Tick();

/**
 * @brief Read right tach input-capture speed and update SystemState.
 *
 * This function should be called from the right tach timer input-capture
 * callback. It does not command the motor.
 *
 * @param htim Timer handle used for tach input capture.
 * @param timerChannel HAL timer channel.
 * @param timerClockHz Timer counter clock after prescaler.
 */
void RunMotors_RightTachCapture(TIM_HandleTypeDef* htim,
                                std::uint32_t timerChannel,
                                std::uint32_t timerClockHz);

} // namespace motors
