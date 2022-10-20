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

if(NOT BOOTLOADER AND NOT BOOTLOADED)
	message(FATAL_ERROR "You must set either BOOTLOADER or BOOTLOADED. See build_release_all.sh for a valid cmake setup.")
endif()

set(CMAKE_C_COMPILER_WORKS 1 CACHE INTERNAL "")
set(CMAKE_CXX_COMPILER_WORKS 1 CACHE INTERNAL "")

# Boot loader
if(BOOTLOADER)
	set(PRJ_NAME lightkraken_bootloader)
	set(CMSISLINKLD GD32F107RC_FLASH_BOOTLOADER.ld)
	set(BASE_ADDRESS 0x08000000)
    message(status "Building bootloader binary")
elseif(BOOTLOADED)
	set(PRJ_NAME lightkraken_bootloaded)
	set(CMSISLINKLD GD32F107RC_FLASH_BOOTLOADED.ld)
	set(BASE_ADDRESS 0x08008000)
    message(status "Building bootloaded binary")
else(BOOTLOADER)
	set(PRJ_NAME lightkraken_debugging)
	set(CMSISLINKLD GD32F107RC_FLASH.ld)
	set(BASE_ADDRESS 0x08000000)
    message(status "Building debugging binary")
endif(BOOTLOADER)

project(${PRJ_NAME} C CXX ASM)

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}.")

set(ARM_ARCH cortex-m3)
set(GD32ARCH GD32F10X_CL)
set(GD32MCU GD32F107RC)

# CMSIS
set(CMSISDEVINC GD32F10x)
set(CMSISSTARTUPS startup_gd32f10x_cl.S)
set(CMSISSYSTEMC system_gd32f10x.c)

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

set(COMMON_FLAGS "-DGD32 -DNVIC_BASE_ADDRESS=${BASE_ADDRESS} -D${GD32ARCH} -D${GD32MCU} \
				  -Wall -Wextra -Wno-error=format -Wno-strict-aliasing -mtune=${ARM_ARCH} \
				  -mcpu=${ARM_ARCH} -ffunction-sections -ffast-math -mthumb -mfloat-abi=soft")

if(BOOTLOADER)
	set(COMMON_FLAGS "${COMMON_FLAGS} -DBOOTLOADER=1")
elseif(BOOTLOADED)
	set(COMMON_FLAGS "${COMMON_FLAGS} -DBOOTLOADED=1")
endif(BOOTLOADER)

set(CMAKE_ASM_FLAGS "-mcpu=${ARM_ARCH}")

set(CMAKE_C_FLAGS "${COMMON_FLAGS} -std=gnu99")
set(CMAKE_C_FLAGS_RELEASE "-flto=auto -Wl,-flto=auto -Os")
set(CMAKE_C_FLAGS_DEBUG "-Og -g3")

#-Wsign-conversion -Weffc++ -Wcast-align -Wno-volatile \
#-Wshadow -Wnon-virtual-dtor -Wunused \
#-Woverloaded-virtual -Wlogical-op \

set(CMAKE_CXX_FLAGS "${COMMON_FLAGS} -Wall -Wextra \
	-Wno-strict-aliasing -Wdouble-promotion \
	-Wmisleading-indentation -Wduplicated-cond \
	-Wlogical-op -Wshadow -Wfloat-conversion -Wnull-dereference \
	-Wformat=2 -std=c++17 -fno-rtti -fno-exceptions")
set(CMAKE_CXX_FLAGS_RELEASE "-flto -Wl,-flto -Os")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g3")

set(CMAKE_EXE_LINKER_FLAGS "-g3 --specs=nosys.specs \
	--specs=nano.specs -Wl,--warn-unresolved-symbols \
	-Wl,--no-warn-rwx-segments \
	-Wl,--no-wchar-size-warning -Wl,--start-group -lm \
	-Wl,--end-group -Wl,-gc-sections \
	-T\"${CMAKE_SOURCE_DIR}/${LINKLDPATH}\"")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")

set(LWIP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lwip-ajax)
set(LWIP_INCLUDE_DIRS "${LWIP_DIR}/src/include")
include(${LWIP_DIR}/src/Filelists.cmake)

if(BOOTLOADER)
	set(main_SRCS 
		main.cpp 
		glue.cpp 
		ethernetif.cpp 
		netconf.cpp 
		systick.cpp 
		status.cpp 
		bootloader.cpp 
		multipartparser.cpp)
else(BOOTLOADER)
	set(main_SRCS
		main.cpp 
		as73211.cpp 
		sacn.cpp 
		random.cpp 
		color.cpp 
		control.cpp 
		uart.cpp 
		pwmtimer.cpp 
		spi.cpp
		glue.cpp 
		rest.cpp 
		ethernetif.cpp 
		netconf.cpp 
		systick.cpp 
		artnet.cpp 
		driver.cpp 
		model.cpp
		strip.cpp
		status.cpp 
		perf.cpp
		ryu/f2s.c)
endif(BOOTLOADER)

set(SOURCE_FILES 
		${DEVICESTARTUPPATH}
		${SYSTEMCPATH} 
		${HAL_SRC} 
		${lwipcore_SRCS} 
		${lwipcore4_SRCS} 
		${lwipnetif_SRCS} 
		${lwiphttp_SRCS} 
		${main_SRCS})

add_executable(${PROJECT_NAME}.elf ${SOURCE_FILES})

# Generate version.h
find_package(Git)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE GIT_SHORT_SHA OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-list HEAD --count WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE GIT_REV_COUNT OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${GIT_EXECUTABLE} show -s --format=%ad HEAD --date=iso-strict WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE GIT_COMMIT_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
    set(GIT_SHORT_SHA "unknown")
    set(GIT_REV_COUNT "unknown")
    set(GIT_COMMIT_DATE "unknown")
endif()

include_directories(${CMAKE_BINARY_DIR})
configure_file("${PROJECT_SOURCE_DIR}/version.h.in" "${CMAKE_BINARY_DIR}/version.h" @ONLY)

target_include_directories(${PROJECT_NAME}.elf PRIVATE ${LWIP_INCLUDE_DIRS} ryu)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=\"${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map\"")
set(HEX_FILE ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin)
set(DUMP_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.S)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
		COMMAND ${CMAKE_OBJDUMP} -D $<TARGET_FILE:${PROJECT_NAME}.elf> > ${DUMP_FILE}
		COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
		COMMAND ${CMAKE_OBJCOPY} -O ihex -R .eeprom -R .fuse -R .lock -R .signature $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
		COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
		COMMENT "Building ${HEX_FILE} \nBuilding ${BIN_FILE}")

set(PROGRAM_CMD "./openocd -f ./stlink.cfg -f ./stm32f1x.cfg -c \"program ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin ${BASE_ADDRESS} verify reset exit\"")
install(CODE "execute_process(COMMAND ${PROGRAM_CMD} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/openocd)")

#set(PROGRAM_CMD "sudo openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c \"program ${PROJECT_NAME}.bin ${BASE_ADDRESS} verify reset exit\"")
#install(CODE "execute_process(COMMAND ${PROGRAM_CMD})")
