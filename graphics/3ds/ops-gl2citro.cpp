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

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// culling

void N3DContext::opCullFace(ENUM3DS_CULLFACE mode) {
	assert(mode == ENUM3DS_CULLFACE_FRONT ||
	       mode == ENUM3DS_CULLFACE_BACK  ||
	       mode == ENUM3DS_CULLFACE_FRONT_AND_BACK);

	cullFace_faceToCull = mode;
	dirtyFlags |= kDirtyCulling;
}

void N3DContext::opFrontFace(ENUM3DS_FRONTFACE mode) {
	assert(mode == ENUM3DS_FRONTFACE_CW || mode == ENUM3DS_FRONTFACE_CCW);

	cullFace_frontFace = mode;
	dirtyFlags |= kDirtyCulling;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// depth map + polygon offsetting

void N3DContext::opPolygonOffset(float pOffUnits) {
	depthMap_pOffUnits = pOffUnits;
	dirtyFlags |= kDirtyDepthMap;
}

void N3DContext::opDepthRange(float near, float far) {
	depthMap_rangeN = near;
	depthMap_rangeF = far;
	dirtyFlags |= kDirtyDepthMap;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// scissoring

void N3DContext::opScissorMode(GPU_SCISSORMODE mode) {
	scissor_mode = mode;
	dirtyFlags |= kDirtyScissor;
}

void N3DContext::opScissorDims(u32 x, u32 y, u32 w, u32 h) {
	scissor_x = x;
	scissor_y = y;
	scissor_w = w;
	scissor_h = h;
	dirtyFlags |= kDirtyScissor;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// alpha testing

void N3DContext::opAlphaFunc(GPU_TESTFUNC func, int ref) {
	alphaTest_func = func;
	alphaTest_ref = ref;
	dirtyFlags |= kDirtyAlphaTest;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// stencil testing

void N3DContext::opStencilFunc(GPU_TESTFUNC func, int ref, int mask) {
	stencilTest_func = func;
	stencilTest_ref  = ref;
	stencilTest_mask = mask;
	dirtyFlags |= kDirtyStencilTest;
}

void N3DContext::opStencilMask(int writeMask) {
	stencilTest_writeMask = writeMask;
	dirtyFlags |= kDirtyStencilTest;
}

void N3DContext::opStencilOp(GPU_STENCILOP fail, GPU_STENCILOP zfail, GPU_STENCILOP zpass) {
	stencilOp_fail  = fail;
	stencilOp_zfail = zfail;
	stencilOp_zpass = zpass;
	dirtyFlags |= kDirtyStencilOp;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// depth testing

void N3DContext::opDepthFunc(GPU_TESTFUNC func) {
	depthTest_func = func;
	dirtyFlags |= kDirtyDepthTest;
}

void N3DContext::opDepthMask(bool depthState) {
	depthTest_writeMask = (GPU_WRITEMASK)((depthTest_writeMask & ~GPU_WRITE_DEPTH) | (depthState ? GPU_WRITE_DEPTH : 0));
	dirtyFlags |= kDirtyDepthTest;
}

void N3DContext::opColorMask(bool redState, bool greenState, bool blueState, bool alphaState) {
	depthTest_writeMask = (GPU_WRITEMASK)((depthTest_writeMask & ~GPU_WRITE_COLOR) |
	                                      (redState   ? GPU_WRITE_RED   : 0)       |
	                                      (greenState ? GPU_WRITE_GREEN : 0)       |
	                                      (blueState  ? GPU_WRITE_BLUE  : 0)       |
	                                      (alphaState ? GPU_WRITE_ALPHA : 0));
	dirtyFlags |= kDirtyDepthTest;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// EARLY depth testing

void N3DContext::opEarlyDepthFunc(GPU_EARLYDEPTHFUNC func) {
	earlyDepthTest_func = func;
	dirtyFlags |= kDirtyEarlyDepthTest;
}

void N3DContext::opClearEarlyDepth(u32 clearValue) {
	earlyDepthTest_clear = clearValue;
	dirtyFlags |= kDirtyEarlyDepthTest;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// blending

void N3DContext::opBlendEquation(GPU_BLENDEQUATION equation) {
	blend_colorEq = blend_alphaEq = equation;
	dirtyFlags |= kDirtyBlendLogicOp;
}

void N3DContext::opBlendEquationSeparate(GPU_BLENDEQUATION colorEq, GPU_BLENDEQUATION alphaEq) {
	blend_colorEq = colorEq;
	blend_alphaEq = alphaEq;
	dirtyFlags |= kDirtyBlendLogicOp;
}

void N3DContext::opBlendFunc(GPU_BLENDFACTOR srcFactor, GPU_BLENDFACTOR dstFactor) {
	blend_srcColor = blend_srcAlpha = srcFactor;
	blend_dstColor = blend_dstAlpha = dstFactor;
	dirtyFlags |= kDirtyBlendLogicOp;
}

void N3DContext::opBlendFuncSeparate(GPU_BLENDFACTOR srcColor, GPU_BLENDFACTOR dstColor, GPU_BLENDFACTOR srcAlpha, GPU_BLENDFACTOR dstAlpha) {
	blend_srcColor = srcColor;
	blend_dstColor = dstColor;
	blend_srcAlpha = srcAlpha;
	blend_dstAlpha = dstAlpha;
	dirtyFlags |= kDirtyBlendLogicOp;
}

void N3DContext::opBlendColor(u8 red, u8 green, u8 blue, u8 alpha) {
	blend_color[0] = red;
	blend_color[1] = green;
	blend_color[2] = blue;
	blend_color[3] = alpha;
	dirtyFlags |= kDirtyBlendingColor;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// color logic

void N3DContext::opLogicOp(GPU_LOGICOP op) {
	colorLogicOp_op = op;
	dirtyFlags |= kDirtyBlendLogicOp;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// enables

void N3DContext::opEnable(ENUM3DS_CAP cap) {
	switch (cap) {
	case ENUM3DS_CAP_POLYGON_OFFSET:
		depthMap_enabled = true;
		dirtyFlags |= kDirtyDepthMap;
		break;
	case ENUM3DS_CAP_CULL_FACE:
		cullFace_enabled = true;
		dirtyFlags |= kDirtyCulling;
		break;
	case ENUM3DS_CAP_STENCIL_TEST:
		stencilTest_enabled = true;
		dirtyFlags |= kDirtyStencilTest;
		break;
	case ENUM3DS_CAP_DEPTH_TEST_EARLY:
		earlyDepthTest_enabled = true;
		dirtyFlags |= kDirtyEarlyDepthTest;
		break;
	case ENUM3DS_CAP_DEPTH_TEST:
		depthTest_enabled = true;
		dirtyFlags |= kDirtyDepthTest;
		break;
	case ENUM3DS_CAP_ALPHA_TEST:
		alphaTest_enabled = true;
		dirtyFlags |= kDirtyAlphaTest;
		break;
	case ENUM3DS_CAP_BLEND:
		blend_enabled = true;
		dirtyFlags |= kDirtyBlendLogicOp;
		break;
	case ENUM3DS_CAP_COLOR_LOGIC_OP:
		colorLogicOp_enabled = true;
		dirtyFlags |= kDirtyBlendLogicOp;
		break;
	default:
		error("N3DContext::opEnable: Not a valid parameter.");
	}
}

void N3DContext::opDisable(ENUM3DS_CAP cap) {
	switch (cap) {
	case ENUM3DS_CAP_POLYGON_OFFSET:
		depthMap_enabled = false;
		dirtyFlags |= kDirtyDepthMap;
		break;
	case ENUM3DS_CAP_CULL_FACE:
		cullFace_enabled = false;
		dirtyFlags |= kDirtyCulling;
		break;
	case ENUM3DS_CAP_STENCIL_TEST:
		stencilTest_enabled = false;
		dirtyFlags |= kDirtyStencilTest;
		break;
	case ENUM3DS_CAP_DEPTH_TEST_EARLY:
		earlyDepthTest_enabled = false;
		dirtyFlags |= kDirtyEarlyDepthTest;
		break;
	case ENUM3DS_CAP_DEPTH_TEST:
		depthTest_enabled = false;
		dirtyFlags |= kDirtyDepthTest;
		break;
	case ENUM3DS_CAP_ALPHA_TEST:
		alphaTest_enabled = false;
		dirtyFlags |= kDirtyAlphaTest;
		break;
	case ENUM3DS_CAP_BLEND:
		blend_enabled = false;
		dirtyFlags |= kDirtyBlendLogicOp;
		break;
	case ENUM3DS_CAP_COLOR_LOGIC_OP:
		colorLogicOp_enabled = false;
		dirtyFlags |= kDirtyBlendLogicOp;
		break;
	default:
		error("N3DContext::opDisable: Not a valid parameter.");
	}
}

} // end namespace N3DS_3D
