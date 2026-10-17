#include <nitro/fx/fx_mtx22.h>
#include <nitro/fx/fx_const.h>
#include <nitro/fx/fx_cp.h>

inline fx32 mul64 (fx64 x, fx32 y)
{
	return (fx32)((x * y) >> FX32_SHIFT);
}

#include <nitro/code32.h>

#include <nitro/code16.h>

#include <nitro/codereset.h>

void MTX_ScaleApply22 (const MtxFx22 *pSrc, MtxFx22 *pDst, fx32 x, fx32 y)
{
	fx64 v;

	SDK_NULL_ASSERT(pSrc);
	SDK_NULL_ASSERT(pDst);

	v = (fx64)x;
	pDst->_00 = mul64(v, pSrc->_00);
	pDst->_01 = mul64(v, pSrc->_01);

	v = (fx64)y;
	pDst->_10 = mul64(v, pSrc->_10);
	pDst->_11 = mul64(v, pSrc->_11);
}

int MTX_Inverse22 (const MtxFx22 *pSrc, MtxFx22 *pDst)
{
	MtxFx22 tmp;
	MtxFx22 *p;
	fx32 det;

	SDK_NULL_ASSERT(pSrc);
	SDK_NULL_ASSERT(pDst);

	if (pSrc == pDst) {
		p = &tmp;
	} else {
		p = pDst;
	}

	det = (fx32)(((fx64)pSrc->_00 * pSrc->_11 -
	              (fx64)pSrc->_01 * pSrc->_10 + (fx64)(FX32_ONE >> 1)) >> FX32_SHIFT);

	if (0 == det) {
		return -1;
	}

	FX_InvAsync(det);

	det = FX_GetDivResult();

	p->_00 = (fx32)(((fx64)det * pSrc->_11) >> FX32_SHIFT);
	p->_01 = -(fx32)(((fx64)det * pSrc->_01) >> FX32_SHIFT);

	p->_10 = -(fx32)(((fx64)det * pSrc->_10) >> FX32_SHIFT);
	p->_11 = (fx32)(((fx64)det * pSrc->_00) >> FX32_SHIFT);

	if (p == &tmp) {
		MI_Copy16B(&tmp, pDst);
	}

	return 0;
}

void MTX_Concat22 (const MtxFx22 *a, const MtxFx22 *b, MtxFx22 *ab)
{
	MtxFx22 tmp;
	MtxFx22 *p;

	register fx32 x, y;

	SDK_NULL_ASSERT(a);
	SDK_NULL_ASSERT(b);
	SDK_NULL_ASSERT(ab);

	if (ab == b) {
		p = &tmp;
	} else {
		p = ab;
	}

	x = a->_00;
	y = a->_01;

	p->_00 = (fx32)(((fx64)x * b->_00 + (fx64)y * b->_10) >> FX32_SHIFT);
	p->_01 = (fx32)(((fx64)x * b->_01 + (fx64)y * b->_11) >> FX32_SHIFT);

	x = a->_10;
	y = a->_11;

	p->_10 = (fx32)(((fx64)x * b->_00 + (fx64)y * b->_10) >> FX32_SHIFT);
	p->_11 = (fx32)(((fx64)x * b->_01 + (fx64)y * b->_11) >> FX32_SHIFT);

	if (p == &tmp) {
		*ab = tmp;
	}
}
