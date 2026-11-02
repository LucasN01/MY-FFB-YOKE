cmake_minimum_required(VERSION 3.13)
set(PICO_BOARD pico2 CACHE STRING "")
set(PICO_PLATFORM rp2350-arm-s CACHE STRING "")
include(pico_sdk_import.cmake)
project(miyoke_pd C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE MinSizeRel CACHE STRING "" FORCE)
endif()
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -g -DNDEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -g -DNDEBUG")
pico_sdk_init()

set(MIYOKE_HAL_SOURCES src/hal/rp2350/time_gpio.cpp src/hal/rp2350/pwm_adc.cpp src/hal/rp2350/i2c.cpp
    src/hal/rp2350/watchdog.cpp src/hal/rp2350/eeprom_flash.cpp src/hal/rp2350/usb_cdc.cpp
    src/hal/rp2350/usb_descriptors.cpp src/hal/rp2350/newlib_locks.cpp)
set(MIYOKE_LIBS pico_stdlib hardware_pwm hardware_adc hardware_i2c hardware_flash hardware_watchdog
    hardware_sync hardware_irq pico_unique_id pico_bootrom tinyusb_device)

function(miyoke_firmware name)
  add_executable(${name} src/main.cpp ${MIYOKE_HAL_SOURCES})
  target_include_directories(${name} PRIVATE include src/hal/rp2350)
  target_compile_definitions(${name} PRIVATE YOKE_TARGET_RP2350=1
      PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64 ${ARGN})
  target_compile_options(${name} PRIVATE -Werror=return-type -Wno-psabi
      $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions> $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
  target_link_libraries(${name} PRIVATE ${MIYOKE_LIBS})
  pico_enable_stdio_usb(${name} 0)
  pico_enable_stdio_uart(${name} 0)
  pico_set_printf_implementation(${name} compiler)
  pico_add_extra_outputs(${name})
endfunction()

miyoke_firmware(myk_cfg)
miyoke_firmware(myk_enc YOKE_POSITION_SOURCE=0)
miyoke_firmware(myk_pot YOKE_POSITION_SOURCE=2)
miyoke_firmware(myk_as YOKE_POSITION_SOURCE=3)
miyoke_firmware(myk_falla YOKE_POSITION_SOURCE=2 YOKE_TEST_HANG=3 YOKE_TEST_HANG_MS=20000)
