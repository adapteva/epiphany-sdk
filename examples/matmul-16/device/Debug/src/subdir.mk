################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/coordinates.c \
../src/main.c 

OBJS += \
./src/coordinates.o \
./src/main.o 

C_DEPS += \
./src/coordinates.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Epiphany compiler'
	e-gcc -Dasm=__asm__ -I"../../matmul-16_commonlib" -I../../matmul-16_commonlib/src -O0 -g -Wall -c -fmessage-length=0 -ffast-math -ftree-vectorize -funroll-loops -std=c99 -Wunused-variable -ffp-contract=fast -mlong-calls -mfp-mode=truncate -ffunction-sections -fdata-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


