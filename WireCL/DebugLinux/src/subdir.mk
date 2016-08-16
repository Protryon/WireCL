################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/glew.c \
../src/gui.c \
../src/main.c \
../src/render.c \
../src/streams.c \
../src/world.c \
../src/xstring.c 

OBJS += \
./src/glew.o \
./src/gui.o \
./src/main.o \
./src/render.o \
./src/streams.o \
./src/world.o \
./src/xstring.o 

C_DEPS += \
./src/glew.d \
./src/gui.d \
./src/main.d \
./src/render.d \
./src/streams.d \
./src/world.d \
./src/xstring.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -std=gnu11 -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


