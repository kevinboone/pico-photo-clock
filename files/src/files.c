/* =======================================================================
 
  files/files.c

  Functions for handling files using the VFAT driver.

  Copyright (c)2023 Kevin Boone, GPLv3.0

 ======================================================================= */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <pico/stdlib.h>
#include <files/files.h>
#include <ff.h>
#include <klib/list.h>
#include <gfx/gfxconsole.h>
#include <gfx/picojpeg.h>
#include <log/log.h>
#include <waveshare_lcd/waveshare_lcd.h>

#define BLACK 0

FATFS fatfs;

/*============================================================================
 * files_fresult_to_errno
 * Convert a FATFS error into a Linux errno, so we can display it using
 *   strerror()
 * ==========================================================================*/
int files_fresult_to_errno (FRESULT err) 
  {
  switch (err)
    {
    case 0: return 0;
    case FR_NOT_ENOUGH_CORE:
      return ENOMEM;  
    case FR_TOO_MANY_OPEN_FILES:
      return ENFILE;  
    case FR_NOT_READY:
      return EAGAIN;  
    case FR_DISK_ERR:
    case FR_NOT_ENABLED:
    case FR_INT_ERR:
      return EIO;  
    case FR_NO_FILE:
    case FR_NO_PATH:
      return ENOENT;  
    case FR_DENIED:
    case FR_EXIST:
      return EACCES;  
    case FR_INVALID_NAME:
      return EIO;  
    case FR_INVALID_OBJECT:
    case FR_INVALID_PARAMETER:
      return EINVAL;  
    case FR_WRITE_PROTECTED:
      return EROFS;  
    case FR_INVALID_DRIVE:
      return ENOENT;  
    case FR_TIMEOUT:
      return ETIME;  
    case FR_NO_FILESYSTEM:
      return EIO;  
    default:
      return EIO;
    }
  return EIO;
  }

/* =======================================================================
   files_pjpeg_callback
   Called by the JPEG decompressor when it wants more deta.
 ======================================================================= */
static unsigned char files_pjpeg_callback (unsigned char *buf, 
        unsigned char buf_size, unsigned char *bytes_actually_read, 
        void *data)
  {
  UINT br;
  FIL *fp = (FIL *)data;
  f_read (fp, buf, buf_size, &br); 
  *bytes_actually_read = (unsigned char)br;
  return 0;
  }

/*=========================================================================
  rgb888_to_rgb565
=========================================================================*/
static inline uint16_t files_rgb888_to_rgb565 (uint8_t r, 
         uint8_t g, uint8_t b)
  {
  return (((uint16_t)r & 0xF8) << 8) | (((uint16_t)g & 0xFC) << 3) | (b >> 3);
  }

/* =======================================================================
   files_show_jpeg
   Draw a JPEG file with the specified path on the LCD display. The
     gfxconsole argument is used only for error messages, which will
     only be visible if the JPEG decompression fails.
 ======================================================================= */
void files_show_jpeg (GfxConsole *console, WSLCD *wslcd, const char *path)
  {
  FIL fp;
  FRESULT fr = f_open (&fp, path, FA_READ);
  if (fr == 0)
    {
    pjpeg_image_info_t image_info;
    unsigned char r = pjpeg_decode_init (&image_info,
                        files_pjpeg_callback, &fp, 0); 

    if (r == 0)
      {
      int decoded_width = image_info.m_width;
      int decoded_height = image_info.m_height;
      int block_width = image_info.m_MCUWidth;
      int block_height = image_info.m_MCUHeight;
      int display_width = wslcd_get_width (wslcd);
      int display_height = wslcd_get_height (wslcd);
      int xoffset = (display_width - decoded_width) / 2;
      int yoffset = (display_height - decoded_height) / 2;

      //printf ("y offset = %d\n", yoffset);

      if (yoffset > 0)
        {
        wslcd_fill_area (wslcd, 0, 0, (uint16_t)display_width, 
          (uint16_t)yoffset, BLACK);
        wslcd_fill_area (wslcd, 0, (uint16_t)(display_height - yoffset), 
           (uint16_t)display_width, (uint16_t)display_height, BLACK);
        }

      if (xoffset > 0)
        {
        wslcd_fill_area (wslcd, 0, 0, (uint16_t)xoffset, 
          (uint16_t)display_height, BLACK);
        wslcd_fill_area (wslcd, (uint16_t)(display_width - xoffset - 1), 0, 
           (uint16_t)display_width, (uint16_t)display_height, BLACK);
        }

      uint16_t *block = 
        malloc ((size_t)(block_width * block_height) * sizeof (uint16_t));
      
      int mcu_x = 0, mcu_y = 0;
      for (;;)
        {
        unsigned char r = pjpeg_decode_mcu();
        if (r)
          {
          // TODO -- show error, if we haven't run out of data
          break;
          }

	int target_x = mcu_x * block_width + xoffset;
	int target_y = mcu_y * block_height + yoffset;

	for (int y = 0; y < block_height; y += 8)
	  {
	  int by_limit = decoded_height - (mcu_y * block_height + y);
	  if (by_limit > 8) by_limit = 8;

	  for (int x = 0; x < block_width; x += 8)
	    {
	    int bx_limit = decoded_width - (mcu_x * block_width + x);
	    if (bx_limit > 8) bx_limit = 8;

	    int src_ofs = (x * 8) + (y * 16);
	    uint8_t *pSrcR = image_info.m_pMCUBufR + src_ofs;
	    uint8_t *pSrcG = image_info.m_pMCUBufG + src_ofs;
	    uint8_t *pSrcB = image_info.m_pMCUBufB + src_ofs;

	    int bc = 0;
	    for (int by = 0; by < by_limit; by++)
	      {
	      for (int bx = 0; bx < bx_limit; bx++)
		 {
		 uint8_t r = *pSrcR++;
		 uint8_t g = *pSrcG++;
		 uint8_t b = *pSrcB++;
		 block [bc] = files_rgb888_to_rgb565 (r, g, b); 
		 bc++; 
		 }

	      pSrcR += (8 - bx_limit);
	      pSrcG += (8 - bx_limit);
	      pSrcB += (8 - bx_limit);
	      }

	    if (target_x + bx_limit < display_width &&
	          target_y + by_limit < display_height)
              {
              wslcd_write_window (wslcd, block, (uint16_t)bx_limit, 
                  (uint16_t)by_limit, (uint16_t)target_x, (uint16_t)target_y);
              }
	    }
	  }

	mcu_x++;
	if (mcu_x == image_info.m_MCUSPerRow)
	  {
	  mcu_x = 0;
	  mcu_y++;
	  }
        }

      free (block);

      }
    else if (r == PJPG_UNSUPPORTED_MODE)
      {
      log_write (console, "Progressive JPEGs not supported\n");
      }
    else
      {
      log_write (console, "Bad JPEG, error %d\n", r);
      }

    f_close (&fp);
    }
  else
    {
    log_write (console, "Can't open: %s\n", path);
    }
  }

/* =======================================================================
   files_list_dir
   Fill the List object with matching files. The list should be 
     initialized, but empty, on entry. Returns an errno on failure.
 ======================================================================= */
int files_list_dir (const char *dir, const char *pattern, List *files)
  {
  DIR dp;
  FILINFO fi;
  FRESULT fr = f_findfirst (&dp, &fi, dir, pattern);
  if (fr == 0)
    {
    do
      {
      list_append (files, strdup (fi.fname));
      f_findnext (&dp, &fi);
      } while (fi.fname[0]);
    }
  return files_fresult_to_errno (fr);
  }

/* =======================================================================
  files_mount
  Initialize the filesystem on the SD card. Returns an errno on error.
 ======================================================================= */
int files_mount (void)
  {
  FRESULT fr = f_mount (&fatfs, "0:", 0);
  return files_fresult_to_errno (fr);
  }

/* =======================================================================
  files_read_to_string
 ======================================================================= */
int files_read_to_string (const char *file, char **s)
  {
  FIL fp;
  FILINFO fi;
  int ret = 0;
  FRESULT fr = f_stat (file, &fi);
  if (fr == 0)
    {
    unsigned int len = (unsigned int)fi.fsize;
    FRESULT fr = f_open (&fp, file, FA_READ);
    if (fr == 0)
      {
      *s = malloc (len + 1); // We will put a zero on the end
      if (*s)
        {
        int br;
        f_read (&fp, *s, len, (UINT *)&br); 
        (*s)[len] = 0;
        }
      else
        ret = ENOMEM;
      f_close (&fp);
      }
    else
      ret = files_fresult_to_errno (fr);
    }
  else
    ret = files_fresult_to_errno (fr);
  return ret;
  }

