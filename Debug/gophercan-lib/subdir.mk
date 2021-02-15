################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/hmoca/Desktop/gm-firmware/gophercan-lib/GopherCAN.c \
C:/Users/hmoca/Desktop/gm-firmware/gophercan-lib/GopherCAN_ring_buffer.c 

OBJS += \
./gophercan-lib/GopherCAN.o \
./gophercan-lib/GopherCAN_ring_buffer.o 

C_DEPS += \
./gophercan-lib/GopherCAN.d \
./gophercan-lib/GopherCAN_ring_buffer.d 


# Each subdirectory must supply rules for building sources it contributes
gophercan-lib/GopherCAN.o: C:/Users/hmoca/Desktop/gm-firmware/gophercan-lib/GopherCAN.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F767xx -DDEBUG -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I../DLM_code/Inc -I../DLM_code/Src -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I"C:/Users/hmoca/Desktop/gm-firmware/gophercan-lib" -I"C:/Users/hmoca/Desktop/gm-firmware/C-Utils" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"gophercan-lib/GopherCAN.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
gophercan-lib/GopherCAN_ring_buffer.o: C:/Users/hmoca/Desktop/gm-firmware/gophercan-lib/GopherCAN_ring_buffer.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F767xx -DDEBUG -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I../DLM_code/Inc -I../DLM_code/Src -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I"C:/Users/hmoca/Desktop/gm-firmware/gophercan-lib" -I"C:/Users/hmoca/Desktop/gm-firmware/C-Utils" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"gophercan-lib/GopherCAN_ring_buffer.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

