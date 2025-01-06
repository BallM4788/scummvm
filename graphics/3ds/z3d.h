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

#ifndef GRAPHICS_3DS_N3DOBJECTS_H
#define GRAPHICS_3DS_N3DOBJECTS_H

#define MAX_TEX_UNITS 3

#include "graphics/3ds/ngl.h"
#include "backends/platform/3ds/osystem.h"
#include "common/system.h"
#include "common/queue.h"
#include "common/hashmap.h"
#include "math/vector4d.h"

namespace N3DS_3D {

struct dirtyFVec {
	C3D_FVec *ptr;
	int pos;
	int dirtyRows;
	dirtyFVec(C3D_FVec *vec, int startPosition, int numRows) : ptr(vec), pos(startPosition), dirtyRows(numRows) {}
};

typedef Common::HashMap<Common::String, int> UniformsMap;
typedef Common::Queue<dirtyFVec> FVecQueue;
class ShaderObj;

struct N3DContext {
	GPU_CULLMODE       cullFace_mode;
	N3D_CULLFACE       cullFace_faceToCull;
	N3D_FRONTFACE      cullFace_frontFace;
	bool               cullFace_enabled;

	float              depthMap_pOffUnits;
	float              depthMap_wScale;
	float              depthMap_rangeN;
	float              depthMap_rangeF;
	float              depthMap_zScale;
	float              depthMap_zOffset;
	bool               depthMap_enabled;

	GPU_SCISSORMODE    scissor_mode;
	u32                scissor_x,
	                   scissor_y,
	                   scissor_w,
	                   scissor_h;

	GPU_TESTFUNC       alphaTest_func;
	int                alphaTest_ref;
	bool               alphaTest_enabled;

	GPU_TESTFUNC       stencilTest_func;
	int                stencilTest_ref;
	int                stencilTest_mask;
	int                stencilTest_writeMask;
	GPU_STENCILOP      stencilOp_fail;
	GPU_STENCILOP      stencilOp_zfail;
	GPU_STENCILOP      stencilOp_zpass;
	bool               stencilTest_enabled;

	GPU_TESTFUNC       depthTest_func;
	GPU_WRITEMASK      depthTest_writeMask;       // INCLUDES COLOR MASK
	bool               depthTest_enabled;

	GPU_EARLYDEPTHFUNC earlyDepthTest_func;
	u32                earlyDepthTest_clear;
	bool               earlyDepthTest_enabled;

	GPU_BLENDEQUATION  blend_colorEq,   blend_alphaEq;
	GPU_BLENDFACTOR    blend_srcColor,  blend_dstColor;
	GPU_BLENDFACTOR    blend_srcAlpha,  blend_dstAlpha;
	u8                 blend_color[4];
	bool               blend_enabled;

	GPU_LOGICOP        colorLogicOp_op;
	bool               colorLogicOp_enabled;

	GPU_FRAGOPMODE     fragOpMode;

	C3D_Tex *boundTexUnits[MAX_TEX_UNITS];

	ShaderObj *activeShaderObj;

private:
	void updateCullMode();
	void updateDepthMap();
	void updateBlend();
public:
	void updateEntireContext();
	void changeShader(ShaderObj *shaderObj);
#include "graphics/3ds/opinfo.h"
	void init(N3DContext *source = nullptr);
	void initOGL();
	void deinit();
};

extern N3DContext *activeContext;
N3DContext *getActiveContext();


#define N3DSMACRO_SHADER_INSTANCE(shaderEnum) \
	((shaderEnum == 0) ? (_program->vertexShader) : (_program->geometryShader))
#define N3DSMACRO_UNIF_MAP(shaderEnum) \
	((shaderEnum == 0) ? _vert_UniformMap : _geom_UniformMap)
#define N3DSMACRO_UNIF_FVECS(shaderEnum) \
	((shaderEnum == 0) ? _vert_unif_FVecs : _geom_unif_FVecs)
#define N3DSMACRO_DIRTY_FVECS(shaderEnum) \
	((shaderEnum == 0) ? _vert_dirtyFVecs : _geom_dirtyFVecs)

enum SHADERINSTANCEFLAG {
	SI_VERTEX = 0x01,
	SI_GEOM   = 0x10
};

class ShaderObj {
public:
	ShaderObj(const u8 *shbin, u32 shbin_size, u8 geomStride = 0);
	// Copy constructor
	ShaderObj(ShaderObj *original);
	// ShaderObj destructor, preserving the vshader and uniform map if this is a clone
	~ShaderObj();


	// matrix uniform / vector uniform (convert from Math::Matrix to Citro3D arrangement)
	template <int R, int C>
	bool setUniform(const Common::String &uniform, GPU_SHADER_TYPE shaderEnum, const Math::Matrix<R,C> &svmMtx) {
//		int columns = C;
		int pos = getUniformLocation(uniform, shaderEnum);
		if (pos != -1) {
			if (C > 1) {
				if (getActiveContext()->activeShaderObj == this) {
					// if this is the active shader object, go ahead and send to Citro3D
					C3D_Mtx m;
					Mtx_Zeros(&m);
					for (int row = 0; row < R; row++) {
						for (int col = 0; col < C; col++) {
							m.r[col].c[3-row] = svmMtx(row, col);
						}
					}
					C3D_FVUnifMtxNx4(shaderEnum, pos, &m, C);
				} else {
					// otherwise, queue this up for the next time this ShaderObj is used
					C3D_FVec *unifPtr = N3DSMACRO_UNIF_FVECS(shaderEnum) + pos;
					for (int row = 0; row < R; row++) {
						for (int col = 0; col < C; col++) {
							unifPtr[col].c[3-row] = svmMtx(row, col);
						}
					}
					N3DSMACRO_DIRTY_FVECS(shaderEnum)->push(dirtyFVec(unifPtr, pos, C));
				}
				return true;
			} else {
				// handle vector uniform values
				float temp[4] = {0.0, 0.0, 0.0, 0.0};
				for (int f = 0; f < R; f++) {
					temp[f] = svmMtx(f, 0);
				}
				return setUniform(uniform, shaderEnum, temp[0], temp[1], temp[2], temp[3]);
			}
		} else {
			return false;
		}
	}

	// matrix uniform (C3D_Mtx)
	bool setUniform(const Common::String &uniform, GPU_SHADER_TYPE shaderEnum, const C3D_Mtx &c3dMtx, int rows = 4) {
		int pos = getUniformLocation(uniform, shaderEnum);
		if (pos != -1) {
			if (getActiveContext()->activeShaderObj == this) {
				// if this is the active shader object, go ahead and send to Citro3D
				C3D_FVUnifMtxNx4(shaderEnum, pos, &c3dMtx, rows);
			} else {
				// otherwise, queue this up for the next time this ShaderObj is used
				C3D_FVec *unifPtr = N3DSMACRO_UNIF_FVECS(shaderEnum) + pos;
				for (int R = 0; R < rows; R++) {
					unifPtr[R] = c3dMtx.r[R];
				}
				N3DSMACRO_DIRTY_FVECS(shaderEnum)->push(dirtyFVec(unifPtr, pos, rows));
			}
			return true;
		} else {
			return false;
		}
	}

	// vector uniform values (float)
	bool setUniform(const Common::String &uniform, GPU_SHADER_TYPE shaderEnum, float x, float y = 0.0f, float z = 0.0f, float w = 0.0f) {
		int pos = getUniformLocation(uniform, shaderEnum);
		if (pos != -1) {
			if (getActiveContext()->activeShaderObj == this) {
				// if this is the active shader object, go ahead and send to Citro3D
				C3D_FVUnifSet(shaderEnum, pos, x, y, z, w);
			} else {
				// otherwise, queue this up for the next time this ShaderObj is used
				C3D_FVec *unifPtr = N3DSMACRO_UNIF_FVECS(shaderEnum) + pos;
				unifPtr->x = x;
				unifPtr->y = y;
				unifPtr->z = z;
				unifPtr->w = w;
				N3DSMACRO_DIRTY_FVECS(shaderEnum)->push(dirtyFVec(unifPtr, pos, 1));
			}
			return true;
		} else {
			return false;
		}
	}

	// vector uniform (C3D_FVec)
	bool setUniform(const Common::String &uniform, GPU_SHADER_TYPE shaderEnum, const C3D_FVec &c3dfvec) {
		return setUniform(uniform, shaderEnum, c3dfvec.x, c3dfvec.y, c3dfvec.z, c3dfvec.w);
	}

	// uniform vector arrays (floats)
	bool setUniformNfv(const Common::String &uniform, GPU_SHADER_TYPE shaderEnum, float *dataPtr, int vecCount, int floatsPerVec) {
		int pos = getUniformLocation(uniform, shaderEnum);
		if (pos != -1) {
			if (getActiveContext()->activeShaderObj == this) {
				// if this is the active shader object, go ahead and send to Citro3D
				for (int v = 0; v < vecCount; v++) {
					float temp[4] = {0.0, 0.0, 0.0, 0.0};
					for (int f = 0; f < floatsPerVec; f++) {
						temp[f] = dataPtr[(v*floatsPerVec+f)];
					}
					C3D_FVUnifSet(shaderEnum, pos + v, temp[0], temp[1], temp[2], temp[3]);
				}
			} else {
				// otherwise, queue this up for the next time this ShaderObj is used
				C3D_FVec *unifPtr = N3DSMACRO_UNIF_FVECS(shaderEnum) + pos;
				for (int v = 0; v < vecCount; v++) {
					for (int f = 0; f < floatsPerVec; f++) {
						unifPtr[v].c[3-f] = dataPtr[(v*floatsPerVec+f)];
					}
				}
				N3DSMACRO_DIRTY_FVECS(shaderEnum)->push(dirtyFVec(unifPtr, pos, vecCount));
			}
			return true;
		} else {
			return false;
		}
	}
	bool setUniform4fv(const Common::String &uniform, GPU_SHADER_TYPE shaderEnum, float *dataPtr, int vecCount) {
		return setUniformNfv(uniform, shaderEnum, dataPtr, vecCount, 4);
	}
	bool setUniform3fv(const Common::String &uniform, GPU_SHADER_TYPE shaderEnum, float *dataPtr, int vecCount) {
		return setUniformNfv(uniform, shaderEnum, dataPtr, vecCount, 3);
	}
	bool setUniform2fv(const Common::String &uniform, GPU_SHADER_TYPE shaderEnum, float *dataPtr, int vecCount) {
		return setUniformNfv(uniform, shaderEnum, dataPtr, vecCount, 2);
	}
	bool setUniform1fv(const Common::String &uniform, GPU_SHADER_TYPE shaderEnum, float *dataPtr, int vecCount) {
		return setUniformNfv(uniform, shaderEnum, dataPtr, vecCount, 1);
	}

	// vector uniform values (int)
	bool setUniform(const Common::String &uniform, GPU_SHADER_TYPE shaderEnum, int x, int y = 0, int z = 0, int w = 0) {
		int pos = getUniformLocation(uniform, shaderEnum);
		if (pos != -1) {
			if (getActiveContext()->activeShaderObj == this) {
				// if this is the active shader object, go ahead and send to Citro3D
				C3D_IVUnifSet(shaderEnum, pos, x, y, z, w);
			} else {
				// otherwise, save this for the next time this ShaderObj is used
				int *unifPtr = _unif_IVecs + (pos * 4);
				unifPtr[0] = x;
				unifPtr[1] = y;
				unifPtr[2] = z;
				unifPtr[3] = w;
				_dirtyIVecs[pos] = true;
			}
			return true;
		} else {
			return false;
		}
	}

	// bool uniform
	bool setUniform(const Common::String &uniform, GPU_SHADER_TYPE shaderEnum, bool boolval) {
		int pos = getUniformLocation(uniform, shaderEnum);
		if (pos != -1) {
			if (getActiveContext()->activeShaderObj == this) {
				// if this is the active shader object, go ahead and send to Citro3D
				C3D_BoolUnifSet(shaderEnum, pos, boolval);
			} else {
				// otherwise, save this for the next time this ShaderObj is used
				_unif_bools[2 * shaderEnum + pos] = boolval;
				_dirtyBools[2 * shaderEnum + pos] = true;
			}
			return true;
		} else {
			return false;
		}
	}

	// get location of uniform
	int getUniformLocation(const Common::String &uniform, GPU_SHADER_TYPE shaderEnum) const {
		UniformsMap::iterator kv = N3DSMACRO_UNIF_MAP(shaderEnum)->find(uniform);
		if (kv == N3DSMACRO_UNIF_MAP(shaderEnum)->end()) {
			int ret = shaderInstanceGetUniformLocation(N3DSMACRO_SHADER_INSTANCE(shaderEnum), uniform.c_str());
			N3DSMACRO_UNIF_MAP(shaderEnum)->setVal(uniform, ret);
			return ret;
		} else {
			return kv->_value;
		}
	}

	void addAttrLoader(int regId, GPU_FORMATS format, int count) {
		AttrInfo_AddLoader(&_attrInfo, regId, format, count);
	}

	void addBufInfo(const void* data, ptrdiff_t stride, int attribCount, u64 permutation) {
		BufInfo_Add(&_bufInfo, data, stride, attribCount, permutation);
	}

	// destroy a buffer whose address is stored in one of the shader's stored buffer configurations (usually the first config)
	void freeAttachedBuffer(/*C3D_BufInfo *bufInfo, bool inLinearMem, */int bufIdx = 0);

	// add buffer info to the shader struct, or modify it if it already exists
	int BufInfo_AddOrModify(const void* data, ptrdiff_t stride, int attribCount, u64 permutation, int id = 0);

	// Send queued uniforms to Citro3D. Only does something when the shader is the active shader.
	void sendDirtyUniforms();


	// vars
	DVLB_s             *_binary;
	shaderProgram_s    *_program;
	u8                  _si_flags;
	C3D_AttrInfo        _attrInfo;
	C3D_BufInfo         _bufInfo;

private:
	int                 _vert_numFVecs,    _geom_numFVecs;
	UniformsMap        *_vert_UniformMap, *_geom_UniformMap;
	C3D_FVec           *_vert_unif_FVecs, *_geom_unif_FVecs;
	FVecQueue          *_vert_dirtyFVecs, *_geom_dirtyFVecs;
	int                *_unif_IVecs;
	bool               *_unif_bools;
	bool               *_dirtyIVecs;
	bool               *_dirtyBools;
	bool                _isClone;

};

} // end of namespace N3DS_3D

#endif // GRAPHICS_3DS_N3DCONTEXT_H