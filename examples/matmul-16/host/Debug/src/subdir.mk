################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../../device/src/matlib.c \
../src/matmul_host.c

OBJS += \
./src/matlib.o \
./src/matmul_host.o

C_DEPS += \
./src/matlib.d \
./src/matmul_host.d


# Each subdirectory must supply rules for building sources it contributes
src/matmul_host.o: ../src/matmul_host.c ../../device/src/matlib.h ../../device/src/matmul.h ../../device/src/common_buffers.h
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc  -O0 -g \
	     -Wall \
	     -ffast-math \
	     -D__HOST__ \
	     -Dasm=__asm__ \
	     -Drestrict= \
	     -I ../../device/src \
	     -I ${EPIPHANY_HOME}/tools/host/include \
	     -c \
	     -Wno-unused-result \
	     -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/matlib.o: ../../device/src/matlib.c ../../device/src/matlib.h
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc  -O0 -g \
	     -Wall \
	     -ffast-math \
	     -D__HOST__ \
	     -Dasm=__asm__ \
	     -Drestrict= \
	     -I ../../device/src \
	     -I ${EPIPHANY_HOME}/tools/host/include \
	     -c \
	     -Wno-unused-result \
	     -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

