################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include source/subdir.mk
-include SDK/startup/subdir.mk
-include SDK/CMSIS/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: TPF_ALL_.axf

# Tool invocations
TPF_ALL_.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"C:\Users\Tobias Scala\Documents\MCUXpressoIDE_10.2.0_759\HC05\lib\helix" -Xlinker -Map="TPF_ALL_.map" -Xlinker --gc-sections -Xlinker -print-memory-usage -Xlinker --sort-section=alignment -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -T "MyBlink_Debug.ld" -o "TPF_ALL_.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) TPF_ALL_.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "TPF_ALL_.axf"; # arm-none-eabi-objcopy -v -O binary "TPF_ALL_.axf" "TPF_ALL_.bin" ; # checksum -p MK64FN1M0xxx12 -d "TPF_ALL_.bin";
	-@echo ' '

.PHONY: all clean dependents post-build

-include ../makefile.targets
