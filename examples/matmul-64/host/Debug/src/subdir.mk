################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/e-probe-offsets.c \
../src/e-probe.c \
../../matmul-16_commonlib/src/matlib.c \
../src/matmul_host.c 

OBJS += \
./src/e-probe-offsets.o \
./src/e-probe.o \
./src/matlib.o \
./src/matmul_host.o 

C_DEPS += \
./src/e-probe-offsets.d \
./src/e-probe.d \
./src/matlib.d \
./src/matmul_host.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	/home/ysapir/CodeSourcery/Sourcery_CodeBench_Lite_for_ARM_GNU_Linux/bin/arm-none-linux-gnueabi-g++ -D__HOST__ -Dasm=__asm__ -Drestrict= -I../../matmul-16_commonlib/src -I/home/ysapir/Projects/zed/e-host/src -I/home/ysapir/Projects/zed/e-loader/src -O0 -falign-loops=8 -funroll-loops -ffast-math -g -Wall -c -fmessage-length=0 -std=c99 -Wno-unused-result -static -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/matlib.o: ../../matmul-16_commonlib/src/matlib.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	/home/ysapir/CodeSourcery/Sourcery_CodeBench_Lite_for_ARM_GNU_Linux/bin/arm-none-linux-gnueabi-g++ -D__HOST__ -Dasm=__asm__ -Drestrict= -I../../matmul-16_commonlib/src -I/home/ysapir/Projects/zed/e-host/src -I/home/ysapir/Projects/zed/e-loader/src -O0 -falign-loops=8 -funroll-loops -ffast-math -g -Wall -c -fmessage-length=0 -std=c99 -Wno-unused-result -static -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


