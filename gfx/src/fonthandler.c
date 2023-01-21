/* =======================================================================

  gfx/fonthandler.c

  An object for rendering font glyphs.

  Copyright (c)2022 Kevin Boone, GPLv3.0

 ======================================================================= */
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <gfx/fonthandler.h>
#include <gfx/picojpeg.h>

/* =======================================================================
  Opaque struct
 ======================================================================= */
struct _FontHandler
  {
  unsigned char *font_data[95];
  unsigned int *font_length[95];
  unsigned int font_width;
  unsigned int font_height;
  unsigned char *glyph_buffer;
  uint16_t *glyph_buffer_565;
  };

/*============================================================================
 * DecoderContext 
 * This is used to carry the state of the JPEG decode process. PicoJPEG
 *   works by requesting new data using a callback function. This function
 *   can only take one application-specific argument, so all the data about
 *   the data buffer and current position must be carried in a single structure.
 * =========================================================================*/
typedef struct _DecoderContext
  {
  unsigned char *data;
  unsigned int pos;
  unsigned int remain;
  } DecoderContext;

/* =======================================================================
 * fonthandler_pjpeg_callback
 * This function is called by PicoJPEG each time it needs more JPEG image
 *   data. The application is expected to read up to buf_size bytes, and
 *   return the number of bytes actually read. In principle, we can signal
 *   an error by returning non-zero; but as the image data is actually
 *   in memory, no (detectable) error is possible.
 * The DecoderContext object keeps track of the amount of data that 
 *   remains to read.
 ======================================================================= */
static unsigned char fonthandler_pjpeg_callback (unsigned char *buf, 
        unsigned char buf_size, unsigned char *bytes_actually_read, 
        void *data)
  {
  DecoderContext *context = data;
  unsigned int avail = context->remain; 
  if (avail > buf_size) avail = buf_size;
  memcpy (buf, context->data + context->pos, avail);
  *bytes_actually_read = (unsigned char)avail;
  context->remain -= avail;
  context->pos += avail;
  return 0;
  }

/* =======================================================================
  fonthandler_get_glyph
 ======================================================================= */
const unsigned char *fonthandler_get_glyph (FontHandler *self, int c)
  {
  memset (self->glyph_buffer, 0, self->font_height * self->font_width);

  if (c >= 32 && c <= 126)
    {
    unsigned char *data = self->font_data[c - ' ']; 
    unsigned int len = *(self->font_length[c - ' ']);
    DecoderContext context;
    context.data = data; // Tell the decompressor callback where to start
    context.pos = 0; // Store where in the input buffer we currently are
    context.remain = len; // Store how much is left to read
    pjpeg_image_info_t image_info;
    unsigned char r = pjpeg_decode_init (&image_info,
                        fonthandler_pjpeg_callback, &context, 0); 
    if (r == 0)
      {
      unsigned int decoded_width = (unsigned int)image_info.m_width;
      unsigned int decoded_height = (unsigned int)image_info.m_height;
      unsigned int block_width = (unsigned int)image_info.m_MCUWidth;
      unsigned int block_height = (unsigned int)image_info.m_MCUHeight;
      unsigned int x_offset = (self->font_width - decoded_width) / 2;

      unsigned int mcu_x = 0, mcu_y = 0;
      for (;;)
	{
	// Get the next block of data from the decoder
	unsigned char r = pjpeg_decode_mcu();
	if (r)
	  {
	  // TODO -- show error, if we haven't run out of data
	  // It isn't necessarily an error if we get here -- most likely
	  //   we've just run out of data to decode
	  break;
	  }

	unsigned int target_x = mcu_x * block_width;
	unsigned int target_y = mcu_y * block_height;

	for (unsigned int y = 0; y < block_height; y += 8)
	  {
	  unsigned int by_limit = decoded_height - (mcu_y * block_height + y);
	  if (by_limit > 8) by_limit = 8;

	  for (unsigned int x = 0; x < block_width; x += 8)
	    {
	    unsigned int bx_limit = decoded_width - (mcu_x * block_width + x);
	    if (bx_limit > 8) bx_limit = 8;

	    unsigned int src_ofs = (x * 8U) + (y * 16U);
	    // Although the image is grayscale, we'll pick up the 
	    //   decompressed data from the green channel -- I guess that's
	    //   just how PicoJPEG works.
	    unsigned char *p = image_info.m_pMCUBufG + src_ofs;

	    for (unsigned int by = 0; by < by_limit; by++)
	      {
	      for (unsigned int bx = 0; bx < bx_limit; bx++)
		 {
		 unsigned char g = *p++;
		 self->glyph_buffer [(target_y + by) * self->font_width 
		   + (x_offset + target_x + bx)] = g; 
		 }

	      p += (8 - bx_limit);
	      }
	    }
	  }

	mcu_x++;
	if (mcu_x == (unsigned int)image_info.m_MCUSPerRow)
	  {
	  mcu_x = 0;
	  mcu_y++;
	  }
	}
      }
    }

  return self->glyph_buffer;
  }

/*=========================================================================
  rgb888_to_rgb565
=========================================================================*/
static inline uint16_t fonthandler_rgb888_to_rgb565 (uint8_t r, 
         uint8_t g, uint8_t b)
  {
  return (((uint16_t)r & 0xF8) << 8) | (((uint16_t)g & 0xFC) << 3) | (b >> 3);
  }

/* =======================================================================
  fonthandler_get_glyph_565
 ======================================================================= */
const uint16_t *fonthandler_get_glyph_565 (FontHandler *self, int c)
  {
  fonthandler_get_glyph (self, c);
  unsigned int len = self->font_width * self->font_height;
  for (unsigned int i = 0; i < len; i++)
    {
    unsigned char b = self->glyph_buffer[i];
    self->glyph_buffer_565[i] = fonthandler_rgb888_to_rgb565 (b, b, b); 
    }
  return self->glyph_buffer_565;
  }

/* =======================================================================
  fonthandler_get_font_width
 ======================================================================= */
unsigned int fonthandler_get_font_width (const FontHandler *self)
  {
  return self->font_width;
  }

/* =======================================================================
  fonthandler_get_font_height
 ======================================================================= */
unsigned int fonthandler_get_font_height (const FontHandler *self)
  {
  return self->font_height;
  }

/* =======================================================================
  fonthandler_new 
 ======================================================================= */
FontHandler *fonthandler_new (unsigned char *font_data[95], 
     unsigned int *font_length[95], unsigned int font_width, 
     unsigned int font_height)
  {
  FontHandler *self = malloc (sizeof (FontHandler));
  memset (self, 0, sizeof (FontHandler));
  memcpy (self->font_data, font_data, 95 * sizeof (char *)); 
  memcpy (self->font_length, font_length, 95 * sizeof (int *)); 
  self->font_width = font_width;
  self->font_height = font_height;
  self->glyph_buffer = malloc (font_height * font_width);
  self->glyph_buffer_565 = malloc (font_height * font_width 
     * sizeof (uint16_t));
  return self;
  }

/* =======================================================================
  fonthandler_destroy
 ======================================================================= */
void fonthandler_destroy (FontHandler *self)
  {
  free (self->glyph_buffer_565);
  free (self->glyph_buffer);
  free (self);
  }




