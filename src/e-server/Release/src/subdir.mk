################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/GdbServerSC.cpp \
../src/GdbserverCreate.cpp \
../src/MpHash.cpp \
../src/RspConnection.cpp \
../src/RspPacket.cpp \
../src/Utils.cpp \
../src/e_printf.cpp \
../src/main.cpp \
../src/targetCntrlHardware.cpp 

OBJS += \
./src/GdbServerSC.o \
./src/GdbserverCreate.o \
./src/MpHash.o \
./src/RspConnection.o \
./src/RspPacket.o \
./src/Utils.o \
./src/e_printf.o \
./src/main.o \
./src/targetCntrlHardware.o 

CPP_DEPS += \
./src/GdbServerSC.d \
./src/GdbserverCreate.d \
./src/MpHash.d \
./src/RspConnection.d \
./src/RspPacket.d \
./src/Utils.d \
./src/e_printf.d \
./src/main.d \
./src/targetCntrlHardware.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I../../ -O0 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


