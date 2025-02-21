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

#ifndef GRAPHICS_3DS_NGL_H
#define GRAPHICS_3DS_NGL_H

#define FORBIDDEN_SYMBOL_EXCEPTION_time_h	// for <3ds.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE		// for <tex3ds.h> in n3d.h
#include <3ds.h>
#include <citro3d.h>
#include <tex3ds.h>
#include "common/str.h"

enum ENUM3DS_CULLFACE {
	ENUM3DS_CULLFACE_FRONT          = 0,
	ENUM3DS_CULLFACE_BACK           = 1,
	ENUM3DS_CULLFACE_FRONT_AND_BACK = 2,
};

enum ENUM3DS_FRONTFACE {
	ENUM3DS_FRONTFACE_CW  = 0,
	ENUM3DS_FRONTFACE_CCW = 1,
};

enum ENUM3DS_CAP {
	ENUM3DS_CAP_POLYGON_OFFSET,
	ENUM3DS_CAP_CULL_FACE,
	ENUM3DS_CAP_STENCIL_TEST,
	ENUM3DS_CAP_DEPTH_TEST_EARLY,
	ENUM3DS_CAP_DEPTH_TEST,
	ENUM3DS_CAP_ALPHA_TEST,
	ENUM3DS_CAP_BLEND,
	ENUM3DS_CAP_COLOR_LOGIC_OP
};

// from "graphics/3ds/ops-gl2citro.cpp"
// Equivalent to glPolygonOffset, except no "factor" parameter exists
void glTo3DS_PolygonOffset(float pOffUnits);
// Equivalent to glDepthRangef
void glTo3DS_DepthRange(float near, float far);
// Equivalent to glCullFace
void glTo3DS_CullFace(ENUM3DS_CULLFACE mode);
// Equivalent to glFrontFace
void glTo3DS_FrontFace(ENUM3DS_FRONTFACE mode);
// Equivalent to glStencilFunc
void glTo3DS_StencilFunc(GPU_TESTFUNC func, int ref, int mask);
// Equivalent to glStencilMask
void glTo3DS_StencilMask(int writeMask);
// Equivalent to glStencilOp
void glTo3DS_StencilOp(GPU_STENCILOP fail, GPU_STENCILOP zfail, GPU_STENCILOP zpass);
// Equivalent to glBlendColor
void glTo3DS_BlendColor(u8 red, u8 green, u8 blue, u8 alpha);
// Like glDepthFunc, but for early depth testing
void glTo3DS_EarlyDepthFunc(GPU_EARLYDEPTHFUNC func);
// Clear value for early depth testing
void glTo3DS_EarlyDepthClear(u32 clearValue);
// Equivalent to glDepthFunc
void glTo3DS_DepthFunc(GPU_TESTFUNC func);
// Equivalent to glDepthMask
void glTo3DS_DepthMask(bool depthState);
// Equivalent to glColorMask
void glTo3DS_ColorMask(bool redState, bool greenState, bool blueState, bool alphaState);
// Equivalent to glAlphaFunc
void glTo3DS_AlphaFunc(GPU_TESTFUNC func, int ref);
// Equivalent to glBlendEquation
void glTo3DS_BlendEquation(GPU_BLENDEQUATION equation);
// Equivalent to glBlendEquationSeparate
void glTo3DS_BlendEquationSeparate(GPU_BLENDEQUATION colorEq, GPU_BLENDEQUATION alphaEq);
// Equivalent to glBlendFunc
void glTo3DS_BlendFunc(GPU_BLENDFACTOR srcFactor, GPU_BLENDFACTOR dstFactor);
// Equivalent to glBlendFuncSeparate
void glTo3DS_BlendFuncSeparate(GPU_BLENDFACTOR srcColor, GPU_BLENDFACTOR dstColor, GPU_BLENDFACTOR srcAlpha, GPU_BLENDFACTOR dstAlpha);
// Equivalent to glLogicOp
void glTo3DS_LogicOp(GPU_LOGICOP op);
// Search "GPU_SCISSORMODE" at https://libctru.devkitpro.org/
void glTo3DS_ScissorMode(GPU_SCISSORMODE mode);
// Equivalent to glScissor
void glTo3DS_ScissorDims(u32 x, u32 y, u32 w, u32 h);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~CLEAR STENCIL????????????
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~CLEAR DEPTH?????????????
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~CLEAR COLOR???????????????
// Equivalent to glEnable
void glTo3DS_Enable(ENUM3DS_CAP cap);
// Equivalent to glDisable
void glTo3DS_Disable(ENUM3DS_CAP cap);
// Equivalent to glBindTexture
void glTo3DS_BindTexture(int unitId, C3D_Tex* tex);

// CUSTOM FUNCS
// Return a pointer to the C3D_Tex "_gameTopTexture.texture" from the 3DS backend.
C3D_Tex         *custom3DS_GetGameScreen();
// Create a buffer in the 3DS's linear memory, and optionally populates it with data.
void            *custom3DS_CreateBuffer(size_t size, const void *data = nullptr, size_t alignment = 0x80);
// Free a buffer that was created in linear memory.
void             custom3DS_FreeBuffer(void *linearBuffer);

// Copy a rectangular area of arbitrary size from one buffer, Morton-swizzle it (if it isn't already),
// 	and paste it into an arbitrary location of another buffer.
// PARAMETERS:
//	srcBuf:     pointer to the source buffer
//	dstBuf:     pointer to the destination buffer
//	copyWidth:  width of the area to copy
//	copyHeight: height of the area to copy
//	xSource:    unswizzled X offset of the copy area's top-left corner
//	ySource:    unswizzled, unflipped Y offset of the copy area's top-left corner
//	wSource:    width of the source buffer (not just the copy area)
//	hSource:    height of the source buffer (not just the copy area)
//	xDest:      unswizzled X offset of the paste area's top-left corner
//	yDest:      unswizzled, unflipped Y offset of the paste area's top-left corner
//	wDest:      width of the destination buffer (not just the paste area)
//	hDest:      height of the destination buffer (not just the paste area)
//	format:     texture format of the source and destination buffers (Search "GPU_TEXCOLOR" at https://libctru.devkitpro.org/)
//	isDepthBuf: whether or not the source buffer contains depth values to be DMA'd to VRAM
//	isBlockSrc: whether or not the source buffer is already in a Morton-swizzled pixel order

// !!! NOTE !!!
// wSource, hSource, wDest, and hDest MUST be powers of two.
void             custom3DS_ArbDataToArbBlockTexOffset(u32 *srcBuf, u32 *dstBuf, int copyWidth, int copyHeight,
                                                      int xSource, int ySource, int wSource,   int hSource,
                                                      int xDest,   int yDest,   int wDest,     int hDest,
                                                      GPU_TEXCOLOR format, bool isDepthBuf = false, bool isBlockSrc = false);

// Same as custom3DS_ArbDataToArbBlockTexOffset, except there is no destination offset
void             custom3DS_DataToBlockTex(u32 *srcBuf, u32 *dstBuf, int xSource, int ySource,
                                          int wSource, int hSource, int wDest, int hDest,
                                          GPU_TEXCOLOR format, bool isDepthBuf = false, bool isBlockSrc = false);

#endif // GRAPHICS_3DS_NGL_H
