################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include ThirdParty/SEGGER/SEGGER/Syscalls/subdir.mk
-include ThirdParty/SEGGER/SEGGER/subdir.mk
-include ThirdParty/SEGGER/Rec/subdir.mk
-include ThirdParty/SEGGER/OS/subdir.mk
-include ThirdParty/SEGGER/Config/subdir.mk
-include ThirdParty/FreeRTOS/portable/MemMang/subdir.mk
-include ThirdParty/FreeRTOS/portable/GCC/ARM_CM4F/subdir.mk
-include ThirdParty/FreeRTOS/subdir.mk
-include Startup/subdir.mk
-include Src/subdir.mk
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


BUILD_ARTIFACT_NAME := Base_FreeRTOS
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
Base_FreeRTOS.elf \

MAP_FILES += \
Base_FreeRTOS.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
Base_FreeRTOS.list \


# All Target
all: main-build

# Main-build Target
main-build: Base_FreeRTOS.elf secondary-outputs

# Tool invocations
Base_FreeRTOS.elf Base_FreeRTOS.map: $(OBJS) $(USER_OBJS) B:\Documentos\Universidad-Materias\Fundamentos\ de\ Robotica\Robotic\WorkSpaces\Base_FreeRTOS\STM32F411RETX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "Base_FreeRTOS.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"B:\Documentos\Universidad-Materias\Fundamentos de Robotica\Robotic\WorkSpaces\Base_FreeRTOS\STM32F411RETX_FLASH.ld" --specs=nosys.specs -Wl,-Map="Base_FreeRTOS.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

Base_FreeRTOS.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "Base_FreeRTOS.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) Base_FreeRTOS.elf Base_FreeRTOS.list Base_FreeRTOS.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
