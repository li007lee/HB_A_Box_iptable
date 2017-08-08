################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../md5/md5.c \
../md5/md5gen.c 

OBJS += \
./md5/md5.o \
./md5/md5gen.o 

C_DEPS += \
./md5/md5.d \
./md5/md5gen.d 


# Each subdirectory must supply rules for building sources it contributes
md5/%.o: ../md5/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


