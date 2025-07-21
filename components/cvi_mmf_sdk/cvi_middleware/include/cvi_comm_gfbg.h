#ifndef _CVI_COMM_GFBG_H_
#define _CVI_COMM_GFBG_H_

#include <linux/fb.h>
#include "cvi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define IOC_TYPE_GFBG       'G'

#define DRV_GFBG_IOCTL_CMD_NUM_MIN 10
#define DRV_GFBG_IOCTL_CMD_NUM_MAX 100

/* To get the origin of an overlay layer on the screen */
#define FBIOGET_SCREEN_ORIGIN_GFBG	_IOR(IOC_TYPE_GFBG, 10, cvi_fb_point)
/* To set the origin of an overlay layer on the screen */
#define FBIOPUT_SCREEN_ORIGIN_GFBG	_IOW(IOC_TYPE_GFBG, 11, cvi_fb_point)
/* To obtain the display state of an overlay layer */
#define FBIOGET_SHOW_GFBG		_IOR(IOC_TYPE_GFBG, 12, CVI_BOOL)
/* To set the display state of an overlay layer */
#define FBIOPUT_SHOW_GFBG		_IOW(IOC_TYPE_GFBG, 13, CVI_BOOL)
/* get the screen output size */
#define FBIOGET_SCREEN_SIZE		_IOR(IOC_TYPE_GFBG, 14, cvi_fb_size)
/* set the screen output size */
#define FBIOPUT_SCREEN_SIZE		_IOW(IOC_TYPE_GFBG, 15, cvi_fb_size)
/* To wait for the vertical blanking region of an overlay layer */
#define FBIOGET_VER_BLANK_GFBG		_IO(IOC_TYPE_GFBG, 16)
/* To obtain the colorkey of an overlay layer */
#define FBIOGET_COLORKEY_GFBG		_IOR(IOC_TYPE_GFBG, 17, cvi_fb_colorkey)
/* To set the colorkey of an overlay layer */
#define FBIOPUT_COLORKEY_GFBG		_IOW(IOC_TYPE_GFBG, 18, cvi_fb_colorkey)
/* To get the layer information */
#define FBIOGET_LAYER_INFO		_IOR(IOC_TYPE_GFBG, 19, cvi_fb_layer_info)
/* To set the layer information */
#define FBIOPUT_LAYER_INFO		_IOW(IOC_TYPE_GFBG, 20, cvi_fb_layer_info)
/* To refresh the displayed contents in extended mode */
#define FBIO_REFRESH			_IOW(IOC_TYPE_GFBG, 21, cvi_fb_buf)
/* To get canvas buf */
#define FBIOGET_CANVAS_BUF		_IOR(IOC_TYPE_GFBG, 22, cvi_fb_buf)
/* To display multiple surfaces in turn and set the colorkey attributes */
#define FBIOFLIP_SURFACE		_IOW(IOC_TYPE_GFBG, 23, cvi_fb_surfaceex)
/* To set the compression function status of an overlay layer */
#define FBIOPUT_COMPRESSION_GFBG	_IOW(IOC_TYPE_GFBG, 24, CVI_BOOL)

typedef struct {
	CVI_U32 width;
	CVI_U32 height;
} cvi_fb_size;

typedef struct {
	CVI_BOOL enable;	/* colorkey enable flag */
	CVI_U32 value;		/* colorkey value */
} cvi_fb_colorkey;

typedef struct {
	CVI_S32 x;
	CVI_S32 y;
	CVI_S32 width;
	CVI_S32 height;
} cvi_fb_rect;

typedef struct {
	CVI_S32 x_pos;         /* <  horizontal position */
	CVI_S32 y_pos;         /* <  vertical position */
} cvi_fb_point;

typedef enum {
	CVI_FB_FORMAT_ARGB8888,	/* ARGB8888 */
	CVI_FB_FORMAT_ARGB4444,	/* ARGB4444 */
	CVI_FB_FORMAT_ARGB1555,	/* ARGB1555 */
	CVI_FB_FORMAT_LUT_256,	/* 256 LUT */
	CVI_FB_FORMAT_LUT_16,	/* 16 LUT */
	CVI_FB_FORMAT_BUTT
} cvi_fb_color_format;

/* refresh mode */
typedef enum {
	CVI_FB_LAYER_BUF_DOUBLE = 0x0,	/* 2 display buf in fb */
	CVI_FB_LAYER_BUF_ONE    = 0x1,	/* 1 display buf in fb */
	CVI_FB_LAYER_BUF_NONE	= 0x2,	/* no display buf in fb,the buf user refreshed will be directly set to VO */
	CVI_FB_LAYER_BUF_DOUBLE_IMMEDIATE = 0x3, /* 2 display buf in fb, each refresh will be displayed */
	CVI_FB_LAYER_BUF_BUTT
} cvi_fb_layer_buf;

/* surface info */
typedef struct {
	CVI_U64 phys_addr;		/* start physical address */
	CVI_U32 width;			/* width pixels */
	CVI_U32 height;			/* height pixels */
	CVI_U32 pitch;			/* line pixels */
	cvi_fb_color_format format;	/* color format */
} cvi_fb_surface;

typedef struct {
	CVI_U64 phys_addr;
	cvi_fb_colorkey colorkey;
} cvi_fb_surfaceex;

/* refresh surface info */
typedef struct {
	cvi_fb_surface canvas;
	cvi_fb_rect update_rect;	/* refresh region */
} cvi_fb_buf;

/* layer info maskbit */
typedef enum {
	CVI_FB_LAYER_MASK_BUF_MODE = 0x1,		/* buf mode bitmask */
	CVI_FB_LAYER_MASK_POS = 0x4,			/* the position bitmask */
	CVI_FB_LAYER_MASK_CANVAS_SIZE = 0x8,		/* canvassize bitmask */
	CVI_FB_LAYER_MASK_DISPLAY_SIZE = 0x10,		/* displaysize bitmask */
	CVI_FB_LAYER_MASK_SCREEN_SIZE = 0x20,		/* screensize bitmask */
	CVI_FB_LAYER_MASK_BUTT
} cvi_fb_layer_info_maskbit;

/* layer info */
typedef struct {
	cvi_fb_layer_buf buf_mode;
	CVI_S32 x_pos;		/*  the x pos of origin point in screen */
	CVI_S32 y_pos;		/*  the y pos of origin point in screen */
	CVI_U32 canvas_width;	/*  the width of canvas buffer */
	CVI_U32 canvas_height;	/*  the height of canvas buffer */
	/* the width of display buf in fb.for 0 buf, there is no display buf in fb, so it's effectless */
	CVI_U32 display_width;
	CVI_U32 display_height;	/*  the height of display buf in fb. */
	CVI_U32 screen_width;	/*  the width of screen */
	CVI_U32 screen_height;	/*  the height of screen */
	CVI_U32 mask;		/*  param modify mask bit */
} cvi_fb_layer_info;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* _CVI_COMM_GFBG_H_ */
