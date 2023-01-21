/* =======================================================================

  pico-photo-clock
  
  settings.c

  Copyright (c)2023 Kevin Boone, GPLv3.0

 ======================================================================= */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <pico/stdlib.h>
#include <files/files.h>
#include <screens/settings.h>

/* =======================================================================
  settings_fron_file
 ======================================================================= */
int settings_from_file (Settings *settings, const char *file)
  {
  char *s;
  int ret = files_read_to_string (file, &s);
  if (ret == 0)
    {
    char *line = strtok (s, "\n");
    while (line)
      {
      if (line[0])
        {
        int l = (int)strlen (line);
        if (line[l - 1] == '\r') line[l - 1] = 0;
        if (line[0] != '#')
          {
	  char *eqpos = strchr (line, '=');
	  if (eqpos)
	    {
	    *eqpos = 0;
	    const char *key = line;
	    const char *value = eqpos + 1;
	    if (strcmp (key, "mins_per_background_change") == 0)
	      settings->mins_per_background_change = (unsigned int)atoi (value);
	    else if (strcmp (key, "clock_x") == 0)
	      settings->clock_x = (unsigned int)atoi (value);
	    else if (strcmp (key, "clock_y") == 0)
	      settings->clock_y = (unsigned int)atoi (value);
	    printf ("key=%s, val=%s\n", key, value);
	    }
	  }
        }
      line = strtok (NULL, "\n");
      }

    free (s);
    }
  return ret;
  }

