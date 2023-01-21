/* =======================================================================

  screens/photoclock.c

  Copyright (c)2022 Kevin Boone, GPLv3.0

 ======================================================================= */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <waveshare_lcd/waveshare_lcd.h>
#include <screens/photoclock.h>
#include <ds3231/ds3231.h>
#include <gfx/fonthandler.h>
#include <gfx/clock.h>
#include <files/files.h>
#include <gfx/gfxconsole.h>
#include <klib/list.h>
//#include "courier_bold_72.h"
//#include "courier_bold_36.h"
#include "dejavu_sans_mono_72.h"
#include "dejavu_sans_mono_36.h"

/* =======================================================================
  Opaque struct
 ======================================================================= */
struct _PhotoClock
  {
  WSLCD *wslcd;
  const Settings *settings;
  FontHandler *big_fh;
  FontHandler *small_fh;
  Clock *clock;
  unsigned int current_file;
  unsigned int ticks;
  unsigned int mins_this_background;
  const List *file_list;
  GfxConsole *console;
  uint16_t *indexes;
  unsigned int nfiles; // Number of photos
  const DS3231 *ds3231;
  unsigned int display_width;
  unsigned int display_height;
  };

/* =======================================================================
  photoclock_get_clock_size
 ======================================================================= */
/*
 * static void photoclock_get_clock_size (const PhotoClock *self, 
               unsigned int *width, unsigned int *height)
  {
  return clock_get_size (self->clock, width, height);
  }
*/

/* =======================================================================
  photoclock_draw_current_background 
 ======================================================================= */
void photoclock_draw_current_background (PhotoClock *self)
  {
  unsigned int l = (unsigned int)list_length (self->file_list);
  if (l == 0)
    wslcd_clear (self->wslcd, 0);
  else
    {
    if (self->current_file >= l)
      wslcd_clear (self->wslcd, 0);
    else
      {
      int filenum = self->indexes[self->current_file];
      const char *file = list_get (self->file_list, 
        (int)filenum);
      printf ("Setting background to %s\n", file);
      files_show_jpeg (self->console, self->wslcd, file);
      }
    }
  }

/* =======================================================================
  photoclock_draw_next_background
 ======================================================================= */
void photoclock_draw_next_background (PhotoClock *self)
  {
  unsigned int l = (unsigned int)list_length (self->file_list);
  if (l > 0)
    {
    self->current_file++;
    if (self->current_file >= l)
      self->current_file = 0;
    photoclock_draw_current_background (self);
    printf ("Changing to background %d\n", self->current_file);
    }
  clock_draw_all (self->clock);
  }

/* =======================================================================
  photoclock_tock
 ======================================================================= */
void photoclock_tock (PhotoClock *self)
  {
  self->mins_this_background++;
  if (self->mins_this_background >= self->settings->mins_per_background_change)
    {
    self->mins_this_background = 0;
    photoclock_draw_next_background (self);
    }
  else
    clock_draw_all (self->clock);
  }
  
/* =======================================================================
  photoclock_tick
 ======================================================================= */
void photoclock_tick (PhotoClock *self)
  {
  //clock_draw_seconds (self->clock);
  self->ticks++;
  if (self->ticks == 60)
    {
    self->ticks = 0;
    photoclock_tock (self); 
    }
  }

/* =======================================================================
  photoclock_draw_all
 ======================================================================= */
void photoclock_draw_all (PhotoClock *self)
  {
  photoclock_draw_current_background (self);
  clock_draw_all (self->clock);
  }

/* =======================================================================
  photoclock_new 
 ======================================================================= */
PhotoClock *photoclock_new (const Settings *settings, WSLCD *wslcd,
              const DS3231 *ds3231, const List *file_list, 
              GfxConsole *console)
  {
  PhotoClock *self = malloc (sizeof (PhotoClock));
  memset (self, 0, sizeof (PhotoClock));
  self->console = console;
  self->wslcd = wslcd;
  self->settings = settings;
  self->file_list = file_list;
  self->current_file = 0;
  self->ticks = 0;
  self->mins_this_background = 0;
  self->ds3231 = ds3231;

  self->display_width = (unsigned)wslcd_get_width (wslcd);
  self->display_height = (unsigned)wslcd_get_height (wslcd);

  self->big_fh = fonthandler_new (dejavu_sans_mono_72_data, 
     dejavu_sans_mono_72_length, dejavu_sans_mono_72_width, 
     dejavu_sans_mono_72_height);

  //self->big_fh = fonthandler_new (courier_bold_72_data, 
  //   courier_bold_72_length, courier_bold_72_width, courier_bold_72_height);

  self->small_fh = fonthandler_new (dejavu_sans_mono_36_data, 
     dejavu_sans_mono_36_length, dejavu_sans_mono_36_width, 
     dejavu_sans_mono_36_height);

  //self->small_fh = fonthandler_new (courier_bold_36_data, 
  //   courier_bold_36_length, courier_bold_36_width, courier_bold_36_height);

  self->clock = clock_new (wslcd, self->big_fh, self->small_fh, ds3231);

  unsigned int clock_width, clock_height;
  clock_get_size (self->clock, &clock_width, &clock_height);

  unsigned int clock_x = self->settings->clock_x;
  unsigned int clock_y = self->settings->clock_y;

  if (clock_x + clock_width > self->display_width)
    clock_x = self->display_width - clock_width - 1;
  if (clock_y + clock_height > self->display_height)
    clock_y = self->display_height - clock_height - 1;

  //printf ("disp width=%d\n", self->display_width);

  clock_position_at (self->clock, clock_x, clock_y);
  //printf ("cw =%d ch = %d\n", clock_x, clock_y);

  int nfiles = list_length (file_list);
  self->nfiles = (unsigned)nfiles;

  self->indexes = malloc ((size_t)nfiles * sizeof (uint16_t));
  for (int i = 0; i < nfiles; i++)
    {
    self->indexes[i] = (uint16_t)(nfiles - i - 1);
    }
  

  if (nfiles > 1) 
    {
    int a, b, c, d, e, f;
    ds3231_get_datetime (self->ds3231, &a, 
        &b, &c, &d, &e, &f);
    srand48 (a + b + c + d + e + f);
    int i;
    for (i = nfiles - 1; i > 0; i--) 
      {
      int j = (int) (drand48() * (i + 1));
      int t = self->indexes[j];
      self->indexes[j] = self->indexes[i];
      self->indexes[i] = (uint16_t)t;
      }
    }

  return self;
  }

/* =======================================================================
  photoclock_destroy
 ======================================================================= */
void photoclock_destroy (PhotoClock *self)
  {
  clock_destroy (self->clock);
  fonthandler_destroy (self->big_fh);
  fonthandler_destroy (self->small_fh);
  free (self->indexes);
  free (self);
  }


