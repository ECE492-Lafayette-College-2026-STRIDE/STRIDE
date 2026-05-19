/*
 * run_jetson.hpp
 *
 *  Created on: May 19, 2026
 *      Author: otienom
 */

#ifndef SRC_APP_JETSON_RUN_JETSON_HPP_
#define SRC_APP_JETSON_RUN_JETSON_HPP_

namespace jetson {

/**
 * @brief Initialize the Jetson runner.
 *
 * This initializes the Jetson UART driver and starts interrupt-based receive.
 */
void runJetsonInit();

/**
 * @brief Run the Jetson communication loop.
 *
 * This should be called repeatedly from globalTick().
 * It processes completed UART commands and sends feedback from SystemState
 * back to Jetson.
 */
void runJetsonTick();

/**
 * @brief Handle USART6 RX complete interrupt for the Jetson UART.
 *
 * This is called from HAL_UART_RxCpltCallback().
 */
void runJetsonHandleRxComplete();

} // namespace jetson

#endif /* SRC_APP_JETSON_RUN_JETSON_HPP_ */
