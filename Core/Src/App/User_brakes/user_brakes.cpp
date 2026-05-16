/*
 * UserBrakes.cpp
 *
 *  Created on: May 16, 2026
 *      Author: otienom
 */

#include "user_brakes.hpp"
#include "../System_State/system_state.hpp"

extern "C" {
#include "main.h"
}

namespace user_brakes {

void UserBrakes::init()
{
    tick();
}

void UserBrakes::tick()
{
    system_state::UserInputState input{};

    input.leftUserBrake = readLeftBrake();
    input.rightUserBrake = readRightBrake();

    input.combinedUserBrake =
        (input.leftUserBrake > input.rightUserBrake)
        ? input.leftUserBrake
        : input.rightUserBrake;

    input.emergencyStop = false;
    input.valid = true;

    system_state::SystemState::setUserInputState(input);
}

float UserBrakes::readLeftBrake()
{
    return HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_7) == GPIO_PIN_SET ? 1.0f : 0.0f;
}

float UserBrakes::readRightBrake()
{
    return HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_11) == GPIO_PIN_SET ? 1.0f : 0.0f;
}

} // namespace user_brakes
