################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lv_examples/src/lv_ex_style/lv_ex_style_1.c \
../lv_examples/src/lv_ex_style/lv_ex_style_10.c \
../lv_examples/src/lv_ex_style/lv_ex_style_11.c \
../lv_examples/src/lv_ex_style/lv_ex_style_2.c \
../lv_examples/src/lv_ex_style/lv_ex_style_3.c \
../lv_examples/src/lv_ex_style/lv_ex_style_4.c \
../lv_examples/src/lv_ex_style/lv_ex_style_5.c \
../lv_examples/src/lv_ex_style/lv_ex_style_6.c \
../lv_examples/src/lv_ex_style/lv_ex_style_7.c \
../lv_examples/src/lv_ex_style/lv_ex_style_8.c \
../lv_examples/src/lv_ex_style/lv_ex_style_9.c 

OBJS += \
./lv_examples/src/lv_ex_style/lv_ex_style_1.o \
./lv_examples/src/lv_ex_style/lv_ex_style_10.o \
./lv_examples/src/lv_ex_style/lv_ex_style_11.o \
./lv_examples/src/lv_ex_style/lv_ex_style_2.o \
./lv_examples/src/lv_ex_style/lv_ex_style_3.o \
./lv_examples/src/lv_ex_style/lv_ex_style_4.o \
./lv_examples/src/lv_ex_style/lv_ex_style_5.o \
./lv_examples/src/lv_ex_style/lv_ex_style_6.o \
./lv_examples/src/lv_ex_style/lv_ex_style_7.o \
./lv_examples/src/lv_ex_style/lv_ex_style_8.o \
./lv_examples/src/lv_ex_style/lv_ex_style_9.o 

C_DEPS += \
./lv_examples/src/lv_ex_style/lv_ex_style_1.d \
./lv_examples/src/lv_ex_style/lv_ex_style_10.d \
./lv_examples/src/lv_ex_style/lv_ex_style_11.d \
./lv_examples/src/lv_ex_style/lv_ex_style_2.d \
./lv_examples/src/lv_ex_style/lv_ex_style_3.d \
./lv_examples/src/lv_ex_style/lv_ex_style_4.d \
./lv_examples/src/lv_ex_style/lv_ex_style_5.d \
./lv_examples/src/lv_ex_style/lv_ex_style_6.d \
./lv_examples/src/lv_ex_style/lv_ex_style_7.d \
./lv_examples/src/lv_ex_style/lv_ex_style_8.d \
./lv_examples/src/lv_ex_style/lv_ex_style_9.d 


# Each subdirectory must supply rules for building sources it contributes
lv_examples/src/lv_ex_style/%.o: ../lv_examples/src/lv_ex_style/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/horinek/eclipse-workspace" -O0 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


