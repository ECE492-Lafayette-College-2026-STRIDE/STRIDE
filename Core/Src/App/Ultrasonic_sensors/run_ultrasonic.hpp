/*
 * run_ultrasonic.hpp
 *
 *  Created on: May 18, 2026
 *      Author: otienom
 *
 *  Test runner for ultrasonic receiver.
 *  This file is called from globalInit() and globalTick().
 */

#ifndef SRC_APP_ULTRASONIC_SENSORS_RUN_ULTRASONIC_HPP_
#define SRC_APP_ULTRASONIC_SENSORS_RUN_ULTRASONIC_HPP_

#ifdef __cplusplus
extern "C" {
#endif

void runUltrasonicInit(void);
void runUltrasonicTick(void);

#ifdef __cplusplus
}
#endif

#endif /* SRC_APP_ULTRASONIC_SENSORS_RUN_ULTRASONIC_HPP_ */
