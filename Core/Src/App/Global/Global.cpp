/*
 * global.cpp
 *
 *  Created on: Mar 5, 2026
 *      Author: otienom
 */

extern "C" {
#include "main.h"
}

#include "Global.h"
#include "../System_State/System_state.hpp"
#include "../System_State/System_state_debug.hpp"
#include "../Battery_Monitor/Battery_status.hpp"
#include "../Motor_control/Motor_run.hpp"
#include "../Jetson/run_jetson.hpp"
#include "../Ultrasonic_Sensors/run_ultrasonic.hpp"
#include "../Control_cluster/run_control_cluster.hpp"

static battery::BatteryStatusLTC2944::Measurements battery_measurements_{};
static std::uint32_t last_battery_read_ms_ = 0U;
static constexpr std::uint32_t BATTERY_READ_PERIOD_MS = 500U;

void globalInit(void)
{
    // Initialize SystemState first.
    // Every driver reads from or writes to this shared state.
    system_state::SystemState::init();

    battery::BatteryStatusLTC2944::batteryInit();

    // Initialize motor runner.
    // This initializes motor hardware only.
    // Motor commands must be provided through SystemState.
    motors::RunMotors_Init();

    // Initialize Jetson communication runner.
    // This receives commands from Jetson and sends SystemState feedback.
    jetson::runJetsonInit();

    // Initialize SystemState debug printing over COM1 serial terminal.
    system_state_debug::init();

    runControlClusterInit();
    runUltrasonicInit();
}

void globalTick(void)
{
    // Receive Jetson commands and send feedback from SystemState.
    // This should run before motors so new Jetson commands are available
    // to the motor driver in the same loop.
    jetson::runJetsonTick();

    // Motor objects pull commands from SystemState and push feedback/status back.
    motors::RunMotors_Tick();

    // Run ultrasonic receiver task.
    runUltrasonicTick();

    // Run control cluster task.
    runControlClusterTick();

    // Temporarily disabled while debugging Jetson UART.
    // system_state_debug::tick(1000U);

    const std::uint32_t now = HAL_GetTick();

    // Read battery periodically.
    if ((now - last_battery_read_ms_) >= BATTERY_READ_PERIOD_MS) {
        last_battery_read_ms_ = now;
        battery::BatteryStatusLTC2944::tick_read(battery_measurements_);
    }
}

