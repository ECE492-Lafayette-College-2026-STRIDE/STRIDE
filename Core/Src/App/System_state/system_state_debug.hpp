/*
 * System_state_debug.hpp
 *
 *  Created on: May 19, 2026
 *      Author: otienom
 */

#ifndef SRC_APP_SYSTEM_STATE_SYSTEM_STATE_DEBUG_HPP_
#define SRC_APP_SYSTEM_STATE_SYSTEM_STATE_DEBUG_HPP_

#include <cstdint>

namespace system_state_debug {

/**
 * @brief Initialize the SystemState debug printer.
 *
 * This does not initialize SystemState itself.
 * It only initializes the print timing.
 */
void init();

/**
 * @brief Print SystemState periodically to the COM1 serial terminal.
 *
 * Call this repeatedly from globalTick().
 *
 * @param periodMs Print period in milliseconds.
 */
void tick(std::uint32_t periodMs = 1000U);

/**
 * @brief Print SystemState once to the COM1 serial terminal.
 */
void printOnce();

} // namespace system_state_debug

#endif /* SRC_APP_SYSTEM_STATE_SYSTEM_STATE_DEBUG_HPP_ */
