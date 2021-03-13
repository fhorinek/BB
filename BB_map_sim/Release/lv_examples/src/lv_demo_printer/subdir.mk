################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lv_examples/src/lv_demo_printer/lv_demo_printer.c \
../lv_examples/src/lv_demo_printer/lv_demo_printer_theme.c 

OBJS += \
./lv_examples/src/lv_demo_printer/lv_demo_printer.o \
./lv_examples/src/lv_demo_printer/lv_demo_printer_theme.o 

C_DEPS += \
./lv_examples/src/lv_demo_printer/lv_demo_printer.d \
./lv_examples/src/lv_demo_printer/lv_demo_printer_theme.d 


# Each subdirectory must supply rules for building sources it contributes
lv_examples/src/lv_demo_printer/%.o: ../lv_examples/src/lv_demo_printer/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/horinek/eclipse-workspace" -O0 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


