################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lvgl/src/lv_gpu/lv_gpu_nxp_pxp.c \
../lvgl/src/lv_gpu/lv_gpu_nxp_pxp_osa.c \
../lvgl/src/lv_gpu/lv_gpu_nxp_vglite.c \
../lvgl/src/lv_gpu/lv_gpu_stm32_dma2d.c 

OBJS += \
./lvgl/src/lv_gpu/lv_gpu_nxp_pxp.o \
./lvgl/src/lv_gpu/lv_gpu_nxp_pxp_osa.o \
./lvgl/src/lv_gpu/lv_gpu_nxp_vglite.o \
./lvgl/src/lv_gpu/lv_gpu_stm32_dma2d.o 

C_DEPS += \
./lvgl/src/lv_gpu/lv_gpu_nxp_pxp.d \
./lvgl/src/lv_gpu/lv_gpu_nxp_pxp_osa.d \
./lvgl/src/lv_gpu/lv_gpu_nxp_vglite.d \
./lvgl/src/lv_gpu/lv_gpu_stm32_dma2d.d 


# Each subdirectory must supply rules for building sources it contributes
lvgl/src/lv_gpu/%.o: ../lvgl/src/lv_gpu/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/horinek/eclipse-workspace" -O0 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


