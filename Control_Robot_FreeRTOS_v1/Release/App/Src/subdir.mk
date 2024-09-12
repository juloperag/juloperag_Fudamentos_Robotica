################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/Src/MPUaccel.c \
../App/Src/MotorDriver.c \
../App/Src/PositionRobot.c \
../App/Src/main.c \
../App/Src/task_handler.c 

OBJS += \
./App/Src/MPUaccel.o \
./App/Src/MotorDriver.o \
./App/Src/PositionRobot.o \
./App/Src/main.o \
./App/Src/task_handler.o 

C_DEPS += \
./App/Src/MPUaccel.d \
./App/Src/MotorDriver.d \
./App/Src/PositionRobot.d \
./App/Src/main.d \
./App/Src/task_handler.d 


# Each subdirectory must supply rules for building sources it contributes
App/Src/%.o App/Src/%.su: ../App/Src/%.c App/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DNUCLEO_F411RE -DSTM32 -DSTM32F4 -DSTM32F411RETx -c -I../Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-Src

clean-App-2f-Src:
	-$(RM) ./App/Src/MPUaccel.d ./App/Src/MPUaccel.o ./App/Src/MPUaccel.su ./App/Src/MotorDriver.d ./App/Src/MotorDriver.o ./App/Src/MotorDriver.su ./App/Src/PositionRobot.d ./App/Src/PositionRobot.o ./App/Src/PositionRobot.su ./App/Src/main.d ./App/Src/main.o ./App/Src/main.su ./App/Src/task_handler.d ./App/Src/task_handler.o ./App/Src/task_handler.su

.PHONY: clean-App-2f-Src

