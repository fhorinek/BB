################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lvgl/tests/lv_test_assert.c \
../lvgl/tests/lv_test_main.c 

OBJS += \
./lvgl/tests/lv_test_assert.o \
./lvgl/tests/lv_test_main.o 

C_DEPS += \
./lvgl/tests/lv_test_assert.d \
./lvgl/tests/lv_test_main.d 


# Each subdirectory must supply rules for building sources it contributes
lvgl/tests/%.o: ../lvgl/tests/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/horinek/eclipse-workspace" -O0 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


