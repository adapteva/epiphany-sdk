################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
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
all: matmul-16.core.32_08.elf

dependents:
	-cd ../../matmul-16_commonlib/Debug && $(MAKE) clean all

# Tool invocations
matmul-16.core.32_08.elf: $(OBJS) $(USER_OBJS) ../../matmul-16_commonlib/Debug/libmatmul-16_commonlib.a
	@echo 'Building target: $@'
	@echo 'Invoking: Epiphany linker'
	e-gcc -L"../../matmul-16_commonlib/Debug" -L../../matmul-16_commonlib/Debug -T ../../matmul-16_commonlib/src/matmul-64_internal.ldf -Wl,--gc-sections -o "matmul-16.core.32_08.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) matmul-16.core.32_08.elf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
../../matmul-16_commonlib/Debug/libmatmul-16_commonlib.a:

-include ../makefile.targets
