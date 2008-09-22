/* unicap
 *
 * Copyright (C) 2004-2008 Arne Caspari ( arne@unicap-imaging.org )
 *
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef   	UCIL_H_
# define   	UCIL_H_

#ifdef __cplusplus
#define UCIL_BEGIN_DECLS extern "C" {
#define UCIL_END_DECLS }
#else
#define UCIL_BEGIN_DECLS
#define UCIL_END_DECLS
#endif

#include <unicap.h>
#include <ucil_version.h>
#include <glib.h>

#define UCIL_FOURCC(a,b,c,d) (unsigned int)((((unsigned int)d)<<24)+(((unsigned int)c)<<16)+(((unsigned int)b)<<8)+a)

typedef enum
{
   UCIL_COLORSPACE_RGB24, 
   UCIL_COLORSPACE_RGB32, 
   UCIL_COLORSPACE_Y8, 
   UCIL_COLORSPACE_YUV, 
   
   UCIL_COLORSPACE_UNKNOWN = 0xfffff
} ucil_colorspace_t;

typedef enum
{
   UCIL_INTERP_NEAREST, 
   UCIL_INTERP_BILINEAR
} ucil_interpolation_type_t;

struct ucil_rgb24_t
{
      unsigned char r;
      unsigned char g;
      unsigned char b;
};

typedef struct ucil_rgb24_t ucil_rgb24_t;

struct ucil_rgb32_t
{
      unsigned char r;
      unsigned char g;
      unsigned char b;
      unsigned char a;
};

typedef struct ucil_rgb32_t ucil_rgb32_t;

struct ucil_yuv_t
{
      unsigned char y;
      unsigned char u;
      unsigned char v;
};

typedef struct ucil_yuv_t ucil_yuv_t;

struct ucil_y8_t
{
      unsigned char y;
};

typedef struct ucil_y8_t ucil_y8_t;

struct ucil_color_t
{
      ucil_colorspace_t colorspace;
      
      union
      {
	    ucil_yuv_t yuv;
	    ucil_rgb24_t rgb24;
	    ucil_rgb32_t rgb32;
	    ucil_y8_t y8;
      };
};

typedef struct ucil_color_t ucil_color_t;

struct ucil_convolution_mask_t
{
      ucil_colorspace_t colorspace;
      
      ucil_color_t *mask;
      int size;
};

typedef struct ucil_convolution_mask_t ucil_convolution_mask_t;

struct ucil_font_object_t
{
      void *context;
      void *layout;
};

typedef struct ucil_font_object_t ucil_font_object_t;


struct ucil_video_file_object_t
{
      int ucil_codec_id;
      void *codec_data;
};

typedef struct ucil_video_file_object_t ucil_video_file_object_t;


UCIL_BEGIN_DECLS

typedef void(*ucil_processing_info_func_t)(void *data, double pos);



unicap_status_t ucil_check_version( unsigned int major, unsigned int minor, unsigned int micro );




/*

  Drawing functions

*/
/**
 * ucil_set_pixel: 
 * @data_buffer: target buffer 
 * @color: an #ucil_color_t. The colorspace of the color has to match
 * the colorspace of the data_buffer.
 * @x: x position
 * @y: y position
 * 
 * Draws a pixel on the data buffer.
 */
void ucil_set_pixel( unicap_data_buffer_t *data_buffer, ucil_color_t *color, int x, int y );
/**
 * ucil_set_pixel_alpha: 
 * @data_buffer: target buffer
 * @color: an #ucil_color_t. The colorspace of the color has to match
 * the colorspace of the data_buffer.
 * @alpha: the alpha value that should be applied to this pixel
 * @x: x position
 * @y: y position
 * 
 * Draws a pixel on the data buffer, applying an alpha ( transparency
 * ) value to the pixel. 
 *
 */
void ucil_set_pixel_alpha( unicap_data_buffer_t *data_buffer, ucil_color_t *color, int alpha, int x, int y );
/** 
 * ucil_draw_line: 
 * @data_buffer: target buffer
 * @color: an #ucil_color_t. The colorspace of the color has to match
 * the colorspace of the data_buffer.
 * @x1: starting point of the line ( x position )
 * @y1: starting point of the line ( y position )
 * @x2: endpoint of the line ( x position )
 * @y2: endpoint of the line ( y position )
 * 
 * Draws a line on the data buffer. The endpoints are clipped to the
 * buffer dimensions
 * 
 */
void ucil_draw_line( unicap_data_buffer_t *data_buffer, ucil_color_t *color, int x1, int y1, int x2, int y2 );
/**
 * ucil_draw_rect:
 * @data_buffer: target buffer
 * @color: an #ucil_color_t. The colorspace of the color has to match
 * the colorspace of the data_buffer.
 * @x1: starting point of the rectangle ( x position )
 * @y1: starting point of the rectangle ( y position )
 * @x2: endpoint of the rectangle ( x position )
 * @y2: endpoint of the rectangle ( y position )
 *
 * Draws a rectangle filled with color.
 */
void ucil_draw_rect( unicap_data_buffer_t *data_buffer, ucil_color_t *color, int x1, int y1, int x2, int y2 );
/**
 * ucil_fill:
 * @data_buffer: target buffer
 * @color: an #ucil_color_t. The colorspace of the color has to match
 * the colorspace of the data_buffer.
 * 
 * Fill the buffer with a color.
 * 
 */
void ucil_fill( unicap_data_buffer_t *data_buffer, ucil_color_t *color );
/**
 * ucil_draw_box: 
 * @data_buffer: target buffer
 * @color: an #ucil_color_t. The colorspace of the color has to match
 * the colorspace of the data_buffer.
 * @x1: starting point of the box ( x position )
 * @y1: starting point of the box ( y position )
 * @x2: endpoint of the box ( x position )
 * @y2: endpoint of the box ( y position )
 * 
 *
 * Draws a box
 */
void ucil_draw_box( unicap_data_buffer_t *data_buffer, ucil_color_t *color, int x1, int y1, int x2, int y2 );
/**
 * ucil_draw_circle:
 * @data_buffer: target buffer
 * @color: an #ucil_color_t. The colorspace of the color has to match
 * the colorspace of the data_buffer.
 * @cx: center of the circle ( x position )
 * @cy: center of the circle ( y position )
 * @r: radius
 * 
 * Draws a circle.
 *
 */
void ucil_draw_circle( unicap_data_buffer_t *dest, ucil_color_t *color, int cx, int cy, int r );
/**
 * ucil_create_font_object:
 * @size: size of font in points
 * @font: name of font or NULL to use default font
 * 
 * Creates a font object required for text operations. The application
 * must free the font object with #ucil_destroy_font_object. 
 * 
 * Returns: a new #ucil_font_object_t
 */
ucil_font_object_t *ucil_create_font_object( int size, const char *font );
/**
 * ucil_draw_text: 
 * @dest: target buffer
 * @color: an #ucil_color_t. The colorspace of the color has to match
 * the colorspace of the data_buffer.
 * @fobj: an #ucil_font_object_t
 * @text: text string to draw
 * @x: x position
 * @y: y position
 * 
 * Draws a text string onto the target buffer.
 *
 */
void ucil_draw_text( unicap_data_buffer_t *dest, ucil_color_t *color, ucil_font_object_t *fobj, const char *text, int x, int y );
/**
 * ucil_destroy_font_object:
 * @fobj:
 * 
 * Frees all resources allocated by the font object
 *
 */
void ucil_destroy_font_object( ucil_font_object_t *fobj );
/**
 * ucil_text_get_size:
 * @fobj: an #ucil_font_object_t
 * @text: text string
 * @width: pointer to int which will receive the width of the text
 * @height: pointer to int which will receive the height of the text
 * 
 * Determines the size in pixels a text string will take up when drawn
 * onto a buffer. 
 */
void ucil_text_get_size( ucil_font_object_t *fobj, const char *text, int *width, int *height );

/**
 * ucil_get_pixel: 
 * @data_buffer: buffer
 * @color: pointer to an #ucil_color_t to store the result
 * @x: x position 
 * @y: y position
 * 
 * Reads the pixel at position (x,y) and stores the result in <structfield>color</structfield>.
 *
 */
void ucil_get_pixel( unicap_data_buffer_t *data_buffer, ucil_color_t *color, int x, int y );



/*

  Colorspace transformation

 */

/* void ucil_get_xfminfo_from_fourcc( unsigned int src_fourcc,  */
/* 				   unsigned int dest_fourcc,  */
/* 				   xfm_info_t *info ); */

/**
 * ucil_convert_color:
 * @dest: target color
 * @src: source color
 * 
 * Convert colors between colorspaces. The
 * <structfield>colorspace</structfield> field of
 * <structname>dest</structname> needs to be set to the target
 * colorspace, like in this example:
 * 
 * <informalexample>
 *  <programlisting>
src.colorspace = UCIL_COLORSPACE_RGB24;
src.rgb24.r = 0xff;
src.rgb24.g = 0xff;
src.rgb24.b = 0xff;
dest.colorspace = UCIL_COLORSPACE_YUV;
ucil_convert_color( &dest, &src );
 *  </programlisting>
 * </informalexample>
 *
 */
void ucil_convert_color( ucil_color_t *dest, ucil_color_t *src );

/**
 * ucil_convert_buffer:
 * @dest: target buffer
 * @src: source buffer
 * 
 * Convert the colorspace of a data buffer. The colorspaces are
 * denoted by the <structfield>buffer.format.fourcc</structfield>
 * field. The dest->format.fourcc gets set to the correct value. 
 * 
 * Returns: STATUS_SUCCESS if the buffer could be converted
 * successfully. STATUS_FAILURE if no conversion exists.
 */
unicap_status_t ucil_convert_buffer( unicap_data_buffer_t *dest, unicap_data_buffer_t *src );
/*
  Returns 1 if ucil_convert_buffer can convert from src_fourcc to dest_fourcc
 */
/**
 * ucil_conversion_supported:
 * @dest_fourcc: target fourcc
 * @src_fourcc: source fourcc
 * 
 * Tests whether a specific conversion is supported.
 * 
 * Returns: 1 when the conversion is supported
 */
int ucil_conversion_supported( unsigned int dest_fourcc, unsigned int src_fourcc );

/**
 * ucil_get_colorspace_from_fourcc:
 * @fourcc: fourcc
 * 
 * Gets the #ucil_colorspace_t that matches the fourcc. 
 *
 * Returns: an #ucil_colorspace_t. This is UCIL_COLORSPACE_UNKNOWN
 * when no conversion exists.
 */
ucil_colorspace_t ucil_get_colorspace_from_fourcc( unsigned int fourcc );



/*

  Buffer operations

 */
/**
 * ucil_blend_alpha:
 * @dest: target buffer
 * @bg: background buffer
 * @fg: foreground buffer
 * @alpha: transparency value
 * 
 * Blends two buffers into a target buffer. Colorspaces of buffers
 * should match.
 *
 */
void ucil_blend_alpha( unicap_data_buffer_t *dest, unicap_data_buffer_t *bg, unicap_data_buffer_t *fg, int alpha );
/**
 * ucil_convolution_mask:
 * @dest: target buffer
 * @src: source buffer
 * @mask: an #ucil_convolution_mask
 *
 * Apply a convolution mask
 *
 */
void ucil_convolution_mask( unicap_data_buffer_t *dest, unicap_data_buffer_t *src, ucil_convolution_mask_t *mask );

ucil_convolution_mask_t *ucil_create_convolution_mask( unsigned char *array, int size, ucil_colorspace_t cs, int mode );


void ucil_composite( unicap_data_buffer_t *dest, 
		     unicap_data_buffer_t *img, 
		     int xpos, 
		     int ypos, 
		     double scalex, 
		     double scaley, 
		     ucil_interpolation_type_t interp );


/*

  Video File

*/
/**
 * ucil_create_video_file:
 * @path: filename with full path
 * @format: image format of individual frames
 * @codec: codec name or NULL to use default codec
 * @...: codec parameters in the form ["parameter",value...] , terminated with NULL
 *
 * Creates a video file to be used for video recording. After
 * creation, add frames to the video file with #ucil_encode_frame. 
 * 
 * Returns: A new #ucil_video_file_object_t or NULL on error.
 */
ucil_video_file_object_t *ucil_create_video_file( const char *path, unicap_format_t *format, const char *codec, ...);

/**
 * ucil_encode_frame:
 * @vobj: a video file object
 * @buffer: data buffer
 * 
 * Adds a frame to a video file.
 *
 * Returns: STATUS_SUCCESS if the frame got added successfully.
 */
unicap_status_t ucil_encode_frame( ucil_video_file_object_t *vobj, unicap_data_buffer_t *buffer );

/**
 * ucil_close_video_file:
 * @vobj: an #ucil_video_file_object
 * 
 * Close a video file created with #ucil_create_video_file
 *
 * Returns:
 */
unicap_status_t ucil_close_video_file( ucil_video_file_object_t *vobj );

/** 
 * ucil_open_video_file:
 * @unicap_handle: pointer to an #unicap_handle_t where the new handle
 * gets stored. 
 * @filename: full path of the video file
 *
 * Opens a video file for playback. The resulting handle can be used
 * like a video capture device.
 * 
 * Returns:
 */
unicap_status_t ucil_open_video_file( unicap_handle_t *unicap_handle, char *filename );

/**
 * ucil_get_video_file_extension: 
 * @codec: codec name
 * 
 * Get the file extension of a codec. Eg. for the "ogg/theora" codec,
 * this will return "ogg". The returned string is owned by ucil and
 * should not be freed.
 * 
 * Returns: string
 */
const char *ucil_get_video_file_extension( const char *codec );

/**
 * ucil_theora_combine_av_file: 
 * @path: base path name of video and audio files
 * @remove: remove video and audio files after combining
 * @procfunc: callback to be called with current position
 * @error: 
 *
 * Returns: TRUE on success, FALSE on error
 */
gboolean ucil_theora_combine_av_file( gchar *path, gboolean remove, ucil_processing_info_func_t procfunc, void *func_data, GError **error );

UCIL_END_DECLS

#endif 	    /* !UCIL_H_ */
