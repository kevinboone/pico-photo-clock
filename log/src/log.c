/* =======================================================================
  
  log.c

  Copyright (c)2022 Kevin Boone, GPLv3.0

 ======================================================================= */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <pico/stdlib.h>
#include <gfx/gfxconsole.h>

/* =======================================================================
   log_write
 ======================================================================= */
void log_write (GfxConsole *console, const char *fmt,...)
  {
  va_list ap;
  va_start (ap, fmt);
  char *s;
  vasprintf (&s, fmt, ap);
#if PICO_ON_DEVICE
  gfxconsole_print (console, s);
#else 
  printf ("%s", s);
#endif
  va_end (ap);
  free (s);
  }


