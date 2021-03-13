################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lv_drivers/gtkdrv/gtkdrv.c 

OBJS += \
./lv_drivers/gtkdrv/gtkdrv.o 

C_DEPS += \
./lv_drivers/gtkdrv/gtkdrv.d 


# Each subdirectory must supply rules for building sources it contributes
lv_drivers/gtkdrv/%.o: ../lv_drivers/gtkdrv/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/horinek/eclipse-workspace" -O0 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


