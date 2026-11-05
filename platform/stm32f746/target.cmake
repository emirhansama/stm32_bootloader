# STM32F746ZG target configuration

set(CMSIS_Dvendor STMicroelectronics:13)
set(CMSIS_Dname STM32F746ZGT6)
set(CMSIS_Dcore Cortex-M7)
set(CMSIS_Dfpu SP_FPU)
set(CMSIS_Ddsp DSP)
set(CMSIS_Dmve NO_MVE)
set(CMSIS_Dendian Little-endian)
set(CMSIS_Tcompiler GCC)

set(CPU ${CMSIS_Dcore})
set(FPU ${CMSIS_Dfpu})
set(DSP ${CMSIS_Ddsp})
set(MVE ${CMSIS_Dmve})
set(BYTE_ORDER ${CMSIS_Dendian})