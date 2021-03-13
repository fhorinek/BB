################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lv_drivers/indev/AD_touch.c \
../lv_drivers/indev/FT5406EE8.c \
../lv_drivers/indev/XPT2046.c \
../lv_drivers/indev/evdev.c \
../lv_drivers/indev/keyboard.c \
../lv_drivers/indev/libinput.c \
../lv_drivers/indev/mouse.c \
../lv_drivers/indev/mousewheel.c 

OBJS += \
./lv_drivers/indev/AD_touch.o \
./lv_drivers/indev/FT5406EE8.o \
./lv_drivers/indev/XPT2046.o \
./lv_drivers/indev/evdev.o \
./lv_drivers/indev/keyboard.o \
./lv_drivers/indev/libinput.o \
./lv_drivers/indev/mouse.o \
./lv_drivers/indev/mousewheel.o 

C_DEPS += \
./lv_drivers/indev/AD_touch.d \
./lv_drivers/indev/FT5406EE8.d \
./lv_drivers/indev/XPT2046.d \
./lv_drivers/indev/evdev.d \
./lv_drivers/indev/keyboard.d \
./lv_drivers/indev/libinput.d \
./lv_drivers/indev/mouse.d \
./lv_drivers/indev/mousewheel.d 


# Each subdirectory must supply rules for building sources it contributes
lv_drivers/indev/%.o: ../lv_drivers/indev/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/horinek/eclipse-workspace" -O0 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


