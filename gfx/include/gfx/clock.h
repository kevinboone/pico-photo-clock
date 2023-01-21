/*============================================================================
 *  gfx/gfx_console.h
 *
 * Displays a time and date
 *
 * Copyright (c)2023 Kevin Boone, GPL v3.0
 * ==========================================================================*/
#pragma once

#include <waveshare_lcd/waveshare_lcd.h>
#include <gfx/fonthandler.h>
#include <ds3231/ds3231.h>

struct _Clock;
typedef struct _Clock Clock;

#ifdef __cplusplus
extern "C" {
#endif

/** Initialise the object, specifying the display, two font handlers, and
    the RTC. big_fh is used to dispay the time digits, small_fh for the
    month/day. */
extern Clock *clock_new (WSLCD *wslcd, FontHandler *big_fh, 
               FontHandler *small_fh, const DS3231 *ds3231);

extern void clock_destroy (Clock *self);

/** Specify where the clock will be displayed on the screen. */
extern void clock_position_at (Clock *self, unsigned int x, 
               unsigned int y);

/** Force and update of the entire clock -- time and date.
    Because we don't display seconds yet, this is the only function
    provided to update the display, and it should be called once
    a minute (but pobably no more).  */
extern void clock_draw_all (Clock *self);

/** Return the overall size of the clock, calculated from the supplied
    fonts. */
extern void clock_get_size (const Clock *self, 
               unsigned int *width, unsigned int *height);

#ifdef __cplusplus
}
#endif


