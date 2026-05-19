################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/CMSIS/DSP/ComputeLibrary/Source/arm_cl_tables.c 

OBJS += \
./Drivers/CMSIS/DSP/ComputeLibrary/Source/arm_cl_tables.o 

C_DEPS += \
./Drivers/CMSIS/DSP/ComputeLibrary/Source/arm_cl_tables.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/CMSIS/DSP/ComputeLibrary/Source/%.o Drivers/CMSIS/DSP/ComputeLibrary/Source/%.su Drivers/CMSIS/DSP/ComputeLibrary/Source/%.cyclo: ../Drivers/CMSIS/DSP/ComputeLibrary/Source/%.c Drivers/CMSIS/DSP/ComputeLibrary/Source/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F439xx -c -I../LWIP/App -I../LWIP/Target -I../Core/Inc -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/BSP/Components/lan8742 -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-CMSIS-2f-DSP-2f-ComputeLibrary-2f-Source

clean-Drivers-2f-CMSIS-2f-DSP-2f-ComputeLibrary-2f-Source:
	-$(RM) ./Drivers/CMSIS/DSP/ComputeLibrary/Source/arm_cl_tables.cyclo ./Drivers/CMSIS/DSP/ComputeLibrary/Source/arm_cl_tables.d ./Drivers/CMSIS/DSP/ComputeLibrary/Source/arm_cl_tables.o ./Drivers/CMSIS/DSP/ComputeLibrary/Source/arm_cl_tables.su

.PHONY: clean-Drivers-2f-CMSIS-2f-DSP-2f-ComputeLibrary-2f-Source

