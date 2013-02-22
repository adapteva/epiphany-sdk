################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/e_coreid_address_from_coreid.c \
../src/e_coreid_coords_from_coreid.c \
../src/e_coreid_coreid_from_address.c \
../src/e_coreid_coreid_from_coords.c \
../src/e_coreid_coreid_origin.c \
../src/e_coreid_get_coreid.c \
../src/e_coreid_is_oncore.c \
../src/e_coreid_neighbor_id.c \
../src/e_ctimer_stop.c \
../src/e_dma_busy.c \
../src/e_dma_copy.c \
../src/e_dma_start.c \
../src/e_ic_gid.c \
../src/e_ic_gie.c \
../src/e_ic_gie_restore.c \
../src/e_ic_irq_disable.c \
../src/e_ic_irq_enable.c \
../src/e_ic_irq_lower.c \
../src/e_ic_irq_raise.c \
../src/e_ic_irq_remote_raise.c \
../src/e_ic_irq_restore.c \
../src/e_regs_sysreg_read.c \
../src/e_regs_sysreg_write.c 

S_SRCS += \
../src/e_ctimer_get.s \
../src/e_ctimer_set.s \
../src/e_ctimer_start.s \
../src/e_mutex_init.s \
../src/e_mutex_lock.s \
../src/e_mutex_trylock.s \
../src/e_mutex_unlock.s 

OBJS += \
./src/e_coreid_address_from_coreid.o \
./src/e_coreid_coords_from_coreid.o \
./src/e_coreid_coreid_from_address.o \
./src/e_coreid_coreid_from_coords.o \
./src/e_coreid_coreid_origin.o \
./src/e_coreid_get_coreid.o \
./src/e_coreid_is_oncore.o \
./src/e_coreid_neighbor_id.o \
./src/e_ctimer_get.o \
./src/e_ctimer_set.o \
./src/e_ctimer_start.o \
./src/e_ctimer_stop.o \
./src/e_dma_busy.o \
./src/e_dma_copy.o \
./src/e_dma_start.o \
./src/e_ic_gid.o \
./src/e_ic_gie.o \
./src/e_ic_gie_restore.o \
./src/e_ic_irq_disable.o \
./src/e_ic_irq_enable.o \
./src/e_ic_irq_lower.o \
./src/e_ic_irq_raise.o \
./src/e_ic_irq_remote_raise.o \
./src/e_ic_irq_restore.o \
./src/e_mutex_init.o \
./src/e_mutex_lock.o \
./src/e_mutex_trylock.o \
./src/e_mutex_unlock.o \
./src/e_regs_sysreg_read.o \
./src/e_regs_sysreg_write.o 

C_DEPS += \
./src/e_coreid_address_from_coreid.d \
./src/e_coreid_coords_from_coreid.d \
./src/e_coreid_coreid_from_address.d \
./src/e_coreid_coreid_from_coords.d \
./src/e_coreid_coreid_origin.d \
./src/e_coreid_get_coreid.d \
./src/e_coreid_is_oncore.d \
./src/e_coreid_neighbor_id.d \
./src/e_ctimer_stop.d \
./src/e_dma_busy.d \
./src/e_dma_copy.d \
./src/e_dma_start.d \
./src/e_ic_gid.d \
./src/e_ic_gie.d \
./src/e_ic_gie_restore.d \
./src/e_ic_irq_disable.d \
./src/e_ic_irq_enable.d \
./src/e_ic_irq_lower.d \
./src/e_ic_irq_raise.d \
./src/e_ic_irq_remote_raise.d \
./src/e_ic_irq_restore.d \
./src/e_regs_sysreg_read.d \
./src/e_regs_sysreg_write.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Epiphany compiler'
	e-gcc -I../include -I/opt/adapteva/esdk/tools/gnu/epiphany-elf/include -O0 -g3 -Wall -c -fmessage-length=0 -ffp-contract=fast -mlong-calls -mfp-mode=round-nearest -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.s
	@echo 'Building file: $<'
	@echo 'Invoking: Epiphany assembler'
	e-as -g -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


