################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../crc_test-20170424/crc16.c \
../crc_test-20170424/crc_test.c 

OBJS += \
./crc_test-20170424/crc16.o \
./crc_test-20170424/crc_test.o 

C_DEPS += \
./crc_test-20170424/crc16.d \
./crc_test-20170424/crc_test.d 


# Each subdirectory must supply rules for building sources it contributes
crc_test-20170424/%.o: ../crc_test-20170424/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/local/Cellar/mysql/5.7.18_1/include/mysql -I/home/pyt64/mysql-connector-c-6.1.5-src/include -O0 -g3 -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


