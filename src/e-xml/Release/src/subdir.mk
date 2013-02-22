################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/epiphany_xml.cpp \
../src/parse.cpp \
../src/xml.cpp 

OBJS += \
./src/epiphany_xml.o \
./src/parse.o \
./src/xml.o 

CPP_DEPS += \
./src/epiphany_xml.d \
./src/parse.d \
./src/xml.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0  -fPIC -DPIC -Wno-write-strings -DLINUX -DUNIX -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


