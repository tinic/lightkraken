# Copyright 2019 Tinic Uro
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

set(PRJ_NAME lightguy)

cmake_minimum_required(VERSION 3.10)

project(${PRJ_NAME} C CXX ASM)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)

set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-as)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_SIZE arm-none-eabi-size)

set(ARM_ARCH cortex-m3)
set(ARM_FLOAT_ABI softfp)

set(CMSISDEVINC STM32F1xx)
set(CMSISSTARTUPS startup_stm32f107xc.s)
set(CMSISSYSTEMC system_stm32f1xx.c)
set(CMSISLINKLD STM32F107XC_FLASH.ld)

set(STM32ARCH STM32F1xx)
set(STM32MCU STM32F107xC)

include_directories(CMSIS/Include)
include_directories(CMSIS/Device/ST/STM32F1xx/Include)

set(LINKLDPATH CMSIS/Device/ST/STM32F1xx/Source/Templates/gcc/linker/${CMSISLINKLD})
set(DEVICESTARTUPPATH CMSIS/Device/ST/STM32F1xx/Source/Templates/gcc/${CMSISSTARTUPS})
set(SYSTEMCPATH CMSIS/Device/ST/STM32F1xx/Source/Templates/${CMSISSYSTEMC})

set(COMMON_FLAGS "-DSTM32 -D${STM32ARCH} -D${STM32MCU} -Werror -Wall -Wextra -Wno-strict-aliasing -mcpu=${ARM_ARCH} -ffunction-sections -ffast-math -mthumb -mfloat-abi=${ARM_FLOAT_ABI} -mlong-calls")

set(CMAKE_ASM_FLAGS "-mcpu=${ARM_ARCH}")

set(CMAKE_C_FLAGS "${COMMON_FLAGS} -std=gnu99")
set(CMAKE_C_FLAGS_RELEASE "-Os")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")

set(CMAKE_CXX_FLAGS "${COMMON_FLAGS} -Werror -Wall -Wextra -Wno-strict-aliasing -Wdouble-promotion -Wshadow -Wfloat-conversion -Wnull-dereference -Wformat=2 -std=c++17 -fno-rtti -fno-exceptions")
set(CMAKE_CXX_FLAGS_RELEASE "-Os")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")

set(CMAKE_EXE_LINKER_FLAGS "--specs=nosys.specs --specs=nano.specs -Wl,--warn-unresolved-symbols -Wl,--start-group -lm -Wl,--end-group -Wl,-gc-sections -T${CMAKE_SOURCE_DIR}/${LINKLDPATH}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")

set(CMAKE_OSX_SYSROOT "")
set(CMAKE_C_LINK_FLAGS "")
set(CMAKE_CXX_LINK_FLAGS "")
set(CMAKE_LINKER_FLAGS "")

set(SOURCE_FILES main.cpp ${DEVICESTARTUPPATH} ${SYSTEMCPATH})

add_executable(${PROJECT_NAME}.elf ${SOURCE_FILES})

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${PROJECT_SOURCE_DIR}/build/${PROJECT_NAME}.map")
set(HEX_FILE ${PROJECT_SOURCE_DIR}/build/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_SOURCE_DIR}/build/${PROJECT_NAME}.bin)
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
		COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
		COMMAND ${CMAKE_OBJCOPY} -O ihex -R .eeprom -R .fuse -R .lock -R .signature $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
		COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
		COMMENT "Building ${HEX_FILE} \nBuilding ${BIN_FILE}")
