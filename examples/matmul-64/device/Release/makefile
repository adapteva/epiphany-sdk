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
all: e_matmul.elf

dependents:
	-cd ../../commonlib/Release && $(MAKE) clean all

# Tool invocations
e_matmul.elf: $(OBJS) $(USER_OBJS) ../../commonlib/Release/libcommonlib.a
	@echo 'Building target: $@'
	@echo 'Invoking: Epiphany linker'
	e-gcc -L"../../commonlib/Release" -L../../commonlib/Release -T ../../commonlib/src/matmul_internal.ldf -Wl,--gc-sections -o "e_matmul.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) e_matmul.elf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
../../commonlib/Release/libcommonlib.a:

-include ../makefile.targets
