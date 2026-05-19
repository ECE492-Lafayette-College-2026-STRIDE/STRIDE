/*
 * ultrasonic.hpp
 *
 *  Created on: May 18, 2026
 *      Author: otienom
 *
 *  Receives ultrasonic sensor string from the sensor MCU and updates SystemState.
 *
 *  Expected UART message:
 *  ultrasonic distance:350,ultrasonic status:1\r\n
 */

#ifndef SRC_APP_ULTRASONIC_SENSORS_ULTRASONIC_HPP_
#define SRC_APP_ULTRASONIC_SENSORS_ULTRASONIC_HPP_

#include "stm32h7xx_hal.h"
#include <cstdint>

class UltrasonicReceiver {
public:
    /*
     * Initializes the ultrasonic UART receiver.
     * Call this once after MX_USART1_UART_Init() and SystemState::init().
     */
    static void init();

    /*
     * Call this from HAL_UART_RxCpltCallback().
     */
    static void rxCompleteCallback(UART_HandleTypeDef *huart);

    /*
     * Call this from HAL_UART_ErrorCallback().
     */
    static void errorCallback(UART_HandleTypeDef *huart);

    /*
     * Call this repeatedly inside globalTick() or the main while loop.
     * This handles stale/timeout detection.
     */
    static void process();

private:
    static void restartReceive();
    static void handleReceivedByte(std::uint8_t byte);

    static bool parseLine(const char *line,
                          std::uint16_t *distance_mm,
                          std::uint8_t *status);

    static void updateSystemState(std::uint16_t distance_mm,
                                  std::uint8_t status);
};

#endif /* SRC_APP_ULTRASONIC_SENSORS_ULTRASONIC_HPP_ */
