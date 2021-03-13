################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lv_examples/assets/img_cogwheel_alpha16.c \
../lv_examples/assets/img_cogwheel_argb.c \
../lv_examples/assets/img_cogwheel_chroma_keyed.c \
../lv_examples/assets/img_cogwheel_indexed16.c \
../lv_examples/assets/img_cogwheel_rgb.c \
../lv_examples/assets/img_hand.c \
../lv_examples/assets/lv_font_montserrat_12_compr_az.c \
../lv_examples/assets/lv_font_montserrat_16_compr_az.c \
../lv_examples/assets/lv_font_montserrat_28_compr_az.c 

OBJS += \
./lv_examples/assets/img_cogwheel_alpha16.o \
./lv_examples/assets/img_cogwheel_argb.o \
./lv_examples/assets/img_cogwheel_chroma_keyed.o \
./lv_examples/assets/img_cogwheel_indexed16.o \
./lv_examples/assets/img_cogwheel_rgb.o \
./lv_examples/assets/img_hand.o \
./lv_examples/assets/lv_font_montserrat_12_compr_az.o \
./lv_examples/assets/lv_font_montserrat_16_compr_az.o \
./lv_examples/assets/lv_font_montserrat_28_compr_az.o 

C_DEPS += \
./lv_examples/assets/img_cogwheel_alpha16.d \
./lv_examples/assets/img_cogwheel_argb.d \
./lv_examples/assets/img_cogwheel_chroma_keyed.d \
./lv_examples/assets/img_cogwheel_indexed16.d \
./lv_examples/assets/img_cogwheel_rgb.d \
./lv_examples/assets/img_hand.d \
./lv_examples/assets/lv_font_montserrat_12_compr_az.d \
./lv_examples/assets/lv_font_montserrat_16_compr_az.d \
./lv_examples/assets/lv_font_montserrat_28_compr_az.d 


# Each subdirectory must supply rules for building sources it contributes
lv_examples/assets/%.o: ../lv_examples/assets/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/horinek/eclipse-workspace" -O0 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


