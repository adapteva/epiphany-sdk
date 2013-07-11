################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/matlib.c \
../src/matmul_main.c \
../src/static_buffers.c

OBJS += \
./src/matlib.o \
./src/matmul_main.o \
./src/static_buffers.o

C_DEPS += \
./src/matlib.d \
./src/matmul_main.d \
./src/static_buffers.d


src/matlib.o: ../src/matlib.c ../src/matlib.h
	@echo 'Building file: $<'
	@echo 'Invoking: Epiphany compiler'
	e-gcc -Dasm=__asm__ -Ofast -funroll-loops -falign-loops=8 -falign-functions=8 -Wall -c -fmessage-length=0 -ffast-math -ftree-vectorize -std=c99 -Wunused-variable -ffp-contract=fast -mlong-calls -mfp-mode=round-nearest -MMD -MP -MF"$(@:%.o=%.d)" -MT"src/matlib.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/matmul_main.o: ../src/matmul_main.c ../src/matlib.h ../src/matmul.h ../src/static_buffers.h ../src/common_buffers.h
	@echo 'Building file: $<'
	@echo 'Invoking: Epiphany compiler'
	e-gcc -Dasm=__asm__ -O2 -falign-loops=8 -falign-functions=8 -Wall -c -fmessage-length=0 -ffast-math -ftree-vectorize -std=c99 -Wunused-variable -ffp-contract=fast -mlong-calls -mfp-mode=round-nearest -MMD -MP -MF"$(@:%.o=%.d)" -MT"src/matmul_main.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/static_buffers.o: ../src/static_buffers.c ../src/static_buffers.h ../src/matmul.h
	@echo 'Building file: $<'
	@echo 'Invoking: Epiphany compiler'
	e-gcc -Dasm=__asm__ -O2 -falign-loops=8 -falign-functions=8 -Wall -c -fmessage-length=0 -ffast-math -ftree-vectorize -std=c99 -Wunused-variable -ffp-contract=fast -mlong-calls -mfp-mode=round-nearest -MMD -MP -MF"$(@:%.o=%.d)" -MT"src/static_buffers.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

