/*===========================================================================

  files/files.h

  functions for handling files using the FAT filesystem library.

  Copyright (2)2022 Kevin Boone, GPLv3.0 

===========================================================================*/

#pragma once

#include <klib/list.h>
#include <gfx/gfxconsole.h>
#include <waveshare_lcd/waveshare_lcd.h>

#ifdef __cplusplus
extern "C" { 
#endif

/** Show a JPEG file on the display. The GfxConsole here is used to
    display error messages if the operation fails. */
extern void files_show_jpeg (GfxConsole *console, WSLCD *wslcd, 
               const char *path);

/** Fill the List with files in the directory dir that match the pattern. */
extern int files_list_dir (const char *dir, const char *pattern, List *files);

/** Initialize the FATFS library. Returns an errno on failure. */
extern int files_mount (void);

extern int files_read_to_string (const char *file, char **s);

#ifdef __cplusplus
}
#endif



