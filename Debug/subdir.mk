################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../get_set_config.c \
../hf_plant_api.c \
../hf_plant_net.c \
../ipcheck.c \
../my_iptable.c 

OBJS += \
./get_set_config.o \
./hf_plant_api.o \
./hf_plant_net.o \
./ipcheck.o \
./my_iptable.o 

C_DEPS += \
./get_set_config.d \
./hf_plant_api.d \
./hf_plant_net.d \
./ipcheck.d \
./my_iptable.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


