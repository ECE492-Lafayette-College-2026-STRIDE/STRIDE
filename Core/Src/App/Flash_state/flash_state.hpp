/*
 * FlashState.hpp
 *
 *  Created on: May 16, 2026
 *      Author: otienom
 */

#ifndef SRC_APP_FLASH_STATE_FLASHSTATE_HPP_
#define SRC_APP_FLASH_STATE_FLASHSTATE_HPP_

#include <cstdint>

namespace flash_state {

class FlashState {
public:
    static bool init();

    static bool loadBatteryCapacityToSystemState();
    static bool tick();

private:
    struct BatteryFlashRecord {
        std::uint32_t magic;
        std::uint32_t version;
        float remainingCapacity_mAh;
        float batteryPercent;
        std::uint32_t checksum;
    };

    static constexpr std::uint32_t MAGIC = 0x53545244U;   // "STRD"
    static constexpr std::uint32_t VERSION = 1U;

    static constexpr float PERCENT_SAVE_DELTA = 1.0f;

    static bool readRecord(BatteryFlashRecord& record);
    static bool writeRecord(BatteryFlashRecord record);
    static bool isValid(const BatteryFlashRecord& record);
    static std::uint32_t checksum(const BatteryFlashRecord& record);

    static float lastSavedPercent_;
};

} // namespace flash_state

#endif
