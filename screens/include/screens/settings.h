/*===========================================================================

  settings.h

  Copyright (2)2023 Kevin Boone, GPLv3.0 

===========================================================================*/

#pragma once

typedef struct _Settings
  {
  unsigned int mins_per_background_change;
  unsigned int clock_x; 
  unsigned int clock_y; 
  } Settings;


#ifdef __cplusplus
extern "C" {
#endif

extern int settings_from_file (Settings *settings, const char *file);

#ifdef __cplusplus
}
#endif



