/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your N3D_tion) any later version.
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

#ifndef GRAPHICS_3DS_NGL_H
#define GRAPHICS_3DS_NGL_H

#define FORBIDDEN_SYMBOL_EXCEPTION_time_h	// for <3ds.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE		// for <tex3ds.h> in n3d.h
#include <3ds.h>
#include <citro3d.h>
#include <tex3ds.h>

enum N3D_CULLFACE {
	N3D_CULLFACE_FRONT          = 0,
	N3D_CULLFACE_BACK           = 1,
	N3D_CULLFACE_FRONT_AND_BACK = 2,
};

enum N3D_FRONTFACE {
	N3D_FRONTFACE_CW  = 0,
	N3D_FRONTFACE_CCW = 1,
};

// from "graphics/3ds/ops-gl2citro.cpp"
//void N3D_Viewport(u32 x, u32 y, u32 w, u32 h);
void N3D_CullFace(N3D_CULLFACE mode);
void N3D_FrontFace(N3D_FRONTFACE mode);
void N3D_CullFaceEnabled(bool state);
void N3D_PolygonOffset(float pOffUnits);
void N3D_WScale(float wScale);
void N3D_DepthRange(float near, float far);
void N3D_PolygonOffsetEnabled(bool state);
void N3D_ScissorMode(GPU_SCISSORMODE mode);
void N3D_ScissorDims(u32 x, u32 y, u32 w, u32 h);
void N3D_AlphaFunc(GPU_TESTFUNC func, int ref);
void N3D_AlphaTestEnabled(bool state);
void N3D_StencilFunc(GPU_TESTFUNC func, int ref, int mask);
void N3D_StencilMask(int writeMask);
void N3D_StencilOp(GPU_STENCILOP fail, GPU_STENCILOP zfail, GPU_STENCILOP zpass);
// CLEAR STENCIL????????????
void N3D_StencilTestEnabled(bool state);
void N3D_DepthFunc(GPU_TESTFUNC func);
void N3D_DepthMask(bool depthState);
void N3D_ColorMask(bool redState, bool greenState, bool blueState, bool alphaState);
// CLEAR DEPTH?????????????
void N3D_DepthTestEnabled(bool state);
void N3D_EarlyDepthFunc(GPU_EARLYDEPTHFUNC func);
void N3D_ClearEarlyDepth(u32 clearValue);
void N3D_EarlyDepthTestEnabled(bool state);
void N3D_BlendEquation(GPU_BLENDEQUATION equation);
void N3D_BlendEquationSeparate(GPU_BLENDEQUATION colorEq, GPU_BLENDEQUATION alphaEq);
void N3D_BlendFunc(GPU_BLENDFACTOR srcFactor, GPU_BLENDFACTOR dstFactor);
void N3D_BlendFuncSeparate(GPU_BLENDFACTOR srcColor, GPU_BLENDFACTOR dstColor, GPU_BLENDFACTOR srcAlpha, GPU_BLENDFACTOR dstAlpha);
void N3D_BlendColor(u8 red, u8 green, u8 blue, u8 alpha);
void N3D_BlendEnabled(bool state);
// CLEAR COLOR???????????????
void N3D_LogicOp(GPU_LOGICOP N3D_);
void N3D_ColorLogicOpEnabled(bool state);


// CUSTOM FUNCS
C3D_Tex *N3D_GetGameScreen();
void     N3D_ArbDataToArbBlockTexOffset(u32 *srcBuf, u32 *dstBuf, int copyWidth, int copyHeight,
                                      int xSource, int ySource, int wSource,   int hSource,
                                      int xDest,   int yDest,   int wDest,     int hDest,
                                      GPU_TEXCOLOR format, int scale, bool isBlockSrc);
void     N3D_DataToBlockTex(u32 *srcBuf, u32 *dstBuf, int xSource, int ySource,
                          int wSource, int hSource, int wDest, int hDest,
                          GPU_TEXCOLOR format, int scale, bool isBlockSrc);

// from citro3d/include/c3d/maths.h
C3D_FVec  N3D_FVec4_New(float x, float y, float z, float w);
C3D_FVec  N3D_FVec4_Subtract(C3D_FVec lhs, C3D_FVec rhs);
C3D_FVec  N3D_FVec4_Negate(C3D_FVec v);
C3D_FVec  N3D_FVec4_Scale(C3D_FVec v, float s);
C3D_FVec  N3D_FVec4_PerspDivide(C3D_FVec v);
float     N3D_FVec4_Dot(C3D_FVec lhs, C3D_FVec rhs);
float     N3D_FVec4_Magnitude(C3D_FVec v);
C3D_FVec  N3D_FVec4_Normalize(C3D_FVec v);
C3D_FVec  N3D_FVec3_New(float x, float y, float z);
float     N3D_FVec3_Dot(C3D_FVec lhs, C3D_FVec rhs);
float     N3D_FVec3_Magnitude(C3D_FVec v);
C3D_FVec  N3D_FVec3_Normalize(C3D_FVec v);
C3D_FVec  N3D_FVec3_Add(C3D_FVec lhs, C3D_FVec rhs);
C3D_FVec  N3D_FVec3_Subtract(C3D_FVec lhs, C3D_FVec rhs);
float     N3D_FVec3_Distance(C3D_FVec lhs, C3D_FVec rhs);
C3D_FVec  N3D_FVec3_Scale(C3D_FVec v, float s);
C3D_FVec  N3D_FVec3_Negate(C3D_FVec v);
C3D_FVec  N3D_FVec3_Cross(C3D_FVec lhs, C3D_FVec rhs);
void      N3D_Mtx_Zeros(C3D_Mtx* out);
void      N3D_Mtx_Copy(C3D_Mtx* out, const C3D_Mtx* in);
void      N3D_Mtx_Diagonal(C3D_Mtx* out, float x, float y, float z, float w);
void      N3D_Mtx_Identity(C3D_Mtx* out);
void      N3D_Mtx_Transpose(C3D_Mtx* out);
void      N3D_Mtx_Add(C3D_Mtx* out, const C3D_Mtx* lhs, const C3D_Mtx* rhs);
void      N3D_Mtx_Subtract(C3D_Mtx* out, const C3D_Mtx* lhs, const C3D_Mtx* rhs);
void      N3D_Mtx_Multiply(C3D_Mtx* out, const C3D_Mtx* a, const C3D_Mtx* b);
float     N3D_Mtx_Inverse(C3D_Mtx* out);
C3D_FVec  N3D_Mtx_MultiplyFVec3(const C3D_Mtx* mtx, C3D_FVec v);
C3D_FVec  N3D_Mtx_MultiplyFVec4(const C3D_Mtx* mtx, C3D_FVec v);
C3D_FVec  N3D_Mtx_MultiplyFVecH(const C3D_Mtx* mtx, C3D_FVec v);
void      N3D_Mtx_FromQuat(C3D_Mtx* m, C3D_FQuat q);
void      N3D_Mtx_Translate(C3D_Mtx* mtx, float x, float y, float z, bool bRightSide);
void      N3D_Mtx_Scale(C3D_Mtx* mtx, float x, float y, float z);
void      N3D_Mtx_Rotate(C3D_Mtx* mtx, C3D_FVec axis, float angle, bool bRightSide);
void      N3D_Mtx_RotateX(C3D_Mtx* mtx, float angle, bool bRightSide);
void      N3D_Mtx_RotateY(C3D_Mtx* mtx, float angle, bool bRightSide);
void      N3D_Mtx_RotateZ(C3D_Mtx* mtx, float angle, bool bRightSide);
void      N3D_Mtx_Ortho(C3D_Mtx* mtx, float left, float right, float bottom, float top, float near, float far, bool isLeftHanded);
void      N3D_Mtx_Persp(C3D_Mtx* mtx, float fovy, float aspect, float near, float far, bool isLeftHanded);
void      N3D_Mtx_PerspStereo(C3D_Mtx* mtx, float fovy, float aspect, float near, float far, float iod, float screen, bool isLeftHanded);
void      N3D_Mtx_OrthoTilt(C3D_Mtx* mtx, float left, float right, float bottom, float top, float near, float far, bool isLeftHanded);
void      N3D_Mtx_PerspTilt(C3D_Mtx* mtx, float fovy, float aspect, float near, float far, bool isLeftHanded);
void      N3D_Mtx_PerspStereoTilt(C3D_Mtx* mtx, float fovy, float aspect, float near, float far, float iod, float screen, bool isLeftHanded);
void      N3D_Mtx_LookAt(C3D_Mtx* out, C3D_FVec cameraPosition, C3D_FVec cameraTarget, C3D_FVec cameraUpVector, bool isLeftHanded);
//#define   N3D_Quat_New(i,j,k,r)      N3D_FVec4_New(i,j,k,r)
//#define   N3D_Quat_Negate(q)         N3D_FVec4_Negate(q)
//#define   N3D_Quat_Add(lhs,rhs)      N3D_FVec4_Add(lhs,rhs)
//#define   N3D_Quat_Subtract(lhs,rhs) N3D_FVec4_Subtract(lhs,rhs)
//#define   N3D_Quat_Scale(q,s)        N3D_FVec4_Scale(q,s)
//#define   N3D_Quat_Normalize(q)      N3D_FVec4_Normalize(q)
//#define   N3D_Quat_Dot(lhs,rhs)      N3D_FVec4_Dot(lhs,rhs)
C3D_FQuat N3D_Quat_Multiply(C3D_FQuat lhs, C3D_FQuat rhs);
C3D_FQuat N3D_Quat_Pow(C3D_FQuat q, float p);
C3D_FVec  N3D_Quat_CrossFVec3(C3D_FQuat q, C3D_FVec v);
C3D_FQuat N3D_Quat_Rotate(C3D_FQuat q, C3D_FVec axis, float r, bool bRightSide);
C3D_FQuat N3D_Quat_RotateX(C3D_FQuat q, float r, bool bRightSide);
C3D_FQuat N3D_Quat_RotateY(C3D_FQuat q, float r, bool bRightSide);
C3D_FQuat N3D_Quat_RotateZ(C3D_FQuat q, float r, bool bRightSide);
C3D_FQuat N3D_Quat_FromMtx(const C3D_Mtx* m);
C3D_FQuat N3D_Quat_Identity(void);
C3D_FQuat N3D_Quat_Conjugate(C3D_FQuat q);
C3D_FQuat N3D_Quat_Inverse(C3D_FQuat q);
C3D_FVec  N3D_FVec3_CrossQuat(C3D_FVec v, C3D_FQuat q);
C3D_FQuat N3D_Quat_FromPitchYawRoll(float pitch, float yaw, float roll, bool bRightSide);
C3D_FQuat N3D_Quat_LookAt(C3D_FVec source, C3D_FVec target, C3D_FVec forwardVector, C3D_FVec upVector);
C3D_FQuat N3D_Quat_FromAxisAngle(C3D_FVec axis, float angle);

// from citro3d/include/c3d/mtxstack.h
C3D_Mtx *N3D_MtxStack_Cur(C3D_MtxStack* stk);
void     N3D_MtxStack_Init(C3D_MtxStack* stk);
void     N3D_MtxStack_Bind(C3D_MtxStack* stk, GPU_SHADER_TYPE unifType, int unifPos, int unifLen);
C3D_Mtx *N3D_MtxStack_Push(C3D_MtxStack* stk);
C3D_Mtx *N3D_MtxStack_Pop(C3D_MtxStack* stk);
void     N3D_MtxStack_Update(C3D_MtxStack* stk);

// from citro3d/include/c3d/uniforms.h
C3D_FVec *N3D_C3D_FVUnifWritePtr(GPU_SHADER_TYPE type, int id, int size);
C3D_IVec *N3D_C3D_IVUnifWritePtr(GPU_SHADER_TYPE type, int id);
void      N3D_C3D_FVUnifMtxNx4(GPU_SHADER_TYPE type, int id, const C3D_Mtx* mtx, int num);
void      N3D_C3D_FVUnifMtx4x4(GPU_SHADER_TYPE type, int id, const C3D_Mtx* mtx);
void      N3D_C3D_FVUnifMtx3x4(GPU_SHADER_TYPE type, int id, const C3D_Mtx* mtx);
void      N3D_C3D_FVUnifMtx2x4(GPU_SHADER_TYPE type, int id, const C3D_Mtx* mtx);
void      N3D_C3D_FVUnifSet(GPU_SHADER_TYPE type, int id, float x, float y, float z, float w);
void      N3D_C3D_IVUnifSet(GPU_SHADER_TYPE type, int id, int x, int y, int z, int w);
void      N3D_C3D_BoolUnifSet(GPU_SHADER_TYPE type, int id, bool value);
void      N3D_C3D_UpdateUniforms(GPU_SHADER_TYPE type);

// from citro3d/include/c3d/attribs.h
void          N3D_AttrInfo_Init(C3D_AttrInfo* info);
int           N3D_AttrInfo_AddLoader(C3D_AttrInfo* info, int regId, GPU_FORMATS format, int count);
int           N3D_AttrInfo_AddFixed(C3D_AttrInfo* info, int regId);
C3D_AttrInfo *N3D_C3D_GetAttrInfo(void);
void          N3D_C3D_SetAttrInfo(C3D_AttrInfo* info);

// from citro3d/include/c3d/buffers.h
void         N3D_BufInfo_Init(C3D_BufInfo* info);
int          N3D_BufInfo_Add(C3D_BufInfo* info, const void* data, ptrdiff_t stride, int attribCount, u64 permutation);
C3D_BufInfo *N3D_C3D_GetBufInfo(void);
void         N3D_C3D_SetBufInfo(C3D_BufInfo* info);

// from citro3d/include/c3d/base.h
float     N3D_C3D_GetCmdBufUsage(void);
void      N3D_C3D_BindProgram(shaderProgram_s* program);
void      N3D_C3D_SetViewport(u32 x, u32 y, u32 w, u32 h);
void      N3D_C3D_SetScissor(GPU_SCISSORMODE mode, u32 left, u32 top, u32 right, u32 bottom);
void      N3D_C3D_DrawArrays(GPU_Primitive_t primitive, int first, int size);
void      N3D_C3D_DrawElements(GPU_Primitive_t primitive, int count, int type, const void* indices);
void      N3D_C3D_ImmDrawBegin(GPU_Primitive_t primitive);
void      N3D_C3D_ImmSendAttrib(float x, float y, float z, float w);
void      N3D_C3D_ImmDrawEnd(void);
void      N3D_C3D_ImmDrawRestartPrim(void);
C3D_FVec *N3D_C3D_FixedAttribGetWritePtr(int id);
void      N3D_C3D_FixedAttribSet(int id, float x, float y, float z, float w);

// from citro3d/include/c3d/texenv.h
C3D_TexEnv *N3D_C3D_GetTexEnv(int id);
void        N3D_C3D_SetTexEnv(int id, C3D_TexEnv* env);
void        N3D_C3D_DirtyTexEnv(C3D_TexEnv* env);
void        N3D_C3D_TexEnvBufUpdate(int mode, int mask);
void        N3D_C3D_TexEnvBufColor(u32 color);
void        N3D_C3D_TexEnvInit(C3D_TexEnv* env);
void        N3D_C3D_TexEnvSrc(C3D_TexEnv* env, C3D_TexEnvMode mode,
	GPU_TEVSRC s1,
	GPU_TEVSRC s2 = GPU_PRIMARY_COLOR,
	GPU_TEVSRC s3 = GPU_PRIMARY_COLOR);
void        N3D_C3D_TexEnvOpRgb(C3D_TexEnv* env,
	GPU_TEVOP_RGB o1,
	GPU_TEVOP_RGB o2 = GPU_TEVOP_RGB_SRC_COLOR,
	GPU_TEVOP_RGB o3 = GPU_TEVOP_RGB_SRC_COLOR);
void        N3D_C3D_TexEnvOpAlpha(C3D_TexEnv* env,
	GPU_TEVOP_A o1,
	GPU_TEVOP_A o2 = GPU_TEVOP_A_SRC_ALPHA,
	GPU_TEVOP_A o3 = GPU_TEVOP_A_SRC_ALPHA);
void        N3D_C3D_TexEnvFunc(C3D_TexEnv* env, C3D_TexEnvMode mode, GPU_COMBINEFUNC param);
void        N3D_C3D_TexEnvColor(C3D_TexEnv* env, u32 color);
void        N3D_C3D_TexEnvScale(C3D_TexEnv* env, int mode, GPU_TEVSCALE param);

// from citro3d/include/c3d/effect.h
void N3D_C3D_FragOpShadow(float scale, float bias);

// from citro3d/include/c3d/texture.h
bool                   N3D_C3D_TexInitWithParams(C3D_Tex* tex, C3D_TexCube* cube, C3D_TexInitParams p);
void                   N3D_C3D_TexLoadImage(C3D_Tex* tex, const void* data, GPU_TEXFACE face, int level);
void                   N3D_C3D_TexGenerateMipmap(C3D_Tex* tex, GPU_TEXFACE face);
void                   N3D_C3D_TexBind(int unitId, C3D_Tex* tex);
void                   N3D_C3D_TexFlush(C3D_Tex* tex);
void                   N3D_C3D_TexDelete(C3D_Tex* tex);
void                   N3D_C3D_TexShadowParams(bool perspective, float bias);
int                    N3D_C3D_TexCalcMaxLevel(u32 width, u32 height);
u32                    N3D_C3D_TexCalcLevelSize(u32 size, int level);
u32                    N3D_C3D_TexCalcTotalSize(u32 size, int maxLevel);
bool                   N3D_C3D_TexInit(C3D_Tex* tex, u16 width, u16 height, GPU_TEXCOLOR format);
bool                   N3D_C3D_TexInitMipmap(C3D_Tex* tex, u16 width, u16 height, GPU_TEXCOLOR format);
bool                   N3D_C3D_TexInitCube(C3D_Tex* tex, C3D_TexCube* cube, u16 width, u16 height, GPU_TEXCOLOR format);
bool                   N3D_C3D_TexInitVRAM(C3D_Tex* tex, u16 width, u16 height, GPU_TEXCOLOR format);
bool                   N3D_C3D_TexInitShadow(C3D_Tex* tex, u16 width, u16 height);
bool                   N3D_C3D_TexInitShadowCube(C3D_Tex* tex, C3D_TexCube* cube, u16 width, u16 height);
GPU_TEXTURE_MODE_PARAM N3D_C3D_TexGetType(C3D_Tex* tex);
void                  *N3D_C3D_TexGetImagePtr(C3D_Tex* tex, void* data, int level, u32* size);
void                  *N3D_C3D_Tex2DGetImagePtr(C3D_Tex* tex, int level, u32* size);
void                  *N3D_C3D_TexCubeGetImagePtr(C3D_Tex* tex, GPU_TEXFACE face, int level, u32* size);
void                   N3D_C3D_TexUpload(C3D_Tex* tex, const void* data);
void                   N3D_C3D_TexSetFilter(C3D_Tex* tex, GPU_TEXTURE_FILTER_PARAM magFilter, GPU_TEXTURE_FILTER_PARAM minFilter);
void                   N3D_C3D_TexSetFilterMipmap(C3D_Tex* tex, GPU_TEXTURE_FILTER_PARAM filter);
void                   N3D_C3D_TexSetWrap(C3D_Tex* tex, GPU_TEXTURE_WRAP_PARAM wrapS, GPU_TEXTURE_WRAP_PARAM wrapT);
void                   N3D_C3D_TexSetLodBias(C3D_Tex* tex, float lodBias);

// from citro3d/include/c3d/proctex.
// from citro3d/include/c3d/light.h
// from citro3d/include/c3d/lughtlut.h
// from citro3d/include/c3d/fog.h

// from citro3d/include/c3d/framebuffer.h
u32           N3D_C3D_CalcColorBufSize(u32 width, u32 height, GPU_COLORBUF fmt);
u32           N3D_C3D_CalcDepthBufSize(u32 width, u32 height, GPU_DEPTHBUF fmt);
C3D_FrameBuf *N3D_C3D_GetFrameBuf(void);
void          N3D_C3D_SetFrameBuf(C3D_FrameBuf* fb);
void          N3D_C3D_FrameBufTex(C3D_FrameBuf* fb, C3D_Tex* tex, GPU_TEXFACE face, int level);
void          N3D_C3D_FrameBufClear(C3D_FrameBuf* fb, C3D_ClearBits clearBits, u32 clearColor, u32 clearDepth);
void          N3D_C3D_FrameBufTransfer(C3D_FrameBuf* fb, gfxScreen_t screen, gfx3dSide_t side, u32 transferFlags);
void          N3D_C3D_FrameBufAttrib(C3D_FrameBuf* fb, u16 width, u16 height, bool block32);
void          N3D_C3D_FrameBufColor(C3D_FrameBuf* fb, void* buf, GPU_COLORBUF fmt);
void          N3D_C3D_FrameBufDepth(C3D_FrameBuf* fb, void* buf, GPU_DEPTHBUF fmt);

// from citro3d/include/c3d/renderqueue.h
float             N3D_C3D_FrameRate(float fps);
void              N3D_C3D_FrameSync(void);
u32               N3D_C3D_FrameCounter(int id);
bool              N3D_C3D_FrameBegin(u8 flags);
bool              N3D_C3D_FrameDrawOn(C3D_RenderTarget* target);
void              N3D_C3D_FrameSplit(u8 flags);
void              N3D_C3D_FrameEnd(u8 flags);
void              N3D_C3D_FrameEndHook(void (* hook)(void*), void* param);
float             N3D_C3D_GetDrawingTime(void);
float             N3D_C3D_GetProcessingTime(void);
C3D_RenderTarget *N3D_C3D_RenderTargetCreate(int width, int height, GPU_COLORBUF colorFmt, C3D_DEPTHTYPE depthFmt);
C3D_RenderTarget *N3D_C3D_RenderTargetCreateFromTex(C3D_Tex* tex, GPU_TEXFACE face, int level, C3D_DEPTHTYPE depthFmt);
void              N3D_C3D_RenderTargetDelete(C3D_RenderTarget* target);
void              N3D_C3D_RenderTargetSetOutput(C3D_RenderTarget* target, gfxScreen_t screen, gfx3dSide_t side, u32 transferFlags);
void              N3D_C3D_RenderTargetDetachOutput(C3D_RenderTarget* target);
void              N3D_C3D_RenderTargetClear(C3D_RenderTarget* target, C3D_ClearBits clearBits, u32 clearColor, u32 clearDepth);
void              N3D_C3D_SyncDisplayTransfer(u32* inadr, u32 indim, u32* outadr, u32 outdim, u32 flags);
void              N3D_C3D_SyncTextureCopy(u32* inadr, u32 indim, u32* outadr, u32 outdim, u32 size, u32 flags);
void              N3D_C3D_SyncMemoryFill(u32* buf0a, u32 buf0v, u32* buf0e, u16 control0, u32* buf1a, u32 buf1v, u32* buf1e, u16 control1);

#endif // GRAPHICS_3DS_NGL_H