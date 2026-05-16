#include "Battery_status.hpp"
#include "../System_state/system_state.hpp"

#include "i2c.h"
#include "stm32h7xx_nucleo.h"
#include <cstdio>

float totalCharge = 0.0f; // mAh
float totalPower  = 0.0f; // W

static battery::BatteryStatusLTC2944 battery_(&hi2c1, 0.001f);
static bool inited_ = false;

static constexpr float DEFAULT_BATTERY_CAPACITY_MAH = 30000.0f;

extern "C" int _write(int file, char *ptr, int len)
{
    (void)file;

    HAL_UART_Transmit(&hcom_uart[COM1],
                      reinterpret_cast<uint8_t*>(ptr),
                      static_cast<uint16_t>(len),
                      HAL_MAX_DELAY);

    return len;
}

void battery::BatteryStatusLTC2944::batteryInit()
{
    if (inited_) return;

    if (!battery_.init(battery::BatteryStatusLTC2944::AdcMode::Scan10s,
                       battery::BatteryStatusLTC2944::Prescaler::M256,
                       battery::BatteryStatusLTC2944::AlccMode::Alert,
                       false))
    {
        printf("LTC2944 init failed\r\n");
        return;
    }

    auto batteryState = system_state::SystemState::getBatteryState();

    float startupCapacity_mAh = DEFAULT_BATTERY_CAPACITY_MAH;

    if (batteryState.flashCapacity_mAh > 0.0f &&
        batteryState.flashCapacity_mAh <= DEFAULT_BATTERY_CAPACITY_MAH)
    {
        startupCapacity_mAh = batteryState.flashCapacity_mAh;
    }

    battery_.setAccumulatedCharge_mAh(startupCapacity_mAh);

    battery_.setVoltageThresholds(3.0f, 4.5f);
    battery_.setCurrentThresholds(-1.0f, 1.0f);

    batteryState.realtimeCapacity_mAh = startupCapacity_mAh;
    batteryState.percent = (startupCapacity_mAh / DEFAULT_BATTERY_CAPACITY_MAH) * 100.0f;
    batteryState.valid = true;

    system_state::SystemState::setBatteryState(batteryState);

    inited_ = true;

    printf("LTC2944 init ok\r\n");
    printf("Startup capacity: %.2f mAh\r\n", startupCapacity_mAh);
}

bool battery::BatteryStatusLTC2944::tick_read(
    battery::BatteryStatusLTC2944::Measurements& battery_measurements)
{
    if (!inited_) batteryInit();
    if (!inited_) return false;

    uint8_t raw_status = 0;
    battery::BatteryStatusLTC2944::StatusBits s{};

    if (!battery_.readStatus(raw_status, s))
    {
        printf("readStatus failed\r\n");
        return false;
    }

    auto batteryState = system_state::SystemState::getBatteryState();

    if (batteryState.resetCapacityToFullRequest) {
        battery_.setAccumulatedCharge_mAh(30000.0f);

        batteryState.realtimeCapacity_mAh = 30000.0f;
        batteryState.flashCapacity_mAh = 30000.0f;
        batteryState.percent = 100.0f;
        batteryState.resetCapacityToFullRequest = false;
        batteryState.valid = true;

        system_state::SystemState::setBatteryState(batteryState);
    }
    if (!battery_.readAll(battery_measurements))
    {
        printf("readAll failed\r\n");
        return false;
    }

    totalCharge = battery_measurements.acr_mAh;
    totalPower  = battery_measurements.power;

    batteryState.voltage = battery_measurements.v_batt_V;
    batteryState.current = battery_measurements.i_batt_A;
    batteryState.power = battery_measurements.power;

    batteryState.realtimeCapacity_mAh = battery_measurements.acr_mAh;
    batteryState.percent = battery_measurements.capacity_percent;

    batteryState.valid = true;

    system_state::SystemState::setBatteryState(batteryState);

    uint32_t time_s = HAL_GetTick() / 1000;

    printf("%lu,%.3f,%.3f,%.3f,%.3f,%.3f\r\n",
           time_s,
           battery_measurements.v_batt_V,
           battery_measurements.i_batt_A,
           battery_measurements.power,
           battery_measurements.acr_mAh,
           battery_measurements.capacity_percent);

    return true;
}
