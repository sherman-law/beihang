################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../crc.c \
../leddar.c \
../leddar_protocol.c \
../main.c \
../uart.c 

OBJS += \
./crc.o \
./leddar.o \
./leddar_protocol.o \
./main.o \
./uart.o 

C_DEPS += \
./crc.d \
./leddar.d \
./leddar_protocol.d \
./main.d \
./uart.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	armv7l-timesys-linux-gnueabi-gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


