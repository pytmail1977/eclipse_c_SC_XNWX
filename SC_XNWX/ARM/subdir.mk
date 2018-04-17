################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../SC.cpp \
../commonTool.cpp \
../global.cpp 

OBJS += \
./SC.o \
./commonTool.o \
./global.o 

CPP_DEPS += \
./SC.d \
./commonTool.d \
./global.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	/home/pyt64/timesys/i_MX6QSABRELite/toolchain/bin/armv7l-timesys-linux-gnueabi-g++ -I/home/pyt64/mysql-build/libmysqlclient-deb/include/mysql -O0 -g3 -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


