/* =======================================================================

  gfx/clock.c

  Displays a time and date

  Copyright (c)2022 Kevin Boone, GPLv3.0

 ======================================================================= */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <waveshare_lcd/waveshare_lcd.h>
#include <gfx/clock.h>
#include <gfx/fonthandler.h>
#include "config.h" 

/* =======================================================================
  Opaque struct
 ======================================================================= */
struct _Clock
  {
  WSLCD *wslcd;
  FontHandler *big_fh;
  FontHandler *small_fh;
  const DS3231 *ds3231;
  unsigned int display_width;
  unsigned int display_height;
  unsigned int big_font_width;
  unsigned int big_font_height;
  unsigned int small_font_width;
  unsigned int small_font_height;
  unsigned int x;
  unsigned int y;
  };

/* =======================================================================
  clock_position_at
 ======================================================================= */
void clock_position_at (Clock *self, unsigned int x, 
     unsigned int y)
  {
  self->x = x;
  self->y = y;
  }

/* =======================================================================
  clock_draw_seconds
  Not currently used
 ======================================================================= */
void clock_draw_seconds (Clock *self)
  {
  (void)self;
  /*
  int s = 1; 

  int dummy;
  ds3231_get_datetime (self->ds3231, &dummy, 
        &dummy, &dummy, &dummy, &dummy, &s);

  if (s == 0)
     clock_draw_all (self);
  else
    {
    char ss[10];
    sprintf (ss, "%02d", s);
    unsigned int y = self->y;
    for (unsigned int i = 0; i < 2; i++)
      {
      unsigned int x = self->x + self->font_width * (i + 6); 
      const uint16_t *glyph565 = fonthandler_get_glyph_565 
	(self->fh, ss[i]);
      wslcd_transfer_window (self->wslcd, glyph565, 
	(uint16_t)self->font_width, (uint16_t)self->font_height, 
       (uint16_t) x, (uint16_t)y); 
      }
    }
  */
  }

/* =======================================================================
  clock_draw_all
 ======================================================================= */
static void clock_draw_char (Clock *self, int size, unsigned int x, 
        unsigned int y, int c)
  {
  FontHandler *fh;
  unsigned int font_width;
  unsigned int font_height;
  if (size == 0)
    {
    fh = self->big_fh;
    font_width = self->big_font_width;
    font_height = self->big_font_height;
    }
  else
    {
    fh = self->small_fh;
    font_width = self->small_font_width;
    font_height = self->small_font_height;
    }

  const uint16_t *glyph565 = fonthandler_get_glyph_565 (fh, c);

  wslcd_write_window (self->wslcd, glyph565, 
      (uint16_t)font_width, (uint16_t)font_height, 
     (uint16_t)x, (uint16_t)y); 
  }


/* =======================================================================
  clock_draw_text
 ======================================================================= */
static void clock_draw_text (Clock *self, int size, unsigned int x, 
             unsigned int y, const char *s)
  {
  unsigned int font_width;
  if (size == 0)
    font_width = self->big_font_width;
  else
    font_width = self->small_font_width;

  while (*s)
    {
    clock_draw_char (self, size, x, y, *s);
    x += font_width;
    s++;
    }
  }

/* =======================================================================
  clock_get_month_name
 ======================================================================= */
static const char *clock_get_month_name (int month)
  {
  static const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                               "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  if (month >= 1 && month <= 12)
    return months[month - 1];
                                
  return "???";
  }

/* =======================================================================
  clock_draw_all
 ======================================================================= */
void clock_draw_all (Clock *self)
  {
  int month = 1;
  int day = 1;
  int h = 1;
  int m = 1;
  int s = 1; 

  int dummy;
  ds3231_get_datetime (self->ds3231, &dummy, 
        &month, &day, &h, &m, &s);

  char ss[20];
  sprintf (ss, "%02d%02d", h, m);

  clock_draw_char (self, 0, self->x + self->big_font_width * 3 / 2 + 10, 
    self->y, ':');

  unsigned int x = self->x;
  unsigned int y = self->y;
  clock_draw_char (self, 0, x, y, ss[0]);
  x += self->big_font_width;
  clock_draw_char (self, 0, x, y, ss[1]);
  x += self->big_font_width + 20;
  clock_draw_char (self, 0, x, y, ss[2]);
  x += self->big_font_width;
  clock_draw_char (self, 0, x, y, ss[3]);
  
  // TODO AM/PM
  // Jan 29
  unsigned int date_width = 6 * self->small_font_width;
  unsigned int clock_width = 4 * self->big_font_width + 20;
  unsigned date_x = 0;
  if (clock_width > date_width)
    date_x = (clock_width - date_width) / 2;

  sprintf (ss, "%s %02d", clock_get_month_name (month), day);
  
  clock_draw_text (self, 1, self->x + date_x, 
    self->y + self->big_font_height - DATE_Y_ADJUST, ss);
 
  if (date_x > 0)
    {
    wslcd_fill_area (self->wslcd, (uint16_t)self->x, 
     (uint16_t)(self->y + self->big_font_height - DATE_Y_ADJUST),
     (uint16_t)(self->x + date_x), 
     (uint16_t)(self->y + self->big_font_height 
        - DATE_Y_ADJUST + self->small_font_height), 
      0); 

    wslcd_fill_area (self->wslcd, (uint16_t)(self->x + clock_width - date_x), 
     (uint16_t)(self->y + self->big_font_height - DATE_Y_ADJUST),
     (uint16_t)(self->x + clock_width), 
     (uint16_t)(self->y + self->big_font_height 
        - DATE_Y_ADJUST + self->small_font_height), 
      0); 

    }
  }

/* =======================================================================
  clock_get_size
 ======================================================================= */
void clock_get_size (const Clock *self, 
               unsigned int *width, unsigned int *height)
  {
  *height = fonthandler_get_font_height (self->big_fh);
  *height += fonthandler_get_font_height (self->small_fh) - 16;
  *width = 4 * fonthandler_get_font_width (self->big_fh) + 20;
  }

/* =======================================================================
  clock_new 
 ======================================================================= */
Clock *clock_new (WSLCD *wslcd, FontHandler *big_fh, FontHandler *small_fh, 
               const DS3231 *ds3231)
  {
  Clock *self = malloc (sizeof (Clock));
  memset (self, 0, sizeof (Clock));
  self->wslcd = wslcd;
  self->big_fh = big_fh;
  self->small_fh = small_fh;
  self->ds3231 = ds3231;
  self->display_width = (unsigned int)wslcd_get_width (wslcd);
  self->display_height = (unsigned int)wslcd_get_height (wslcd);
  self->big_font_width = fonthandler_get_font_width (big_fh);
  self->big_font_height = fonthandler_get_font_height (big_fh);
  self->small_font_width = fonthandler_get_font_width (small_fh);
  self->small_font_height = fonthandler_get_font_height (small_fh);
  return self;
  }

/* =======================================================================
  clock_destroy
 ======================================================================= */
void clock_destroy (Clock *self)
  {
  free (self);
  }




