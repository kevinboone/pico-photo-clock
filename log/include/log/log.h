/*============================================================================
 *
 * log/log.h
 *
 * Copyright (c)2023 Kevin Boone, GPL v3.0
 * 
* ==========================================================================*/
#pragma once

#include <gfx/gfxconsole.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void log_write (GfxConsole *console, const char *fmt,...);

#ifdef __cplusplus
}
#endif


