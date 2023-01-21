/*============================================================================
 
   waveshare_lcd/waveshare_lcd.h

   A driver for the ILI9488 LCD panel controller in the Waveshare
   320x480 display for Pico  

   Usage:
 
   WSLCD *wslcd = wslcd_new (...);
   wslcd_init (wslcd);
   wslcd_write_window (wslcd, buffer, width, height, x, y);
   ...
   wslcd_destroy (wslcd);
 
  Copyright (c)2022-3 Kevin Boone, GPL v3.0
  
 * ==========================================================================*/
#pragma once

#include <stdint.h>

#if PICO_ON_DEVICE
#include <hardware/spi.h>
#endif 

#define	WSLCD_COLOR  uint16_t
#define	WSLCD_POINT uint16_t
#define	WSLCD_LENGTH uint16_t

/*============================================================================
 * scan directions
 * ==========================================================================*/
typedef enum _WSLCDScanDir 
  {
  WSLCD_SCAN_PORTRAIT  = 0,
  WSLCD_SCAN_LANDSCAPE
  } WSLCDScanDir; 

typedef struct _WSLCD WSLCD;

/** Create a new instance of the driver, specifying the various
    pin assignments. Note that this method only initializes the
    object, it doesn't initialize the hardware. */
extern WSLCD *wslcd_new (int spi, uint gpio_cs, uint gpio_miso, 
    uint gpio_mosi, uint gpio_sck, uint gpio_rst, uint gpio_dc, 
    uint gpio_bl, int baud_rate, WSLCDScanDir scan_dir);

extern void wslcd_destroy (WSLCD *self);

/** Initialize the hardware. */
extern void wslcd_init (WSLCD *self);

/** Get the width in pixels, which will depend on the scan direction. */
extern int wslcd_get_width (const WSLCD *self);

/** Get the height in pixels, which will depend on the scan direction. */
extern int wslcd_get_height (const WSLCD *self);

/** Fills an area with a specified RGB565 color. Note that the paramters
    are the start and end points, not start point and size. Also, the 
    arguments are inclusive in start and exclusive in end. That is, the 
    end coordinates are the first row and column that are _not_ filled. 
    Working this way makes it easy to get the end locations by
    adding width or height to the start locations. The full screen in
    landscape mode needs xstart=0 ystart=0 xend=480 yend=320. */
extern void wslcd_fill_area (const WSLCD *self, uint16_t xstart, 
         uint16_t ystart, uint16_t xend, uint16_t yend,        
         uint16_t colour);

/** Set a specific pixel to an RGB555 colour. The function checks that the
    position is within range, but beware that there is no way to check for
    a carelessly-computed negative value with unsigned integers. */ 
extern void wslcd_set_pixel (const WSLCD *self, uint16_t x, uint16_t y, 
      uint16_t colour);

/** Clear the whole screen to the specific colour. */
extern void wslcd_clear (WSLCD *self, uint16_t colour);

/** Write the data at buff to the display, with coordinates as 
    specified. The window to be written can be any size, in principle,
    but it isn't possible to draw the entire screen from the Pico's RAM,
    because there isn't enough. */ 
extern void wslcd_write_window 
        (const WSLCD *self, const uint16_t *buff, uint16_t w, 
        uint16_t h, uint16_t x, uint16_t y);

// DOES NOT WORK. The hardware does not even provide a way to select 
//   between read and write operations.
extern void wslcd_read_window 
        (const WSLCD *self, uint16_t *buff, uint16_t w, 
        uint16_t h, uint16_t x, uint16_t y);

