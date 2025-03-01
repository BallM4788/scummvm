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

enum RenderContextFlags : u16 {
	kDirtyNone           = 0x0000, // It's just 0
	kDirtyDepthMap       = 0x0001, // 0b000000000001 = 0001
	kDirtyCulling        = 0x0002, // 0b000000000010 = 0002
	kDirtyStencilTest    = 0x0004, // 0b000000000100 = 0004
	kDirtyStencilOp      = 0x0008, // 0b000000001000 = 0008
	kDirtyBlendingColor  = 0x0010, // 0b000000010000 = 0016
	kDirtyEarlyDepthTest = 0x0020, // 0b000000100000 = 0032
	kDirtyDepthTest      = 0x0040, // 0b000001000000 = 0064
	kDirtyAlphaTest      = 0x0080, // 0b000010000000 = 0128
	kDirtyBlendLogicOp   = 0x0100, // 0b000100000000 = 0256
	kDirtyFragOp         = 0x0200, // 0b001000000000 = 0512
	kDirtyScissor        = 0x0400, // 0b010000000000 = 1024
	kDirtyTexBind        = 0x0800, // 0b100000000000 = 2048
	kDirtyAll            = 0x0FFF  // 0b111111111111 = 4095 (4096 - 1)
};

//class ShaderObj;

struct N3DContext {
	u16                dirtyFlags;
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

private:
public:
	void applyContextState(bool forceApply = false);
#include "graphics/3ds/opinfo.h"
	void init(N3DContext *source = nullptr);
	void initOGL();
	void deinit();
};

extern N3DContext *activeContext;
// Return a pointer to the active context.
N3DContext *getActiveContext();

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

struct dirtyFVec {
	C3D_FVec *ptr;
	int pos;
	int dirtyRows;
	dirtyFVec(C3D_FVec *vec = nullptr, int startPosition = 0, int numRows = 0) : ptr(vec), pos(startPosition), dirtyRows(numRows) {}
};

typedef Common::HashMap<Common::String, int> UniformsMap;
typedef Common::Queue<dirtyFVec> FVecQueue;


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

class ShaderObj;
extern ShaderObj *activeShaderObj;
ShaderObj *getActiveShader();
void changeShader(ShaderObj *shaderObj);

class ShaderObj {
public:
	// Constructor
	ShaderObj(u32* shbinData, u32 shbinSize, u8 geomStride = 0);
	// Copy constructor
	ShaderObj(ShaderObj *original);
	// ShaderObj destructor, preserving the vshader and uniform map if this is a clone
	~ShaderObj();

	// Set the float matrix or vector(s) uniform associated with "uniformID" while converting
	//	it from ScummVM format to Citro3D format.
	template <int R, int C>
	bool setUniform(const Common::String &uniformID, GPU_SHADER_TYPE shaderEnum, const Math::Matrix<R,C> &svmMtx) {
		int pos = getUniformLocation(uniformID, shaderEnum);
		if (pos != -1) {
			if (C > 1) {
				if (getActiveShader() == this) {
					// If this is the active shader object, go ahead and send to Citro3D.
					C3D_Mtx m;
					Mtx_Zeros(&m);
					for (int row = 0; row < R; row++) {
						for (int col = 0; col < C; col++) {
							m.r[col].c[3-row] = svmMtx(row, col);
						}
					}
					C3D_FVUnifMtxNx4(shaderEnum, pos, &m, C);
				} else {
					// Otherwise, queue this up for the next time this ShaderObj is used.
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
				// Handle vector uniform values.
				float temp[4] = {0.0, 0.0, 0.0, 0.0};
				for (int f = 0; f < R; f++) {
					temp[f] = svmMtx(f, 0);
				}
				return setUniform(uniformID, shaderEnum, temp[0], temp[1], temp[2], temp[3]);
			}
		} else {
			return false;
		}
	}

	// Set the float matrix or vector(s) uniform associated with "uniformID".
	bool setUniform(const Common::String &uniformID, GPU_SHADER_TYPE shaderEnum, const C3D_Mtx &c3dMtx, int rows = 4) {
		int pos = getUniformLocation(uniformID, shaderEnum);
		if (pos != -1) {
			if (getActiveShader() == this) {
				// If this is the active shader object, go ahead and send to Citro3D.
				C3D_FVUnifMtxNx4(shaderEnum, pos, &c3dMtx, rows);
			} else {
				// Otherwise, queue this up for the next time this ShaderObj is used.
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

	// Set the float vector uniform associated with "uniformID".
	bool setUniform(const Common::String &uniformID, GPU_SHADER_TYPE shaderEnum, float x, float y = 0.0f, float z = 0.0f, float w = 0.0f) {
		int pos = getUniformLocation(uniformID, shaderEnum);
		if (pos != -1) {
			if (getActiveShader() == this) {
				// If this is the active shader object, go ahead and send to Citro3D.
				C3D_FVUnifSet(shaderEnum, pos, x, y, z, w);
			} else {
				// Otherwise, queue this up for the next time this ShaderObj is used.
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

	// Same as above, but takes a Citro3D vector instead of four floats.
	bool setUniform(const Common::String &uniformID, GPU_SHADER_TYPE shaderEnum, const C3D_FVec &c3dfvec) {
		return setUniform(uniformID, shaderEnum, c3dfvec.x, c3dfvec.y, c3dfvec.z, c3dfvec.w);
	}

	// Set the array of N-length float vector uniforms associated with "uniformID" while
	//	converting them from ScummVM format to Citro3D format.
	bool setUniformNfv(const Common::String &uniformID, GPU_SHADER_TYPE shaderEnum, float *dataPtr, int vecCount, int floatsPerVec) {
		int pos = getUniformLocation(uniformID, shaderEnum);
		if (pos != -1) {
			if (getActiveShader() == this) {
				// If this is the active shader object, go ahead and send to Citro3D.
				for (int v = 0; v < vecCount; v++) {
					float temp[4] = {0.0, 0.0, 0.0, 0.0};
					for (int f = 0; f < floatsPerVec; f++) {
						temp[f] = dataPtr[(v*floatsPerVec+f)];
					}
					C3D_FVUnifSet(shaderEnum, pos + v, temp[0], temp[1], temp[2], temp[3]);
				}
			} else {
				// Otherwise, queue this up for the next time this ShaderObj is used.
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
	bool setUniform4fv(const Common::String &uniformID, GPU_SHADER_TYPE shaderEnum, float *dataPtr, int vecCount) {
		return setUniformNfv(uniformID, shaderEnum, dataPtr, vecCount, 4);
	}
	bool setUniform3fv(const Common::String &uniformID, GPU_SHADER_TYPE shaderEnum, float *dataPtr, int vecCount) {
		return setUniformNfv(uniformID, shaderEnum, dataPtr, vecCount, 3);
	}
	bool setUniform2fv(const Common::String &uniformID, GPU_SHADER_TYPE shaderEnum, float *dataPtr, int vecCount) {
		return setUniformNfv(uniformID, shaderEnum, dataPtr, vecCount, 2);
	}
	bool setUniform1fv(const Common::String &uniformID, GPU_SHADER_TYPE shaderEnum, float *dataPtr, int vecCount) {
		return setUniformNfv(uniformID, shaderEnum, dataPtr, vecCount, 1);
	}

	// Set the integer vector uniform associated with "uniformID".
	bool setUniform(const Common::String &uniformID, GPU_SHADER_TYPE shaderEnum, int x, int y = 0, int z = 0, int w = 0) {
		int pos = getUniformLocation(uniformID, shaderEnum);
		if (pos != -1) {
			if (getActiveShader() == this) {
				// If this is the active shader object, go ahead and send to Citro3D.
				C3D_IVUnifSet(shaderEnum, pos, x, y, z, w);
			} else {
				// Otherwise, save this for the next time this ShaderObj is used.
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

	// Set the boolean uniform associated with "uniformID".
	bool setUniform(const Common::String &uniformID, GPU_SHADER_TYPE shaderEnum, bool boolval) {
		int pos = getUniformLocation(uniformID, shaderEnum);
		if (pos != -1) {
			if (getActiveShader() == this) {
				// If this is the active shader object, go ahead and send to Citro3D.
				C3D_BoolUnifSet(shaderEnum, pos, boolval);
			} else {
				// Otherwise, save this for the next time this ShaderObj is used.
				_unif_bools[2 * shaderEnum + pos] = boolval;
				_dirtyBools[2 * shaderEnum + pos] = true;
			}
			return true;
		} else {
			return false;
		}
	}

	// Get the location of the uniform associated with "uniformID", or create a new
	//	uniform and return its location.
	int getUniformLocation(const Common::String &uniformID, GPU_SHADER_TYPE shaderEnum) const {
		UniformsMap::iterator kv = N3DSMACRO_UNIF_MAP(shaderEnum)->find(uniformID);
		if (kv == N3DSMACRO_UNIF_MAP(shaderEnum)->end()) {
			int ret = shaderInstanceGetUniformLocation(N3DSMACRO_SHADER_INSTANCE(shaderEnum), uniformID.c_str());
			N3DSMACRO_UNIF_MAP(shaderEnum)->setVal(uniformID, ret);
			return ret;
		} else {
			return kv->_value;
		}
	}

	// Add attribute configuration information.
	int addAttrLoader(int regId, GPU_FORMATS format, int count) {
		return AttrInfo_AddLoader(&_attrInfo, regId, format, count);
	}

	// Add buffer configuration information.
	int addBufInfo(const void* data, ptrdiff_t stride, int attribCount, u64 permutation) {
		return BufInfo_Add(&_bufInfo, data, stride, attribCount, permutation);
	}

	// Destroy a buffer whose address is stored in one of _program's stored buffer
	//	configurations (usually the first config).
	void freeAttachedBuffer(/*C3D_BufInfo *bufInfo, bool inLinearMem, */int bufIdx = 0);

	// Add buffer info to the ShaderObj, or modify it if it already exists.
	int BufInfo_AddOrModify(const void* data, ptrdiff_t stride, int attribCount, u64 permutation, int id = 0);

	// Send queued uniforms to Citro3D. Only does something when the shaderObj is the
	//	active ShaderObj.
	void sendDirtyUniforms();


	// vars
	shaderProgram_s    *_program;
	C3D_AttrInfo        _attrInfo;
	C3D_BufInfo         _bufInfo;

private:
	DVLB_s             *_dvlb;
	u32                 _vert_numFVecs,    _geom_numFVecs;
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

#endif // GRAPHICS_3DS_N3DOBJECTS_H
