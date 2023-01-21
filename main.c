/* =======================================================================

  pico-photo-clock
  
  main.c

  Copyright (c)2022 Kevin Boone, GPLv3.0

 ======================================================================= */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <pico/stdlib.h>
#include <ds3231/ds3231.h>
#include <waveshare_lcd/waveshare_lcd.h>
#include <files/files.h>
#include <sdcard/sdcard.h>
#include <gfx/gfxconsole.h>
#include <gfx/clock.h>
#include <screens/photoclock.h>
#include <screens/settings.h>
#include <klib/list.h>
#include <log/log.h>

#if PICO_ON_DEVICE
#include <hardware/i2c.h>
#endif
#include "config.h"
#include "version.h"

PhotoClock *photoclock = NULL;
List *file_list;

/* =======================================================================
  tick 
  Called by the input handler at one-second intervals
 ======================================================================= */
void tick (PhotoClock *photoclock)
  {
  photoclock_tick (photoclock);
  }

/* =======================================================================
   readline() is a crude function to read a line of text. The Pico SDK 
     seems to implement gets(), etc., but they don't work for me. 
 ======================================================================= */
static char _getchar (PhotoClock *photoclock)
  {
  int c;
  int jiffies = 0;
  while ((c = getchar_timeout_us (0)) < 0) 
    {
    sleep_ms (10);
    //printf ("gpio=%d\n", gpio_get (WSLCD_TP_INT));
    jiffies++;
    if (jiffies == 100)
      {
      tick (photoclock);
      jiffies = 0;
      }
    }
  return (char)c;
  }

static char *readline (char *str, unsigned int L, PhotoClock *photoclock)
  {
#if PICO_ON_DEVICE
  int eol = '\r';
#else
  int eol = '\n';
#endif
  char u, *p;
  for (p = str, u = _getchar (photoclock); 
         u != eol && p-str < L; 
         u = _getchar (photoclock)) 
    {
    *p++ = u;
#if PICO_ON_DEVICE
    putchar (u);
#endif
    }
  *p = 0;  
  return str;
  }

/* =======================================================================
  help 
 ======================================================================= */
static void help (void)
  {
  printf 
  ("set {year} {month} {day] {hour} {minute} {second} -- set date/time\n");
  printf 
  ("get              -- show date/time\n");
  printf 
  ("help             -- show this\n");
  printf 
  ("list             -- list discovered filenames\n");
  printf 
  ("next             -- show next background in list\n");
  printf 
  ("show {filename}  -- show the image file (from 'list')\n");
  printf 
  ("version          -- show program version\n");
  }
 
/* =======================================================================
  cmd_loop 
 ======================================================================= */
void cmd_loop (PhotoClock *photoclock, GfxConsole *gfxconsole, 
        DS3231 *ds3231, WSLCD *wslcd, const Settings *settings)
  {
  bool stop = false;
  const int L = 128;
  char str[L + 1];
  while (!stop)
    {
    printf ("> ");
    readline (str, L, photoclock);
    printf ("\n");
    if (strncmp (str, "set ", 4) == 0)
      {
      // If the user enters 'set xxx...', read six numbers and use them
      //   to set the date and time on the DS3231
      int year, month, day, hour, min, sec;
      if (sscanf (str + 4, "%d %d %d %d %d %d", 
          &year, &month, &day, &hour, &min, &sec) == 6)
        {
        int ret = ds3231_set_datetime (ds3231, year, month, day, 
           hour, min, sec);
        if (ret != 0)
          printf ("Failed to set date: %s\n", strerror (ret));
        photoclock_draw_all (photoclock);
        }
      else
        {
        printf ("Enter six numbers: "
                 "year month day hour min sec\n");
        }
      }
    else if (strncmp (str, "get", 3) == 0)
      {
      // If the user enter 'get', print the current date, time, and
      //   temperature from the DS3231.
      int year, month, day, hour, min, sec;
      ds3231_get_datetime (ds3231, &year, &month, &day, &hour, &min, &sec);
      printf ("%d-%d-%d %02d:%02d:%02d\n", year, month, day,
        hour, min, sec);
      int t = ds3231_get_temp (ds3231);
      // Temperature is in 'millicelcius', so we must divide by 1000
      printf ("temp=%g deg. C\n", t / 1000.0);
      }
    else if (strncmp (str, "show ", 5) == 0)
      {
      files_show_jpeg (gfxconsole, wslcd, str + 5);
      }
    else if (strncmp (str, "next", 4) == 0)
      {
      photoclock_draw_next_background (photoclock);
      }
    else if (strncmp (str, "list", 4) == 0)
      {
      for (int i = 0; i < list_length (file_list); i++)
	{
	const char *file = list_get (file_list, i);
	printf ("file: %s\n", file);
	}
      }
    else if (strncmp (str, "version", 7) == 0)
      {
      printf (PROG_NAME " version %d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR,
         VERSION_MICRO);
      printf ("Copyright (c)2023 Kevin Boone\n");
      printf ("Released under the terms of the GNU Public Licence, v3.0\n");
      }
    else if (strncmp (str, "settings", 8) == 0)
      {
/*
    int error = settings_from_file (&settings, SETTINGS_FILE);
    if (error)
      {
      log_write (gfxconsole, "Couldn't read settings file: %sn", strerror (error)); 
      }
    }
*/
    printf ("clock_x=%d\n", settings->clock_x);
    printf ("clock_y=%d\n", settings->clock_y);
    printf ("mins_per_background_change=%d\n", settings->mins_per_background_change);
      }
    else if (strncmp (str, "quit", 4) == 0)
      {
      stop = true;
      }
    else if (strncmp (str, "help", 4) == 0)
      {
      help();
      }
    else
      printf ("Enter 'help' for commands\n");
    }
  }

/* =======================================================================
  main
 ======================================================================= */
int main ()
  {
  gpio_init (WSLCD_TP_INT);
  gpio_set_dir (WSLCD_TP_INT, GPIO_IN);

  file_list = list_create (free);

  stdio_init_all();

  // Initialize the real-time clock driver
  DS3231 *ds3231 = ds3231_new (CLOCK_I2C_DEV, CLOCK_SDA, 
   CLOCK_SCL, CLOCK_I2C_BAUD);

  // Get the LCD up early, so we can use it for error messages
  WSLCD *wslcd = wslcd_new (WSLCD_SPI, WSLCD_CS, WSLCD_MISO, WSLCD_MOSI, 
    WSLCD_SCK, WSLCD_RST, WSLCD_DC, WSLCD_BL, WSLCD_BAUD, WSLCD_SCAN_LANDSCAPE);

  wslcd_init (wslcd);

  // Put the graphical console on the LCD. With luck, we'll never see
  //   anything on this console, because it will be replaced by the
  //   first photo. If we see the graphical console, the program
  //   has crashed.
  GfxConsole *gfxconsole = gfxconsole_new (wslcd);
  gfxconsole_init (gfxconsole);
  log_write (gfxconsole, PROG_NAME " starting...\n");

  Settings settings;
  settings.mins_per_background_change = DEFAULT_MINS_PER_PHOTO;
  settings.clock_x = CLOCK_DEFAULT_X;
  settings.clock_y = CLOCK_DEFAULT_Y;
 
  // Initialze the SD card. Do this last, because it's the most likely
  //   to fail, and we want to see any error message.
  SDCard *sdcard = sdcard_new (SD_SPI, SD_DRIVE_STRENGTH, SD_CHIP_SELECT, 
   SD_MISO, SD_MOSI, SD_SCK, SD_BAUD);

  sdcard_init (sdcard);
  SDError sderr = sdcard_insert_card (sdcard);
  if (sderr == 0)
    {
    log_write (gfxconsole, "Card initalized\n");
    int ret = files_mount ();
    if (ret == 0)
      {
      // Assuming that the SD initializes, get a list of matching
      //   photo files into file_list.
      log_write (gfxconsole, "Listing files... ");
      files_list_dir (JPEG_DIR, JPEG_PATTERN, file_list);
      log_write (gfxconsole, "found %d\n", list_length (file_list));
      }
    else
      {
      log_write (gfxconsole, "Couldn't mount card: %s\n", strerror (ret));
      } 
    }
  else
    {
    log_write (gfxconsole, "Card init failed, error %d\n", sderr);
    }

  if (sderr == 0)
   {
    // Read settings from file
    log_write (gfxconsole, PROG_NAME " Reading settings...\n");
    int error = settings_from_file (&settings, SETTINGS_FILE);
    if (error)
      {
      log_write (gfxconsole, "Couldn't read settings file: %sn", strerror (error)); 
      }
    }

  printf ("clock_x= %d\n", settings.clock_x);
  printf ("clock_y= %d\n", settings.clock_y);
  printf ("mins_per_background_change = %d\n", settings.mins_per_background_change);

  // Create the main display, and draw it
  photoclock = photoclock_new (&settings, wslcd, ds3231, file_list, gfxconsole);
  photoclock_draw_all (photoclock);

  // Process commands.
  cmd_loop (photoclock, gfxconsole, ds3231, wslcd, &settings);

  // In the Pico version, we never get here. But clean up anyway, so we
  //   can check for memory leaks in a Linux build.
  if (photoclock) photoclock_destroy (photoclock);
  sdcard_destroy (sdcard);
  gfxconsole_destroy (gfxconsole);
  wslcd_destroy (wslcd);
  ds3231_destroy (ds3231);
  if (file_list) list_destroy (file_list);
  }


