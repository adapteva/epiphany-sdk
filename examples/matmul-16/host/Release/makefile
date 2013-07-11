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
all: matmul.elf

# Tool invocations
matmul.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L ${EPIPHANY_HOME}/tools/host/lib \
	      -o "matmul.elf" \
	      $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
#	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) matmul.elf

	-@echo ' '

post-build:
#	@echo 'Creating srec file:'
#	e-objcopy --srec-forceS3 --output-target srec e_matmul.elf e_matmul.srec
#	chmod -x e_matmul.srec

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
