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

/*
 * This file is based on, or a modified version of code from TinyGL (C) 1997-2022 Fabrice Bellard,
 * which is licensed under the MIT license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */

//#include <3ds.h>
//#include <citro3d.h>
//#include <tex3ds.h>
#include "graphics/3ds/n3d.h"
#include "graphics/3ds/z3d.h"


#include "common/singleton.h"
#include "common/array.h"

namespace N3DS_3D {

class Native3D : public Common::Singleton<Native3D> {
private:
	Common::Array<N3DContext *> _N3DContextArray;

public:
	N3DContext *createContext() {
		N3DContext *ctx = new N3DContext;
		_N3DContextArray.push_back(ctx);
		return ctx;
	}

	void destroyContext(ContextHandle *handle) {
		for (Common::Array<N3DContext *>::iterator it = _N3DContextArray.begin(); it != _N3DContextArray.end(); it++) {
			if (*it == (N3DContext *)handle) {
				(*it)->deinit();
				delete *it;
				_N3DContextArray.erase(it);
				break;
			}
		}
	}

	void destroyContexts() {
		for (Common::Array<N3DContext *>::iterator it = _N3DContextArray.begin(); it != _N3DContextArray.end(); it++) {
			if (*it != nullptr) {
				(*it)->deinit();
				delete *it;
			}
		}
		_N3DContextArray.clear();
	}

	bool existsContexts() {
		return _N3DContextArray.size() != 0;
	}

	N3DContext *getContext(ContextHandle *handle) {
		for (Common::Array<N3DContext *>::iterator it = _N3DContextArray.begin(); it != _N3DContextArray.end(); it++) {
			if ((*it) == (N3DContext *)handle) {
				return *it;
			}
		}
		return nullptr;
	}
};

} // end of namespace N3DS_3D

namespace Common {
	DECLARE_SINGLETON(N3DS_3D::Native3D);
}


namespace N3DS_3D {

N3DContext *activeContext;

N3DContext *getActiveContext() {
	assert(activeContext);
	return activeContext;
}



ContextHandle *createContext() {
	activeContext = Native3D::instance().createContext();
	activeContext->init();
	return (ContextHandle *)activeContext;
}

ContextHandle *createContext(ContextHandle *source) {
	activeContext = Native3D::instance().createContext();
	activeContext->init((N3DContext *)source);
	return (ContextHandle *)activeContext;
}

ContextHandle *createOGLContext() {
	activeContext = Native3D::instance().createContext();
	activeContext->initOGL();
	return (ContextHandle *)activeContext;
}

void destroyNative3D() {
	Native3D::instance().destroyContexts();
	Native3D::destroy();
	activeContext = nullptr;
}


void destroyContext(ContextHandle *handle) {
	Native3D::instance().destroyContext(handle);
	if ((N3DContext *)handle == activeContext)
		activeContext = nullptr;
	if (!Native3D::instance().existsContexts())
		Native3D::destroy();
}

void setContext(ContextHandle *handle) {
	N3DContext *ctx = Native3D::instance().getContext(handle);
	if (ctx == nullptr) {
		error("N3DS_3D: Context not found");
	}
	activeContext = ctx;
	activeContext->updateEntireContext();
}

N3DContext *getContext(ContextHandle *handle) {
	N3DContext *ctx = Native3D::instance().getContext(handle);
	if (ctx == nullptr) {
		error("N3DS_3D: Context not found");
	}
	return ctx;
}

void N3DContext::init(N3DContext *source) {
	if (source != nullptr) {
		vport_x                 = source->vport_x;
		vport_y                 = source->vport_y;
		vport_w                 = source->vport_w;
		vport_h                 = source->vport_h;
		cullFace_mode           = source->cullFace_mode;
		cullFace_faceToCull     = source->cullFace_faceToCull;
		cullFace_frontFace      = source->cullFace_frontFace;
		cullFace_enabled        = source->cullFace_enabled;
		depthMap_pOffUnits      = source->depthMap_pOffUnits;
		depthMap_wScale         = source->depthMap_wScale;
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
	} else {
		// Citro3D starting values?
		vport_x                 = 0;
		vport_y                 = 0;
		vport_w                 = 0;
		vport_h                 = 0;
		cullFace_mode           = GPU_CULL_BACK_CCW;
		cullFace_faceToCull     = N3D_CULLFACE_BACK;
		cullFace_frontFace      = N3D_FRONTFACE_CCW;
		cullFace_enabled        = false;
		depthMap_pOffUnits      = 0;
		depthMap_wScale         = 0.0f;
		depthMap_rangeN         = 0.0f;
		depthMap_rangeF         = 1.0f;
		depthMap_zScale         = -1.0f;
		depthMap_zOffset        = 0.0f;
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
		stencilTest_writeMask   = 0x00;
		stencilOp_fail          = GPU_STENCIL_KEEP;
		stencilOp_zfail         = GPU_STENCIL_KEEP;
		stencilOp_zpass         = GPU_STENCIL_KEEP;
		stencilTest_enabled     = false;
		depthTest_func          = GPU_GREATER;
		depthTest_writeMask     = GPU_WRITE_ALL;
		depthTest_enabled       = true;
		earlyDepthTest_func     = GPU_EARLYDEPTH_GREATER;
		earlyDepthTest_clear    = 0;
		earlyDepthTest_enabled  = false;
		blend_color[0]          = 0;
		blend_color[1]          = 0;
		blend_color[2]          = 0;
		blend_color[3]          = 0;
		blend_colorEq           = GPU_BLEND_ADD;
		blend_alphaEq           = GPU_BLEND_ADD;
		blend_srcColor          = GPU_SRC_ALPHA;
		blend_dstColor          = GPU_ONE_MINUS_SRC_ALPHA;
		blend_srcAlpha          = GPU_SRC_ALPHA;
		blend_dstAlpha          = GPU_ONE_MINUS_SRC_ALPHA;
		blend_enabled           = true;
		colorLogicOp_op         = GPU_LOGICOP_COPY;
		colorLogicOp_enabled    = false;
		fragOpMode              = GPU_FRAGOPMODE_GL;
		boundTexUnits[0]        = nullptr;
		boundTexUnits[1]        = nullptr;
		boundTexUnits[2]        = nullptr;
		activeShaderObj         = nullptr;
	}
}

void N3DContext::initOGL() {
	vport_x                 = 0;
	vport_y                 = 0;
	vport_w                 = 0;
	vport_h                 = 0;
	cullFace_mode           = GPU_CULL_BACK_CCW;
	cullFace_faceToCull     = N3D_CULLFACE_BACK;
	cullFace_frontFace      = N3D_FRONTFACE_CCW;
	cullFace_enabled        = false;
	depthMap_pOffUnits      = 0;
	depthMap_wScale         = 0.0f;
	depthMap_rangeN         = 0.0f;
	depthMap_rangeF         = 1.0f;
	depthMap_zScale         = -1.0f;
	depthMap_zOffset        = 0.0f;
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
	blend_enabled           = true;
	colorLogicOp_op         = GPU_LOGICOP_COPY;
	colorLogicOp_enabled    = false;
	fragOpMode              = GPU_FRAGOPMODE_GL;
}

void N3DContext::deinit() {
	boundTexUnits[0] = nullptr;
	boundTexUnits[1] = nullptr;
	boundTexUnits[2] = nullptr;
//	boundTexUnits[3] = nullptr;
}

} // end of namespace N3DS_3D
