################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/Src/App/Battery_Monitor/Battery_read.cpp \
../Core/Src/App/Battery_Monitor/Battery_status.cpp 

OBJS += \
./Core/Src/App/Battery_Monitor/Battery_read.o \
./Core/Src/App/Battery_Monitor/Battery_status.o 

CPP_DEPS += \
./Core/Src/App/Battery_Monitor/Battery_read.d \
./Core/Src/App/Battery_Monitor/Battery_status.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/App/Battery_Monitor/%.o Core/Src/App/Battery_Monitor/%.su Core/Src/App/Battery_Monitor/%.cyclo: ../Core/Src/App/Battery_Monitor/%.cpp Core/Src/App/Battery_Monitor/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_PWR_DIRECT_SMPS_SUPPLY -DUSE_NUCLEO_64 -DUSE_HAL_DRIVER -DSTM32H7A3xxQ -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/BSP/STM32H7xx_Nucleo -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-App-2f-Battery_Monitor

clean-Core-2f-Src-2f-App-2f-Battery_Monitor:
	-$(RM) ./Core/Src/App/Battery_Monitor/Battery_read.cyclo ./Core/Src/App/Battery_Monitor/Battery_read.d ./Core/Src/App/Battery_Monitor/Battery_read.o ./Core/Src/App/Battery_Monitor/Battery_read.su ./Core/Src/App/Battery_Monitor/Battery_status.cyclo ./Core/Src/App/Battery_Monitor/Battery_status.d ./Core/Src/App/Battery_Monitor/Battery_status.o ./Core/Src/App/Battery_Monitor/Battery_status.su

.PHONY: clean-Core-2f-Src-2f-App-2f-Battery_Monitor

