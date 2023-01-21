/*============================================================================
 *
 * screens/photoclock.h
 *
 * Copyright (c)2023 Kevin Boone, GPL v3.0
 *
 * ==========================================================================*/
#pragma once

#include <waveshare_lcd/waveshare_lcd.h>
#include <screens/settings.h>
#include <ds3231/ds3231.h>
#include <klib/list.h>
#include <gfx/gfxconsole.h>

struct _PhotoClock;
typedef struct _PhotoClock PhotoClock;

#ifdef __cplusplus
extern "C" {
#endif

extern PhotoClock  *photoclock_new (const Settings *settings, 
                       WSLCD *wslcd, const DS3231 *ds3231, 
                       const List *file_list, GfxConsole *console);
extern void         photoclock_destroy (PhotoClock *self);
extern void         photoclock_draw_current_background (PhotoClock *self);
extern void         photoclock_draw_next_background (PhotoClock *self);
extern void         photoclock_tock (PhotoClock *self);
extern void         photoclock_tick (PhotoClock *self);
extern void         photoclock_draw_all (PhotoClock *self);

#ifdef __cplusplus
}
#endif


