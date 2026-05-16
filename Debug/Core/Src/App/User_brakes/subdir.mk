################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/Src/App/User_brakes/user_brakes.cpp 

OBJS += \
./Core/Src/App/User_brakes/user_brakes.o 

CPP_DEPS += \
./Core/Src/App/User_brakes/user_brakes.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/App/User_brakes/%.o Core/Src/App/User_brakes/%.su Core/Src/App/User_brakes/%.cyclo: ../Core/Src/App/User_brakes/%.cpp Core/Src/App/User_brakes/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_PWR_DIRECT_SMPS_SUPPLY -DUSE_NUCLEO_64 -DUSE_HAL_DRIVER -DSTM32H7A3xxQ -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/BSP/STM32H7xx_Nucleo -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-App-2f-User_brakes

clean-Core-2f-Src-2f-App-2f-User_brakes:
	-$(RM) ./Core/Src/App/User_brakes/user_brakes.cyclo ./Core/Src/App/User_brakes/user_brakes.d ./Core/Src/App/User_brakes/user_brakes.o ./Core/Src/App/User_brakes/user_brakes.su

.PHONY: clean-Core-2f-Src-2f-App-2f-User_brakes

