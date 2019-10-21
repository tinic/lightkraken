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

cmake_minimum_required(VERSION 3.10)

set(PRJ_NAME lightguy)

if(BOOTLOADER)
set(PRJ_NAME bootloader)
endif(BOOTLOADER)

if(FORBOOTLOADER)
set(PRJ_NAME lightguy_bootload)
endif(FORBOOTLOADER)

project(${PRJ_NAME} C CXX ASM)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)

set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-as)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_RANLIB arm-none-eabi-ranlib)
set(CMAKE_SIZE arm-none-eabi-size)

set(ARM_ARCH cortex-m3)
set(ARM_FLOAT_ABI softfp)

set(CMSISDEVINC GD32F10x)
set(CMSISSTARTUPS startup_gd32f10x_cl.s)
set(CMSISSYSTEMC system_gd32f10x.c)

set(CMSISLINKLD GD32F107RC_FLASH.ld)

if(BOOTLOADER)
set(CMSISLINKLD GD32F107RC_FLASH_BOOTLOADER.ld)
endif(BOOTLOADER)

if(FORBOOTLOADER)
set(CMSISLINKLD GD32F107RC_FLASH_FORBOOTLOADER.ld)
endif(FORBOOTLOADER)

set(GD32ARCH GD32F10X_CL)
set(GD32MCU GD32F107RC)

# CMSIS
include_directories(.)
include_directories(CMSIS/Include)
include_directories(CMSIS/GD/GD32F10x/Include)

set(LINKLDPATH CMSIS/GD/GD32F10x/Source/GCC/linker/${CMSISLINKLD})
set(DEVICESTARTUPPATH CMSIS/GD/GD32F10x/Source/GCC/${CMSISSTARTUPS})
set(SYSTEMCPATH CMSIS/GD/GD32F10x/Source/${CMSISSYSTEMC})

# HAL
include_directories(GD32F10x_standard_peripheral/Include)
file(GLOB_RECURSE HAL_SRC GD32F10x_standard_peripheral/Source/*.c)

# LwIP
include_directories(GD32F10x_lwip)

set(COMMON_FLAGS "-DGD32 -D${GD32ARCH} -D${GD32MCU} -Werror -Wall -Wextra -Wno-error=format -Wno-strict-aliasing -mcpu=${ARM_ARCH} -ffunction-sections -ffast-math -mthumb -mfloat-abi=${ARM_FLOAT_ABI} -mlong-calls")

if(BOOTLOADER)
set(COMMON_FLAGS "${COMMON_FLAGS} -D BOOTLOADER")
endif( BOOTLOADER)

set(CMAKE_ASM_FLAGS "-mcpu=${ARM_ARCH}")
set(CMAKE_ASM_FLAGS_RELEASE "")
set(CMAKE_ASM_FLAGS_DEBUG "")

set(CMAKE_C_FLAGS "${COMMON_FLAGS} -std=gnu99")
set(CMAKE_C_FLAGS_RELEASE "-flto -Wl,-flto -Os")
set(CMAKE_C_FLAGS_DEBUG "-Og -g3")

set(CMAKE_CXX_FLAGS "${COMMON_FLAGS} -Werror -Wall -Wextra -Wno-strict-aliasing -Wdouble-promotion -Wshadow -Wfloat-conversion -Wnull-dereference -Wformat=2 -std=c++17 -fno-rtti -fno-exceptions")
set(CMAKE_CXX_FLAGS_RELEASE "-flto -Wl,-flto -Os")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g3")

set(CMAKE_EXE_LINKER_FLAGS "-g3 --specs=nosys.specs --specs=nano.specs -Wl,--warn-unresolved-symbols -Wl,--start-group -lm -Wl,--end-group -Wl,-gc-sections -T${CMAKE_SOURCE_DIR}/${LINKLDPATH}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")

set(CMAKE_OSX_DEPLOYMENT_TARGET "")
set(CMAKE_OSX_SYSROOT "")
set(CMAKE_C_LINK_FLAGS "")
set(CMAKE_CXX_LINK_FLAGS "")
set(CMAKE_LINKER_FLAGS "")

set(LWIP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lwip-ajax)
set(LWIP_INCLUDE_DIRS "${LWIP_DIR}/src/include")
include(${LWIP_DIR}/src/Filelists.cmake)

if(BOOTLOADER)
set(bootloader_SRCS bootloader.cpp multipartparser.cpp)
endif(BOOTLOADER)

set(SOURCE_FILES ${bootloader_SRCS} ${lwipnoapps_SRCS} ${lwipallapps_SRCS} main.cpp control.cpp uart.cpp pwmtimer.cpp spi.cpp glue.cpp rest.cpp ethernetif.cpp netconf.cpp systick.cpp artnet.cpp driver.cpp model.cpp strip.cpp status.cpp ${DEVICESTARTUPPATH} ${SYSTEMCPATH} ${HAL_SRC})

add_executable(${PROJECT_NAME}.elf ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}.elf PRIVATE ${LWIP_INCLUDE_DIRS})

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${PROJECT_SOURCE_DIR}/build/${PROJECT_NAME}.map")
set(HEX_FILE ${PROJECT_SOURCE_DIR}/build/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_SOURCE_DIR}/build/${PROJECT_NAME}.bin)
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
		COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
		COMMAND ${CMAKE_OBJCOPY} -O ihex -R .eeprom -R .fuse -R .lock -R .signature $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
		COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
		COMMENT "Building ${HEX_FILE} \nBuilding ${BIN_FILE}")

set(PROGRAM_CMD "sudo openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c \"program ${PROJECT_NAME}.elf verify reset exit\"")
install(CODE "execute_process(COMMAND ${PROGRAM_CMD})")
