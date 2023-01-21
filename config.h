/*===========================================================================

  pico-photo-clock

  config.h

  General configuration. The GPIO assignments are for the Waveshare
  480x320 LCD panel with SD card, plus the Waveshare DS3231 real-time
  clock module. 

  Copyright (2)2023 Kevin Boone, GPLv3.0 

===========================================================================*/

#pragma once

/*================== DS3231 I2C setings =================================== */

#define CLOCK_I2C_DEV 0
#define CLOCK_SDA 20 
#define CLOCK_SCL 21 
#define CLOCK_I2C_BAUD 100000

/*======================== LCD settings =================================== */

#define WSLCD_SPI         1 
#define WSLCD_CS          9
#define WSLCD_MISO        12
#define WSLCD_MOSI        11
#define WSLCD_SCK         10
#define WSLCD_RST         15
#define WSLCD_DC          8
#define WSLCD_BL          13
#define WSLCD_TP_INT      17
#define WSLCD_BAUD        (20000 * 1000)
#define WSLCD_SCAN_DIR    WSLCD_SCAN_NORMAL

/*==================== SD card settings =================================== */

#define SD_SPI             1  
#define SD_DRIVE_STRENGTH  GPIO_DRIVE_STRENGTH_2MA
#define SD_CHIP_SELECT     22 
#define SD_MISO            12 
#define SD_MOSI            11
#define SD_SCK             10 
#define SD_BAUD            (20000 * 1000)

/*==================== General settings =================================== */

// Specify the location of the JPEG files, and the pattern to match
#define JPEG_DIR "/"
#define JPEG_PATTERN "*.jpg"

// Default position of the clock -- top left corner
#define CLOCK_DEFAULT_X 5
#define CLOCK_DEFAULT_Y 5

// How long in minutes to show a particular photo
#define DEFAULT_MINS_PER_PHOTO 3

// The name of the setings file, on the SD card. It doesn't matter if
//   this file does not exist -- defaults will be used.
#define SETTINGS_FILE "ppc.rc"

// In the clock/date display, this next setting controls how much _higher_
//   the date line is on the screen than the natural font spacing would
//   suggest. If this is 0, then the date is drawn under the time at the
//   position the font suggests -- this looks fine, but takes up a lot of
//   room on screen. Ajusting the date upwards makes the clock look
//   cramped, but saves screen space. There's really no way to set this
//   optimally for a specific font than trial and error.
#define DATE_Y_ADJUST 16 

