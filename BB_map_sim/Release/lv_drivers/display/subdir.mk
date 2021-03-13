################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lv_drivers/display/GC9A01.c \
../lv_drivers/display/ILI9341.c \
../lv_drivers/display/R61581.c \
../lv_drivers/display/SHARP_MIP.c \
../lv_drivers/display/SSD1963.c \
../lv_drivers/display/ST7565.c \
../lv_drivers/display/UC1610.c \
../lv_drivers/display/drm.c \
../lv_drivers/display/fbdev.c \
../lv_drivers/display/monitor.c 

OBJS += \
./lv_drivers/display/GC9A01.o \
./lv_drivers/display/ILI9341.o \
./lv_drivers/display/R61581.o \
./lv_drivers/display/SHARP_MIP.o \
./lv_drivers/display/SSD1963.o \
./lv_drivers/display/ST7565.o \
./lv_drivers/display/UC1610.o \
./lv_drivers/display/drm.o \
./lv_drivers/display/fbdev.o \
./lv_drivers/display/monitor.o 

C_DEPS += \
./lv_drivers/display/GC9A01.d \
./lv_drivers/display/ILI9341.d \
./lv_drivers/display/R61581.d \
./lv_drivers/display/SHARP_MIP.d \
./lv_drivers/display/SSD1963.d \
./lv_drivers/display/ST7565.d \
./lv_drivers/display/UC1610.d \
./lv_drivers/display/drm.d \
./lv_drivers/display/fbdev.d \
./lv_drivers/display/monitor.d 


# Each subdirectory must supply rules for building sources it contributes
lv_drivers/display/%.o: ../lv_drivers/display/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/horinek/eclipse-workspace" -O0 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


