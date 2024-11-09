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

#define N3DSMACRO_GPUSHADERTYPE_ENUM(num) \
	((num == 0) ? GPU_VERTEX_SHADER : GPU_GEOMETRY_SHADER)
#define N3DSMACRO_IVEC_ID_POS(num, vecIdx) \
	_unif_IVecs[(16 * num) + IV_ID + vecIdx]

#include <3ds.h>
#include <citro3d.h>
#include "graphics/3ds/n3d-context.h"

namespace N3DS_3D {

// new shader object
ShaderObj::ShaderObj(u32 *shbin, u32 shbin_size) {
	_binary = DVLB_ParseFile(shbin, shbin_size);
	_program = new shaderProgram_s();
	shaderProgramInit(_program);
	shaderProgramSetVsh(_program, &_binary->DVLE[0]);
//	_attrInfo = new C3D_AttrInfo();
//	_bufInfo = new C3D_BufInfo();
	AttrInfo_Init(&_attrInfo);
	BufInfo_Init(&_bufInfo);

	_vert_numFVecs = _program->vertexShader->numFloat24Uniforms;
	_geom_numFVecs = _program->geometryShader->numFloat24Uniforms;
	if (_vert_numFVecs > 0) {
		_vert_UniformMap = new UniformsMap();
		_vert_unif_FVecs = new C3D_FVec[_vert_numFVecs];
		memset(_vert_unif_FVecs, 0, _vert_numFVecs * sizeof(C3D_FVec));
		_vert_dirtyFVecs = new FVecQueue();
	}
	if (_geom_numFVecs > 0) {
		_geom_UniformMap = new UniformsMap();
		_geom_unif_FVecs = new C3D_FVec[_geom_numFVecs];
		memset(_geom_unif_FVecs, 0, _geom_numFVecs * sizeof(C3D_FVec));
		_geom_dirtyFVecs = new FVecQueue();
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
ShaderObj::ShaderObj(ShaderObj &orig) : _binary(orig._binary),
                                        _program(orig._program),
//                                        _attrInfo(orig._attrInfo),
//                                        _bufInfo(orig._bufInfo),
//                                        _vert_numFVecs(orig._vert_numFVecs),
//                                        _geom_numFVecs(orig._geom_numFVecs),
                                        _vert_UniformMap(orig._vert_UniformMap),
                                        _vert_unif_FVecs(orig._vert_unif_FVecs),
                                        _vert_dirtyFVecs(orig._vert_dirtyFVecs),
                                        _geom_UniformMap(orig._geom_UniformMap),
                                        _geom_unif_FVecs(orig._geom_unif_FVecs),
                                        _geom_dirtyFVecs(orig._geom_dirtyFVecs),
                                        _unif_IVecs(orig._unif_IVecs),
                                        _unif_bools(orig._unif_bools),
                                        _dirtyIVecs(orig._dirtyIVecs),
                                        _dirtyBools(orig._dirtyBools)
                                        {
	AttrInfo_Init(&_attrInfo);
	BufInfo_Init(&_bufInfo);
	_isClone = true;
}

ShaderObj::~ShaderObj() {
	if (_isClone) {
		_binary = nullptr;
		_program = nullptr;
//		_attrInfo = nullptr;
//		_bufInfo = nullptr;
		_vert_UniformMap = _geom_UniformMap = nullptr;
		_vert_unif_FVecs = _geom_unif_FVecs = nullptr;
		_vert_dirtyFVecs = _geom_dirtyFVecs = nullptr;
		_unif_IVecs = nullptr;
		_unif_bools = nullptr;
		_dirtyIVecs = _dirtyBools = nullptr;
	} else {
//		delete _attrInfo;
//		delete _bufInfo;
//		delete _vert_numFVecs;
//		delete _geom_numFVecs;
		delete _vert_unif_FVecs;
		delete _geom_unif_FVecs;
		delete _vert_dirtyFVecs;
		delete _geom_dirtyFVecs;
		delete _unif_IVecs;
		delete _unif_bools;
		delete _dirtyIVecs;
		delete _dirtyBools;

//		for (UniformsMap::iterator it = _vert_UniformMap->begin(); it != _vert_UniformMap->end(); ++it) {
//			delete it->_value;
//		}
//		delete _vert_UniformMap;
//		for (UniformsMap::iterator it = _geom_UniformMap->begin(); it != _geom_UniformMap->end(); ++it) {
//			delete it->_value;
//		}
//		delete _geom_UniformMap;

		shaderProgramFree(_program);
		delete _program;

		DVLB_Free(_binary);
	}
}




void *ShaderObj::createBuffer(size_t size/*, size_t stride*/, const void *data) {
	void *ptr;

	// if (!stride) {
		ptr = linearAlloc(size);
		if (data == nullptr)
			memset(ptr, 0, size);
		else
			memcpy(ptr, data, size);
	// } else {
	//	ptr = calloc(size, stride);
	//	if (data == nullptr)
	//		memset(ptr, 0, size * stride);
	//	else
	//		memcpy(ptr, data, size * stride);
	//}

	return ptr;
}

void ShaderObj::freeBuffer(void *linearBuffer) {
	linearFree(linearBuffer);
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

void ShaderObj::freeUnnamedBuffer(/*C3D_BufInfo *bufInfo, bool inLinearMem, */int bufIdx) {
	// needed physical address = buffer info base address (0x18000000) + configuration offset
	u32 paddr = _bufInfo.base_paddr + _bufInfo.buffers[bufIdx].offset;
	// get virtual address from physical address
	u32 vaddr = convertPhysToVirt((void *)paddr);

	// if (inLinearMem)
		linearFree((void *)vaddr);
	// else
	//	free((void *)vaddr);
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

void ShaderObj::sendDirtyUniforms() {
	GPU_SHADER_TYPE shaderEnum;

	for (int shaderType = 0; shaderType < 2; shaderType++) {
		shaderEnum = N3DSMACRO_GPUSHADERTYPE_ENUM(shaderType);

		while (!N3DSMACRO_DIRTY_FVECS(shaderEnum)->empty()) {
			dirtyFVec temp = N3DSMACRO_DIRTY_FVECS(shaderEnum)->pop();
			if (temp.dirtyRows <= 4) {
				C3D_FVUnifMtxNx4(shaderEnum, temp.pos, (const C3D_Mtx *)temp.ptr, temp.dirtyRows);
			} else {
				for (int row = 0; row < temp.dirtyRows; row++) {
					C3D_FVUnifSet(shaderEnum, temp.pos, temp.ptr[row].x,
					                                    temp.ptr[row].y,
					                                    temp.ptr[row].z,
					                                    temp.ptr[row].w);
				}
			}
		}

		for (int IV_ID = 0; IV_ID < 4; IV_ID++) {
			if (_dirtyIVecs[(shaderType * 4) + IV_ID] == true) {
				C3D_IVUnifSet(shaderEnum, IV_ID, N3DSMACRO_IVEC_ID_POS(shaderType, 0),
				                                 N3DSMACRO_IVEC_ID_POS(shaderType, 1),
				                                 N3DSMACRO_IVEC_ID_POS(shaderType, 2),
				                                 N3DSMACRO_IVEC_ID_POS(shaderType, 3));
				_dirtyIVecs[(shaderType * 4) + IV_ID] = false;
			}
		}

		for (int bool_ID = 0; bool_ID < 2; bool_ID++) {
			if (_dirtyBools[(shaderType * 2) + bool_ID] == true) {
				C3D_BoolUnifSet(shaderEnum, bool_ID, _unif_bools[(shaderType * 2) + bool_ID]);
				_dirtyBools[(shaderType * 2) + bool_ID] = false;
			}
		}
	}
}

} // end namespace N3DS_3D