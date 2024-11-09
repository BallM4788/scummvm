/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <3ds.h>
#include <citro3d.h>
#include "common/system.h"
#include "graphics/3ds/n3d.h"
#include "graphics/3ds/n3d-context.h"

namespace N3DS_3D {

Native3D::Native3D() {
	// new context
	_defaultContext.vport_x                = _savedContext.vport_x                = 0;
	_defaultContext.vport_y                = _savedContext.vport_y                = 0;
	_defaultContext.vport_w                = _savedContext.vport_w                = 0;
	_defaultContext.vport_h                = _savedContext.vport_h                = 0;
	_defaultContext.cullFace_mode          = _savedContext.cullFace_mode          = GPU_CULL_BACK_CCW;
	_defaultContext.cullFace_faceToCull    = _savedContext.cullFace_faceToCull    = N3D_CULLFACE_BACK;
	_defaultContext.cullFace_frontFace     = _savedContext.cullFace_frontFace     = N3D_FRONTFACE_CCW;
	_defaultContext.cullFace_enabled       = _savedContext.cullFace_enabled       = false;
	_defaultContext.depthMap_pOffUnits     = _savedContext.depthMap_pOffUnits     = 0;
	_defaultContext.depthMap_wScale        = _savedContext.depthMap_wScale        = 0.0f;
	_defaultContext.depthMap_rangeN        = _savedContext.depthMap_rangeN        = 0.0f;
	_defaultContext.depthMap_rangeF        = _savedContext.depthMap_rangeF        = 1.0f;
	_defaultContext.depthMap_zScale        = _savedContext.depthMap_zScale        = -1.0f;
	_defaultContext.depthMap_zOffset       = _savedContext.depthMap_zOffset       = 0.0f;
	_defaultContext.depthMap_enabled       = _savedContext.depthMap_enabled       = true;
	_defaultContext.scissor_mode           = _savedContext.scissor_mode           = GPU_SCISSOR_DISABLE;
	_defaultContext.scissor_x              = _savedContext.scissor_x              = 0;
	_defaultContext.scissor_y              = _savedContext.scissor_y              = 0;
	_defaultContext.scissor_w              = _savedContext.scissor_w              = 0;
	_defaultContext.scissor_h              = _savedContext.scissor_h              = 0;
	_defaultContext.alphaTest_func         = _savedContext.alphaTest_func         = GPU_ALWAYS;
	_defaultContext.alphaTest_ref          = _savedContext.alphaTest_ref          = 0;
	_defaultContext.alphaTest_enabled      = _savedContext.alphaTest_enabled      = false;
	_defaultContext.stencilTest_func       = _savedContext.stencilTest_func       = GPU_ALWAYS;
	_defaultContext.stencilTest_ref        = _savedContext.stencilTest_ref        = 0x00;
	_defaultContext.stencilTest_mask       = _savedContext.stencilTest_mask       = 0xFF;
	_defaultContext.stencilTest_writeMask  = _savedContext.stencilTest_writeMask  = 0x00;
	_defaultContext.stencilOp_fail         = _savedContext.stencilOp_fail         = GPU_STENCIL_KEEP;
	_defaultContext.stencilOp_zfail        = _savedContext.stencilOp_zfail        = GPU_STENCIL_KEEP;
	_defaultContext.stencilOp_zpass        = _savedContext.stencilOp_zpass        = GPU_STENCIL_KEEP;
	_defaultContext.stencilTest_enabled    = _savedContext.stencilTest_enabled    = false;
	_defaultContext.depthTest_func         = _savedContext.depthTest_func         = GPU_GREATER;
	_defaultContext.depthTest_writeMask    = _savedContext.depthTest_writeMask    = GPU_WRITE_ALL;
	_defaultContext.depthTest_enabled      = _savedContext.depthTest_enabled      = true;
	_defaultContext.earlyDepthTest_func    = _savedContext.earlyDepthTest_func    = GPU_EARLYDEPTH_GREATER;
	_defaultContext.earlyDepthTest_clear   = _savedContext.earlyDepthTest_clear   = 0;
	_defaultContext.earlyDepthTest_enabled = _savedContext.earlyDepthTest_enabled = false;
	_defaultContext.blend_color[0]         = _savedContext.blend_color[0]         = 0;
	_defaultContext.blend_color[1]         = _savedContext.blend_color[1]         = 0;
	_defaultContext.blend_color[2]         = _savedContext.blend_color[2]         = 0;
	_defaultContext.blend_color[3]         = _savedContext.blend_color[3]         = 0;
	_defaultContext.blend_colorEq          = _savedContext.blend_colorEq          = GPU_BLEND_ADD;
	_defaultContext.blend_alphaEq          = _savedContext.blend_alphaEq          = GPU_BLEND_ADD;
	_defaultContext.blend_srcColor         = _savedContext.blend_srcColor         = GPU_SRC_ALPHA;
	_defaultContext.blend_dstColor         = _savedContext.blend_dstColor         = GPU_ONE_MINUS_SRC_ALPHA;
	_defaultContext.blend_srcAlpha         = _savedContext.blend_srcAlpha         = GPU_SRC_ALPHA;
	_defaultContext.blend_dstAlpha         = _savedContext.blend_dstAlpha         = GPU_ONE_MINUS_SRC_ALPHA;
	_defaultContext.blend_enabled          = _savedContext.blend_enabled          = true;
	_defaultContext.colorLogicOp_op        = _savedContext.colorLogicOp_op        = GPU_LOGICOP_COPY;		// <------------???????????????????????????????
	_defaultContext.colorLogicOp_enabled   = _savedContext.colorLogicOp_enabled   = false;
	_defaultContext.fragOpMode             = _savedContext.fragOpMode             = GPU_FRAGOPMODE_GL;
}

Native3D::~Native3D() {
	_defaultContext.boundTexUnits[0] = _savedContext.boundTexUnits[0] = nullptr;
	_defaultContext.boundTexUnits[1] = _savedContext.boundTexUnits[1] = nullptr;
	_defaultContext.boundTexUnits[2] = _savedContext.boundTexUnits[2] = nullptr;
	_defaultContext.boundTexUnits[3] = _savedContext.boundTexUnits[3] = nullptr;
	_defaultContext.activeShaderObj = _savedContext.activeShaderObj = nullptr;
}


void Native3D::updateCullMode() {
	GPU_CULLMODE mode;
	if (_activeContext->cullFace_enabled == true)
		mode = ((_activeContext->cullFace_faceToCull == N3D_CULLFACE_FRONT) ^ (_activeContext->cullFace_frontFace == N3D_FRONTFACE_CCW))
			? GPU_CULL_BACK_CCW : GPU_CULL_FRONT_CCW;
	else
		mode = GPU_CULL_NONE;

	if (_activeContext->cullFace_mode != mode) {
		_activeContext->cullFace_mode = mode;

		C3D_CullFace(mode);
	}
}

void Native3D::updateDepthMap() {
	float scale, offset;
	if (_activeContext->depthMap_wScale != 0.f) {
		scale = -_activeContext->depthMap_wScale;
		offset = 0.f;
	} else {
		scale = _activeContext->depthMap_rangeN - _activeContext->depthMap_rangeF;
		offset = _activeContext->depthMap_rangeN;
	}
	if ((_activeContext->depthMap_enabled == true) && (_activeContext->depthMap_pOffUnits != 0.f))
		offset += _activeContext->depthMap_pOffUnits / 16777215.f;

	_activeContext->depthMap_zScale = scale;
	_activeContext->depthMap_zOffset = offset;

	C3D_DepthMap(_activeContext->depthMap_enabled,
	             _activeContext->depthMap_zScale,
	             _activeContext->depthMap_zOffset);
}

void Native3D::updateBlend() {
	if (_activeContext->blend_enabled && !_activeContext->colorLogicOp_enabled) {
		C3D_AlphaBlend(_activeContext->blend_colorEq,  _activeContext->blend_alphaEq,
		               _activeContext->blend_srcColor, _activeContext->blend_dstColor,
		               _activeContext->blend_srcAlpha, _activeContext->blend_dstAlpha);
		if (_activeContext->fragOpMode != GPU_FRAGOPMODE_GL)
			C3D_FragOpMode(_activeContext->fragOpMode);
	}
}

void Native3D::updateEntireContext() {
//	C3D_BindProgram(_activeContext->activeShaderObj->_program);
//	C3D_SetAttrInfo(&_activeContext->activeShaderObj->_attrInfo);
//	C3D_SetBufInfo(&_activeContext->activeShaderObj->_bufInfo);

	// TEXENV

	updateCullMode();
	updateDepthMap();
	C3D_SetViewport(_activeContext->vport_x,
	                _activeContext->vport_y,
	                _activeContext->vport_w,
	                _activeContext->vport_h);
	C3D_SetScissor(_activeContext->scissor_mode,
	               _activeContext->scissor_x,
	               _activeContext->scissor_y,
	               _activeContext->scissor_w,
	               _activeContext->scissor_h);
	C3D_AlphaTest(_activeContext->alphaTest_enabled,
	              _activeContext->alphaTest_func,
	              _activeContext->alphaTest_ref);
	C3D_StencilTest(_activeContext->stencilTest_enabled,
	                _activeContext->stencilTest_func,
	                _activeContext->stencilTest_ref,
	                _activeContext->stencilTest_mask,
	                _activeContext->stencilTest_writeMask);
	C3D_StencilOp(_activeContext->stencilOp_fail,
	              _activeContext->stencilOp_zfail,
	              _activeContext->stencilOp_zpass);
	C3D_DepthTest(_activeContext->depthTest_enabled,
	              _activeContext->depthTest_func,
	              _activeContext->depthTest_enabled ? _activeContext->depthTest_writeMask :
	                                  (GPU_WRITEMASK)(_activeContext->depthTest_writeMask & ~GPU_WRITE_DEPTH));
	C3D_EarlyDepthTest(_activeContext->earlyDepthTest_enabled,
	                   _activeContext->earlyDepthTest_func,
	                   _activeContext->earlyDepthTest_clear);
	if (_activeContext->colorLogicOp_enabled == true)
		C3D_ColorLogicOp(_activeContext->colorLogicOp_op);
	else {
		(_activeContext->blend_enabled)
			? C3D_AlphaBlend(_activeContext->blend_colorEq,  _activeContext->blend_alphaEq,
			                 _activeContext->blend_srcColor, _activeContext->blend_dstColor,
			                 _activeContext->blend_srcAlpha, _activeContext->blend_dstAlpha)
			: C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD,
			                 GPU_ONE,       GPU_ZERO,
			                 GPU_ONE,       GPU_ZERO);
		if (_activeContext->fragOpMode != GPU_FRAGOPMODE_GL)
			C3D_FragOpMode(_activeContext->fragOpMode);
	}
	C3D_BlendingColor((_activeContext->blend_color[3] << 24) |
	                  (_activeContext->blend_color[2] << 16) |
	                  (_activeContext->blend_color[1] << 8)  |
	                   _activeContext->blend_color[0]);

}

N3DContext *Native3D::getContext() {
	return _activeContext;
}

void Native3D::setContext(N3DContext *context) {
	_activeContext = context;
}

void Native3D::saveContext(N3DContext *contextDst) {
	N3DContext *destination = (contextDst == nullptr) ? &_savedContext : contextDst;

	*destination = *_activeContext; // copies values to destination
}

void Native3D::restoreContext(N3DContext *contextSrc) {
	N3DContext *source = (contextSrc == nullptr) ? &_savedContext : contextSrc;

	*_activeContext = *source; // copies source values to _activeContext
	updateEntireContext();
}

void Native3D::defaultContext() {
	_activeContext = &_defaultContext;
	updateEntireContext();
}

void Native3D::changeShader(ShaderObj *shaderObj) {
	if (_activeContext->activeShaderObj == shaderObj)
		return;

	_activeContext->activeShaderObj = shaderObj;
	C3D_BindProgram(_activeContext->activeShaderObj->_program);
	C3D_SetAttrInfo(&_activeContext->activeShaderObj->_attrInfo);
	C3D_SetBufInfo(&_activeContext->activeShaderObj->_bufInfo);
	_activeContext->activeShaderObj->sendDirtyUniforms();
}


C3D_Tex *Native3D::getGameScreen() {
	return static_cast<C3D_Tex *>(g_system->getGameSurface());
}




static u32 getPixelSizeInBytes(GPU_TEXCOLOR format, u32 *isHalfByte) {
	switch(format) {
		case GPU_RGBA8:
			return 4;
		case GPU_RGB8:
			return 3;
		case GPU_LA8:
		case GPU_HILO8:
		case GPU_RGB565:
		case GPU_RGBA5551:
		case GPU_RGBA4:
			return 2;
		case GPU_A8:
		case GPU_L8:
		case GPU_LA4:
			return 1;
		case GPU_L4:
		case GPU_A4:
			if (isHalfByte != nullptr) *isHalfByte = 1;
			return 1;
		default:
			return 0;
	}
}

static u32 mortonInterleave(u32 x, u32 y) {
	static u32 xlut[] = {0x00, 0x01, 0x04, 0x05, 0x10, 0x11, 0x14, 0x15};
	static u32 ylut[] = {0x00, 0x02, 0x08, 0x0a, 0x20, 0x22, 0x28, 0x2a};

	return xlut[x % 8] + ylut[y % 8];
}

static u32 mortonOffset(u32 x, u32 y) {
	u32 i = mortonInterleave(x, y);
	u32 xCoarse = x & ~7;
	u32 offset = xCoarse << 3;

	return (i + offset);
}

void Native3D::arbDataToArbBlockTexOffset(u32 *srcBuf, u32 *dstBuf, int copyWidth, int copyHeight,
                                          int xSource, int ySource, int wSource,   int hSource,
                                          int xDest,   int yDest,   int wDest,     int hDest,
                                          GPU_TEXCOLOR format, int scale, bool isBlockSrc) {
	u32 pixSize = 4, halfByte = 0;
	int yFlip   = !isBlockSrc;

	pixSize = getPixelSizeInBytes(format, &halfByte);

	u32 wSrc = wSource  << scale;
	u32 hSrc = hSource << scale;
	u32 wDst = wDest  << scale;
	u32 hDst = hDest << scale;
	u32 copyW = copyWidth << scale;
	u32 copyH = copyHeight << scale;
	u32 ySrc = hSrc - (ySource << scale) - copyH;
	u32 xSrc = xSource << scale;
	u32 yDst = hDst - (yDest << scale) - copyH;
	u32 xDst = xDest << scale;
	u32 x, y;
	u8 *srcConverted;

	if (!isBlockSrc) {
		int idx;
		int size = wSource * hSource * pixSize;
		if (halfByte) size /= 2;

		srcConverted = (u8 *)linearAlloc((size_t)size);

		// some color formats have reversed component positions under the 3DS's native texture format
		u16 *srcBuf16;
		switch (format) {
			case GPU_RGBA8:
				for (idx = 0; idx < wSource * hSource; idx++) {
					((u32 *)srcConverted)[idx] = (((srcBuf[idx] << 24) & 0xff000000) |
					                              ((srcBuf[idx] << 8)  & 0x00ff0000) |
					                              ((srcBuf[idx] >> 8)  & 0x0000ff00) |
					                              ((srcBuf[idx] >> 24) & 0x000000ff));
				}
				break;
			case GPU_RGB8:
				for (idx = 0; idx < wSource * hSource; idx++) {
					int pxl = idx * 3;
					srcConverted[pxl + 2] = ((u8 *)srcBuf)[pxl + 0];
					srcConverted[pxl + 1] = ((u8 *)srcBuf)[pxl + 1];
					srcConverted[pxl + 0] = ((u8 *)srcBuf)[pxl + 2];
				}
				break;
			case GPU_LA8:
			case GPU_HILO8:
				srcBuf16 = (u16 *)srcBuf;
				for (idx = 0; idx < wSource * hSource; idx++) {
					((u16 *)srcConverted)[idx] = (((srcBuf16[idx] << 8) & 0xff00) |
					                              ((srcBuf16[idx] >> 8) & 0x00ff));
				}
				break;
			case GPU_RGB565:
			case GPU_RGBA5551:
			case GPU_RGBA4:
				for (idx = 0; idx < wSource * hSource; idx++)
					((u16 *)srcConverted)[idx] = ((u16 *)srcBuf)[idx];
				break;
			case GPU_A8:
			case GPU_L8:
			case GPU_LA4:
				for (idx = 0; idx < wSource * hSource; idx++)
					srcConverted[idx] = ((u8 *)srcBuf)[idx];
				break;
			case GPU_L4:
			case GPU_A4:
				for (idx = 0; idx < wSource * hSource / 2; idx++)
					srcConverted[idx] = ((u8 *)srcBuf)[idx];
				break;
			case GPU_ETC1:
			case GPU_ETC1A4:
				linearFree(srcConverted);
				return;
		}
	} else
		srcConverted = (u8 *)srcBuf;

	for (y = 0; y < copyH; y++) {
		u32 yOffSrc = (ySrc + y) * hSource / hSrc;
		u32 yOffSrcCoarse = yOffSrc & ~7;

		u32 yOffDst = (yFlip) ? ((hSrc - y - 1) + yDst) : (y + yDst);
		u32 yOffDstCoarse = yOffDst & ~7;

		for (x = 0; x < copyW; x++) {
			u32 xOffDst = x + xDst;
			u32 offset = mortonOffset(xOffDst, yOffDst) + yOffDstCoarse * wDst;

			u32 xOffSrc = (xSrc + x) * wSource / wSrc;
			u32 locate = (isBlockSrc) ? (mortonOffset(xOffSrc, yOffSrc) + yOffSrcCoarse * wSrc)
			                          : (xOffSrc + yOffSrc * wSource);

			if (offset >= wDst * hDst) continue;
			if (locate >= (u32)(wSource * hSource)) continue;

			u8 *dstByte, *srcByte;

			if (halfByte) {
				srcByte = &srcConverted[(locate * pixSize) >> 1];
				dstByte = &((u8 *)dstBuf)[(offset * pixSize) >> 1];
				if (x & 1)
					*dstByte = (*srcByte & 0xf0) | (*dstByte & 0x0f);
				else
					*dstByte = (*dstByte & 0xf0) | (*srcByte & 0x0f);
			} else {
				srcByte = &srcConverted[locate * pixSize];
				dstByte = &((u8 *)dstBuf)[offset * pixSize];
				for (int i = 0; i < (int)pixSize; i++)
					*dstByte++ = *srcByte++;
			}
		}
	}

	if (!isBlockSrc)
		linearFree((void *)srcConverted);

	return;
}

void Native3D::dataToBlockTex(u32 *srcBuf, u32 *dstBuf, int xSource, int ySource,
                              int wSource, int hSource, int wDest, int hDest,
                              GPU_TEXCOLOR format, int scale, bool isBlockSrc) {
	arbDataToArbBlockTexOffset(srcBuf,  dstBuf,  wSource, hSource,
	                           xSource, ySource, wSource, hSource,
	                           0,       0,       wDest,   hDest,
	                           format, scale, isBlockSrc);
}






void Native3D::viewport(u32 x, u32 y, u32 w, u32 h) {
	if ((_activeContext->vport_x == x) && (_activeContext->vport_y == y) &&
	    (_activeContext->vport_w == w) && (_activeContext->vport_h == h))
		return;

	_activeContext->vport_x = x;
	_activeContext->vport_y = y;
	_activeContext->vport_w = w;
	_activeContext->vport_h = h;

	C3D_SetViewport(_activeContext->vport_x,
	                _activeContext->vport_y,
	                _activeContext->vport_w,
	                _activeContext->vport_h);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// culling

void Native3D::cullFace(N3D_CULLFACE mode) {
	assert(mode == N3D_CULLFACE_FRONT ||
	       mode == N3D_CULLFACE_BACK  ||
	       mode == N3D_CULLFACE_FRONT_AND_BACK);

	if (_activeContext->cullFace_faceToCull == mode)
		return;

	_activeContext->cullFace_faceToCull = mode;

	updateCullMode();
}

void Native3D::frontFace(N3D_FRONTFACE mode) {
	assert(mode == N3D_FRONTFACE_CW ||
	       mode == N3D_FRONTFACE_CCW);

	if (_activeContext->cullFace_frontFace == mode)
		return;

	_activeContext->cullFace_frontFace = mode;

	updateCullMode();
}

void Native3D::cullFaceEnabled(bool state) {
	if (_activeContext->cullFace_enabled == state)
		return;

	_activeContext->cullFace_enabled = state;

	updateCullMode();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// depth map + polygon offsetting

void Native3D::polygonOffset(float pOffUnits) {
	if (_activeContext->depthMap_pOffUnits == pOffUnits)
		return;

	_activeContext->depthMap_pOffUnits = pOffUnits;

	updateDepthMap();
}

void Native3D::wScale(float wScale) {
	if (_activeContext->depthMap_wScale == wScale)
		return;

	_activeContext->depthMap_wScale = wScale;

	updateDepthMap();
}

void Native3D::depthRange(float near, float far) {
	if ((_activeContext->depthMap_rangeN == near) &&
	    (_activeContext->depthMap_rangeF == far ))
		return;

	_activeContext->depthMap_rangeN = near;
	_activeContext->depthMap_rangeF = far;

	updateDepthMap();
}

void Native3D::polygonOffsetEnabled(bool state) {
	if (_activeContext->depthMap_enabled == state)
		return;

	_activeContext->depthMap_enabled = state;

	updateDepthMap();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// scissoring

void Native3D::scissorMode(GPU_SCISSORMODE mode) {
	if (_activeContext->scissor_mode == mode)
		return;

	_activeContext->scissor_mode = mode;

	C3D_SetScissor(_activeContext->scissor_mode,
	               _activeContext->scissor_x,
	               _activeContext->scissor_y,
	               _activeContext->scissor_w,
	               _activeContext->scissor_h);
}

void Native3D::scissorDims(u32 x, u32 y, u32 w, u32 h) {
	if ((_activeContext->scissor_x == x) && (_activeContext->scissor_y == y) &&
	    (_activeContext->scissor_w == w) && (_activeContext->scissor_h == h))
		return;

	_activeContext->scissor_x = x;
	_activeContext->scissor_y = y;
	_activeContext->scissor_w = w;
	_activeContext->scissor_h = h;

	C3D_SetScissor(_activeContext->scissor_mode,
	               _activeContext->scissor_x,
	               _activeContext->scissor_y,
	               _activeContext->scissor_w,
	               _activeContext->scissor_h);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// alpha testing

void Native3D::alphaFunc(GPU_TESTFUNC func, int ref) {
	if ((_activeContext->alphaTest_func == func) &&
	    (_activeContext->alphaTest_ref  == ref))
		return;

	_activeContext->alphaTest_func = func;
	_activeContext->alphaTest_ref = ref;

	C3D_AlphaTest(_activeContext->alphaTest_enabled,
	              _activeContext->alphaTest_func,
	              _activeContext->alphaTest_ref);
}

void Native3D::alphaTestEnabled(bool state) {
	if (_activeContext->alphaTest_enabled == state)
		return;

	_activeContext->alphaTest_enabled = state;

	C3D_AlphaTest(_activeContext->alphaTest_enabled,
	              _activeContext->alphaTest_func,
	              _activeContext->alphaTest_ref);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// stencil testing

void Native3D::stencilFunc(GPU_TESTFUNC func, int ref, int mask) {
	if ((_activeContext->stencilTest_func == func) &&
	    (_activeContext->stencilTest_ref  == ref) &&
	    (_activeContext->stencilTest_mask == mask))
	    return;

	_activeContext->stencilTest_func = func;
	_activeContext->stencilTest_ref  = ref;
	_activeContext->stencilTest_mask = mask;

	C3D_StencilTest(_activeContext->stencilTest_enabled,
	                _activeContext->stencilTest_func,
	                _activeContext->stencilTest_ref,
	                _activeContext->stencilTest_mask,
	                _activeContext->stencilTest_writeMask);
}

void Native3D::stencilMask(int writeMask) {
	if (_activeContext->stencilTest_writeMask == writeMask)
		return;

	_activeContext->stencilTest_writeMask = writeMask;

	C3D_StencilTest(_activeContext->stencilTest_enabled,
	                _activeContext->stencilTest_func,
	                _activeContext->stencilTest_ref,
	                _activeContext->stencilTest_mask,
	                _activeContext->stencilTest_writeMask);
}

void Native3D::stencilOp(GPU_STENCILOP fail, GPU_STENCILOP zfail, GPU_STENCILOP zpass) {
	if ((_activeContext->stencilOp_fail  == fail) &&
	    (_activeContext->stencilOp_zfail == zfail) &&
	    (_activeContext->stencilOp_zpass == zpass))
	    return;

	_activeContext->stencilOp_fail  = fail;
	_activeContext->stencilOp_zfail = zfail;
	_activeContext->stencilOp_zpass = zpass;

	C3D_StencilOp(_activeContext->stencilOp_fail,
	              _activeContext->stencilOp_zfail,
	              _activeContext->stencilOp_zpass);
}

// CLEAR STENCIL????????????

void Native3D::stencilTestEnabled(bool state) {
	if (_activeContext->stencilTest_enabled == state)
		return;

	_activeContext->stencilTest_enabled = state;

	C3D_StencilTest(_activeContext->stencilTest_enabled,
	                _activeContext->stencilTest_func,
	                _activeContext->stencilTest_ref,
	                _activeContext->stencilTest_mask,
	                _activeContext->stencilTest_writeMask);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// depth testing

void Native3D::depthFunc(GPU_TESTFUNC func) {
	if (_activeContext->depthTest_func == func)
		return;

	_activeContext->depthTest_func = func;

	C3D_DepthTest(_activeContext->depthTest_enabled,
	              _activeContext->depthTest_func,
	              _activeContext->depthTest_enabled ? _activeContext->depthTest_writeMask :
	                                  (GPU_WRITEMASK)(_activeContext->depthTest_writeMask & ~GPU_WRITE_DEPTH));
}

void Native3D::depthMask(bool depthState) {
	GPU_WRITEMASK tempMask = (GPU_WRITEMASK)((_activeContext->depthTest_writeMask & ~GPU_WRITE_DEPTH) |
	                                         (depthState ? GPU_WRITE_DEPTH : 0));

	if (_activeContext->depthTest_writeMask == tempMask)
		return;

	_activeContext->depthTest_writeMask = tempMask;

	C3D_DepthTest(_activeContext->depthTest_enabled,
	              _activeContext->depthTest_func,
	              _activeContext->depthTest_enabled ? _activeContext->depthTest_writeMask :
	                                  (GPU_WRITEMASK)(_activeContext->depthTest_writeMask & ~GPU_WRITE_DEPTH));
}

void Native3D::colorMask(bool redState, bool greenState, bool blueState, bool alphaState) {
	GPU_WRITEMASK tempMask = (GPU_WRITEMASK)((_activeContext->depthTest_writeMask & ~GPU_WRITE_COLOR) |
	                                         (redState   ? GPU_WRITE_RED   : 0)     |
	                                         (greenState ? GPU_WRITE_GREEN : 0)     |
	                                         (blueState  ? GPU_WRITE_BLUE  : 0)     |
	                                         (alphaState ? GPU_WRITE_ALPHA : 0));

	if (_activeContext->depthTest_writeMask == tempMask)
		return;

	_activeContext->depthTest_writeMask = tempMask;

	C3D_DepthTest(_activeContext->depthTest_enabled,
	              _activeContext->depthTest_func,
	              _activeContext->depthTest_enabled ? _activeContext->depthTest_writeMask :
	                                  (GPU_WRITEMASK)(_activeContext->depthTest_writeMask & ~GPU_WRITE_DEPTH));
}

// CLEAR DEPTH?????????????

void Native3D::depthTestEnabled(bool state) {
	if (_activeContext->depthTest_enabled == state)
		return;

	_activeContext->depthTest_enabled = state;

	C3D_DepthTest(_activeContext->depthTest_enabled,
	              _activeContext->depthTest_func,
	              _activeContext->depthTest_enabled ? _activeContext->depthTest_writeMask :
	                                  (GPU_WRITEMASK)(_activeContext->depthTest_writeMask & ~GPU_WRITE_DEPTH));
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// EARLY depth testing

void Native3D::earlyDepthFunc(GPU_EARLYDEPTHFUNC func) {
	if (_activeContext->earlyDepthTest_func == func)
		return;

	_activeContext->earlyDepthTest_func = func;

	C3D_EarlyDepthTest(_activeContext->earlyDepthTest_enabled,
	                   _activeContext->earlyDepthTest_func,
	                   _activeContext->earlyDepthTest_clear);
}

void Native3D::clearEarlyDepth(u32 clearValue) {
	if (_activeContext->earlyDepthTest_clear == clearValue)
		return;

	_activeContext->earlyDepthTest_clear = clearValue;

	C3D_EarlyDepthTest(_activeContext->earlyDepthTest_enabled,
	                   _activeContext->earlyDepthTest_func,
	                   _activeContext->earlyDepthTest_clear);
}

void Native3D::earlyDepthTestEnabled(bool state) {
	if (_activeContext->earlyDepthTest_enabled == state)
		return;

	_activeContext->earlyDepthTest_enabled = state;

	C3D_EarlyDepthTest(_activeContext->earlyDepthTest_enabled,
	                   _activeContext->earlyDepthTest_func,
	                   _activeContext->earlyDepthTest_clear);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// blending

void Native3D::blendEquation(GPU_BLENDEQUATION equation) {
	if ((_activeContext->blend_colorEq == equation) &&
	    (_activeContext->blend_alphaEq == equation))
		return;

	_activeContext->blend_colorEq = _activeContext->blend_alphaEq = equation;

	updateBlend();
}

void Native3D::blendEquationSeparate(GPU_BLENDEQUATION colorEq, GPU_BLENDEQUATION alphaEq) {
	if ((_activeContext->blend_colorEq == colorEq) &&
	    (_activeContext->blend_alphaEq == alphaEq))
		return;

	_activeContext->blend_colorEq = colorEq;
	_activeContext->blend_alphaEq = alphaEq;

	updateBlend();
}

void Native3D::blendFunc(GPU_BLENDFACTOR srcFactor, GPU_BLENDFACTOR dstFactor) {
	if ((_activeContext->blend_srcColor == srcFactor) &&
	    (_activeContext->blend_srcAlpha == srcFactor) &&
	    (_activeContext->blend_dstColor == dstFactor) &&
	    (_activeContext->blend_dstAlpha == dstFactor))
		return;

	_activeContext->blend_srcColor = _activeContext->blend_srcAlpha = srcFactor;
	_activeContext->blend_dstColor = _activeContext->blend_dstAlpha = dstFactor;

	updateBlend();
}

void Native3D::blendFuncSeparate(GPU_BLENDFACTOR srcColor, GPU_BLENDFACTOR dstColor,
	                             GPU_BLENDFACTOR srcAlpha, GPU_BLENDFACTOR dstAlpha) {
	if ((_activeContext->blend_srcColor == srcColor) &&
	    (_activeContext->blend_dstColor == dstColor) &&
	    (_activeContext->blend_srcAlpha == srcAlpha) &&
	    (_activeContext->blend_dstAlpha == dstAlpha))
		return;

	_activeContext->blend_srcColor = srcColor;
	_activeContext->blend_dstColor = dstColor;
	_activeContext->blend_srcAlpha = srcAlpha;
	_activeContext->blend_dstAlpha = dstAlpha;

	updateBlend();
}

void Native3D::blendColor(u8 red, u8 green, u8 blue, u8 alpha) {
	if ((_activeContext->blend_color[0] == red) &&
	    (_activeContext->blend_color[1] == green) &&
	    (_activeContext->blend_color[2] == blue) &&
	    (_activeContext->blend_color[3] == alpha))
		return;

	_activeContext->blend_color[0] = red;
	_activeContext->blend_color[1] = green;
	_activeContext->blend_color[2] = blue;
	_activeContext->blend_color[3] = alpha;

	u32 combined = (_activeContext->blend_color[3] << 24) | (_activeContext->blend_color[2] << 16) |
	               (_activeContext->blend_color[1] << 8)  |  _activeContext->blend_color[0];
	C3D_BlendingColor(combined);
}

void Native3D::blendEnabled(bool state) {
	if (_activeContext->blend_enabled == state)
		return;

	_activeContext->blend_enabled = state;

	if (_activeContext->colorLogicOp_enabled == true)
		C3D_ColorLogicOp(_activeContext->colorLogicOp_op);
	else {
		(_activeContext->blend_enabled)
			? C3D_AlphaBlend(_activeContext->blend_colorEq,  _activeContext->blend_alphaEq,
			                 _activeContext->blend_srcColor, _activeContext->blend_dstColor,
			                 _activeContext->blend_srcAlpha, _activeContext->blend_dstAlpha)
			: C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD,
			                 GPU_ONE,       GPU_ZERO,
			                 GPU_ONE,       GPU_ZERO);
		if (_activeContext->fragOpMode != GPU_FRAGOPMODE_GL)
			C3D_FragOpMode(_activeContext->fragOpMode);
	}
}

// CLEAR COLOR???????????????

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// color logic

void Native3D::logicOp(GPU_LOGICOP op) {
	if (_activeContext->colorLogicOp_op == op)
		return;

	_activeContext->colorLogicOp_op = op;

	C3D_ColorLogicOp(_activeContext->colorLogicOp_op);
	if (_activeContext->colorLogicOp_enabled != true) {                              // ??????????????????????????????????
		C3D_AlphaBlend(_activeContext->blend_colorEq,  _activeContext->blend_alphaEq,
		               _activeContext->blend_srcColor, _activeContext->blend_dstColor,
		               _activeContext->blend_srcAlpha, _activeContext->blend_dstAlpha);
		if (_activeContext->fragOpMode != GPU_FRAGOPMODE_GL)
			C3D_FragOpMode(_activeContext->fragOpMode);
	}
}

void Native3D::colorLogicOpEnabled(bool state) {
	if (_activeContext->colorLogicOp_enabled == state)
		return;

	_activeContext->colorLogicOp_enabled = state;

	if (_activeContext->colorLogicOp_enabled == true)
		C3D_ColorLogicOp(_activeContext->colorLogicOp_op);
	else {
		(_activeContext->blend_enabled)
			? C3D_AlphaBlend(_activeContext->blend_colorEq,  _activeContext->blend_alphaEq,
			                 _activeContext->blend_srcColor, _activeContext->blend_dstColor,
			                 _activeContext->blend_srcAlpha, _activeContext->blend_dstAlpha)
			: C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD,
			                 GPU_ONE,       GPU_ZERO,
			                 GPU_ONE,       GPU_ZERO);
		if (_activeContext->fragOpMode != GPU_FRAGOPMODE_GL)
			C3D_FragOpMode(_activeContext->fragOpMode);
	}
}

} // end of namespace N3DS_3D