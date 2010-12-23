
#ifndef _WRAP_GX_
#define _WRAP_GX_

#include "CommonTypes.h"

#define GX_TRIANGLEFAN   0xA0

#define 	GX_MAX_TEVREG   4
#define 	GX_TEVPREV   0
#define 	GX_TEVREG0   1
#define 	GX_TEVREG1   2
#define 	GX_TEVREG2   3

typedef float f32;

typedef f32 	Mtx [3][4];
typedef f32 	Mtx44 [4][4];

typedef struct _vecf
{
	f32 x, y, z;

} guVector;

// watev
struct GXTexObj
{
	u32 	val [8];
};

struct GXColor
{
	u8 r, g, b, a;
};

struct GXColorS10
{
	s16 r, g, b, a;
};

typedef void GXFifoObj;

GXFifoObj * 	GX_Init (void *base, u32 size);

u32 	GX_GetTexBufferSize (u16 wd, u16 ht, u32 fmt, u8 mipmap, u8 maxlod);

void 	GX_InitTexObj (GXTexObj *obj, void *img_ptr, u16 wd, u16 ht, u8 fmt, u8 wrap_s, u8 wrap_t, u8 mipmap);
void 	GX_InitTexObjWrapMode (GXTexObj *obj, u8 wrap_s, u8 wrap_t);
void 	GX_InitTexObjFilterMode (GXTexObj *obj, u8 minfilt, u8 magfilt);

void 	GX_LoadTexObj (GXTexObj *obj, u8 mapid);

void 	GX_Begin (u8 primitve, u8 vtxfmt, u16 vtxcnt);
void 	GX_End ();

#define GX_VTXFMT0 0

void	GX_Position3f32 (f32 x, f32 y, f32 z);
void	GX_Color4u8 (u8 r, u8 g, u8 b, u8 a);
void	GX_Coloru32 (u32 c);
void	GX_TexCoord2f32 (f32 s, f32 t);

void 	GX_SetBlendMode (u8 type, u8 src_fact, u8 dst_fact, u8 op);
void 	GX_SetAlphaCompare (u8 comp0, u8 ref0, u8 aop, u8 comp1, u8 ref1);

void 	GX_SetTevOrder (u8 tevstage, u8 texcoord, u32 texmap, u8 color);
void 	GX_SetTevSwapMode (u8 tevstage, u8 ras_sel, u8 tex_sel);

void 	GX_SetTevIndirect (u8 tevstage, u8 indtexid, u8 format, u8 bias, u8 mtxid,
	u8 wrap_s, u8 wrap_t, u8 addprev, u8 utclod, u8 a);

void 	GX_SetTevKAlphaSel (u8 tevstage, u8 sel);
void 	GX_SetTevKColorSel (u8 tevstage, u8 sel);

void 	GX_SetTevAlphaIn (u8 tevstage, u8 a, u8 b, u8 c, u8 d);
void 	GX_SetTevAlphaOp (u8 tevstage, u8 tevop, u8 tevbias, u8 tevscale, u8 clamp, u8 tevregid);

void 	GX_SetTevColorIn (u8 tevstage, u8 a, u8 b, u8 c, u8 d);
void 	GX_SetTevColorOp (u8 tevstage, u8 tevop, u8 tevbias, u8 tevscale, u8 clamp, u8 tevregid);

void 	GX_SetTevColorS10 (u8 tev_regid, GXColorS10 color);

void 	GX_SetNumTevStages (u8 num);

void 	guMtxIdentity (Mtx mt);
void 	guMtxRotAxisRad (Mtx mt, guVector *axis, f32 rad);
void 	guMtxScale (Mtx mt, f32 xS, f32 yS, f32 zS);
void 	guMtxTrans (Mtx mt, f32 xT, f32 yT, f32 zT);

#define 	GX_TEXCOORD0   0x0

//void 	GX_SetTexCoordGen (u16 texcoord, u32 tgen_typ, u32 tgen_src, u32 mtxsrc);

void 	guLightOrtho (Mtx mt, f32 t, f32 b, f32 l, f32 r, f32 scaleS, f32 scaleT, f32 transS, f32 transT);
void 	guOrtho (Mtx44 mt, f32 t, f32 b, f32 l, f32 r, f32 n, f32 f);

#endif
