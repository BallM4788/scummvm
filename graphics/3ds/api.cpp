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

#include "graphics/3ds/z3d.h"

// from "graphics/3ds/ops-gl2citro.cpp"
void N3D_CullFace(N3D_CULLFACE mode) {
	N3DS_3D::getActiveContext()->opCullFace(mode);
}

void N3D_FrontFace(N3D_FRONTFACE mode) {
	N3DS_3D::getActiveContext()->opFrontFace(mode);
}

void N3D_CullFaceEnabled(bool state) {
	N3DS_3D::getActiveContext()->opCullFaceEnabled(state);
}

void N3D_PolygonOffset(float pOffUnits) {
	N3DS_3D::getActiveContext()->opPolygonOffset(pOffUnits);
}

void N3D_DepthRange(float near, float far) {
	N3DS_3D::getActiveContext()->opDepthRange(near, far);
}

void N3D_PolygonOffsetEnabled(bool state) {
	N3DS_3D::getActiveContext()->opPolygonOffsetEnabled(state);
}

void N3D_ScissorMode(GPU_SCISSORMODE mode) {
	N3DS_3D::getActiveContext()->opScissorMode(mode);
}

void N3D_ScissorDims(u32 x, u32 y, u32 w, u32 h) {
	N3DS_3D::getActiveContext()->opScissorDims(x, y, w, h);
}

void N3D_AlphaFunc(GPU_TESTFUNC func, int ref) {
	N3DS_3D::getActiveContext()->opAlphaFunc(func, ref);
}

void N3D_AlphaTestEnabled(bool state) {
	N3DS_3D::getActiveContext()->opAlphaTestEnabled(state);
}

void N3D_StencilFunc(GPU_TESTFUNC func, int ref, int mask) {
	N3DS_3D::getActiveContext()->opStencilFunc(func, ref, mask);
}

void N3D_StencilMask(int writeMask) {
	N3DS_3D::getActiveContext()->opStencilMask(writeMask);
}

void N3D_StencilOp(GPU_STENCILOP fail, GPU_STENCILOP zfail, GPU_STENCILOP zpass) {
	N3DS_3D::getActiveContext()->opStencilOp(fail, zfail, zpass);
}

// CLEAR STENCIL????????????

void N3D_StencilTestEnabled(bool state) {
	N3DS_3D::getActiveContext()->opStencilTestEnabled(state);
}

void N3D_DepthFunc(GPU_TESTFUNC func) {
	N3DS_3D::getActiveContext()->opDepthFunc(func);
}

void N3D_DepthMask(bool depthState) {
	N3DS_3D::getActiveContext()->opDepthMask(depthState);
}

void N3D_ColorMask(bool redState, bool greenState, bool blueState, bool alphaState) {
	N3DS_3D::getActiveContext()->opColorMask(redState, greenState, blueState, alphaState);
}

// CLEAR DEPTH?????????????

void N3D_DepthTestEnabled(bool state) {
	N3DS_3D::getActiveContext()->opDepthTestEnabled(state);
}

void N3D_EarlyDepthFunc(GPU_EARLYDEPTHFUNC func) {
	N3DS_3D::getActiveContext()->opEarlyDepthFunc(func);
}

void N3D_ClearEarlyDepth(u32 clearValue) {
	N3DS_3D::getActiveContext()->opClearEarlyDepth(clearValue);
}

void N3D_EarlyDepthTestEnabled(bool state) {
	N3DS_3D::getActiveContext()->opEarlyDepthTestEnabled(state);
}

void N3D_BlendEquation(GPU_BLENDEQUATION equation) {
	N3DS_3D::getActiveContext()->opBlendEquation(equation);
}

void N3D_BlendEquationSeparate(GPU_BLENDEQUATION colorEq, GPU_BLENDEQUATION alphaEq) {
	N3DS_3D::getActiveContext()->opBlendEquationSeparate(colorEq, alphaEq);
}

void N3D_BlendFunc(GPU_BLENDFACTOR srcFactor, GPU_BLENDFACTOR dstFactor) {
	N3DS_3D::getActiveContext()->opBlendFunc(srcFactor, dstFactor);
}

void N3D_BlendFuncSeparate(GPU_BLENDFACTOR srcColor, GPU_BLENDFACTOR dstColor, GPU_BLENDFACTOR srcAlpha, GPU_BLENDFACTOR dstAlpha) {
	N3DS_3D::getActiveContext()->opBlendFuncSeparate(srcColor, dstColor, srcAlpha, dstAlpha);
}

void N3D_BlendColor(u8 red, u8 green, u8 blue, u8 alpha) {
	N3DS_3D::getActiveContext()->opBlendColor(red, green, blue, alpha);
}

void N3D_BlendEnabled(bool state) {
	N3DS_3D::getActiveContext()->opBlendEnabled(state);
}

// CLEAR COLOR???????????????

void N3D_LogicOp(GPU_LOGICOP op) {
	N3DS_3D::getActiveContext()->opLogicOp(op);
}

void N3D_ColorLogicOpEnabled(bool state) {
	N3DS_3D::getActiveContext()->opColorLogicOpEnabled(state);
}



// CUSTOM FUNCS
C3D_Tex *N3D_GetGameScreen() {
	return N3DS_3D::getActiveContext()->opGetGameScreen();
//	N3DS::OSystem_3DS *gsys3DS = dynamic_cast<N3DS::OSystem_3DS *>(g_system);
//	return gsys3DS->getGameSurface();
}

void *N3D_CreateBuffer(size_t size, const void *data, size_t alignment) {
	return N3DS_3D::getActiveContext()->opCreateBuffer(size, data, alignment);
//	N3DS::OSystem_3DS *gsys3DS = dynamic_cast<N3DS::OSystem_3DS *>(g_system);
//	return gsys3DS->createBuffer(size, data);
}

void N3D_FreeBuffer(void *linearBuffer) {
	N3DS_3D::getActiveContext()->opFreeBuffer(linearBuffer);
//	N3DS::OSystem_3DS *gsys3DS = dynamic_cast<N3DS::OSystem_3DS *>(g_system);
//	return gsys3DS->freeBuffer(linearBuffer);
}

void N3D_ArbDataToArbBlockTexOffset(u32 *srcBuf, u32 *dstBuf, int copyWidth, int copyHeight,
                                    int xSource, int ySource, int wSource,   int hSource,
                                    int xDest,   int yDest,   int wDest,     int hDest,
                                    GPU_TEXCOLOR format, bool isBlockSrc) {
	N3DS_3D::getActiveContext()->opArbDataToArbBlockTexOffset(srcBuf, dstBuf, copyWidth, copyHeight,
	                                                          xSource, ySource, wSource, hSource,
	                                                          xDest, yDest, wDest, hDest,
	                                                          format, isBlockSrc);
}

void N3D_DataToBlockTex(u32 *srcBuf, u32 *dstBuf, int xSource, int ySource,
                        int wSource, int hSource, int wDest, int hDest,
                        GPU_TEXCOLOR format, bool isBlockSrc) {
	N3DS_3D::getActiveContext()->opDataToBlockTex(srcBuf, dstBuf, xSource, ySource,
	                                              wSource, hSource, wDest, hDest,
	                                              format, isBlockSrc);
}


void N3D_C3D_TexBind(int unitId, C3D_Tex* tex) {
	if (N3DS_3D::getActiveContext()->boundTexUnits[unitId] != tex) {
		C3D_TexBind(unitId, tex);
		N3DS_3D::getActiveContext()->boundTexUnits[unitId] = tex;
	}
}
