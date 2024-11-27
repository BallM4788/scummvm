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

// from "graphics/3ds/ops-gl2citro.cpp"
void opViewport(u32 x, u32 y, u32 w, u32 h);
void opCullFace(N3D_CULLFACE mode);
void opFrontFace(N3D_FRONTFACE mode);
void opCullFaceEnabled(bool state);
void opPolygonOffset(float pOffUnits);
void opWScale(float wScale);
void opDepthRange(float near, float far);
void opPolygonOffsetEnabled(bool state);
void opScissorMode(GPU_SCISSORMODE mode);
void opScissorDims(u32 x, u32 y, u32 w, u32 h);
void opAlphaFunc(GPU_TESTFUNC func, int ref);
void opAlphaTestEnabled(bool state);
void opStencilFunc(GPU_TESTFUNC func, int ref, int mask);
void opStencilMask(int writeMask);
void opStencilOp(GPU_STENCILOP fail, GPU_STENCILOP zfail, GPU_STENCILOP zpass);
// CLEAR STENCIL????????????
void opStencilTestEnabled(bool state);
void opDepthFunc(GPU_TESTFUNC func);
void opDepthMask(bool depthState);
void opColorMask(bool redState, bool greenState, bool blueState, bool alphaState);
// CLEAR DEPTH?????????????
void opDepthTestEnabled(bool state);
void opEarlyDepthFunc(GPU_EARLYDEPTHFUNC func);
void opClearEarlyDepth(u32 clearValue);
void opEarlyDepthTestEnabled(bool state);
void opBlendEquation(GPU_BLENDEQUATION equation);
void opBlendEquationSeparate(GPU_BLENDEQUATION colorEq, GPU_BLENDEQUATION alphaEq);
void opBlendFunc(GPU_BLENDFACTOR srcFactor, GPU_BLENDFACTOR dstFactor);
void opBlendFuncSeparate(GPU_BLENDFACTOR srcColor, GPU_BLENDFACTOR dstColor, GPU_BLENDFACTOR srcAlpha, GPU_BLENDFACTOR dstAlpha);
void opBlendColor(u8 red, u8 green, u8 blue, u8 alpha);
void opBlendEnabled(bool state);
// CLEAR COLOR???????????????
void opLogicOp(GPU_LOGICOP op);
void opColorLogicOpEnabled(bool state);


// CUSTOM FUNCS
C3D_Tex *opGetGameScreen();
void     opArbDataToArbBlockTexOffset(u32 *srcBuf, u32 *dstBuf, int copyWidth, int copyHeight,
                                      int xSource, int ySource, int wSource,   int hSource,
                                      int xDest,   int yDest,   int wDest,     int hDest,
                                      GPU_TEXCOLOR format, int scale, bool isBlockSrc);
void     opDataToBlockTex(u32 *srcBuf, u32 *dstBuf, int xSource, int ySource,
                          int wSource, int hSource, int wDest, int hDest,
                          GPU_TEXCOLOR format, int scale, bool isBlockSrc);

// from citro3d/include/c3d/maths.h
C3D_FVec  opFVec4_New(float x, float y, float z, float w);
C3D_FVec  opFVec4_Subtract(C3D_FVec lhs, C3D_FVec rhs);
C3D_FVec  opFVec4_Negate(C3D_FVec v);
C3D_FVec  opFVec4_Scale(C3D_FVec v, float s);
C3D_FVec  opFVec4_PerspDivide(C3D_FVec v);
float     opFVec4_Dot(C3D_FVec lhs, C3D_FVec rhs);
float     opFVec4_Magnitude(C3D_FVec v);
C3D_FVec  opFVec4_Normalize(C3D_FVec v);
C3D_FVec  opFVec3_New(float x, float y, float z);
float     opFVec3_Dot(C3D_FVec lhs, C3D_FVec rhs);
float     opFVec3_Magnitude(C3D_FVec v);
C3D_FVec  opFVec3_Normalize(C3D_FVec v);
C3D_FVec  opFVec3_Add(C3D_FVec lhs, C3D_FVec rhs);
C3D_FVec  opFVec3_Subtract(C3D_FVec lhs, C3D_FVec rhs);
float     opFVec3_Distance(C3D_FVec lhs, C3D_FVec rhs);
C3D_FVec  opFVec3_Scale(C3D_FVec v, float s);
C3D_FVec  opFVec3_Negate(C3D_FVec v);
C3D_FVec  opFVec3_Cross(C3D_FVec lhs, C3D_FVec rhs);
void      opMtx_Zeros(C3D_Mtx* out);
void      opMtx_Copy(C3D_Mtx* out, const C3D_Mtx* in);
void      opMtx_Diagonal(C3D_Mtx* out, float x, float y, float z, float w);
void      opMtx_Identity(C3D_Mtx* out);
void      opMtx_Transpose(C3D_Mtx* out);
void      opMtx_Add(C3D_Mtx* out, const C3D_Mtx* lhs, const C3D_Mtx* rhs);
void      opMtx_Subtract(C3D_Mtx* out, const C3D_Mtx* lhs, const C3D_Mtx* rhs);
void      opMtx_Multiply(C3D_Mtx* out, const C3D_Mtx* a, const C3D_Mtx* b);
float     opMtx_Inverse(C3D_Mtx* out);
C3D_FVec  opMtx_MultiplyFVec3(const C3D_Mtx* mtx, C3D_FVec v);
C3D_FVec  opMtx_MultiplyFVec4(const C3D_Mtx* mtx, C3D_FVec v);
C3D_FVec  opMtx_MultiplyFVecH(const C3D_Mtx* mtx, C3D_FVec v);
void      opMtx_FromQuat(C3D_Mtx* m, C3D_FQuat q);
void      opMtx_Translate(C3D_Mtx* mtx, float x, float y, float z, bool bRightSide);
void      opMtx_Scale(C3D_Mtx* mtx, float x, float y, float z);
void      opMtx_Rotate(C3D_Mtx* mtx, C3D_FVec axis, float angle, bool bRightSide);
void      opMtx_RotateX(C3D_Mtx* mtx, float angle, bool bRightSide);
void      opMtx_RotateY(C3D_Mtx* mtx, float angle, bool bRightSide);
void      opMtx_RotateZ(C3D_Mtx* mtx, float angle, bool bRightSide);
void      opMtx_Ortho(C3D_Mtx* mtx, float left, float right, float bottom, float top, float near, float far, bool isLeftHanded);
void      opMtx_Persp(C3D_Mtx* mtx, float fovy, float aspect, float near, float far, bool isLeftHanded);
void      opMtx_PerspStereo(C3D_Mtx* mtx, float fovy, float aspect, float near, float far, float iod, float screen, bool isLeftHanded);
void      opMtx_OrthoTilt(C3D_Mtx* mtx, float left, float right, float bottom, float top, float near, float far, bool isLeftHanded);
void      opMtx_PerspTilt(C3D_Mtx* mtx, float fovy, float aspect, float near, float far, bool isLeftHanded);
void      opMtx_PerspStereoTilt(C3D_Mtx* mtx, float fovy, float aspect, float near, float far, float iod, float screen, bool isLeftHanded);
void      opMtx_LookAt(C3D_Mtx* out, C3D_FVec cameraPosition, C3D_FVec cameraTarget, C3D_FVec cameraUpVector, bool isLeftHanded);
//#define   opQuat_New(i,j,k,r)      opFVec4_New(i,j,k,r)
//#define   opQuat_Negate(q)         opFVec4_Negate(q)
//#define   opQuat_Add(lhs,rhs)      opFVec4_Add(lhs,rhs)
//#define   opQuat_Subtract(lhs,rhs) opFVec4_Subtract(lhs,rhs)
//#define   opQuat_Scale(q,s)        opFVec4_Scale(q,s)
//#define   opQuat_Normalize(q)      opFVec4_Normalize(q)
//#define   opQuat_Dot(lhs,rhs)      opFVec4_Dot(lhs,rhs)
C3D_FQuat opQuat_Multiply(C3D_FQuat lhs, C3D_FQuat rhs);
C3D_FQuat opQuat_Pow(C3D_FQuat q, float p);
C3D_FVec  opQuat_CrossFVec3(C3D_FQuat q, C3D_FVec v);
C3D_FQuat opQuat_Rotate(C3D_FQuat q, C3D_FVec axis, float r, bool bRightSide);
C3D_FQuat opQuat_RotateX(C3D_FQuat q, float r, bool bRightSide);
C3D_FQuat opQuat_RotateY(C3D_FQuat q, float r, bool bRightSide);
C3D_FQuat opQuat_RotateZ(C3D_FQuat q, float r, bool bRightSide);
C3D_FQuat opQuat_FromMtx(const C3D_Mtx* m);
C3D_FQuat opQuat_Identity(void);
C3D_FQuat opQuat_Conjugate(C3D_FQuat q);
C3D_FQuat opQuat_Inverse(C3D_FQuat q);
C3D_FVec  opFVec3_CrossQuat(C3D_FVec v, C3D_FQuat q);
C3D_FQuat opQuat_FromPitchYawRoll(float pitch, float yaw, float roll, bool bRightSide);
C3D_FQuat opQuat_LookAt(C3D_FVec source, C3D_FVec target, C3D_FVec forwardVector, C3D_FVec upVector);
C3D_FQuat opQuat_FromAxisAngle(C3D_FVec axis, float angle);

// from citro3d/include/c3d/mtxstack.h
C3D_Mtx *opMtxStack_Cur(C3D_MtxStack* stk);
void     opMtxStack_Init(C3D_MtxStack* stk);
void     opMtxStack_Bind(C3D_MtxStack* stk, GPU_SHADER_TYPE unifType, int unifPos, int unifLen);
C3D_Mtx *opMtxStack_Push(C3D_MtxStack* stk);
C3D_Mtx *opMtxStack_Pop(C3D_MtxStack* stk);
void     opMtxStack_Update(C3D_MtxStack* stk);

// from citro3d/include/c3d/uniforms.h
C3D_FVec *opC3D_FVUnifWritePtr(GPU_SHADER_TYPE type, int id, int size);
C3D_IVec *opC3D_IVUnifWritePtr(GPU_SHADER_TYPE type, int id);
void      opC3D_FVUnifMtxNx4(GPU_SHADER_TYPE type, int id, const C3D_Mtx* mtx, int num);
void      opC3D_FVUnifMtx4x4(GPU_SHADER_TYPE type, int id, const C3D_Mtx* mtx);
void      opC3D_FVUnifMtx3x4(GPU_SHADER_TYPE type, int id, const C3D_Mtx* mtx);
void      opC3D_FVUnifMtx2x4(GPU_SHADER_TYPE type, int id, const C3D_Mtx* mtx);
void      opC3D_FVUnifSet(GPU_SHADER_TYPE type, int id, float x, float y, float z, float w);
void      opC3D_IVUnifSet(GPU_SHADER_TYPE type, int id, int x, int y, int z, int w);
void      opC3D_BoolUnifSet(GPU_SHADER_TYPE type, int id, bool value);
void      opC3D_UpdateUniforms(GPU_SHADER_TYPE type);

// from citro3d/include/c3d/attribs.h
void          opAttrInfo_Init(C3D_AttrInfo* info);
int           opAttrInfo_AddLoader(C3D_AttrInfo* info, int regId, GPU_FORMATS format, int count);
int           opAttrInfo_AddFixed(C3D_AttrInfo* info, int regId);
C3D_AttrInfo *opC3D_GetAttrInfo(void);
void          opC3D_SetAttrInfo(C3D_AttrInfo* info);

// from citro3d/include/c3d/buffers.h
void         opBufInfo_Init(C3D_BufInfo* info);
int          opBufInfo_Add(C3D_BufInfo* info, const void* data, ptrdiff_t stride, int attribCount, u64 permutation);
C3D_BufInfo *opC3D_GetBufInfo(void);
void         opC3D_SetBufInfo(C3D_BufInfo* info);

// from citro3d/include/c3d/base.h
float     opC3D_GetCmdBufUsage(void);
void      opC3D_BindProgram(shaderProgram_s* program);
void      opC3D_SetViewport(u32 x, u32 y, u32 w, u32 h);
void      opC3D_SetScissor(GPU_SCISSORMODE mode, u32 left, u32 top, u32 right, u32 bottom);
void      opC3D_DrawArrays(GPU_Primitive_t primitive, int first, int size);
void      opC3D_DrawElements(GPU_Primitive_t primitive, int count, int type, const void* indices);
void      opC3D_ImmDrawBegin(GPU_Primitive_t primitive);
void      opC3D_ImmSendAttrib(float x, float y, float z, float w);
void      opC3D_ImmDrawEnd(void);
void      opC3D_ImmDrawRestartPrim(void);
C3D_FVec *opC3D_FixedAttribGetWritePtr(int id);
void      opC3D_FixedAttribSet(int id, float x, float y, float z, float w);

// from citro3d/include/c3d/texenv.h
C3D_TexEnv *opC3D_GetTexEnv(int id);
void        opC3D_SetTexEnv(int id, C3D_TexEnv* env);
void        opC3D_DirtyTexEnv(C3D_TexEnv* env);
void        opC3D_TexEnvBufUpdate(int mode, int mask);
void        opC3D_TexEnvBufColor(u32 color);
void        opC3D_TexEnvInit(C3D_TexEnv* env);
void        opC3D_TexEnvSrc(C3D_TexEnv* env, C3D_TexEnvMode mode,
	GPU_TEVSRC s1,
	GPU_TEVSRC s2 = GPU_PRIMARY_COLOR,
	GPU_TEVSRC s3 = GPU_PRIMARY_COLOR);
void        opC3D_TexEnvOpRgb(C3D_TexEnv* env,
	GPU_TEVOP_RGB o1,
	GPU_TEVOP_RGB o2 = GPU_TEVOP_RGB_SRC_COLOR,
	GPU_TEVOP_RGB o3 = GPU_TEVOP_RGB_SRC_COLOR);
void        opC3D_TexEnvOpAlpha(C3D_TexEnv* env,
	GPU_TEVOP_A o1,
	GPU_TEVOP_A o2 = GPU_TEVOP_A_SRC_ALPHA,
	GPU_TEVOP_A o3 = GPU_TEVOP_A_SRC_ALPHA);
void        opC3D_TexEnvFunc(C3D_TexEnv* env, C3D_TexEnvMode mode, GPU_COMBINEFUNC param);
void        opC3D_TexEnvColor(C3D_TexEnv* env, u32 color);
void        opC3D_TexEnvScale(C3D_TexEnv* env, int mode, GPU_TEVSCALE param);

// from citro3d/include/c3d/effect.h
void opC3D_FragOpShadow(float scale, float bias);

// from citro3d/include/c3d/texture.h
bool                   opC3D_TexInitWithParams(C3D_Tex* tex, C3D_TexCube* cube, C3D_TexInitParams p);
void                   opC3D_TexLoadImage(C3D_Tex* tex, const void* data, GPU_TEXFACE face, int level);
void                   opC3D_TexGenerateMipmap(C3D_Tex* tex, GPU_TEXFACE face);
void                   opC3D_TexBind(int unitId, C3D_Tex* tex);
void                   opC3D_TexFlush(C3D_Tex* tex);
void                   opC3D_TexDelete(C3D_Tex* tex);
void                   opC3D_TexShadowParams(bool perspective, float bias);
int                    opC3D_TexCalcMaxLevel(u32 width, u32 height);
u32                    opC3D_TexCalcLevelSize(u32 size, int level);
u32                    opC3D_TexCalcTotalSize(u32 size, int maxLevel);
bool                   opC3D_TexInit(C3D_Tex* tex, u16 width, u16 height, GPU_TEXCOLOR format);
bool                   opC3D_TexInitMipmap(C3D_Tex* tex, u16 width, u16 height, GPU_TEXCOLOR format);
bool                   opC3D_TexInitCube(C3D_Tex* tex, C3D_TexCube* cube, u16 width, u16 height, GPU_TEXCOLOR format);
bool                   opC3D_TexInitVRAM(C3D_Tex* tex, u16 width, u16 height, GPU_TEXCOLOR format);
bool                   opC3D_TexInitShadow(C3D_Tex* tex, u16 width, u16 height);
bool                   opC3D_TexInitShadowCube(C3D_Tex* tex, C3D_TexCube* cube, u16 width, u16 height);
GPU_TEXTURE_MODE_PARAM opC3D_TexGetType(C3D_Tex* tex);
void                  *opC3D_TexGetImagePtr(C3D_Tex* tex, void* data, int level, u32* size);
void                  *opC3D_Tex2DGetImagePtr(C3D_Tex* tex, int level, u32* size);
void                  *opC3D_TexCubeGetImagePtr(C3D_Tex* tex, GPU_TEXFACE face, int level, u32* size);
void                   opC3D_TexUpload(C3D_Tex* tex, const void* data);
void                   opC3D_TexSetFilter(C3D_Tex* tex, GPU_TEXTURE_FILTER_PARAM magFilter, GPU_TEXTURE_FILTER_PARAM minFilter);
void                   opC3D_TexSetFilterMipmap(C3D_Tex* tex, GPU_TEXTURE_FILTER_PARAM filter);
void                   opC3D_TexSetWrap(C3D_Tex* tex, GPU_TEXTURE_WRAP_PARAM wrapS, GPU_TEXTURE_WRAP_PARAM wrapT);
void                   opC3D_TexSetLodBias(C3D_Tex* tex, float lodBias);

// from citro3d/include/c3d/proctex.
// from citro3d/include/c3d/light.h
// from citro3d/include/c3d/lughtlut.h
// from citro3d/include/c3d/fog.h

// from citro3d/include/c3d/framebuffer.h
u32           opC3D_CalcColorBufSize(u32 width, u32 height, GPU_COLORBUF fmt);
u32           opC3D_CalcDepthBufSize(u32 width, u32 height, GPU_DEPTHBUF fmt);
C3D_FrameBuf *opC3D_GetFrameBuf(void);
void          opC3D_SetFrameBuf(C3D_FrameBuf* fb);
void          opC3D_FrameBufTex(C3D_FrameBuf* fb, C3D_Tex* tex, GPU_TEXFACE face, int level);
void          opC3D_FrameBufClear(C3D_FrameBuf* fb, C3D_ClearBits clearBits, u32 clearColor, u32 clearDepth);
void          opC3D_FrameBufTransfer(C3D_FrameBuf* fb, gfxScreen_t screen, gfx3dSide_t side, u32 transferFlags);
void          opC3D_FrameBufAttrib(C3D_FrameBuf* fb, u16 width, u16 height, bool block32);
void          opC3D_FrameBufColor(C3D_FrameBuf* fb, void* buf, GPU_COLORBUF fmt);
void          opC3D_FrameBufDepth(C3D_FrameBuf* fb, void* buf, GPU_DEPTHBUF fmt);

// from citro3d/include/c3d/renderqueue.h
float             opC3D_FrameRate(float fps);
void              opC3D_FrameSync(void);
u32               opC3D_FrameCounter(int id);
bool              opC3D_FrameBegin(u8 flags);
bool              opC3D_FrameDrawOn(C3D_RenderTarget* target);
void              opC3D_FrameSplit(u8 flags);
void              opC3D_FrameEnd(u8 flags);
void              opC3D_FrameEndHook(void (* hook)(void*), void* param);
float             opC3D_GetDrawingTime(void);
float             opC3D_GetProcessingTime(void);
C3D_RenderTarget *opC3D_RenderTargetCreate(int width, int height, GPU_COLORBUF colorFmt, C3D_DEPTHTYPE depthFmt);
C3D_RenderTarget *opC3D_RenderTargetCreateFromTex(C3D_Tex* tex, GPU_TEXFACE face, int level, C3D_DEPTHTYPE depthFmt);
void              opC3D_RenderTargetDelete(C3D_RenderTarget* target);
void              opC3D_RenderTargetSetOutput(C3D_RenderTarget* target, gfxScreen_t screen, gfx3dSide_t side, u32 transferFlags);
void              opC3D_RenderTargetDetachOutput(C3D_RenderTarget* target);
void              opC3D_RenderTargetClear(C3D_RenderTarget* target, C3D_ClearBits clearBits, u32 clearColor, u32 clearDepth);
void              opC3D_SyncDisplayTransfer(u32* inadr, u32 indim, u32* outadr, u32 outdim, u32 flags);
void              opC3D_SyncTextureCopy(u32* inadr, u32 indim, u32* outadr, u32 outdim, u32 size, u32 flags);
void              opC3D_SyncMemoryFill(u32* buf0a, u32 buf0v, u32* buf0e, u16 control0, u32* buf1a, u32 buf1v, u32* buf1e, u16 control1);