/* =======================================================================

  gfx/gfxconsole.c

  Copyright (c)2022 Kevin Boone, GPLv3.0

 ======================================================================= */
#include <string.h>
#include <stdlib.h>
#include <waveshare_lcd/waveshare_lcd.h>
#include <gfx/fonts.h>
#include <gfx/gfxconsole.h>

/* =======================================================================
  Opaque struct
 ======================================================================= */
struct _GfxConsole
  {
  WSLCD *wslcd;
  int row;
  int col;
  int rows;
  int cols;
  int width;
  int height;
  int font_width;
  int font_height;
  sFONT *font;
  uint16_t *glyph_buffer;
  };

/* =======================================================================
  get_glyph_offset 
 ======================================================================= */
static void gfxconsole_get_glyph_offset (const GfxConsole *self, 
          int *x, int *y)
  {
  *x = self->col * self->font_width;
  *y = self->row * self->font_height;
  }

/* =======================================================================
  font_get_offset 
 ======================================================================= */
static const uint8_t *gfxconsole_font_get_offset (const sFONT *font, char c)
  {
  return font->table + ((c - 32) * (font->Bytes * font->Height));
  }

/* =======================================================================
  fill_glyph_buffer 
 ======================================================================= */
static void gfxconsole_fill_glyph_buffer (GfxConsole *self, char c)
  {
  const uint16_t white = 0xFFFF; // TODO real colour

  int font_height = self->font_height;
  int font_width = self->font_width;

  memset (self->glyph_buffer, 0, sizeof(uint16_t) *
     (size_t)self->font_width * (size_t)self->font_height); 
  // TODO background colour

  const uint8_t *offset = gfxconsole_font_get_offset (self->font, c);
  if (!offset)
    offset = gfxconsole_font_get_offset (self->font, '?');
  int font_width_bytes = self->font->Bytes;

  for (int i = 0; i < font_height; i++)
    {
    const uint8_t *cur_offset = offset + i * font_width_bytes;
    uint8_t cur_byte = *cur_offset;

    register int cur_bit_p = 0;
    for (int j = 0; j < font_width; j++)
      {
      if (cur_byte & 0x80)
        {
        self->glyph_buffer [i * font_width + j] = white;
        }
      cur_byte <<= 1;
      cur_bit_p++;
      if (cur_bit_p == 8)
        {
        cur_offset++;
        cur_bit_p = 0;
        cur_byte = *cur_offset;
        }
      }
    }
  }

/* =======================================================================
  print_newline
 ======================================================================= */
static void gfxconsole_print_newline (GfxConsole *self)
  {
  self->row++;
  self->col = 0;
  }

/* =======================================================================
  print_printing_char
 ======================================================================= */
static void gfxconsole_print_printing_char (GfxConsole *self, char c)
  {
  if (self->col < self->cols)
    {
    int xstart, ystart;
    gfxconsole_get_glyph_offset (self, &xstart, &ystart);
    gfxconsole_fill_glyph_buffer (self, c);
    wslcd_write_window (self->wslcd, self->glyph_buffer, 
      (uint16_t)self->font_width, (uint16_t)self->font_height, 
      (uint16_t)xstart, (uint16_t)ystart);
    }
  self->col++;
  // TODO
  }

/* =======================================================================
  print_char
 ======================================================================= */
void gfxconsole_print_char (GfxConsole *self, char c)
  {
  if (c & 0x80) return;
  
  switch (c)
    {
    case 10:
      gfxconsole_print_newline (self);
      break;
    default:
      gfxconsole_print_printing_char (self, c);
    } 
  }

/* =======================================================================
  print 
 ======================================================================= */
void gfxconsole_print (GfxConsole *self, const char *s)
  {
  while (*s)
    {
    gfxconsole_print_char (self, *s);
    s++;
    }
  }

/* =======================================================================
  init 
 ======================================================================= */
void gfxconsole_init (GfxConsole *self)
  {
  self->row = 0;
  self->col = 0;
  self->font = &Font16;
  self->font_width = self->font->Width;
  self->font_height = self->font->Height;
  self->width = wslcd_get_width (self->wslcd);
  self->height = wslcd_get_height (self->wslcd);
  self->rows = self->height / self->font_height;
  self->cols = self->width / self->font_width;
  self->glyph_buffer = 
    malloc ((size_t)self->font_width 
       * (size_t)self->font_height * sizeof (uint16_t));
  wslcd_clear (self->wslcd, 0);
  }

/* =======================================================================
  gfxconsole_new 
 ======================================================================= */
GfxConsole *gfxconsole_new (WSLCD *wslcd)
  {
  GfxConsole *self = malloc (sizeof (GfxConsole));
  memset (self, 0, sizeof (GfxConsole));
  self->wslcd = wslcd;
  return self;
  }

/* =======================================================================
  gfxconsole_destroy
 ======================================================================= */
void gfxconsole_destroy (GfxConsole *self)
  {
  if (self->glyph_buffer) free (self->glyph_buffer);
  free (self);
  }



