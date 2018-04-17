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
	g++ -I/usr/local/Cellar/mysql/5.7.18_1/include/mysql -I/home/pyt64/mysql-connector-c-6.1.5-src/include -O0 -g3 -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


