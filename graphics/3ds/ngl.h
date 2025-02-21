/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your N3D_tion) any later version.
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

enum N3D_CULLFACE {
	N3D_CULLFACE_FRONT          = 0,
	N3D_CULLFACE_BACK           = 1,
	N3D_CULLFACE_FRONT_AND_BACK = 2,
};

enum N3D_FRONTFACE {
	N3D_FRONTFACE_CW  = 0,
	N3D_FRONTFACE_CCW = 1,
};

// from "graphics/3ds/ops-gl2citro.cpp"
// Equivalent to glPolygonOffset, except no "factor" parameter exists
void N3D_PolygonOffset(float pOffUnits);
// Equivalent to glDepthRangef
void N3D_DepthRange(float near, float far);
// Equivalent to gl(Dis/En)able(GL_POLYGON_OFFSET_FILL)
void N3D_PolygonOffsetEnabled(bool state);
// Equivalent to glCullFace
void N3D_CullFace(N3D_CULLFACE mode);
// Equivalent to glFrontFace
void N3D_FrontFace(N3D_FRONTFACE mode);
// Equivalent to gl(Dis/En)able(GL_CULL_FACE)
void N3D_CullFaceEnabled(bool state);
// Equivalent to glStencilFunc
void N3D_StencilFunc(GPU_TESTFUNC func, int ref, int mask);
// Equivalent to glStencilMask
void N3D_StencilMask(int writeMask);
// Equivalent to gl(Dis/En)able(GL_STENCIL_TEST)
void N3D_StencilTestEnabled(bool state);
// Equivalent to glStencilOp
void N3D_StencilOp(GPU_STENCILOP fail, GPU_STENCILOP zfail, GPU_STENCILOP zpass);
// Equivalent to glBlendColor
void N3D_BlendColor(u8 red, u8 green, u8 blue, u8 alpha);
// Like glDepthFunc, but for early depth testing
void N3D_EarlyDepthFunc(GPU_EARLYDEPTHFUNC func);
// Clear value for early depth testing
void N3D_EarlyDepthClear(u32 clearValue);
// Like gl(Dis/En)able(GL_DEPTH_TEST), but for early depth testing
void N3D_EarlyDepthTestEnabled(bool state);
// Equivalent to glDepthFunc
void N3D_DepthFunc(GPU_TESTFUNC func);
// Equivalent to glDepthMask
void N3D_DepthMask(bool depthState);
// Equivalent to glColorMask
void N3D_ColorMask(bool redState, bool greenState, bool blueState, bool alphaState);
// Equivalent to gl(Dis/En)able(GL_DEPTH_TEST)
void N3D_DepthTestEnabled(bool state);
// Equivalent to glAlphaFunc
void N3D_AlphaFunc(GPU_TESTFUNC func, int ref);
// Equivalent to gl(Dis/En)able(GL_ALPHA_TEST)
void N3D_AlphaTestEnabled(bool state);
// Equivalent to glBlendEquation
void N3D_BlendEquation(GPU_BLENDEQUATION equation);
// Equivalent to glBlendEquationSeparate
void N3D_BlendEquationSeparate(GPU_BLENDEQUATION colorEq, GPU_BLENDEQUATION alphaEq);
// Equivalent to glBlendFunc
void N3D_BlendFunc(GPU_BLENDFACTOR srcFactor, GPU_BLENDFACTOR dstFactor);
// Equivalent to glBlendFuncSeparate
void N3D_BlendFuncSeparate(GPU_BLENDFACTOR srcColor, GPU_BLENDFACTOR dstColor, GPU_BLENDFACTOR srcAlpha, GPU_BLENDFACTOR dstAlpha);
// Equivalent to gl(Dis/En)able(GL_BLEND)
void N3D_BlendEnabled(bool state);
// Equivalent to glLogicOp
void N3D_LogicOp(GPU_LOGICOP op);
// Equivalent to gl(Dis/En)able(GL_COLOR_LOGIC_OP)
void N3D_LogicOpEnabled(bool state);
// Search "GPU_SCISSORMODE" at https://libctru.devkitpro.org/
void N3D_ScissorMode(GPU_SCISSORMODE mode);
// Equivalent to glScissor
void N3D_ScissorDims(u32 x, u32 y, u32 w, u32 h);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~CLEAR STENCIL????????????
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~CLEAR DEPTH?????????????
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~CLEAR COLOR???????????????


// CUSTOM FUNCS
// Return a pointer to the C3D_Tex "_gameTopTexture.texture" from the 3DS backend.
C3D_Tex         *N3D_GetGameScreen();
// Create a buffer in the 3DS's linear memory, and optionally populates it with data.
void            *N3D_CreateBuffer(size_t size, const void *data = nullptr, size_t alignment = 0x80);
// Free a buffer that was created in linear memory.
void             N3D_FreeBuffer(void *linearBuffer);

// Copy a rectangular area of arbitrary size from one buffer, Morton-swizzle it (if it isn't already),
// 	and paste it into an arbitrary location of another buffer.
// PARAMETERS:
// 	srcBuf:     pointer to the source buffer
// 	dstBuf:     pointer to the destination buffer
// 	copyWidth:  width of the area to copy
// 	copyHeight: height of the area to copy
// 	xSource:    unswizzled X offset of the copy area's top-left corner
// 	ySource:    unswizzled Y offset of the copy area's top-left corner
// 	wSource:    width of the source buffer (not just the copy area)
// 	hSource:    height of the source buffer (not just the copy area)
// 	xDest:      unswizzled X offset of the paste area's top-left corner
// 	yDest:      unswizzled Y offset of the paste area's top-left corner
// 	wDest:      width of the destination buffer (not just the paste area)
// 	hDest:      height of the destination buffer (not just the paste area)
// 	format:     texture format of the source and destination buffers (Search "GPU_TEXCOLOR" at https://libctru.devkitpro.org/)
// 	isBlockSrc: whether or not the source buffer is already in a Morton-swizzled pixel order

// !!! NOTE !!!
// wSource, hSource, wDest, and hDest MUST be powers of two.
void             N3D_ArbDataToArbBlockTexOffset(u32 *srcBuf, u32 *dstBuf, int copyWidth, int copyHeight,
                                                int xSource, int ySource, int wSource,   int hSource,
                                                int xDest,   int yDest,   int wDest,     int hDest,
                                                GPU_TEXCOLOR format, bool isBlockSrc);

// Same as N3D_ArbDataToArbBlockTexOffset, except there is no destination offset
void             N3D_DataToBlockTex(u32 *srcBuf, u32 *dstBuf, int xSource, int ySource,
                                    int wSource, int hSource, int wDest, int hDest,
                                    GPU_TEXCOLOR format, bool isBlockSrc);

void                   N3D_C3D_TexBind(int unitId, C3D_Tex* tex);

#endif // GRAPHICS_3DS_NGL_H
