/*
 * UserBrakes.hpp
 *
 *  Created on: May 16, 2026
 *      Author: otienom
 */

#ifndef SRC_APP_USER_BRAKES_USERBRAKES_HPP_
#define SRC_APP_USER_BRAKES_USERBRAKES_HPP_

namespace user_brakes {

class UserBrakes {
public:
    static void init();
    static void tick();

private:
    static float readLeftBrake();
    static float readRightBrake();
};

} // namespace user_brakes

#endif
