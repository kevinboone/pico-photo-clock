cmake_minimum_required (VERSION 3.13)

include(pico_sdk_import.cmake)
set (BINARY "pico-photo-clock")
set (PROJ "pico-photo-clock")
project("pico-photo-clock")

set (CMAKE_C_FLAGS_RELEASE "-Wall -Wextra -Wconversion -O3")
set (CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -Wconversion -O3")

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file (GLOB ds3231_src CONFIGURE_DEPENDS "drivers/ds3231/src/*.c")
file (GLOB wslcd_src CONFIGURE_DEPENDS "drivers/waveshare_lcd/src/*.c")
file (GLOB sdcard_src CONFIGURE_DEPENDS "drivers/sdcard/src/*.c")
file (GLOB files_src CONFIGURE_DEPENDS "files/src/*.c")
file (GLOB fat_src CONFIGURE_DEPENDS "fs/ff14a/source/*.c")
file (GLOB fsintf_src CONFIGURE_DEPENDS "fs/interface/src/*.c")
file (GLOB gfx_src CONFIGURE_DEPENDS "gfx/src/*.c")
file (GLOB klib_src CONFIGURE_DEPENDS "klib/src/*.c")
file (GLOB fs_intf_src CONFIGURE_DEPENDS "fs/interface/src/*.c")
file (GLOB log_src CONFIGURE_DEPENDS "log/src/*.c")
file (GLOB screens_src CONFIGURE_DEPENDS "screens/src/*.c")

add_executable(${BINARY} main.c ${ds3231_src} ${wslcd_src} ${files_src} 
      ${fat_src} ${sdcard_src} ${gfx_src} ${fsintf_src} ${klib_src}
      ${fs_intf_src} ${log_src} ${screens_src})

target_include_directories (${BINARY} PUBLIC drivers/ds3231/include)
target_include_directories (${BINARY} PUBLIC drivers/waveshare_lcd/include)
target_include_directories (${BINARY} PUBLIC drivers/sdcard/include)
target_include_directories (${BINARY} PUBLIC files/include)
target_include_directories (${BINARY} PUBLIC gfx/include)
target_include_directories (${BINARY} PUBLIC gfx/src)
target_include_directories (${BINARY} PUBLIC fs/ff14a/source)
target_include_directories (${BINARY} PUBLIC fs/interface/src)
target_include_directories (${BINARY} PUBLIC klib/include)
target_include_directories (${BINARY} PUBLIC log/include)
target_include_directories (${BINARY} PUBLIC screens/include)

target_include_directories (${BINARY} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
if (PICO_ON_DEVICE)
target_link_libraries (${BINARY} PRIVATE pico_stdlib hardware_i2c hardware_spi hardware_dma)
else()
target_link_libraries (${BINARY} PRIVATE pico_stdlib)
endif()

pico_enable_stdio_usb (${BINARY} 1)
pico_enable_stdio_uart (${BINARY} 0)
pico_add_extra_outputs(${BINARY})
