################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../board/board.c \
../board/clock_config.c \
../board/pin_mux.c 

C_DEPS += \
./board/board.d \
./board/clock_config.d \
./board/pin_mux.d 

OBJS += \
./board/board.o \
./board/clock_config.o \
./board/pin_mux.o 


# Each subdirectory must supply rules for building sources it contributes
board/%.o: ../board/%.c board/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_MCXN947VDF -DCPU_MCXN947VDF_cm33 -DCPU_MCXN947VDF_cm33_core0 -DMCUXPRESSO_SDK -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"D:\Workspace\DesignProject\SmartBoat\Software\mcxn9xxbrk_lpuart_edma_transfer1\source" -I"D:\Workspace\DesignProject\SmartBoat\Software\mcxn9xxbrk_lpuart_edma_transfer1\drivers" -I"D:\Workspace\DesignProject\SmartBoat\Software\mcxn9xxbrk_lpuart_edma_transfer1\utilities" -I"D:\Workspace\DesignProject\SmartBoat\Software\mcxn9xxbrk_lpuart_edma_transfer1\device" -I"D:\Workspace\DesignProject\SmartBoat\Software\mcxn9xxbrk_lpuart_edma_transfer1\startup" -I"D:\Workspace\DesignProject\SmartBoat\Software\mcxn9xxbrk_lpuart_edma_transfer1\component\uart" -I"D:\Workspace\DesignProject\SmartBoat\Software\mcxn9xxbrk_lpuart_edma_transfer1\component\lists" -I"D:\Workspace\DesignProject\SmartBoat\Software\mcxn9xxbrk_lpuart_edma_transfer1\CMSIS" -I"D:\Workspace\DesignProject\SmartBoat\Software\mcxn9xxbrk_lpuart_edma_transfer1\board" -I"D:\Workspace\DesignProject\SmartBoat\Software\mcxn9xxbrk_lpuart_edma_transfer1\mcxn9xxbrk\driver_examples\lpuart\edma_transfer\cm33_core0" -O0 -fno-common -g3 -gdwarf-4 -mcpu=cortex-m33 -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-board

clean-board:
	-$(RM) ./board/board.d ./board/board.o ./board/clock_config.d ./board/clock_config.o ./board/pin_mux.d ./board/pin_mux.o

.PHONY: clean-board

