################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/syscalls.c \
../Core/sysmem.c 

OBJS += \
./Core/syscalls.o \
./Core/sysmem.o 

C_DEPS += \
./Core/syscalls.d \
./Core/sysmem.d 


# Each subdirectory must supply rules for building sources it contributes
Core/%.o Core/%.su: ../Core/%.c Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F411RE -DSTM32 -DSTM32F4 -DSTM32F411RETx -DSTM32F411xE -c -I"B:/Documentos/Universidad-Materias/Fundamentos de Robotica/Robotic/WorkSpaces/Control_Robot_v6_IntAStar/App/Inc" -I"B:/Documentos/Universidad-Materias/Fundamentos de Robotica/Robotic/WorkSpaces/Control_Robot_v6_IntAStar/Core/Include" -I"B:/Documentos/Universidad-Materias/Fundamentos de Robotica/Robotic/WorkSpaces/Control_Robot_v6_IntAStar/Core/Device/ST/STM32F4xx/Include" -I"B:/Documentos/Universidad-Materias/Fundamentos de Robotica/Robotic/WorkSpaces/Control_Robot_v6_IntAStar/DSP/Include" -I"B:/Documentos/Universidad-Materias/Fundamentos de Robotica/Robotic/WorkSpaces/PeripheralDrivers/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core

clean-Core:
	-$(RM) ./Core/syscalls.d ./Core/syscalls.o ./Core/syscalls.su ./Core/sysmem.d ./Core/sysmem.o ./Core/sysmem.su

.PHONY: clean-Core

