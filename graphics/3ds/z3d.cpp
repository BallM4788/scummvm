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

void N3DContext::applyContextState(bool forceApply) {
//	if (activeShaderObj != nullptr) {
//		C3D_BindProgram(activeShaderObj->_program);
//		C3D_SetAttrInfo(&activeShaderObj->_attrInfo);
//		C3D_SetBufInfo(&activeShaderObj->_bufInfo);
//		activeShaderObj->sendDirtyUniforms();
//	}

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

void N3DContext::changeShader(ShaderObj *shaderObj) {
//	if (activeShaderObj == shaderObj)
//		return;

	activeShaderObj = shaderObj;
	C3D_BindProgram(activeShaderObj->_program);
	C3D_SetAttrInfo(&activeShaderObj->_attrInfo);
	C3D_SetBufInfo(&activeShaderObj->_bufInfo);
	activeShaderObj->sendDirtyUniforms();
}

ShaderObj::ShaderObj(shaderProgram_s *program, u8 flags) {
	_program = program;
	AttrInfo_Init(&_attrInfo);
	BufInfo_Init(&_bufInfo);

	_vert_numFVecs = _program->vertexShader->numFloat24Uniforms;
	if (_vert_numFVecs > 0) {
		_vert_UniformMap = new UniformsMap();
		_vert_unif_FVecs = new C3D_FVec[_vert_numFVecs];
		memset(_vert_unif_FVecs, 0, _vert_numFVecs * sizeof(C3D_FVec));
		_vert_dirtyFVecs = new FVecQueue();
	}
	if (_program->geometryShader) {
		_geom_numFVecs = _program->geometryShader->numFloat24Uniforms;
		if (_geom_numFVecs > 0) {
			_geom_UniformMap = new UniformsMap();
			_geom_unif_FVecs = new C3D_FVec[_geom_numFVecs];
			memset(_geom_unif_FVecs, 0, _geom_numFVecs * sizeof(C3D_FVec));
			_geom_dirtyFVecs = new FVecQueue();
		}
	}

	_unif_IVecs = new int[32];							// 8 integer vecs (4 vecs per shader instance), 4 ints per vec
	_unif_bools = new bool[4];							// 4 bool values (2 bools per shader instance)
	memset(_unif_IVecs, 0, 8 * sizeof(C3D_IVec));
	memset(_unif_bools, 0, 4 * sizeof(bool));
	_dirtyIVecs = new bool[8];							// 8 bools (4 vecs per shader instance, 1 bool per vec)
	_dirtyBools = new bool[4];							// 4 bools (2 bools per shader instance, 1 bool per bool)
	memset(_dirtyIVecs, 0, 8 * sizeof(bool));
	memset(_dirtyBools, 0, 4 * sizeof(bool));
	_isClone = false;

}

// cloned shader object
ShaderObj::ShaderObj(ShaderObj *original) : _program(original->_program),
                                            _vert_UniformMap(original->_vert_UniformMap),
                                            _vert_unif_FVecs(original->_vert_unif_FVecs),
                                            _vert_dirtyFVecs(original->_vert_dirtyFVecs),
                                            _geom_UniformMap(original->_geom_UniformMap),
                                            _geom_unif_FVecs(original->_geom_unif_FVecs),
                                            _geom_dirtyFVecs(original->_geom_dirtyFVecs),
                                            _unif_IVecs(original->_unif_IVecs),
                                            _unif_bools(original->_unif_bools),
                                            _dirtyIVecs(original->_dirtyIVecs),
                                            _dirtyBools(original->_dirtyBools)
                                            {
	AttrInfo_Init(&_attrInfo);
	BufInfo_Init(&_bufInfo);
	_isClone = true;
}

ShaderObj::~ShaderObj() {
	if (_isClone) {
		_program = nullptr;
		_vert_UniformMap = _geom_UniformMap = nullptr;
		_vert_unif_FVecs = _geom_unif_FVecs = nullptr;
		_vert_dirtyFVecs = _geom_dirtyFVecs = nullptr;
		_unif_IVecs = nullptr;
		_unif_bools = nullptr;
		_dirtyIVecs = _dirtyBools = nullptr;
	} else {
		delete [] _vert_unif_FVecs; // C3D_FVec array on heap
		delete _vert_dirtyFVecs; // FVecQueue (Common::Queue<dirtyFVec>: "common/queue.h")
		delete _vert_UniformMap;
		if (_program->geometryShader) {
			delete [] _geom_unif_FVecs; // C3D_FVec array on heap
			delete _geom_dirtyFVecs; // FVecQueue (Common::Queue<dirtyFVec>: "common/queue.h")
			delete _geom_UniformMap;
		}
		_program = nullptr;
		delete [] _unif_IVecs; // int array on heap
		delete [] _unif_bools; // bool array on heap
		delete [] _dirtyIVecs; // bool array on heap
		delete [] _dirtyBools; // bool array on heap
	}
}

static u32 convertPhysToVirt(const void *addr) {
	u32 paddr = (u32)addr;
	u32 i;
#define REVERT_REGION(_name) \
	i = paddr - (OS_##_name##_PADDR - OS_##_name##_VADDR); \
	if (i >= OS_##_name##_VADDR && i < (OS_##_name##_VADDR + OS_##_name##_SIZE)) \
		return i;

	REVERT_REGION(FCRAM);
	REVERT_REGION(VRAM);
	REVERT_REGION(OLD_FCRAM);
	REVERT_REGION(DSPRAM);
	REVERT_REGION(QTMRAM);
	REVERT_REGION(MMIO);

#undef REVERT_REGION
	return 0;
}

void ShaderObj::freeAttachedBuffer(/*C3D_BufInfo *bufInfo, bool inLinearMem, */int bufIdx) {
	// needed physical address = buffer info base address (0x18000000) + configuration offset
	u32 paddr = _bufInfo.base_paddr + _bufInfo.buffers[bufIdx].offset;
	// get virtual address from physical address
	u32 vaddr = convertPhysToVirt((void *)paddr);

	linearFree((void *)vaddr);
}

int ShaderObj::BufInfo_AddOrModify(const void* data, ptrdiff_t stride, int attribCount, u64 permutation, int id) {
	if (_bufInfo.bufCount < (id + 1)) {
		BufInfo_Add(&_bufInfo, data, stride, attribCount, permutation);
		return 0;
	} else {
		u32 pa = osConvertVirtToPhys(data);
		if (pa < _bufInfo.base_paddr) return -2;

		C3D_BufCfg* buf = &_bufInfo.buffers[id];
		buf->offset = pa - _bufInfo.base_paddr;
		buf->flags[0] = permutation & 0xFFFFFFFF;
		buf->flags[1] = (permutation >> 32) | (stride << 16) | (attribCount << 28);
		return id;
	}
}

#define N3DSMACRO_IVEC_ID_POS(num, vecIdx) \
	_unif_IVecs[(16 * num) + IV_ID + vecIdx]

void ShaderObj::sendDirtyUniforms() {
	bool hasGeomShader = _program->geometryShader ? true : false;

	for (uint shaderType = 0; shaderType < 2; shaderType++) {
		// 0 = GPU_VERTEX_SHADER
		// 1 = GPU_GEOMETRY_SHADER

		if ((shaderType == 1) & !hasGeomShader) {
			continue;
		}
		if (N3DSMACRO_DIRTY_FVECS(shaderType)->empty())
			continue;

		dirtyFVec temp;
		while (!N3DSMACRO_DIRTY_FVECS(shaderType)->empty()) {
			temp = N3DSMACRO_DIRTY_FVECS(shaderType)->pop();
			C3D_FVUnifMtxNx4((GPU_SHADER_TYPE)shaderType, temp.pos, (const C3D_Mtx *)temp.ptr, temp.dirtyRows);
		}

		for (int IV_ID = 0; IV_ID < 4; IV_ID++) {
			if (_dirtyIVecs[(shaderType * 4) + IV_ID] == true) {
				C3D_IVUnifSet((GPU_SHADER_TYPE)shaderType, IV_ID, N3DSMACRO_IVEC_ID_POS(shaderType, 0),
				                                                  N3DSMACRO_IVEC_ID_POS(shaderType, 1),
				                                                  N3DSMACRO_IVEC_ID_POS(shaderType, 2),
				                                                  N3DSMACRO_IVEC_ID_POS(shaderType, 3));
				_dirtyIVecs[(shaderType * 4) + IV_ID] = false;
			}
		}

		for (int bool_ID = 0; bool_ID < 2; bool_ID++) {
			if (_dirtyBools[(shaderType * 2) + bool_ID] == true) {
				C3D_BoolUnifSet((GPU_SHADER_TYPE)shaderType, bool_ID, _unif_bools[(shaderType * 2) + bool_ID]);
				_dirtyBools[(shaderType * 2) + bool_ID] = false;
			}
		}
	}
}

} // end namespace N3DS_3D
