################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include DSP/Source/BasicMathFunctions/subdir.mk
-include Core/Startup/subdir.mk
-include Core/subdir.mk
-include App/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := Control_Robot_v3.6_Control_Jaco_Gyro
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
Control_Robot_v3.6_Control_Jaco_Gyro.elf \

MAP_FILES += \
Control_Robot_v3.6_Control_Jaco_Gyro.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
Control_Robot_v3.6_Control_Jaco_Gyro.list \


# All Target
all: main-build

# Main-build Target
main-build: Control_Robot_v3.6_Control_Jaco_Gyro.elf secondary-outputs

# Tool invocations
Control_Robot_v3.6_Control_Jaco_Gyro.elf Control_Robot_v3.6_Control_Jaco_Gyro.map: $(OBJS) $(USER_OBJS) B:\Documentos\Universidad-Materias\Fundamentos\ de\ Robotica\Robotic\WorkSpaces\Control_Robot_v3.6_Control_Jaco_Gyro\STM32F411RETX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "Control_Robot_v3.6_Control_Jaco_Gyro.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"B:\Documentos\Universidad-Materias\Fundamentos de Robotica\Robotic\WorkSpaces\Control_Robot_v3.6_Control_Jaco_Gyro\STM32F411RETX_FLASH.ld" --specs=nosys.specs -Wl,-Map="Control_Robot_v3.6_Control_Jaco_Gyro.map" -Wl,--gc-sections -static -L"B:\Documentos\Universidad-Materias\Fundamentos de Robotica\Robotic\WorkSpaces/PeripheralDrivers/Debug" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -u _printf_float -u _scanf_float -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

Control_Robot_v3.6_Control_Jaco_Gyro.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "Control_Robot_v3.6_Control_Jaco_Gyro.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) Control_Robot_v3.6_Control_Jaco_Gyro.elf Control_Robot_v3.6_Control_Jaco_Gyro.list Control_Robot_v3.6_Control_Jaco_Gyro.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
