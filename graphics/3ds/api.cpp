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
void N3D_Viewport(u32 x, u32 y, u32 w, u32 h) {
	N3DS_3D::getActiveContext()->opViewport(x, y, w, h);
}

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

void N3D_WScale(float wScale) {
	N3DS_3D::getActiveContext()->opWScale(wScale);
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
}

void N3D_ArbDataToArbBlockTexOffset(u32 *srcBuf, u32 *dstBuf, int copyWidth, int copyHeight,
                                    int xSource, int ySource, int wSource,   int hSource,
                                    int xDest,   int yDest,   int wDest,     int hDest,
                                    GPU_TEXCOLOR format, int scale, bool isBlockSrc) {
	N3DS_3D::getActiveContext()->opArbDataToArbBlockTexOffset(srcBuf, dstBuf, copyWidth, copyHeight,
	                                                          xSource, ySource, wSource, hSource,
	                                                          xDest, yDest, wDest, hDest,
	                                                          format, scale, isBlockSrc);
}

void N3D_DataToBlockTex(u32 *srcBuf, u32 *dstBuf, int xSource, int ySource,
                        int wSource, int hSource, int wDest, int hDest,
                        GPU_TEXCOLOR format, int scale, bool isBlockSrc) {
	N3DS_3D::getActiveContext()->opDataToBlockTex(srcBuf, dstBuf, xSource, ySource,
	                                              wSource, hSource, wDest, hDest,
	                                              format, scale, isBlockSrc);
}



// from citro3d/include/c3d/maths.h
C3D_FVec N3D_FVec4_New(float x, float y, float z, float w) {
	return N3DS_3D::getActiveContext()->opFVec4_New(x, y, z, w);
}

C3D_FVec N3D_FVec4_Subtract(C3D_FVec lhs, C3D_FVec rhs) {
	return N3DS_3D::getActiveContext()->opFVec4_Subtract(lhs, rhs);
}

C3D_FVec N3D_FVec4_Negate(C3D_FVec v) {
	return N3DS_3D::getActiveContext()->opFVec4_Negate(v);
}

C3D_FVec N3D_FVec4_Scale(C3D_FVec v, float s) {
	return N3DS_3D::getActiveContext()->opFVec4_Scale(v, s);
}

C3D_FVec N3D_FVec4_PerspDivide(C3D_FVec v) {
	return N3DS_3D::getActiveContext()->opFVec4_PerspDivide(v);
}

float N3D_FVec4_Dot(C3D_FVec lhs, C3D_FVec rhs) {
	return N3DS_3D::getActiveContext()->opFVec4_Dot(lhs, rhs);
}

float N3D_FVec4_Magnitude(C3D_FVec v) {
	return N3DS_3D::getActiveContext()->opFVec4_Magnitude(v);
}

C3D_FVec N3D_FVec4_Normalize(C3D_FVec v) {
	return N3DS_3D::getActiveContext()->opFVec4_Normalize(v);
}

C3D_FVec N3D_FVec3_New(float x, float y, float z) {
	return N3DS_3D::getActiveContext()->opFVec3_New(x, y, z);
}

float N3D_FVec3_Dot(C3D_FVec lhs, C3D_FVec rhs) {
	return N3DS_3D::getActiveContext()->opFVec3_Dot(lhs, rhs);
}

float N3D_FVec3_Magnitude(C3D_FVec v) {
	return N3DS_3D::getActiveContext()->opFVec3_Magnitude(v);
}

C3D_FVec N3D_FVec3_Normalize(C3D_FVec v) {
	return N3DS_3D::getActiveContext()->opFVec3_Normalize(v);
}

C3D_FVec N3D_FVec3_Add(C3D_FVec lhs, C3D_FVec rhs) {
	return N3DS_3D::getActiveContext()->opFVec3_Add(lhs, rhs);
}

C3D_FVec N3D_FVec3_Subtract(C3D_FVec lhs, C3D_FVec rhs) {
	return N3DS_3D::getActiveContext()->opFVec3_Subtract(lhs, rhs);
}

float N3D_FVec3_Distance(C3D_FVec lhs, C3D_FVec rhs) {
	return N3DS_3D::getActiveContext()->opFVec3_Distance(lhs, rhs);
}

C3D_FVec N3D_FVec3_Scale(C3D_FVec v, float s) {
	return N3DS_3D::getActiveContext()->opFVec3_Scale(v, s);
}

C3D_FVec N3D_FVec3_Negate(C3D_FVec v) {
	return N3DS_3D::getActiveContext()->opFVec3_Negate(v);
}

C3D_FVec N3D_FVec3_Cross(C3D_FVec lhs, C3D_FVec rhs) {
	return N3DS_3D::getActiveContext()->opFVec3_Cross(lhs, rhs);
}

void N3D_Mtx_Zeros(C3D_Mtx* out) {
	N3DS_3D::getActiveContext()->opMtx_Zeros(out);
}

void N3D_Mtx_Copy(C3D_Mtx* out, const C3D_Mtx* in) {
	N3DS_3D::getActiveContext()->opMtx_Copy(out, in);
}

void N3D_Mtx_Diagonal(C3D_Mtx* out, float x, float y, float z, float w) {
	N3DS_3D::getActiveContext()->opMtx_Diagonal(out, x, y, z, w);
}

void N3D_Mtx_Identity(C3D_Mtx* out) {
	N3DS_3D::getActiveContext()->opMtx_Identity(out);
}

void N3D_Mtx_Transpose(C3D_Mtx* out) {
	N3DS_3D::getActiveContext()->opMtx_Transpose(out);
}

void N3D_Mtx_Add(C3D_Mtx* out, const C3D_Mtx* lhs, const C3D_Mtx* rhs) {
	N3DS_3D::getActiveContext()->opMtx_Add(out, lhs, rhs);
}

void N3D_Mtx_Subtract(C3D_Mtx* out, const C3D_Mtx* lhs, const C3D_Mtx* rhs) {
	N3DS_3D::getActiveContext()->opMtx_Subtract(out, lhs, rhs);
}

void N3D_Mtx_Multiply(C3D_Mtx* out, const C3D_Mtx* a, const C3D_Mtx* b) {
	N3DS_3D::getActiveContext()->opMtx_Multiply(out, a, b);
}

float N3D_Mtx_Inverse(C3D_Mtx* out) {
	return N3DS_3D::getActiveContext()->opMtx_Inverse(out);
}

C3D_FVec N3D_Mtx_MultiplyFVec3(const C3D_Mtx* mtx, C3D_FVec v) {
	return N3DS_3D::getActiveContext()->opMtx_MultiplyFVec3(mtx, v);
}

C3D_FVec N3D_Mtx_MultiplyFVec4(const C3D_Mtx* mtx, C3D_FVec v) {
	return N3DS_3D::getActiveContext()->opMtx_MultiplyFVec4(mtx, v);
}

C3D_FVec N3D_Mtx_MultiplyFVecH(const C3D_Mtx* mtx, C3D_FVec v) {
	return N3DS_3D::getActiveContext()->opMtx_MultiplyFVecH(mtx, v);
}

void N3D_Mtx_FromQuat(C3D_Mtx* m, C3D_FQuat q) {
	N3DS_3D::getActiveContext()->opMtx_FromQuat(m, q);
}

void N3D_Mtx_Translate(C3D_Mtx* mtx, float x, float y, float z, bool bRightSide) {
	N3DS_3D::getActiveContext()->opMtx_Translate(mtx, x, y, z, bRightSide);
}

void N3D_Mtx_Scale(C3D_Mtx* mtx, float x, float y, float z) {
	N3DS_3D::getActiveContext()->opMtx_Scale(mtx, x, y, z);
}

void N3D_Mtx_Rotate(C3D_Mtx* mtx, C3D_FVec axis, float angle, bool bRightSide) {
	N3DS_3D::getActiveContext()->opMtx_Rotate(mtx, axis, angle, bRightSide);
}

void N3D_Mtx_RotateX(C3D_Mtx* mtx, float angle, bool bRightSide) {
	N3DS_3D::getActiveContext()->opMtx_RotateX(mtx, angle, bRightSide);
}

void N3D_Mtx_RotateY(C3D_Mtx* mtx, float angle, bool bRightSide) {
	N3DS_3D::getActiveContext()->opMtx_RotateY(mtx, angle, bRightSide);
}

void N3D_Mtx_RotateZ(C3D_Mtx* mtx, float angle, bool bRightSide) {
	N3DS_3D::getActiveContext()->opMtx_RotateZ(mtx, angle, bRightSide);
}

void N3D_Mtx_Ortho(C3D_Mtx* mtx, float left, float right, float bottom, float top, float near, float far, bool isLeftHanded) {
	N3DS_3D::getActiveContext()->opMtx_Ortho(mtx, left, right, bottom, top, near, far, isLeftHanded);
}

void N3D_Mtx_Persp(C3D_Mtx* mtx, float fovy, float aspect, float near, float far, bool isLeftHanded) {
	N3DS_3D::getActiveContext()->opMtx_Persp(mtx, fovy, aspect, near, far, isLeftHanded);
}

void N3D_Mtx_PerspStereo(C3D_Mtx* mtx, float fovy, float aspect, float near, float far, float iod, float screen, bool isLeftHanded) {
	N3DS_3D::getActiveContext()->opMtx_PerspStereo(mtx, fovy, aspect, near, far, iod, screen, isLeftHanded);
}

void N3D_Mtx_OrthoTilt(C3D_Mtx* mtx, float left, float right, float bottom, float top, float near, float far, bool isLeftHanded) {
	N3DS_3D::getActiveContext()->opMtx_OrthoTilt(mtx, left, right, bottom, top, near, far, isLeftHanded);
}

void N3D_Mtx_PerspTilt(C3D_Mtx* mtx, float fovy, float aspect, float near, float far, bool isLeftHanded) {
	N3DS_3D::getActiveContext()->opMtx_PerspTilt(mtx, fovy, aspect, near, far, isLeftHanded);
}

void N3D_Mtx_PerspStereoTilt(C3D_Mtx* mtx, float fovy, float aspect, float near, float far, float iod, float screen, bool isLeftHanded) {
	N3DS_3D::getActiveContext()->opMtx_PerspStereoTilt(mtx, fovy, aspect, near, far, iod, screen, isLeftHanded);
}

void N3D_Mtx_LookAt(C3D_Mtx* out, C3D_FVec cameraPosition, C3D_FVec cameraTarget, C3D_FVec cameraUpVector, bool isLeftHanded) {
	N3DS_3D::getActiveContext()->opMtx_LookAt(out, cameraPosition, cameraTarget, cameraUpVector, isLeftHanded);
}

//#define   opQuat_New(i,j,k,r)      opFVec4_New(i,j,k,r)
//#define   opQuat_Negate(q)         opFVec4_Negate(q)
//#define   opQuat_Add(lhs,rhs)      opFVec4_Add(lhs,rhs)
//#define   opQuat_Subtract(lhs,rhs) opFVec4_Subtract(lhs,rhs)
//#define   opQuat_Scale(q,s)        opFVec4_Scale(q,s)
//#define   opQuat_Normalize(q)      opFVec4_Normalize(q)
//#define   opQuat_Dot(lhs,rhs)      opFVec4_Dot(lhs,rhs)

C3D_FQuat N3D_Quat_Multiply(C3D_FQuat lhs, C3D_FQuat rhs) {
	return N3DS_3D::getActiveContext()->opQuat_Multiply(lhs, rhs);
}

C3D_FQuat N3D_Quat_Pow(C3D_FQuat q, float p) {
	return N3DS_3D::getActiveContext()->opQuat_Pow(q, p);
}

C3D_FVec N3D_Quat_CrossFVec3(C3D_FQuat q, C3D_FVec v) {
	return N3DS_3D::getActiveContext()->opQuat_CrossFVec3(q, v);
}

C3D_FQuat N3D_Quat_Rotate(C3D_FQuat q, C3D_FVec axis, float r, bool bRightSide) {
	return N3DS_3D::getActiveContext()->opQuat_Rotate(q, axis, r, bRightSide);
}

C3D_FQuat N3D_Quat_RotateX(C3D_FQuat q, float r, bool bRightSide) {
	return N3DS_3D::getActiveContext()->opQuat_RotateX(q, r, bRightSide);
}

C3D_FQuat N3D_Quat_RotateY(C3D_FQuat q, float r, bool bRightSide) {
	return N3DS_3D::getActiveContext()->opQuat_RotateY(q, r, bRightSide);
}

C3D_FQuat N3D_Quat_RotateZ(C3D_FQuat q, float r, bool bRightSide) {
	return N3DS_3D::getActiveContext()->opQuat_RotateZ(q, r, bRightSide);
}

C3D_FQuat N3D_Quat_FromMtx(const C3D_Mtx* m) {
	return N3DS_3D::getActiveContext()->opQuat_FromMtx(m);
}

C3D_FQuat N3D_Quat_Identity() {
	return N3DS_3D::getActiveContext()->opQuat_Identity();
}

C3D_FQuat N3D_Quat_Conjugate(C3D_FQuat q) {
	return N3DS_3D::getActiveContext()->opQuat_Conjugate(q);
}

C3D_FQuat N3D_Quat_Inverse(C3D_FQuat q) {
	return N3DS_3D::getActiveContext()->opQuat_Inverse(q);
}

C3D_FVec N3D_FVec3_CrossQuat(C3D_FVec v, C3D_FQuat q) {
	return N3DS_3D::getActiveContext()->opFVec3_CrossQuat(v, q);
}

C3D_FQuat N3D_Quat_FromPitchYawRoll(float pitch, float yaw, float roll, bool bRightSide) {
	return N3DS_3D::getActiveContext()->opQuat_FromPitchYawRoll(pitch, yaw, roll, bRightSide);
}

C3D_FQuat N3D_Quat_LookAt(C3D_FVec source, C3D_FVec target, C3D_FVec forwardVector, C3D_FVec upVector) {
	return N3DS_3D::getActiveContext()->opQuat_LookAt(source, target, forwardVector, upVector);
}

C3D_FQuat N3D_Quat_FromAxisAngle(C3D_FVec axis, float angle) {
	return N3DS_3D::getActiveContext()->opQuat_FromAxisAngle(axis, angle);
}


// from citro3d/include/c3d/mtxstack.h
C3D_Mtx *N3D_MtxStack_Cur(C3D_MtxStack* stk) {
	return N3DS_3D::getActiveContext()->opMtxStack_Cur(stk);
}

void N3D_MtxStack_Init(C3D_MtxStack* stk) {
	N3DS_3D::getActiveContext()->opMtxStack_Init(stk);
}

void N3D_MtxStack_Bind(C3D_MtxStack* stk, GPU_SHADER_TYPE unifType, int unifPos, int unifLen) {
	N3DS_3D::getActiveContext()->opMtxStack_Bind(stk, unifType, unifPos, unifLen);
}

C3D_Mtx *N3D_MtxStack_Push(C3D_MtxStack* stk) {
	return N3DS_3D::getActiveContext()->opMtxStack_Push(stk);
}

C3D_Mtx *N3D_MtxStack_Pop(C3D_MtxStack* stk) {
	return N3DS_3D::getActiveContext()->opMtxStack_Pop(stk);
}

void N3D_MtxStack_Update(C3D_MtxStack* stk) {
	N3DS_3D::getActiveContext()->opMtxStack_Update(stk);
}


// from citro3d/include/c3d/uniforms.h
C3D_FVec *N3D_C3D_FVUnifWritePtr(GPU_SHADER_TYPE type, int id, int size) {
	return N3DS_3D::getActiveContext()->opC3D_FVUnifWritePtr(type, id, size);
}

C3D_IVec *N3D_C3D_IVUnifWritePtr(GPU_SHADER_TYPE type, int id) {
	return N3DS_3D::getActiveContext()->opC3D_IVUnifWritePtr(type, id);
}

void N3D_C3D_FVUnifMtxNx4(GPU_SHADER_TYPE type, int id, const C3D_Mtx* mtx, int num) {
	N3DS_3D::getActiveContext()->opC3D_FVUnifMtxNx4(type, id, mtx, num);
}

void N3D_C3D_FVUnifMtx4x4(GPU_SHADER_TYPE type, int id, const C3D_Mtx* mtx) {
	N3DS_3D::getActiveContext()->opC3D_FVUnifMtx4x4(type, id, mtx);
}

void N3D_C3D_FVUnifMtx3x4(GPU_SHADER_TYPE type, int id, const C3D_Mtx* mtx) {
	N3DS_3D::getActiveContext()->opC3D_FVUnifMtx3x4(type, id, mtx);
}

void N3D_C3D_FVUnifMtx2x4(GPU_SHADER_TYPE type, int id, const C3D_Mtx* mtx) {
	N3DS_3D::getActiveContext()->opC3D_FVUnifMtx2x4(type, id, mtx);
}

void N3D_C3D_FVUnifSet(GPU_SHADER_TYPE type, int id, float x, float y, float z, float w) {
	N3DS_3D::getActiveContext()->opC3D_FVUnifSet(type, id, x, y, z, w);
}

void N3D_C3D_IVUnifSet(GPU_SHADER_TYPE type, int id, int x, int y, int z, int w) {
	N3DS_3D::getActiveContext()->opC3D_IVUnifSet(type, id, x, y, z, w);
}

void N3D_C3D_BoolUnifSet(GPU_SHADER_TYPE type, int id, bool value) {
	N3DS_3D::getActiveContext()->opC3D_BoolUnifSet(type, id, value);
}

void N3D_C3D_UpdateUniforms(GPU_SHADER_TYPE type) {
	N3DS_3D::getActiveContext()->opC3D_UpdateUniforms(type);
}


// from citro3d/include/c3d/attribs.h
void N3D_AttrInfo_Init(C3D_AttrInfo* info) {
	N3DS_3D::getActiveContext()->opAttrInfo_Init(info);
}

int N3D_AttrInfo_AddLoader(C3D_AttrInfo* info, int regId, GPU_FORMATS format, int count) {
	return N3DS_3D::getActiveContext()->opAttrInfo_AddLoader(info, regId, format, count);
}

int N3D_AttrInfo_AddFixed(C3D_AttrInfo* info, int regId) {
	return N3DS_3D::getActiveContext()->opAttrInfo_AddFixed(info, regId);
}

C3D_AttrInfo *N3D_C3D_GetAttrInfo() {
	return N3DS_3D::getActiveContext()->opC3D_GetAttrInfo();
}

void N3D_C3D_SetAttrInfo(C3D_AttrInfo* info) {
	N3DS_3D::getActiveContext()->opC3D_SetAttrInfo(info);
}


// from citro3d/include/c3d/buffers.h
void N3D_BufInfo_Init(C3D_BufInfo* info) {
	N3DS_3D::getActiveContext()->opBufInfo_Init(info);
}

int N3D_BufInfo_Add(C3D_BufInfo* info, const void* data, ptrdiff_t stride, int attribCount, u64 permutation) {
	return N3DS_3D::getActiveContext()->opBufInfo_Add(info, data, stride, attribCount, permutation);
}

C3D_BufInfo *N3D_C3D_GetBufInfo() {
	return N3DS_3D::getActiveContext()->opC3D_GetBufInfo();
}

void N3D_C3D_SetBufInfo(C3D_BufInfo* info) {
	N3DS_3D::getActiveContext()->opC3D_SetBufInfo(info);
}


// from citro3d/include/c3d/base.h
float N3D_C3D_GetCmdBufUsage() {
	return N3DS_3D::getActiveContext()->opC3D_GetCmdBufUsage();
}

void N3D_C3D_BindProgram(shaderProgram_s* program) {
	N3DS_3D::getActiveContext()->opC3D_BindProgram(program);
}

void N3D_C3D_SetViewport(u32 x, u32 y, u32 w, u32 h) {
	N3DS_3D::getActiveContext()->opC3D_SetViewport(x, y, w, h);
}

void N3D_C3D_SetScissor(GPU_SCISSORMODE mode, u32 left, u32 top, u32 right, u32 bottom) {
	N3DS_3D::getActiveContext()->opC3D_SetScissor(mode, left, top, right, bottom);
}

void N3D_C3D_DrawArrays(GPU_Primitive_t primitive, int first, int size) {
	N3DS_3D::getActiveContext()->opC3D_DrawArrays(primitive, first, size);
}

void N3D_C3D_DrawElements(GPU_Primitive_t primitive, int count, int type, const void* indices) {
	N3DS_3D::getActiveContext()->opC3D_DrawElements(primitive, count, type, indices);
}

void N3D_C3D_ImmDrawBegin(GPU_Primitive_t primitive) {
	N3DS_3D::getActiveContext()->opC3D_ImmDrawBegin(primitive);
}

void N3D_C3D_ImmSendAttrib(float x, float y, float z, float w) {
	N3DS_3D::getActiveContext()->opC3D_ImmSendAttrib(x, y, z, w);
}

void N3D_C3D_ImmDrawEnd() {
	return N3DS_3D::getActiveContext()->opC3D_ImmDrawEnd();
}

void N3D_C3D_ImmDrawRestartPrim() {
	return N3DS_3D::getActiveContext()->opC3D_ImmDrawRestartPrim();
}

C3D_FVec *N3D_C3D_FixedAttribGetWritePtr(int id) {
	return N3DS_3D::getActiveContext()->opC3D_FixedAttribGetWritePtr(id);
}

void N3D_C3D_FixedAttribSet(int id, float x, float y, float z, float w) {
	N3DS_3D::getActiveContext()->opC3D_FixedAttribSet(id, x, y, z, w);
}


// from citro3d/include/c3d/texenv.h
C3D_TexEnv *N3D_C3D_GetTexEnv(int id) {
	return N3DS_3D::getActiveContext()->opC3D_GetTexEnv(id);
}

void N3D_C3D_SetTexEnv(int id, C3D_TexEnv* env) {
	N3DS_3D::getActiveContext()->opC3D_SetTexEnv(id, env);
}

void N3D_C3D_DirtyTexEnv(C3D_TexEnv* env) {
	N3DS_3D::getActiveContext()->opC3D_DirtyTexEnv(env);
}

void N3D_C3D_TexEnvBufUpdate(int mode, int mask) {
	N3DS_3D::getActiveContext()->opC3D_TexEnvBufUpdate(mode, mask);
}

void N3D_C3D_TexEnvBufColor(u32 color) {
	N3DS_3D::getActiveContext()->opC3D_TexEnvBufColor(color);
}

void N3D_C3D_TexEnvInit(C3D_TexEnv* env) {
	N3DS_3D::getActiveContext()->opC3D_TexEnvInit(env);
}

void N3D_C3D_TexEnvSrc(C3D_TexEnv* env, C3D_TexEnvMode mode,
	GPU_TEVSRC s1, GPU_TEVSRC s2, GPU_TEVSRC s3) {
	N3DS_3D::getActiveContext()->opC3D_TexEnvSrc(env, mode, s1, s2, s3);
}

void N3D_C3D_TexEnvOpRgb(C3D_TexEnv* env,
	GPU_TEVOP_RGB o1, GPU_TEVOP_RGB o2, GPU_TEVOP_RGB o3) {
	N3DS_3D::getActiveContext()->opC3D_TexEnvOpRgb(env, o1, o2, o3);
}

void N3D_C3D_TexEnvOpAlpha(C3D_TexEnv* env,
	GPU_TEVOP_A o1, GPU_TEVOP_A o2, GPU_TEVOP_A o3) {
	N3DS_3D::getActiveContext()->opC3D_TexEnvOpAlpha(env, o1, o2, o3);
}

void N3D_C3D_TexEnvFunc(C3D_TexEnv* env, C3D_TexEnvMode mode, GPU_COMBINEFUNC param) {
	N3DS_3D::getActiveContext()->opC3D_TexEnvFunc(env, mode, param);
}

void N3D_C3D_TexEnvColor(C3D_TexEnv* env, u32 color) {
	N3DS_3D::getActiveContext()->opC3D_TexEnvColor(env, color);
}

void N3D_C3D_TexEnvScale(C3D_TexEnv* env, int mode, GPU_TEVSCALE param) {
	N3DS_3D::getActiveContext()->opC3D_TexEnvScale(env, mode, param);
}


// from citro3d/include/c3d/effect.h
void N3D_C3D_FragOpShadow(float scale, float bias) {
	N3DS_3D::getActiveContext()->opC3D_FragOpShadow(scale, bias);
}


// from citro3d/include/c3d/texture.h
bool N3D_C3D_TexInitWithParams(C3D_Tex* tex, C3D_TexCube* cube, C3D_TexInitParams p) {
	return N3DS_3D::getActiveContext()->opC3D_TexInitWithParams(tex, cube, p);
}

void N3D_C3D_TexLoadImage(C3D_Tex* tex, const void* data, GPU_TEXFACE face, int level) {
	N3DS_3D::getActiveContext()->opC3D_TexLoadImage(tex, data, face, level);
}

void N3D_C3D_TexGenerateMipmap(C3D_Tex* tex, GPU_TEXFACE face) {
	N3DS_3D::getActiveContext()->opC3D_TexGenerateMipmap(tex, face);
}

void N3D_C3D_TexBind(int unitId, C3D_Tex* tex) {
	N3DS_3D::getActiveContext()->opC3D_TexBind(unitId, tex);
}

void N3D_C3D_TexFlush(C3D_Tex* tex) {
	N3DS_3D::getActiveContext()->opC3D_TexFlush(tex);
}

void N3D_C3D_TexDelete(C3D_Tex* tex) {
	N3DS_3D::getActiveContext()->opC3D_TexDelete(tex);
}

void N3D_C3D_TexShadowParams(bool perspective, float bias) {
	N3DS_3D::getActiveContext()->opC3D_TexShadowParams(perspective, bias);
}

int N3D_C3D_TexCalcMaxLevel(u32 width, u32 height) {
	return N3DS_3D::getActiveContext()->opC3D_TexCalcMaxLevel(width, height);
}

u32 N3D_C3D_TexCalcLevelSize(u32 size, int level) {
	return N3DS_3D::getActiveContext()->opC3D_TexCalcLevelSize(size, level);
}

u32 N3D_C3D_TexCalcTotalSize(u32 size, int maxLevel) {
	return N3DS_3D::getActiveContext()->opC3D_TexCalcTotalSize(size, maxLevel);
}

bool N3D_C3D_TexInit(C3D_Tex* tex, u16 width, u16 height, GPU_TEXCOLOR format) {
	return N3DS_3D::getActiveContext()->opC3D_TexInit(tex, width, height, format);
}

bool N3D_C3D_TexInitMipmap(C3D_Tex* tex, u16 width, u16 height, GPU_TEXCOLOR format) {
	return N3DS_3D::getActiveContext()->opC3D_TexInitMipmap(tex, width, height, format);
}

bool N3D_C3D_TexInitCube(C3D_Tex* tex, C3D_TexCube* cube, u16 width, u16 height, GPU_TEXCOLOR format) {
	return N3DS_3D::getActiveContext()->opC3D_TexInitCube(tex, cube, width, height, format);
}

bool N3D_C3D_TexInitVRAM(C3D_Tex* tex, u16 width, u16 height, GPU_TEXCOLOR format) {
	return N3DS_3D::getActiveContext()->opC3D_TexInitVRAM(tex, width, height, format);
}

bool N3D_C3D_TexInitShadow(C3D_Tex* tex, u16 width, u16 height) {
	return N3DS_3D::getActiveContext()->opC3D_TexInitShadow(tex, width, height);
}

bool N3D_C3D_TexInitShadowCube(C3D_Tex* tex, C3D_TexCube* cube, u16 width, u16 height) {
	return N3DS_3D::getActiveContext()->opC3D_TexInitShadowCube(tex, cube, width, height);
}

GPU_TEXTURE_MODE_PARAM N3D_C3D_TexGetType(C3D_Tex* tex) {
	return N3DS_3D::getActiveContext()->opC3D_TexGetType(tex);
}

void *N3D_C3D_TexGetImagePtr(C3D_Tex* tex, void* data, int level, u32* size) {
	return N3DS_3D::getActiveContext()->opC3D_TexGetImagePtr(tex, data, level, size);
}

void *N3D_C3D_Tex2DGetImagePtr(C3D_Tex* tex, int level, u32* size) {
	return N3DS_3D::getActiveContext()->opC3D_Tex2DGetImagePtr(tex, level, size);
}

void *N3D_C3D_TexCubeGetImagePtr(C3D_Tex* tex, GPU_TEXFACE face, int level, u32* size) {
	return N3DS_3D::getActiveContext()->opC3D_TexCubeGetImagePtr(tex, face, level, size);
}

void N3D_C3D_TexUpload(C3D_Tex* tex, const void* data) {
	N3DS_3D::getActiveContext()->opC3D_TexUpload(tex, data);
}

void N3D_C3D_TexSetFilter(C3D_Tex* tex, GPU_TEXTURE_FILTER_PARAM magFilter, GPU_TEXTURE_FILTER_PARAM minFilter) {
	N3DS_3D::getActiveContext()->opC3D_TexSetFilter(tex, magFilter, minFilter);
}

void N3D_C3D_TexSetFilterMipmap(C3D_Tex* tex, GPU_TEXTURE_FILTER_PARAM filter) {
	N3DS_3D::getActiveContext()->opC3D_TexSetFilterMipmap(tex, filter);
}

void N3D_C3D_TexSetWrap(C3D_Tex* tex, GPU_TEXTURE_WRAP_PARAM wrapS, GPU_TEXTURE_WRAP_PARAM wrapT) {
	N3DS_3D::getActiveContext()->opC3D_TexSetWrap(tex, wrapS, wrapT);
}

void N3D_C3D_TexSetLodBias(C3D_Tex* tex, float lodBias) {
	N3DS_3D::getActiveContext()->opC3D_TexSetLodBias(tex, lodBias);
}

// from citro3d/include/c3d/proctex.
// from citro3d/include/c3d/light.h
// from citro3d/include/c3d/lughtlut.h
// from citro3d/include/c3d/fog.h

// from citro3d/include/c3d/framebuffer.h
u32 N3D_C3D_CalcColorBufSize(u32 width, u32 height, GPU_COLORBUF fmt) {
	return N3DS_3D::getActiveContext()->opC3D_CalcColorBufSize(width, height, fmt);
}

u32 N3D_C3D_CalcDepthBufSize(u32 width, u32 height, GPU_DEPTHBUF fmt) {
	return N3DS_3D::getActiveContext()->opC3D_CalcDepthBufSize(width, height, fmt);
}

C3D_FrameBuf *N3D_C3D_GetFrameBuf() {
	return N3DS_3D::getActiveContext()->opC3D_GetFrameBuf();
}

void N3D_C3D_SetFrameBuf(C3D_FrameBuf* fb) {
	N3DS_3D::getActiveContext()->opC3D_SetFrameBuf(fb);
}

void N3D_C3D_FrameBufTex(C3D_FrameBuf* fb, C3D_Tex* tex, GPU_TEXFACE face, int level) {
	N3DS_3D::getActiveContext()->opC3D_FrameBufTex(fb, tex, face, level);
}

void N3D_C3D_FrameBufClear(C3D_FrameBuf* fb, C3D_ClearBits clearBits, u32 clearColor, u32 clearDepth) {
	N3DS_3D::getActiveContext()->opC3D_FrameBufClear(fb, clearBits, clearColor, clearDepth);
}

void N3D_C3D_FrameBufTransfer(C3D_FrameBuf* fb, gfxScreen_t screen, gfx3dSide_t side, u32 transferFlags) {
	N3DS_3D::getActiveContext()->opC3D_FrameBufTransfer(fb, screen, side, transferFlags);
}

void N3D_C3D_FrameBufAttrib(C3D_FrameBuf* fb, u16 width, u16 height, bool block32) {
	N3DS_3D::getActiveContext()->opC3D_FrameBufAttrib(fb, width, height, block32);
}

void N3D_C3D_FrameBufColor(C3D_FrameBuf* fb, void* buf, GPU_COLORBUF fmt) {
	N3DS_3D::getActiveContext()->opC3D_FrameBufColor(fb, buf, fmt);
}

void N3D_C3D_FrameBufDepth(C3D_FrameBuf* fb, void* buf, GPU_DEPTHBUF fmt) {
	N3DS_3D::getActiveContext()->opC3D_FrameBufDepth(fb, buf, fmt);
}


// from citro3d/include/c3d/renderqueue.h
float N3D_C3D_FrameRate(float fps) {
	return N3DS_3D::getActiveContext()->opC3D_FrameRate(fps);
}

void N3D_C3D_FrameSync() {
	return N3DS_3D::getActiveContext()->opC3D_FrameSync();
}

u32 N3D_C3D_FrameCounter(int id) {
	return N3DS_3D::getActiveContext()->opC3D_FrameCounter(id);
}

bool N3D_C3D_FrameBegin(u8 flags) {
	return N3DS_3D::getActiveContext()->opC3D_FrameBegin(flags);
}

bool N3D_C3D_FrameDrawOn(C3D_RenderTarget* target) {
	return N3DS_3D::getActiveContext()->opC3D_FrameDrawOn(target);
}

void N3D_C3D_FrameSplit(u8 flags) {
	N3DS_3D::getActiveContext()->opC3D_FrameSplit(flags);
}

void N3D_C3D_FrameEnd(u8 flags) {
	N3DS_3D::getActiveContext()->opC3D_FrameEnd(flags);
}

void N3D_C3D_FrameEndHook(void (* hook)(void*), void* param) {
	N3DS_3D::getActiveContext()->opC3D_FrameEndHook(hook, param);
}

float N3D_C3D_GetDrawingTime() {
	return N3DS_3D::getActiveContext()->opC3D_GetDrawingTime();
}

float N3D_C3D_GetProcessingTime() {
	return N3DS_3D::getActiveContext()->opC3D_GetProcessingTime();
}

C3D_RenderTarget *N3D_C3D_RenderTargetCreate(int width, int height, GPU_COLORBUF colorFmt, C3D_DEPTHTYPE depthFmt) {
	return N3DS_3D::getActiveContext()->opC3D_RenderTargetCreate(width, height, colorFmt, depthFmt);
}

C3D_RenderTarget *N3D_C3D_RenderTargetCreateFromTex(C3D_Tex* tex, GPU_TEXFACE face, int level, C3D_DEPTHTYPE depthFmt) {
	return N3DS_3D::getActiveContext()->opC3D_RenderTargetCreateFromTex(tex, face, level, depthFmt);
}

void N3D_C3D_RenderTargetDelete(C3D_RenderTarget* target) {
	N3DS_3D::getActiveContext()->opC3D_RenderTargetDelete(target);
}

void N3D_C3D_RenderTargetSetOutput(C3D_RenderTarget* target, gfxScreen_t screen, gfx3dSide_t side, u32 transferFlags) {
	N3DS_3D::getActiveContext()->opC3D_RenderTargetSetOutput(target, screen, side, transferFlags);
}

void N3D_C3D_RenderTargetDetachOutput(C3D_RenderTarget* target) {
	N3DS_3D::getActiveContext()->opC3D_RenderTargetDetachOutput(target);
}

void N3D_C3D_RenderTargetClear(C3D_RenderTarget* target, C3D_ClearBits clearBits, u32 clearColor, u32 clearDepth) {
	N3DS_3D::getActiveContext()->opC3D_RenderTargetClear(target, clearBits, clearColor, clearDepth);
}

void N3D_C3D_SyncDisplayTransfer(u32* inadr, u32 indim, u32* outadr, u32 outdim, u32 flags) {
	N3DS_3D::getActiveContext()->opC3D_SyncDisplayTransfer(inadr, indim, outadr, outdim, flags);
}

void N3D_C3D_SyncTextureCopy(u32* inadr, u32 indim, u32* outadr, u32 outdim, u32 size, u32 flags) {
	N3DS_3D::getActiveContext()->opC3D_SyncTextureCopy(inadr, indim, outadr, outdim, size, flags);
}

void N3D_C3D_SyncMemoryFill(u32* buf0a, u32 buf0v, u32* buf0e, u16 control0, u32* buf1a, u32 buf1v, u32* buf1e, u16 control1) {
	N3DS_3D::getActiveContext()->opC3D_SyncMemoryFill(buf0a, buf0v, buf0e, control0, buf1a, buf1v, buf1e, control1);
}
