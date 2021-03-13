################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lv_examples/src/lv_demo_widgets/lv_demo_widgets.c 

OBJS += \
./lv_examples/src/lv_demo_widgets/lv_demo_widgets.o 

C_DEPS += \
./lv_examples/src/lv_demo_widgets/lv_demo_widgets.d 


# Each subdirectory must supply rules for building sources it contributes
lv_examples/src/lv_demo_widgets/%.o: ../lv_examples/src/lv_demo_widgets/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/horinek/eclipse-workspace" -O0 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


