################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/AD8232_APIv2.c \
../source/App.c \
../source/FreescaleIAP.c \
../source/MAX30102_API.c \
../source/PIT_driver.c \
../source/TempSensor.c \
../source/adc.c \
../source/algorithm.c \
../source/algorithm_by_RF.c \
../source/audio.c \
../source/dma.c \
../source/flash.c \
../source/hc05.c \
../source/i2c.c \
../source/i2s.c \
../source/packageTxHandler.c \
../source/portpin.c \
../source/uart.c \
../source/uda1380.c 

OBJS += \
./source/AD8232_APIv2.o \
./source/App.o \
./source/FreescaleIAP.o \
./source/MAX30102_API.o \
./source/PIT_driver.o \
./source/TempSensor.o \
./source/adc.o \
./source/algorithm.o \
./source/algorithm_by_RF.o \
./source/audio.o \
./source/dma.o \
./source/flash.o \
./source/hc05.o \
./source/i2c.o \
./source/i2s.o \
./source/packageTxHandler.o \
./source/portpin.o \
./source/uart.o \
./source/uda1380.o 

C_DEPS += \
./source/AD8232_APIv2.d \
./source/App.d \
./source/FreescaleIAP.d \
./source/MAX30102_API.d \
./source/PIT_driver.d \
./source/TempSensor.d \
./source/adc.d \
./source/algorithm.d \
./source/algorithm_by_RF.d \
./source/audio.d \
./source/dma.d \
./source/flash.d \
./source/hc05.d \
./source/i2c.d \
./source/i2s.d \
./source/packageTxHandler.d \
./source/portpin.d \
./source/uart.d \
./source/uda1380.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_MK64FN1M0VLL12 -D__USE_CMSIS -DDEBUG -I../source -I../ -I../SDK/CMSIS -I../SDK/startup -O0 -fno-common -g3 -Wall -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


