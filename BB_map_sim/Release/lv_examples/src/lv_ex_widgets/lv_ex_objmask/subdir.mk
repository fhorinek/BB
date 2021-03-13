################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lv_examples/src/lv_ex_widgets/lv_ex_objmask/lv_ex_objmask_1.c \
../lv_examples/src/lv_ex_widgets/lv_ex_objmask/lv_ex_objmask_2.c 

OBJS += \
./lv_examples/src/lv_ex_widgets/lv_ex_objmask/lv_ex_objmask_1.o \
./lv_examples/src/lv_ex_widgets/lv_ex_objmask/lv_ex_objmask_2.o 

C_DEPS += \
./lv_examples/src/lv_ex_widgets/lv_ex_objmask/lv_ex_objmask_1.d \
./lv_examples/src/lv_ex_widgets/lv_ex_objmask/lv_ex_objmask_2.d 


# Each subdirectory must supply rules for building sources it contributes
lv_examples/src/lv_ex_widgets/lv_ex_objmask/%.o: ../lv_examples/src/lv_ex_widgets/lv_ex_objmask/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/horinek/eclipse-workspace" -O0 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


