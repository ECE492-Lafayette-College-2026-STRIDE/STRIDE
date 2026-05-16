/*
 * FlashState.cpp
 *
 *  Created on: May 16, 2026
 *      Author: otienom
 */

#include "flash_state.hpp"
#include "../System_state/system_state.hpp"

extern "C" {
#include "main.h"
}

#include <cstring>
#include <cmath>

namespace flash_state {

static constexpr std::uint32_t FLASH_STATE_ADDRESS = 0x081E0000U;
static constexpr std::uint32_t FLASH_STATE_BANK = FLASH_BANK_2;
static constexpr std::uint32_t FLASH_STATE_SECTOR = FLASH_SECTOR_7;

static constexpr std::uint32_t FLASH_WORD_BYTES = 32U;

float FlashState::lastSavedPercent_ = -1000.0f;

bool FlashState::init()
{
    return loadBatteryCapacityToSystemState();
}

bool FlashState::loadBatteryCapacityToSystemState()
{
    BatteryFlashRecord record{};

    if (!readRecord(record)) {
        return false;
    }

    auto battery = system_state::SystemState::getBatteryState();

    /*
     * flashCapacity_mAh is the restored battery capacity from flash.
     * The battery monitor can use this value during startup configuration.
     */
    battery.flashCapacity_mAh = record.remainingCapacity_mAh;
    battery.percent = record.batteryPercent;
    battery.valid = true;

    system_state::SystemState::setBatteryState(battery);

    lastSavedPercent_ = record.batteryPercent;

    return true;
}

bool FlashState::tick()
{
    const auto battery = system_state::SystemState::getBatteryState();

    if (!battery.valid) {
        return false;
    }

    const float currentPercent = battery.percent;

    if (std::fabs(currentPercent - lastSavedPercent_) < PERCENT_SAVE_DELTA) {
        return false;
    }

    BatteryFlashRecord record{};

    record.magic = MAGIC;
    record.version = VERSION;
    record.remainingCapacity_mAh = battery.realtimeCapacity_mAh;
    record.batteryPercent = battery.percent;
    record.checksum = 0U;
    record.checksum = checksum(record);

    if (!writeRecord(record)) {
        return false;
    }

    lastSavedPercent_ = currentPercent;

    return true;
}

bool FlashState::readRecord(BatteryFlashRecord& record)
{
    std::memcpy(
        &record,
        reinterpret_cast<const void*>(FLASH_STATE_ADDRESS),
        sizeof(BatteryFlashRecord)
    );

    return isValid(record);
}

bool FlashState::writeRecord(BatteryFlashRecord record)
{
    record.magic = MAGIC;
    record.version = VERSION;
    record.checksum = 0U;
    record.checksum = checksum(record);

    alignas(32) std::uint8_t flashWord[FLASH_WORD_BYTES];
    std::memset(flashWord, 0xFF, sizeof(flashWord));
    std::memcpy(flashWord, &record, sizeof(BatteryFlashRecord));

    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef eraseInit{};
    std::uint32_t sectorError = 0U;

    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.Banks = FLASH_STATE_BANK;
    eraseInit.Sector = FLASH_STATE_SECTOR;
    eraseInit.NbSectors = 1U;

    if (HAL_FLASHEx_Erase(&eraseInit, &sectorError) != HAL_OK) {
        HAL_FLASH_Lock();
        return false;
    }

    const HAL_StatusTypeDef status = HAL_FLASH_Program(
        FLASH_TYPEPROGRAM_FLASHWORD,
        FLASH_STATE_ADDRESS,
        reinterpret_cast<std::uint32_t>(flashWord)
    );

    HAL_FLASH_Lock();

    return status == HAL_OK;
}

bool FlashState::isValid(const BatteryFlashRecord& record)
{
    if (record.magic != MAGIC) {
        return false;
    }

    if (record.version != VERSION) {
        return false;
    }

    if (record.remainingCapacity_mAh < 0.0f || record.remainingCapacity_mAh > 100000.0f) {
        return false;
    }

    if (record.batteryPercent < 0.0f || record.batteryPercent > 100.0f) {
        return false;
    }

    return record.checksum == checksum(record);
}

std::uint32_t FlashState::checksum(const BatteryFlashRecord& record)
{
    BatteryFlashRecord temp = record;
    temp.checksum = 0U;

    const auto* bytes = reinterpret_cast<const std::uint8_t*>(&temp);

    std::uint32_t sum = 0U;

    for (std::uint32_t i = 0U; i < sizeof(BatteryFlashRecord); ++i) {
        sum = (sum << 5U) ^ (sum >> 27U) ^ bytes[i];
    }

    return sum;
}

} // namespace flash_state
