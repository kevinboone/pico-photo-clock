/*============================================================================
 *
 *  gfx/gfx_console.h
 *
 * A rudimentary text console for display text on an LCD panel. 
 * This implemetation presently only understands ASCII characters and
 *   newlines.
 *
 * Copyright (c)2023 Kevin Boone, GPL v3.0
 *
 * ==========================================================================*/
#pragma once

#include <waveshare_lcd/waveshare_lcd.h>

struct _GfxConsole;
typedef struct _GfxConsole GfxConsole;

#ifdef __cplusplus
extern "C" {
#endif

extern GfxConsole *gfxconsole_new (WSLCD *wslcd);
extern void        gfxconsole_destroy (GfxConsole *self);
extern void        gfxconsole_init (GfxConsole *self);
extern void        gfxconsole_print_char (GfxConsole *self, char s);
extern void        gfxconsole_print (GfxConsole *self, const char *s);

#ifdef __cplusplus
}
#endif

