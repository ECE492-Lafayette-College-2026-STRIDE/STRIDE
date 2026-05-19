/*
 * motor_run.cpp
 *
 *  Created on: Mar 4, 2026
 *      Author: otienom
 *
 *  Purpose:
 *  Motor runner for STRIDE.
 *
 *  This file does not create motor commands.
 *  It only runs motor objects so they can:
 *  - Pull commands from SystemState
 *  - Apply commands to motor hardware
 *  - Read motor feedback
 *  - Push feedback back to SystemState
 */

#include "Motor_run.hpp"
#include "Motors.hpp"
#include "main.h"
#include "tim.h"
#include <cstdint>

namespace motors {

// Motor objects are created here.
// They represent physical motor channels.
static Motors leftMotor_(Motors::Channel::Left);
static Motors rightMotor_(Motors::Channel::Right);

static bool initialized_ = false;

// TIM15 is on APB2. From your current clock tree, APB2 is not divided,
// so TIM15 counter clock is 64 MHz when Prescaler = 0.
static constexpr std::uint32_t RIGHT_TACH_TIMER_CLOCK_HZ = 64000000U;

void RunMotors_Init()
{
  /*
   * Initialize shared motor hardware.
   *
   * This starts DAC channels and forces DAC outputs to 0 V through Motors::init().
   * It does not write commands to SystemState.
   */
  (void)Motors::init();

  /*
   * Start right tach input capture interrupt.
   *
   * Right tach:
   * PE6 -> TIM15_CH2
   */
  (void)HAL_TIM_IC_Start_IT(&htim15, TIM_CHANNEL_2);

  initialized_ = true;
}

void RunMotors_Tick()
{
  if (!initialized_) {
    return;
  }

  /*
   * Each motor object:
   * - Pulls its own command from SystemState
   * - Applies DAC/GPIO outputs
   * - Reads fault GPIO
   * - Pushes fault feedback to SystemState
   */
  leftMotor_.tick();
  rightMotor_.tick();
}

void RunMotors_RightTachCapture(TIM_HandleTypeDef* htim,
                                std::uint32_t timerChannel,
                                std::uint32_t timerClockHz)
{
  if (!initialized_) {
    return;
  }

  /*
   * Right tach only for now.
   *
   * This reads the timer input-capture value inside the motor driver,
   * calculates measured speed in mph, and pushes it to SystemState.
   */
  rightMotor_.readTachSpeedAndPushToSystemState(
      htim,
      timerChannel,
      timerClockHz);
}

} // namespace motors

extern "C" void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef* htim)
{
  if (htim->Instance == TIM15 &&
      htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
  {
    motors::RunMotors_RightTachCapture(
        htim,
        TIM_CHANNEL_2,
        motors::RIGHT_TACH_TIMER_CLOCK_HZ);
  }
}
