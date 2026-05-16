/*
 * Motor_run.cpp
 *
 *  Created on: Mar 4, 2026
 *      Author: otienom
 */

#include "Motors.hpp"
#include "main.h"
#include <cstdint>

namespace motors {

static Motors left_(Motors::Channel::Left);
static Motors right_(Motors::Channel::Right);

static std::uint32_t start_ms_ = 0;
static bool started_ = false;

void Motors::initRunner()
{
    (void)Motors::init();

    // Safe startup state
    left_.writeMotor(0.0f);
    right_.writeMotor(0.0f);

    left_.setDirection(false);
    right_.setDirection(false);

    left_.setBrake(true);
    right_.setBrake(true);

    left_.setEnable(false);
    right_.setEnable(false);

    start_ms_ = HAL_GetTick();
    started_ = true;
}

void Motors::tick_Motors()
{
    if (!started_) {
        return;
    }

    //const std::uint32_t elapsed = HAL_GetTick() - start_ms_;

    // Hold safe idle state for first 2 seconds
//    if (elapsed < 2000U) {
//
//        left_.writeMotor(0.0f);
//        right_.writeMotor(0.0f);
//
//        left_.setEnable(false);
//        right_.setEnable(false);
//
//        left_.setBrake(true);
//        right_.setBrake(true);
//
//        return;
//    }

    // Simple movement test
   left_.setDirection(true);
    right_.setDirection(true);

    left_.setBrake(false);
    right_.setBrake(false);

    left_.setEnable(false);
    right_.setEnable(false);

   left_.writeMotor(0.5f);
    right_.writeMotor(0.5f);
}

} // namespace motors












///*
// * Motor_run.cpp
// *
// *  Created on: Mar 4, 2026
// *      Author: otienom
// */
//
//#include "Motors.hpp"
//#include "main.h"
//#include <cstdint>
//
//namespace motors {
//
//enum class RunState : std::uint8_t {
//    Default,
//    Enable,
//    Drive,
//    Fault
//};
//
//static Motors left_(Motors::Channel::Left);
//static Motors right_(Motors::Channel::Right);
//
//static RunState state_ = RunState::Default;
//
////// cached requested values
////static bool enable_ = false;
////static bool brake_ = true;
////static bool direction_ = false;
////static std::uint32_t speed_mph_ = 0;
//
//static std::uint32_t state_enter_ms_ = 0;
//static constexpr std::uint32_t STATE_DWELL_MS = 5000;
//
//static inline void enterState(RunState new_state)
//{
//    state_ = new_state;
//    state_enter_ms_ = HAL_GetTick();
//}
//
//static inline bool stateElapsed(std::uint32_t dwell_ms)
//{
//    return (HAL_GetTick() - state_enter_ms_) >= dwell_ms;
//}
//
//
//
//void motors::Motors::initRunner()
//{
//    (void)Motors::init();
//
//    left_.setEnable(true);
//    right_.setEnable(true);
//
//    left_.setBrake(false);
//    right_.setBrake(false);
//
//    left_.setDirection(false);
//    right_.setDirection(false);
//
//    left_.writeMotor(1.0f);
//     right_.writeMotor(1.0f);
//
//   enterState(RunState::Default);
//}
//
//void motors::Motors::tick_Motors()
//{
//    (void)left_.readFault();
//    (void)right_.readFault();
//
//    if (left_.faultCached() & right_.faultCached()) {
//        enterState(RunState::Fault);
//    }
//
//    switch (state_) {
//    case RunState::Default:
//    {
//
//
//    	 left_.setEnable(true);
//    	  right_.setEnable(true);
//
//    	    left_.setBrake(false);
//    	    right_.setBrake(false);
//
//    	    left_.setDirection(false);
//    	    right_.setDirection(false);
//
//    	    left_.writeMotor(1.0f);
//    	     right_.writeMotor(1.0f);
//
//        // only leave default if enable request is true
//        if (!left_.enableCached() && !right_.enableCached() && stateElapsed(STATE_DWELL_MS*6)) {
//            enterState(RunState::Enable);
//        }
//        break;
//    }
//
//    case RunState::Enable:
//    {
//    	 left_.setEnable(true);
//    	    right_.setEnable(true);
//
//    	    left_.setBrake(false);
//    	    right_.setBrake(false);
//
//    	    left_.setDirection(false);
//    	    right_.setDirection(false);
//
//    	    left_.writeMotor(1.0f);
//    	     right_.writeMotor(1.0f);
//
//        // if command no longer valid, go back to default
//        if (!left_.enableCached() && !right_.enableCached()) {
//            enterState(RunState::Default);
//        }
//        // after 5 s, only go to Drive if brake request is false
//        else if (!left_.brakeCached() && !right_.brakeCached()&& stateElapsed(STATE_DWELL_MS*6)) {
//            enterState(RunState::Drive);
//        }
//        break;
//    }
//
//    case RunState::Drive:
//    {
//    left_.setEnable(true);
//    right_.setEnable(true);
//
//    left_.setBrake(false);
//    right_.setBrake(false);
//
//    left_.setDirection(false);
//    right_.setDirection(false);
//
//    left_.writeMotor(1.0f);
//    right_.writeMotor(1.0f);
//        //left_.writeMotor(left_.speedToVolts(left_.commandedSpeedCached()));
//      //  right_.writeMotor(right_.speedToVolts(right_.commandedSpeedCached()));
//
//        // if enable becomes false, go safe
//        if (!left_.enableCached() || !right_.enableCached() ) {
//            enterState(RunState::Default);
//        }
//        // if brake becomes true, return to enable state
//        else if (left_.brakeCached() || right_.brakeCached()) {
//            enterState(RunState::Enable);
//        }
//        break;
//    }
//
//    case RunState::Fault:
//    {
//    	 left_.setEnable(true);
//    	  right_.setEnable(true);
//
//    	 left_.setBrake(false);
//    	 right_.setBrake(false);
//
//         left_.setDirection(false);
//    	  right_.setDirection(false);
//
//    	 left_.writeMotor(1.0f);
//         right_.writeMotor(1.0f);
//
//        //(void)left_.readFault();
//      //  (void)right_.readFault();
//
//        if (!left_.faultCached() && !right_.faultCached()) {
//           enterState(RunState::Default);
//        }
//        break;
//    }
//
//    default:
//        enterState(RunState::Fault);
//        break;
//    }
//}
//
//} // namespace motors
