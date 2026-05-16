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
#include "../Battery_Monitor/Battery_status.hpp"
#include "../Motor_control/Motors.hpp"

static battery::BatteryStatusLTC2944::Measurements battery_measurements_{};
static std::uint32_t last_battery_read_ms_ = 0U;
static constexpr std::uint32_t BATTERY_READ_PERIOD_MS = 500U;

void globalInit(void)
{
	battery::BatteryStatusLTC2944::batteryInit();
	motors::Motors::initRunner();

}

void globalTick(void)
{
    // Keep motor FSM running every loop
	 motors::Motors::tick_Motors();

    const std::uint32_t now = HAL_GetTick();
    // Read battery periodically
    if ((now - last_battery_read_ms_) >= BATTERY_READ_PERIOD_MS) {
        last_battery_read_ms_ = now;
        battery::BatteryStatusLTC2944::tick_read(battery_measurements_);
    }
}
