/*
 * run_ultrasonic.cpp
 *
 *  Created on: May 18, 2026
 *      Author: otienom
 *
 *  Test runner for ultrasonic receiver.
 *  Prints ultrasonic sensor state to serial terminal.
 */

extern "C" {
#include "main.h"
}

#include "run_ultrasonic.hpp"
#include "ultrasonic.hpp"
#include "../System_State/system_state.hpp"

#include <cstdint>
#include <cstdio>

using namespace system_state;

namespace {

static std::uint32_t last_print_ms_ = 0U;

/*
 * Print period for testing.
 * Change this if you want faster/slower terminal output.
 */
static constexpr std::uint32_t ULTRASONIC_PRINT_PERIOD_MS = 500U;

} // anonymous namespace

void runUltrasonicInit(void)
{
    /*
     * Starts USART1 one-byte interrupt reception for the ultrasonic MCU.
     * Make sure MX_USART1_UART_Init() has already been called in main().
     */
    UltrasonicReceiver::init();

    printf("Ultrasonic test runner initialized\r\n");
}

void runUltrasonicTick(void)
{
    /*
     * Handles stale/timeout detection.
     * Actual byte receive is interrupt-driven through HAL_UART_RxCpltCallback().
     */
    UltrasonicReceiver::process();

    const std::uint32_t now = HAL_GetTick();

    if ((now - last_print_ms_) >= ULTRASONIC_PRINT_PERIOD_MS)
    {
        last_print_ms_ = now;

        SensorState sensors = SystemState::getSensorState();

        printf("ULTRASONIC | distance_mm: %.1f | status: %u | valid: %u\r\n",
               static_cast<double>(sensors.frontDistance),
               static_cast<unsigned int>(sensors.frontStatus),
               sensors.valid ? 1U : 0U);
    }
}
