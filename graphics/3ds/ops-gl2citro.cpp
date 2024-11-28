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

namespace N3DS_3D {

//void N3DContext::opViewport(u32 x, u32 y, u32 w, u32 h) {
//	if ((vport_x == x) && (vport_y == y) && (vport_w == w) && (vport_h == h))
//		return;
//
//	vport_x = x;
//	vport_y = y;
//	vport_w = w;
//	vport_h = h;
//
//	C3D_SetViewport(vport_x, vport_y, vport_w, vport_h);
//}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// culling

void N3DContext::opCullFace(N3D_CULLFACE mode) {
	assert(mode == N3D_CULLFACE_FRONT ||
	       mode == N3D_CULLFACE_BACK  ||
	       mode == N3D_CULLFACE_FRONT_AND_BACK);

	if (cullFace_faceToCull == mode)
		return;

	cullFace_faceToCull = mode;

	updateCullMode();
}

void N3DContext::opFrontFace(N3D_FRONTFACE mode) {
	assert(mode == N3D_FRONTFACE_CW || mode == N3D_FRONTFACE_CCW);

	if (cullFace_frontFace == mode)
		return;

	cullFace_frontFace = mode;

	updateCullMode();
}

void N3DContext::opCullFaceEnabled(bool state) {
	if (cullFace_enabled == state)
		return;

	cullFace_enabled = state;

	updateCullMode();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// depth map + polygon offsetting

void N3DContext::opPolygonOffset(float pOffUnits) {
	if (depthMap_pOffUnits == pOffUnits)
		return;

	depthMap_pOffUnits = pOffUnits;

	updateDepthMap();
}

void N3DContext::opWScale(float wScale) {
	if (depthMap_wScale == wScale)
		return;

	depthMap_wScale = wScale;

	updateDepthMap();
}

void N3DContext::opDepthRange(float near, float far) {
	if ((depthMap_rangeN == near) && (depthMap_rangeF == far ))
		return;

	depthMap_rangeN = near;
	depthMap_rangeF = far;

	updateDepthMap();
}

void N3DContext::opPolygonOffsetEnabled(bool state) {
	if (depthMap_enabled == state)
		return;

	depthMap_enabled = state;

	updateDepthMap();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// scissoring

void N3DContext::opScissorMode(GPU_SCISSORMODE mode) {
	if (scissor_mode == mode)
		return;

	scissor_mode = mode;

	C3D_SetScissor(scissor_mode, scissor_x, scissor_y, scissor_w, scissor_h);
}

void N3DContext::opScissorDims(u32 x, u32 y, u32 w, u32 h) {
	if ((scissor_x == x) && (scissor_y == y) && (scissor_w == w) && (scissor_h == h))
		return;

	scissor_x = x;
	scissor_y = y;
	scissor_w = w;
	scissor_h = h;

	C3D_SetScissor(scissor_mode, scissor_x, scissor_y, scissor_w, scissor_h);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// alpha testing

void N3DContext::opAlphaFunc(GPU_TESTFUNC func, int ref) {
	if ((alphaTest_func == func) && (alphaTest_ref  == ref))
		return;

	alphaTest_func = func;
	alphaTest_ref = ref;

	C3D_AlphaTest(alphaTest_enabled, alphaTest_func, alphaTest_ref);
}

void N3DContext::opAlphaTestEnabled(bool state) {
	if (alphaTest_enabled == state)
		return;

	alphaTest_enabled = state;

	C3D_AlphaTest(alphaTest_enabled, alphaTest_func, alphaTest_ref);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// stencil testing

void N3DContext::opStencilFunc(GPU_TESTFUNC func, int ref, int mask) {
	if ((stencilTest_func == func) && (stencilTest_ref  == ref) && (stencilTest_mask == mask))
		return;

	stencilTest_func = func;
	stencilTest_ref  = ref;
	stencilTest_mask = mask;

	C3D_StencilTest(stencilTest_enabled, stencilTest_func, stencilTest_ref, stencilTest_mask, stencilTest_writeMask);
}

void N3DContext::opStencilMask(int writeMask) {
	if (stencilTest_writeMask == writeMask)
		return;

	stencilTest_writeMask = writeMask;

	C3D_StencilTest(stencilTest_enabled, stencilTest_func, stencilTest_ref, stencilTest_mask, stencilTest_writeMask);
}

void N3DContext::opStencilOp(GPU_STENCILOP fail, GPU_STENCILOP zfail, GPU_STENCILOP zpass) {
	if ((stencilOp_fail  == fail) && (stencilOp_zfail == zfail) && (stencilOp_zpass == zpass))
		return;

	stencilOp_fail  = fail;
	stencilOp_zfail = zfail;
	stencilOp_zpass = zpass;

	C3D_StencilOp(stencilOp_fail, stencilOp_zfail, stencilOp_zpass);
}

// CLEAR STENCIL????????????

void N3DContext::opStencilTestEnabled(bool state) {
	if (stencilTest_enabled == state)
		return;

	stencilTest_enabled = state;

	C3D_StencilTest(stencilTest_enabled, stencilTest_func, stencilTest_ref, stencilTest_mask, stencilTest_writeMask);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// depth testing

void N3DContext::opDepthFunc(GPU_TESTFUNC func) {
	if (depthTest_func == func)
		return;

	depthTest_func = func;

	C3D_DepthTest(depthTest_enabled, depthTest_func,
	              depthTest_enabled ? depthTest_writeMask : (GPU_WRITEMASK)(depthTest_writeMask & ~GPU_WRITE_DEPTH));
}

void N3DContext::opDepthMask(bool depthState) {
	GPU_WRITEMASK tempMask = (GPU_WRITEMASK)((depthTest_writeMask & ~GPU_WRITE_DEPTH) | (depthState ? GPU_WRITE_DEPTH : 0));

	if (depthTest_writeMask == tempMask)
		return;

	depthTest_writeMask = tempMask;

	C3D_DepthTest(depthTest_enabled, depthTest_func,
	              depthTest_enabled ? depthTest_writeMask : (GPU_WRITEMASK)(depthTest_writeMask & ~GPU_WRITE_DEPTH));
}

void N3DContext::opColorMask(bool redState, bool greenState, bool blueState, bool alphaState) {
	GPU_WRITEMASK tempMask = (GPU_WRITEMASK)((depthTest_writeMask & ~GPU_WRITE_COLOR) |
	                                         (redState   ? GPU_WRITE_RED   : 0)       |
	                                         (greenState ? GPU_WRITE_GREEN : 0)       |
	                                         (blueState  ? GPU_WRITE_BLUE  : 0)       |
	                                         (alphaState ? GPU_WRITE_ALPHA : 0));

	if (depthTest_writeMask == tempMask)
		return;

	depthTest_writeMask = tempMask;

	C3D_DepthTest(depthTest_enabled, depthTest_func,
	              depthTest_enabled ? depthTest_writeMask : (GPU_WRITEMASK)(depthTest_writeMask & ~GPU_WRITE_DEPTH));
}

// CLEAR DEPTH?????????????

void N3DContext::opDepthTestEnabled(bool state) {
	if (depthTest_enabled == state)
		return;

	depthTest_enabled = state;

	C3D_DepthTest(depthTest_enabled, depthTest_func,
	              depthTest_enabled ? depthTest_writeMask : (GPU_WRITEMASK)(depthTest_writeMask & ~GPU_WRITE_DEPTH));
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// EARLY depth testing

void N3DContext::opEarlyDepthFunc(GPU_EARLYDEPTHFUNC func) {
	if (earlyDepthTest_func == func)
		return;

	earlyDepthTest_func = func;

	C3D_EarlyDepthTest(earlyDepthTest_enabled, earlyDepthTest_func, earlyDepthTest_clear);
}

void N3DContext::opClearEarlyDepth(u32 clearValue) {
	if (earlyDepthTest_clear == clearValue)
		return;

	earlyDepthTest_clear = clearValue;

	C3D_EarlyDepthTest(earlyDepthTest_enabled, earlyDepthTest_func, earlyDepthTest_clear);
}

void N3DContext::opEarlyDepthTestEnabled(bool state) {
	if (earlyDepthTest_enabled == state)
		return;

	earlyDepthTest_enabled = state;

	C3D_EarlyDepthTest(earlyDepthTest_enabled, earlyDepthTest_func, earlyDepthTest_clear);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// blending

void N3DContext::opBlendEquation(GPU_BLENDEQUATION equation) {
	if ((blend_colorEq == equation) && (blend_alphaEq == equation))
		return;

	blend_colorEq = blend_alphaEq = equation;

	updateBlend();
}

void N3DContext::opBlendEquationSeparate(GPU_BLENDEQUATION colorEq, GPU_BLENDEQUATION alphaEq) {
	if ((blend_colorEq == colorEq) && (blend_alphaEq == alphaEq))
		return;

	blend_colorEq = colorEq;
	blend_alphaEq = alphaEq;

	updateBlend();
}

void N3DContext::opBlendFunc(GPU_BLENDFACTOR srcFactor, GPU_BLENDFACTOR dstFactor) {
	if ((blend_srcColor == srcFactor) && (blend_srcAlpha == srcFactor) && (blend_dstColor == dstFactor) && (blend_dstAlpha == dstFactor))
		return;

	blend_srcColor = blend_srcAlpha = srcFactor;
	blend_dstColor = blend_dstAlpha = dstFactor;

	updateBlend();
}

void N3DContext::opBlendFuncSeparate(GPU_BLENDFACTOR srcColor, GPU_BLENDFACTOR dstColor, GPU_BLENDFACTOR srcAlpha, GPU_BLENDFACTOR dstAlpha) {
	if ((blend_srcColor == srcColor) && (blend_dstColor == dstColor) && (blend_srcAlpha == srcAlpha) && (blend_dstAlpha == dstAlpha))
		return;

	blend_srcColor = srcColor;
	blend_dstColor = dstColor;
	blend_srcAlpha = srcAlpha;
	blend_dstAlpha = dstAlpha;

	updateBlend();
}

void N3DContext::opBlendColor(u8 red, u8 green, u8 blue, u8 alpha) {
	if ((blend_color[0] == red ) && (blend_color[1] == green) && (blend_color[2] == blue) && (blend_color[3] == alpha))
		return;

	blend_color[0] = red;
	blend_color[1] = green;
	blend_color[2] = blue;
	blend_color[3] = alpha;

	u32 combined = (blend_color[3] << 24) | (blend_color[2] << 16) | (blend_color[1] << 8)  |  blend_color[0];
	C3D_BlendingColor(combined);
}

void N3DContext::opBlendEnabled(bool state) {
	if (blend_enabled == state)
		return;

	blend_enabled = state;

	if (colorLogicOp_enabled == true)
		C3D_ColorLogicOp(colorLogicOp_op);
	else {
		(blend_enabled)
			? C3D_AlphaBlend(blend_colorEq, blend_alphaEq, blend_srcColor, blend_dstColor, blend_srcAlpha, blend_dstAlpha)
			: C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_ONE, GPU_ZERO, GPU_ONE, GPU_ZERO);
		if (fragOpMode != GPU_FRAGOPMODE_GL)
			C3D_FragOpMode(fragOpMode);
	}
}

// CLEAR COLOR???????????????

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// color logic

void N3DContext::opLogicOp(GPU_LOGICOP op) {
	if (colorLogicOp_op == op)
		return;

	colorLogicOp_op = op;

	C3D_ColorLogicOp(colorLogicOp_op);
	if (colorLogicOp_enabled != true) {
		C3D_AlphaBlend(blend_colorEq, blend_alphaEq, blend_srcColor, blend_dstColor, blend_srcAlpha, blend_dstAlpha);
		if (fragOpMode != GPU_FRAGOPMODE_GL)
			C3D_FragOpMode(fragOpMode);
	}
}

void N3DContext::opColorLogicOpEnabled(bool state) {
	if (colorLogicOp_enabled == state)
		return;

	colorLogicOp_enabled = state;

	if (colorLogicOp_enabled == true)
		C3D_ColorLogicOp(colorLogicOp_op);
	else {
		(blend_enabled)
			? C3D_AlphaBlend(blend_colorEq, blend_alphaEq, blend_srcColor, blend_dstColor, blend_srcAlpha, blend_dstAlpha)
			: C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_ONE, GPU_ZERO, GPU_ONE, GPU_ZERO);
		if (fragOpMode != GPU_FRAGOPMODE_GL)
			C3D_FragOpMode(fragOpMode);
	}
}

} // end namespace N3DS_3D