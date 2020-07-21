################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32L4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
stm32l476-sx1272-accel.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
stm32l476-sx1272-accel.list \

OBJCOPY_BIN += \
stm32l476-sx1272-accel.bin \


# All Target
all: stm32l476-sx1272-accel.elf secondary-outputs

# Tool invocations
stm32l476-sx1272-accel.elf: $(OBJS) $(USER_OBJS) /home/user/Рабочий\ стол/project2/stm32l476-sx1272-accel/STM32L476RGTX_FLASH.ld
	arm-none-eabi-gcc -o "stm32l476-sx1272-accel.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"/home/user/Рабочий стол/project2/stm32l476-sx1272-accel/STM32L476RGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="stm32l476-sx1272-accel.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

stm32l476-sx1272-accel.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "stm32l476-sx1272-accel.list"
	@echo 'Finished building: $@'
	@echo ' '

stm32l476-sx1272-accel.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "stm32l476-sx1272-accel.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
