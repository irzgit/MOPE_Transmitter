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
stm32f446-receiver-blue.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
stm32f446-receiver-blue.list \

OBJCOPY_BIN += \
stm32f446-receiver-blue.bin \


# All Target
all: stm32f446-receiver-blue.elf secondary-outputs

# Tool invocations
stm32f446-receiver-blue.elf: $(OBJS) $(USER_OBJS) /home/dmitriy/Рабочий\ стол/MOPE_Transmitter/stm32f446-receiver-blue/STM32F446RETX_FLASH.ld
	arm-none-eabi-gcc -o "stm32f446-receiver-blue.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"/home/dmitriy/Рабочий стол/MOPE_Transmitter/stm32f446-receiver-blue/STM32F446RETX_FLASH.ld" --specs=nosys.specs -Wl,-Map="stm32f446-receiver-blue.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

stm32f446-receiver-blue.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "stm32f446-receiver-blue.list"
	@echo 'Finished building: $@'
	@echo ' '

stm32f446-receiver-blue.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "stm32f446-receiver-blue.bin"
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
