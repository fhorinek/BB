################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../linked_list.c \
../main.c \
../mouse_cursor_icon.c \
../tile.c 

OBJS += \
./linked_list.o \
./main.o \
./mouse_cursor_icon.o \
./tile.o 

C_DEPS += \
./linked_list.d \
./main.d \
./mouse_cursor_icon.d \
./tile.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/horinek/eclipse-workspace" -O0 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


