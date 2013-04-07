################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/matlib.c \
../src/matmul_dma.c \
../src/matmul_unit.c \
../src/static_buffers.c 

OBJS += \
./src/matlib.o \
./src/matmul_dma.o \
./src/matmul_unit.o \
./src/static_buffers.o 

C_DEPS += \
./src/matlib.d \
./src/matmul_dma.d \
./src/matmul_unit.d \
./src/static_buffers.d 


# Each subdirectory must supply rules for building sources it contributes
src/matlib.o: ../src/matlib.c
	@echo 'Building file: $<'
	@echo 'Invoking: Epiphany compiler'
	e-gcc -Dasm=__asm__ -I../../commonlib/src -O3 -Wall -c -fmessage-length=0 -ffast-math -ftree-vectorize -funroll-loops -std=c99 -Wunused-variable -ffp-contract=fast -mlong-calls -mfp-mode=round-nearest -MMD -MP -MF"$(@:%.o=%.d)" -MT"src/matlib.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Epiphany compiler'
	e-gcc -Dasm=__asm__ -I../../commonlib/src -O0 -g -Wall -c -fmessage-length=0 -ffast-math -ftree-vectorize -funroll-loops -std=c99 -Wunused-variable -ffp-contract=fast -mlong-calls -mfp-mode=round-nearest -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


