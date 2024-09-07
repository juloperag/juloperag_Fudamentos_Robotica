################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../DSP/Source/BasicMathFunctions/arm_abs_f32.c 

OBJS += \
./DSP/Source/BasicMathFunctions/arm_abs_f32.o 

C_DEPS += \
./DSP/Source/BasicMathFunctions/arm_abs_f32.d 


# Each subdirectory must supply rules for building sources it contributes
DSP/Source/BasicMathFunctions/%.o DSP/Source/BasicMathFunctions/%.su: ../DSP/Source/BasicMathFunctions/%.c DSP/Source/BasicMathFunctions/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DNUCLEO_F411RE -DSTM32 -DSTM32F4 -DSTM32F411RETx -c -I../Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-DSP-2f-Source-2f-BasicMathFunctions

clean-DSP-2f-Source-2f-BasicMathFunctions:
	-$(RM) ./DSP/Source/BasicMathFunctions/arm_abs_f32.d ./DSP/Source/BasicMathFunctions/arm_abs_f32.o ./DSP/Source/BasicMathFunctions/arm_abs_f32.su

.PHONY: clean-DSP-2f-Source-2f-BasicMathFunctions

