#ifndef __OSDC_H__
#define __OSDC_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum _OSDC_OSD_FORMAT_E {
	OSD_ARGB8888 = 0,
	OSD_ARGB4444 = 4,
	OSD_ARGB1555 = 5,
	OSD_LUT8 = 8,
	OSD_LUT4 = 10,
	NUM_OF_FORMAT
} OSDC_OSD_FORMAT_E;

typedef enum _OSDC_DRAW_OBJ_TYPE_E {
	RECT = 0,
	STROKE_RECT,
	BIT_MAP,
	LINE,
	CMPR_BIT_MAP,
	NUM_OF_DRAW_OBJ
} OSDC_DRAW_OBJ_TYPE_E;

typedef struct _OSDC_Canvas_Attr_S {
	int width;
	int height;
	OSDC_OSD_FORMAT_E format;
	uint32_t bg_color_code;
} OSDC_Canvas_Attr_S;

typedef struct _OSDC_RECT_ATTR_S {
	int x;
	int y;
	int width;
	int height;
	int thickness;
} OSDC_RECT_ATTR_S;

typedef struct _OSDC_POINT_ATTR_S {
	int x;
	int y;
} OSDC_POINT_ATTR_S;

typedef struct _OSDC_LINE_ATTR_S {
	float _mx; // slope of two end-point vector
	float _bx[2];
	float _by[2];
	float _ex[2];
	float _ey[2];
	float ts_h; // thickness proj. on horizontal slice
} OSDC_LINE_ATTR_S;

typedef struct _OSDC_BITMAP_ATTR_S {
	OSDC_RECT_ATTR_S rect;
	union {
		uint32_t stride;
		uint32_t bs_offset;
	};
} OSDC_BITMAP_ATTR_S;

typedef union _OSDC_COLOR_S {
	uint32_t code;
	uint8_t *buf;
} OSDC_COLOR_S;

typedef struct _OSDC_DRAW_OBJ_S {
	OSDC_DRAW_OBJ_TYPE_E type;
	union {
		OSDC_RECT_ATTR_S rect;
		OSDC_LINE_ATTR_S line;
		OSDC_BITMAP_ATTR_S bitmap;
	};
	OSDC_COLOR_S color;
	int _max_y;
	int _min_y;
} OSDC_DRAW_OBJ_S;

uint32_t OSDC_EstCmprCanvasSize(OSDC_Canvas_Attr_S * canvas, OSDC_DRAW_OBJ_S * objs, uint32_t obj_num);

int OSDC_DrawCmprCanvas(OSDC_Canvas_Attr_S *canvas, OSDC_DRAW_OBJ_S *objs, uint32_t obj_num,
				uint8_t *obuf, uint32_t buf_size, uint32_t *p_osize);

void OSDC_SetRectObjAttr(OSDC_Canvas_Attr_S *canvas, OSDC_DRAW_OBJ_S *obj, uint32_t color_code,
				int pt_x, int pt_y, int width, int height, bool is_filled, int thickness);

void OSDC_SetRectObjAttrEx(OSDC_Canvas_Attr_S *canvas, OSDC_DRAW_OBJ_S *obj, uint32_t color_code,
				OSDC_RECT_ATTR_S *rects, int num, bool is_filled);

void OSDC_SetBitmapObjAttr(OSDC_Canvas_Attr_S *canvas, OSDC_DRAW_OBJ_S *obj_attr, uint8_t *buf,
				  int pt_x, int pt_y, int width, int height, bool is_cmpr);

void OSDC_SetLineObjAttr(OSDC_Canvas_Attr_S *canvas, OSDC_DRAW_OBJ_S *obj, uint32_t color_code,
				int pt_x0, int pt_y0, int pt_x1, int pt_y1, int thickness);

void OSDC_SetLineObjAttrEx(OSDC_Canvas_Attr_S *canvas, OSDC_DRAW_OBJ_S *obj, uint32_t color_code,
				OSDC_POINT_ATTR_S *points, int num, int thickness);

int OSDC_CmprBitmap(OSDC_Canvas_Attr_S *canvas, uint8_t *ibuf, uint8_t *obuf, int width, int height,
				int buf_size, uint32_t *p_osize);

extern uint32_t OSDC_est_cmpr_canvas_size(OSDC_Canvas_Attr_S *canvas, OSDC_DRAW_OBJ_S *objs, uint32_t obj_num);

extern int OSDC_draw_cmpr_canvas(OSDC_Canvas_Attr_S *canvas, OSDC_DRAW_OBJ_S *objs, uint32_t obj_num,
			      uint8_t *obuf, uint32_t buf_size, uint32_t *p_osize);

extern void OSDC_set_rect_obj_attr(OSDC_Canvas_Attr_S *canvas, OSDC_DRAW_OBJ_S *obj, uint32_t color_code,
				int pt_x, int pt_y, int width, int height, bool is_filled, int thickness);

extern void OSDC_set_bitmap_obj_attr(OSDC_Canvas_Attr_S *canvas, OSDC_DRAW_OBJ_S *obj_attr, uint8_t *buf,
				  int pt_x, int pt_y, int width, int height, bool is_cmpr);

extern void OSDC_set_line_obj_attr(OSDC_Canvas_Attr_S *canvas, OSDC_DRAW_OBJ_S *obj, uint32_t color_code,
				int pt_x0, int pt_y0, int pt_x1, int pt_y1, int thickness);

extern int OSDC_cmpr_bitmap(OSDC_Canvas_Attr_S *canvas, uint8_t *ibuf, uint8_t *obuf, int width, int height,
			 int buf_size, uint32_t *p_osize);
#if (CMPR_CANVAS_DBG)
int OSDC_draw_canvas_raw_buffer(OSDC_Canvas_Attr_S &canvas, vector<DRAW_OBJ> obj_vec, uint8_t *obuf);
int OSDC_draw_canvas_raw_buffer2(OSDC_Canvas_Attr_S &canvas, vector<DRAW_OBJ> obj_vec, uint8_t *obuf);
#endif


#ifdef __cplusplus
}
#endif

#endif /*__OSDC_H__ */
