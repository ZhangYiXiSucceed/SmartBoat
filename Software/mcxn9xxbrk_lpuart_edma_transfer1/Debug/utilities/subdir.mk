################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../utilities/fsl_assert.c \
../utilities/fsl_debug_console.c \
../utilities/fsl_str.c 

C_DEPS += \
./utilities/fsl_assert.d \
./utilities/fsl_debug_console.d \
./utilities/fsl_str.d 

OBJS += \
./utilities/fsl_assert.o \
./utilities/fsl_debug_console.o \
./utilities/fsl_str.o 


# Each subdirectory must supply rules for building sources it contributes
utilities/%.o: ../utilities/%.c utilities/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_MCXN947VDF -DCPU_MCXN947VDF_cm33 -DCPU_MCXN947VDF_cm33_core0 -DMCUXPRESSO_SDK -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"D:\Workspace\DesignProject\SmartBoat\Software\mcxn9xxbrk_lpuart_edma_transfer1\source" -I"D:\Workspace\DesignProject\SmartBoat\Software\mcxn9xxbrk_lpuart_edma_transfer1\drivers" -I"D:\Workspace\DesignProject\SmartBoat\Software\mcxn9xxbrk_lpuart_edma_transfer1\utilities" -I"D:\Workspace\DesignProject\SmartBoat\Software\mcxn9xxbrk_lpuart_edma_transfer1\device" -I"D:\Workspace\DesignProject\SmartBoat\Software\mcxn9xxbrk_lpuart_edma_transfer1\startup" -I"D:\Workspace\DesignProject\SmartBoat\Software\mcxn9xxbrk_lpuart_edma_transfer1\component\uart" -I"D:\Workspace\DesignProject\SmartBoat\Software\mcxn9xxbrk_lpuart_edma_transfer1\component\lists" -I"D:\Workspace\DesignProject\SmartBoat\Software\mcxn9xxbrk_lpuart_edma_transfer1\CMSIS" -I"D:\Workspace\DesignProject\SmartBoat\Software\mcxn9xxbrk_lpuart_edma_transfer1\board" -I"D:\Workspace\DesignProject\SmartBoat\Software\mcxn9xxbrk_lpuart_edma_transfer1\mcxn9xxbrk\driver_examples\lpuart\edma_transfer\cm33_core0" -O0 -fno-common -g3 -gdwarf-4 -mcpu=cortex-m33 -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-utilities

clean-utilities:
	-$(RM) ./utilities/fsl_assert.d ./utilities/fsl_assert.o ./utilities/fsl_debug_console.d ./utilities/fsl_debug_console.o ./utilities/fsl_str.d ./utilities/fsl_str.o

.PHONY: clean-utilities

