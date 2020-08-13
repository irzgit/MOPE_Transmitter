################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
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
Stm32f446-Receiver.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
Stm32f446-Receiver.list \

OBJCOPY_BIN += \
Stm32f446-Receiver.bin \


# All Target
all: Stm32f446-Receiver.elf secondary-outputs

# Tool invocations
Stm32f446-Receiver.elf: $(OBJS) $(USER_OBJS) /home/user/Рабочий\ стол/MOPE_Transmitter/Stm32f446-Receiver/STM32F446RETX_FLASH.ld
	arm-none-eabi-gcc -o "Stm32f446-Receiver.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"/home/user/Рабочий стол/MOPE_Transmitter/Stm32f446-Receiver/STM32F446RETX_FLASH.ld" --specs=nosys.specs -Wl,-Map="Stm32f446-Receiver.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

Stm32f446-Receiver.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "Stm32f446-Receiver.list"
	@echo 'Finished building: $@'
	@echo ' '

Stm32f446-Receiver.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "Stm32f446-Receiver.bin"
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
