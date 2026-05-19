################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/Src/App/Ultrasonic_sensors/run_ultrasonic.cpp \
../Core/Src/App/Ultrasonic_sensors/ultrasonic.cpp 

OBJS += \
./Core/Src/App/Ultrasonic_sensors/run_ultrasonic.o \
./Core/Src/App/Ultrasonic_sensors/ultrasonic.o 

CPP_DEPS += \
./Core/Src/App/Ultrasonic_sensors/run_ultrasonic.d \
./Core/Src/App/Ultrasonic_sensors/ultrasonic.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/App/Ultrasonic_sensors/%.o Core/Src/App/Ultrasonic_sensors/%.su Core/Src/App/Ultrasonic_sensors/%.cyclo: ../Core/Src/App/Ultrasonic_sensors/%.cpp Core/Src/App/Ultrasonic_sensors/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_PWR_DIRECT_SMPS_SUPPLY -DUSE_NUCLEO_64 -DUSE_HAL_DRIVER -DSTM32H7A3xxQ -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/BSP/STM32H7xx_Nucleo -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-App-2f-Ultrasonic_sensors

clean-Core-2f-Src-2f-App-2f-Ultrasonic_sensors:
	-$(RM) ./Core/Src/App/Ultrasonic_sensors/run_ultrasonic.cyclo ./Core/Src/App/Ultrasonic_sensors/run_ultrasonic.d ./Core/Src/App/Ultrasonic_sensors/run_ultrasonic.o ./Core/Src/App/Ultrasonic_sensors/run_ultrasonic.su ./Core/Src/App/Ultrasonic_sensors/ultrasonic.cyclo ./Core/Src/App/Ultrasonic_sensors/ultrasonic.d ./Core/Src/App/Ultrasonic_sensors/ultrasonic.o ./Core/Src/App/Ultrasonic_sensors/ultrasonic.su

.PHONY: clean-Core-2f-Src-2f-App-2f-Ultrasonic_sensors

