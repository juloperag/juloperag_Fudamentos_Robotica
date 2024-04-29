################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/Src/MotorDriver.c \
../App/Src/main.c 

OBJS += \
./App/Src/MotorDriver.o \
./App/Src/main.o 

C_DEPS += \
./App/Src/MotorDriver.d \
./App/Src/main.d 


# Each subdirectory must supply rules for building sources it contributes
App/Src/%.o App/Src/%.su: ../App/Src/%.c App/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F411RE -DSTM32 -DSTM32F4 -DSTM32F411RETx -DSTM32F411xE -c -I"B:/Documentos/Universidad-Materias/Fundamentos de Robotica/Robotic/WorkSpaces/Control_Robot_v3.6_Control_Jaco/App/Inc" -I"B:/Documentos/Universidad-Materias/Fundamentos de Robotica/Robotic/WorkSpaces/Control_Robot_v3.6_Control_Jaco/Core/Include" -I"B:/Documentos/Universidad-Materias/Fundamentos de Robotica/Robotic/WorkSpaces/Control_Robot_v3.6_Control_Jaco/Core/Device/ST/STM32F4xx/Include" -I"B:/Documentos/Universidad-Materias/Fundamentos de Robotica/Robotic/WorkSpaces/Control_Robot_v3.6_Control_Jaco/DSP/Include" -I"B:/Documentos/Universidad-Materias/Fundamentos de Robotica/Robotic/WorkSpaces/PeripheralDrivers/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-Src

clean-App-2f-Src:
	-$(RM) ./App/Src/MotorDriver.d ./App/Src/MotorDriver.o ./App/Src/MotorDriver.su ./App/Src/main.d ./App/Src/main.o ./App/Src/main.su

.PHONY: clean-App-2f-Src

