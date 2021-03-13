################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lvgl/src/lv_hal/lv_hal_disp.c \
../lvgl/src/lv_hal/lv_hal_indev.c \
../lvgl/src/lv_hal/lv_hal_tick.c 

OBJS += \
./lvgl/src/lv_hal/lv_hal_disp.o \
./lvgl/src/lv_hal/lv_hal_indev.o \
./lvgl/src/lv_hal/lv_hal_tick.o 

C_DEPS += \
./lvgl/src/lv_hal/lv_hal_disp.d \
./lvgl/src/lv_hal/lv_hal_indev.d \
./lvgl/src/lv_hal/lv_hal_tick.d 


# Each subdirectory must supply rules for building sources it contributes
lvgl/src/lv_hal/%.o: ../lvgl/src/lv_hal/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/horinek/eclipse-workspace" -O0 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


