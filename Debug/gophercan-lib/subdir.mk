################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/calja/github_files/gophercan-lib/GopherCAN.c \
C:/Users/calja/github_files/gophercan-lib/GopherCAN_ids.c \
C:/Users/calja/github_files/gophercan-lib/GopherCAN_ring_buffer.c 

OBJS += \
./gophercan-lib/GopherCAN.o \
./gophercan-lib/GopherCAN_ids.o \
./gophercan-lib/GopherCAN_ring_buffer.o 

C_DEPS += \
./gophercan-lib/GopherCAN.d \
./gophercan-lib/GopherCAN_ids.d \
./gophercan-lib/GopherCAN_ring_buffer.d 


# Each subdirectory must supply rules for building sources it contributes
gophercan-lib/GopherCAN.o: C:/Users/calja/github_files/gophercan-lib/GopherCAN.c gophercan-lib/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F767xx -c -I../Core/Inc -I../FATFS/Target -I../FATFS/App -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I../Middlewares/Third_Party/FatFs/src -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/calja/github_files/gophercan-lib" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"gophercan-lib/GopherCAN.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
gophercan-lib/GopherCAN_ids.o: C:/Users/calja/github_files/gophercan-lib/GopherCAN_ids.c gophercan-lib/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F767xx -c -I../Core/Inc -I../FATFS/Target -I../FATFS/App -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I../Middlewares/Third_Party/FatFs/src -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/calja/github_files/gophercan-lib" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"gophercan-lib/GopherCAN_ids.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
gophercan-lib/GopherCAN_ring_buffer.o: C:/Users/calja/github_files/gophercan-lib/GopherCAN_ring_buffer.c gophercan-lib/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F767xx -c -I../Core/Inc -I../FATFS/Target -I../FATFS/App -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I../Middlewares/Third_Party/FatFs/src -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/calja/github_files/gophercan-lib" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"gophercan-lib/GopherCAN_ring_buffer.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

