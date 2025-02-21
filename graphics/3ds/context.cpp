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

void N3DContext::init(N3DContext *source) {
	if (source != nullptr) {
		dirtyFlags              = source->dirtyFlags;
		cullFace_mode           = source->cullFace_mode;
		cullFace_faceToCull     = source->cullFace_faceToCull;
		cullFace_frontFace      = source->cullFace_frontFace;
		cullFace_enabled        = source->cullFace_enabled;
		depthMap_pOffUnits      = source->depthMap_pOffUnits;
		depthMap_rangeN         = source->depthMap_rangeN;
		depthMap_rangeF         = source->depthMap_rangeF;
		depthMap_zScale         = source->depthMap_zScale;
		depthMap_zOffset        = source->depthMap_zOffset;
		depthMap_enabled        = source->depthMap_enabled;
		scissor_mode            = source->scissor_mode;
		scissor_x               = source->scissor_x;
		scissor_y               = source->scissor_y;
		scissor_w               = source->scissor_w;
		scissor_h               = source->scissor_h;
		alphaTest_func          = source->alphaTest_func;
		alphaTest_ref           = source->alphaTest_ref;
		alphaTest_enabled       = source->alphaTest_enabled;
		stencilTest_func        = source->stencilTest_func;
		stencilTest_ref         = source->stencilTest_ref;
		stencilTest_mask        = source->stencilTest_mask;
		stencilTest_writeMask   = source->stencilTest_writeMask;
		stencilOp_fail          = source->stencilOp_fail;
		stencilOp_zfail         = source->stencilOp_zfail;
		stencilOp_zpass         = source->stencilOp_zpass;
		stencilTest_enabled     = source->stencilTest_enabled;
		depthTest_func          = source->depthTest_func;
		depthTest_writeMask     = source->depthTest_writeMask;
		depthTest_enabled       = source->depthTest_enabled;
		earlyDepthTest_func     = source->earlyDepthTest_func;
		earlyDepthTest_clear    = source->earlyDepthTest_clear;
		earlyDepthTest_enabled  = source->earlyDepthTest_enabled;
		blend_color[0]          = source->blend_color[0];
		blend_color[1]          = source->blend_color[1];
		blend_color[2]          = source->blend_color[2];
		blend_color[3]          = source->blend_color[3];
		blend_colorEq           = source->blend_colorEq;
		blend_alphaEq           = source->blend_alphaEq;
		blend_srcColor          = source->blend_srcColor;
		blend_dstColor          = source->blend_dstColor;
		blend_srcAlpha          = source->blend_srcAlpha;
		blend_dstAlpha          = source->blend_dstAlpha;
		blend_enabled           = source->blend_enabled;
		colorLogicOp_op         = source->colorLogicOp_op;
		colorLogicOp_enabled    = source->colorLogicOp_enabled;
		fragOpMode              = source->fragOpMode;
		boundTexUnits[0]        = source->boundTexUnits[0];
		boundTexUnits[1]        = source->boundTexUnits[1];
		boundTexUnits[2]        = source->boundTexUnits[2];
	} else {
		// Citro3D starting values?
		dirtyFlags              = kDirtyAll;
		cullFace_mode           = GPU_CULL_BACK_CCW;         // C3D default
		cullFace_faceToCull     = N3D_CULLFACE_BACK;         // C3D default
		cullFace_frontFace      = N3D_FRONTFACE_CCW;         // C3D default
		cullFace_enabled        = false;                     // ScummVM backend
		depthMap_pOffUnits      = 0;                         // deduced
		depthMap_rangeN         = 0.0f;                      // deduced
		depthMap_rangeF         = 1.0f;                      // deduced
		depthMap_zScale         = -1.0f;                     // C3D default
		depthMap_zOffset        = 0.0f;                      // C3D default
		depthMap_enabled        = true;                      // C3D default
		scissor_mode            = GPU_SCISSOR_DISABLE;       // C3D library context sets this whenever C3D_FrameDrawOn is called
		scissor_x               = 0;                         // deduced
		scissor_y               = 0;                         // deduced
		scissor_w               = 0;                         // deduced
		scissor_h               = 0;                         // deduced
		alphaTest_func          = GPU_ALWAYS;                // C3D default
		alphaTest_ref           = 0x00;                      // C3D default
		alphaTest_enabled       = false;                     // C3D default
		stencilTest_func        = GPU_ALWAYS;                // C3D default
		stencilTest_ref         = 0x00;                      // C3D default
		stencilTest_mask        = 0xFF;                      // C3D default
		stencilTest_writeMask   = 0x00;                      // C3D default
		stencilOp_fail          = GPU_STENCIL_KEEP;          // C3D default
		stencilOp_zfail         = GPU_STENCIL_KEEP;          // C3D default
		stencilOp_zpass         = GPU_STENCIL_KEEP;          // C3D default
		stencilTest_enabled     = false;                     // C3D default
		depthTest_func          = GPU_GEQUAL;                // ScummVM backend
		depthTest_writeMask     = GPU_WRITE_ALL;             // C3D default
		depthTest_enabled       = false;                     // ScummVM backend
		earlyDepthTest_func     = GPU_EARLYDEPTH_GREATER;    // C3D default
		earlyDepthTest_clear    = 0;                         // C3D default
		earlyDepthTest_enabled  = false;                     // C3D default
		blend_color[0]          = 0;                         // C3D default
		blend_color[1]          = 0;                         // C3D default
		blend_color[2]          = 0;                         // C3D default
		blend_color[3]          = 0;                         // C3D default
		blend_colorEq           = GPU_BLEND_ADD;             // C3D default
		blend_alphaEq           = GPU_BLEND_ADD;             // C3D default
		blend_srcColor          = GPU_SRC_ALPHA;             // C3D default
		blend_dstColor          = GPU_ONE_MINUS_SRC_ALPHA;   // C3D default
		blend_srcAlpha          = GPU_SRC_ALPHA;             // C3D default
		blend_dstAlpha          = GPU_ONE_MINUS_SRC_ALPHA;   // C3D default
		blend_enabled           = true;                      // C3D default
		colorLogicOp_op         = GPU_LOGICOP_COPY;          // assumed
		colorLogicOp_enabled    = false;                     // C3D default
		fragOpMode              = GPU_FRAGOPMODE_GL;         // C3D default
		boundTexUnits[0]        = NULL;
		boundTexUnits[1]        = NULL;
		boundTexUnits[2]        = NULL;
	}
}

void N3DContext::initOGL() {
	dirtyFlags              = kDirtyAll;
	cullFace_mode           = GPU_CULL_BACK_CCW;
	cullFace_faceToCull     = N3D_CULLFACE_BACK;
	cullFace_frontFace      = N3D_FRONTFACE_CCW;
	cullFace_enabled        = false;
	depthMap_pOffUnits      = 0;
	depthMap_rangeN         = 1.0f;                          // N and F flipped per picaGL
	depthMap_rangeF         = 0.0f;                          // N and F flipped per picaGL
	depthMap_zScale         = 1.0f;                          // deduced
	depthMap_zOffset        = 1.0f;                          // deduced
	depthMap_enabled        = true;
	scissor_mode            = GPU_SCISSOR_DISABLE;
	scissor_x               = 0;
	scissor_y               = 0;
	scissor_w               = 0;
	scissor_h               = 0;
	alphaTest_func          = GPU_ALWAYS;
	alphaTest_ref           = 0;
	alphaTest_enabled       = false;
	stencilTest_func        = GPU_ALWAYS;
	stencilTest_ref         = 0x00;
	stencilTest_mask        = 0xFF;
	stencilTest_writeMask   = 0xFF;
	stencilOp_fail          = GPU_STENCIL_KEEP;
	stencilOp_zfail         = GPU_STENCIL_KEEP;
	stencilOp_zpass         = GPU_STENCIL_KEEP;
	stencilTest_enabled     = false;
	depthTest_func          = GPU_LESS;
	depthTest_writeMask     = GPU_WRITE_ALL;
	depthTest_enabled       = false;
	earlyDepthTest_func     = GPU_EARLYDEPTH_LESS;
	earlyDepthTest_clear    = 0X00FFFFFF;
	earlyDepthTest_enabled  = false;
	blend_color[0]          = 0;
	blend_color[1]          = 0;
	blend_color[2]          = 0;
	blend_color[3]          = 0;
	blend_colorEq           = GPU_BLEND_ADD;
	blend_alphaEq           = GPU_BLEND_ADD;
	blend_srcColor          = GPU_ONE;
	blend_dstColor          = GPU_ZERO;
	blend_srcAlpha          = GPU_ONE;
	blend_dstAlpha          = GPU_ZERO;
	blend_enabled           = false;
	colorLogicOp_op         = GPU_LOGICOP_COPY;
	colorLogicOp_enabled    = false;
	fragOpMode              = GPU_FRAGOPMODE_GL;
	boundTexUnits[0]        = NULL;
	boundTexUnits[1]        = NULL;
	boundTexUnits[2]        = NULL;
}

void N3DContext::deinit() {
	boundTexUnits[0] = NULL;
	boundTexUnits[1] = NULL;
	boundTexUnits[2] = NULL;
}

void N3DContext::applyContextState(bool forceApply) {

	// <----------------------------------------------------------------------------------------------------------------------TEXENV?
	if ((dirtyFlags & kDirtyBlendLogicOp) || forceApply) {
		if (colorLogicOp_enabled == true)
			C3D_ColorLogicOp(colorLogicOp_op);
		else {
			(blend_enabled)
				? C3D_AlphaBlend(blend_colorEq, blend_alphaEq, blend_srcColor, blend_dstColor, blend_srcAlpha, blend_dstAlpha)
				: C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_ONE, GPU_ZERO,GPU_ONE, GPU_ZERO);
			if (fragOpMode != GPU_FRAGOPMODE_GL)
				C3D_FragOpMode(fragOpMode);
		}
	}
	if ((dirtyFlags & kDirtyCulling) || forceApply) {
		GPU_CULLMODE mode;
		if (cullFace_enabled == true)
			mode = ((cullFace_faceToCull == N3D_CULLFACE_FRONT) ^ (cullFace_frontFace == N3D_FRONTFACE_CCW))
			     ? GPU_CULL_BACK_CCW : GPU_CULL_FRONT_CCW;
		else
			mode = GPU_CULL_NONE;

		cullFace_mode = mode;
		C3D_CullFace(mode);
	}
	if ((dirtyFlags & kDirtyDepthMap) || forceApply) {
		float scale = depthMap_rangeN - depthMap_rangeF;
		float offset = depthMap_rangeN;

		if ((depthMap_enabled == true) && (depthMap_pOffUnits != 0.f))
			offset += depthMap_pOffUnits / 16777215.f;

		depthMap_zScale = scale;
		depthMap_zOffset = offset;
		C3D_DepthMap(depthMap_enabled, depthMap_zScale, depthMap_zOffset);
	}
	if ((dirtyFlags & kDirtyScissor) || forceApply)
		C3D_SetScissor(scissor_mode, scissor_x, scissor_y, scissor_w, scissor_h);
	if ((dirtyFlags & kDirtyAlphaTest) || forceApply)
		C3D_AlphaTest(alphaTest_enabled, alphaTest_func, alphaTest_ref);
	if ((dirtyFlags & kDirtyStencilTest) || forceApply)
		C3D_StencilTest(stencilTest_enabled, stencilTest_func, stencilTest_ref, stencilTest_mask, stencilTest_writeMask);
	if ((dirtyFlags & kDirtyStencilOp) || forceApply)
		C3D_StencilOp(stencilOp_fail, stencilOp_zfail, stencilOp_zpass);
	if ((dirtyFlags & kDirtyDepthTest) || forceApply)
		C3D_DepthTest(depthTest_enabled, depthTest_func,
		              depthTest_enabled ? depthTest_writeMask : (GPU_WRITEMASK)(depthTest_writeMask & ~GPU_WRITE_DEPTH));
	if ((dirtyFlags & kDirtyEarlyDepthTest) || forceApply)
		C3D_EarlyDepthTest(earlyDepthTest_enabled, earlyDepthTest_func, earlyDepthTest_clear);
	if ((dirtyFlags & kDirtyBlendingColor) || forceApply)
		C3D_BlendingColor((blend_color[3] << 24) | (blend_color[2] << 16) | (blend_color[1] << 8) | blend_color[0]);
//	if ((dirtyFlags & kDirtyTexBind) || forceApply)
		C3D_TexBind(0, boundTexUnits[0]);

	dirtyFlags = kDirtyNone;
}

} // end namespace N3DS_3D
