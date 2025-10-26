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

// Matrix calculations taken from the glm library
// Which is covered by the MIT license
// And has this additional copyright note:
/* Copyright (c) 2005 - 2012 G-Truc Creation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 */

#if defined(__3DS__)
#include "graphics/3ds/n3d.h"

#include "common/endian.h"
#include "common/file.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"

// #if defined(USE_OPENGL_SHADERS)

#include "graphics/surface.h"
// #include "graphics/opengl/context.h"

#include "engines/grim/actor.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/colormap.h"
#include "engines/grim/emi/modelemi.h"
#include "engines/grim/font.h"

// #include "engines/grim/gfx_opengl_shaders.h"
#include "engines/grim/gfx_n3ds.h"
#include "engines/grim/grim.h"
#include "engines/grim/material.h"
#include "engines/grim/model.h"
#include "engines/grim/primitives.h"
#include "engines/grim/set.h"
#include "engines/grim/sprite.h"

#include "engines/grim/shaders-3ds/emi_background_shbin.h"
#include "engines/grim/shaders-3ds/grim_smush_shbin.h"
#include "engines/grim/shaders-3ds/grim_text_shbin.h"
#include "backends/platform/3ds/shaders/common_manualClear_shbin.h"

namespace Grim {


#define CONSTRUCT_SHADER(game, shaderName, geomStride) \
	shaderName##DVLB = DVLB_ParseFile((u32*)const_cast<u8 *>(game##shaderName##_shbin), game##shaderName##_shbin_size); \
	shaderProgramInit(&shaderName##Program); \
	Result shaderName##vsh = shaderProgramSetVsh(&shaderName##Program, &shaderName##DVLB->DVLE[0]); \
	Result shaderName##gsh = shaderProgramSetGsh(&shaderName##Program, &shaderName##DVLB->DVLE[1], geomStride); \
	shaderName##ProgramFlags = ((shaderName##gsh == 0) << 1) | (shaderName##vsh == 0); \
	shaderName##Shader = new N3DS_3D::ShaderObj(&shaderName##Program, shaderName##ProgramFlags);
#define DECOMPOSE_SHADER(shaderName) \
	delete shaderName##Shader; \
	shaderProgramFree(&shaderName##Program); \
	DVLB_Free(shaderName##DVLB);


void GfxN3DS::drawStart(u8 flags, u32 x, u32 y, u32 w, u32 h) {
	if (!_inFrame) {
		N3D_C3D_FrameBegin(flags);
		N3D_C3D_FrameDrawOn(_gameScreenTarget);
		N3D_C3D_SetViewport(x, y, w, h);
		_inFrame = true;
	}
}

void GfxN3DS::drawEnd(u8 flags) {
	N3D_C3D_FrameEnd(flags);
	_inFrame = false;
}




template<class T>
static T nextHigher2(T k) {
	if (k == 0)
		return 1;
	--k;

	for (uint i = 1; i < sizeof(T) * 8; i <<= 1)
		k = k | k >> i;

	return k + 1;
}

static float untextured_quad[] = {
//	X   , Y
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 1.0f,
};

static float textured_quad[] = {
//	X   , Y   , S   , T
	0.0f, 0.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
};

static float textured_quad_centered[] = {
//	 X   ,  Y   , Z   , S   , T
	-0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
	-0.5f, +0.5f, 0.0f, 0.0f, 0.0f,
	+0.5f, +0.5f, 0.0f, 1.0f, 0.0f,
	+0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
};

static float zero_texVerts[] = { 0.0, 0.0 };

struct GrimVertex {
	GrimVertex(const float *verts, const float *texVerts, const float *normals) {
		memcpy(_position, verts, 3 * sizeof(float));
		memcpy(_texcoord, texVerts, 2 * sizeof(float));
		memcpy(_normal, normals, 3 * sizeof(float));
	}
	float _position[3];
	float _texcoord[2];
	float _normal[3];
};

struct TextUserData {
	uint32   characters;
	Color    color;
	C3D_Tex *texture;
	void    *vbo;
};

struct FontUserData {
	int size;
	C3D_Tex texture;
	Tex3DS_SubTexture *subTextures;
};

struct EMIModelUserData {
//	OpenGL::Shader *_shader;
//	OpenGL::Shader *_shaderLights;
//	uint32 _texCoordsVBO;
//	uint32 _colorMapVBO;
//	uint32 _verticesVBO;
//	uint32 _normalsVBO;
	N3DS_3D::ShaderObj *_shader;
	N3DS_3D::ShaderObj *_shaderLights;
	void  *_texCoordsVBO;
	void  *_colorMapVBO;
	void  *_verticesVBO;
	void  *_normalsVBO;
};

struct ModelUserData {
//	OpenGL::Shader *_shader;
//	OpenGL::Shader *_shaderLights;
//	uint32 _meshInfoVBO;
	N3DS_3D::ShaderObj *_shader;
	N3DS_3D::ShaderObj *_shaderLights;
	void *_meshInfoVBO;
};

struct ShadowUserData {
//	uint32 _verticesVBO;
//	uint32 _indicesVBO;
	void *_verticesVBO;
	void *_indicesVBO;
	uint32 _numTriangles;
};

Math::Matrix4 makeLookMatrix(const Math::Vector3d& pos, const Math::Vector3d& interest, const Math::Vector3d& up) {
	Math::Vector3d f = (interest - pos).getNormalized();
	Math::Vector3d u = up.getNormalized();
	Math::Vector3d s = Math::Vector3d::crossProduct(f, u).getNormalized();
	u = Math::Vector3d::crossProduct(s, f);

	Math::Matrix4 look;
	look(0, 0) = s.x();
	look(1, 0) = s.y();
	look(2, 0) = s.z();
	look(0, 1) = u.x();
	look(1, 1) = u.y();
	look(2, 1) = u.z();
	look(0, 2) = -f.x();
	look(1, 2) = -f.y();
	look(2, 2) = -f.z();
	look(3, 0) = -Math::Vector3d::dotProduct(s, pos);
	look(3, 1) = -Math::Vector3d::dotProduct(u, pos);
	look(3, 2) =  Math::Vector3d::dotProduct(f, pos);

	look.transpose();

	// C3D_Mtx look = Mtx_LookAt(pos, interest, up, false);
	// Mtx_Transpose(&look);

	return look;
}

Math::Matrix4 makeRotationMatrix(const Math::Angle& angle, Math::Vector3d axis) {
	float c = angle.getCosine();
	float s = angle.getSine();
	axis.normalize();
	Math::Vector3d temp = (1.f - c) * axis;
	Math::Matrix4 rotate;
	rotate(0, 0) = c + temp.x() * axis.x();
	rotate(0, 1) = temp.x() * axis.y() + s * axis.z();
	rotate(0, 2) = temp.x() * axis.z() - s * axis.y();
	rotate(0, 3) = 0;
	rotate(1, 0) = temp.y() * axis.x() - s * axis.z();
	rotate(1, 1) = c + temp.y() * axis.y();
	rotate(1, 2) = temp.y() * axis.z() + s * axis.x();
	rotate(1, 3) = 0;
	rotate(2, 0) = temp.z() * axis.x() + s * axis.y();
	rotate(2, 1) = temp.z() * axis.y() - s * axis.x();
	rotate(2, 2) = c + temp.z() * axis.z();
	rotate(2, 3) = 0;
	rotate(3, 0) = 0;
	rotate(3, 1) = 0;
	rotate(3, 2) = 0;
	rotate(3, 3) = 1;

	return rotate;
}

Math::Matrix4 makeFrustumMatrix(double left, double right, double bottom, double top, double nclip, double fclip) {
	Math::Matrix4 proj;
	// (row, col), except each row is a col and vice versa
	proj(0, 0) = (2.0f * nclip) / (right - left);
	proj(1, 1) = (2.0f * nclip) / (top - bottom);
	proj(2, 0) = (right + left) / (right - left);
	proj(2, 1) = (top + bottom) / (top - bottom);
//	proj(2, 2) = -(fclip + nclip) / (fclip - nclip);
	proj(2, 2) = (fclip) / (fclip - nclip);																							// DEFINITE? - MODIFIED
	proj(2, 3) = -1.0f;
//	proj(3, 2) = -(2.0f * fclip * nclip) / (fclip - nclip);
	proj(3, 2) = (fclip * nclip) / (fclip - nclip);																					// DEFINITE? - MODIFIED
	proj(3, 3) = 0.0f;

	return proj;
}

GfxBase *CreateGfxN3DS() {
	return new GfxN3DS();
}

GfxN3DS::GfxN3DS() : _alpha(1.f),
//                     _smushTexId(0),
                     _fov(-1.0),
                     _nclip(-1),
                     _fclip(-1),
                     _selectedTexture(nullptr),
//                     _emergTexture(0),
                     _maxLights(8),
//                     _lights(new GLSLight[_maxLights]),
                     _lights(new LightObj[_maxLights]),
                     _lightsEnabled(false),
                     _hasAmbientLight(false),
                     _backgroundShader(nullptr),
                     _smushShader(nullptr),
                     _textShader(nullptr),
                     _emergShader(nullptr),
                     _actorShader(nullptr),
                     _actorLightsShader(nullptr),
                     _spriteShader(nullptr),
                     _primRectShader(nullptr),
                     _primLinesShader(nullptr),
//                     _irisProgram(nullptr),
                     _irisShader(nullptr),
                     _shadowPlaneShader(nullptr),
                     _dimShader(nullptr),
                     _dimPlaneShader(nullptr),
//                     _dimRegionProgram(nullptr),
                     _dimRegionShader(nullptr),
                     _manualClearShader(nullptr),
                     _srcBlend(GPU_SRC_ALPHA),     _dstBlend(GPU_ONE_MINUS_SRC_ALPHA),
                     _inFrame(false),
                     _activeTextObject(nullptr)
 {

	// Create context that corresponds to the Citro3D setting of the 3DS backend.
	_backendContext = N3DS_3D::createContext();
	// Create context that corresponds to Open GL settings to be used for GRIM engine rendering.
	_grimContext = N3DS_3D::createOGLContext();

	_gameScreenTex = N3D_GetGameScreen();
	_gameScreenTarget = N3D_C3D_RenderTargetCreateFromTex(_gameScreenTex, GPU_TEXFACE_2D, 0, GPU_RB_DEPTH24_STENCIL8);

	// Create default texEnv corresponding to OpenGL's starting configuration.
	N3D_C3D_TexEnvInit(&envGRIMDefault);
	N3D_C3D_TexEnvFunc(&envGRIMDefault, C3D_Both, GPU_REPLACE);
	N3D_C3D_TexEnvSrc(&envGRIMDefault, C3D_Both, GPU_PRIMARY_COLOR/*, GPU_PRIMARY_COLOR, GPU_CONSTANT*/);
	N3D_C3D_TexEnvOpRgb(&envGRIMDefault, GPU_TEVOP_RGB_SRC_COLOR/*, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_ALPHA*/);
	N3D_C3D_TexEnvOpAlpha(&envGRIMDefault, GPU_TEVOP_A_SRC_ALPHA/*, GPU_TEVOP_A_SRC_ALPHA, GPU_TEVOP_A_SRC_ALPHA*/);

	// Create texEnv for backgrounds and videos.
	N3D_C3D_TexEnvInit(&envBG_Smush);
	N3D_C3D_TexEnvFunc(&envBG_Smush, C3D_Both, GPU_REPLACE);
	N3D_C3D_TexEnvSrc(&envBG_Smush, C3D_Both, GPU_TEXTURE0/*, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR*/);
	N3D_C3D_TexEnvOpRgb(&envBG_Smush, GPU_TEVOP_RGB_SRC_COLOR/*, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_ALPHA*/);
	N3D_C3D_TexEnvOpAlpha(&envBG_Smush, GPU_TEVOP_A_SRC_ALPHA/*, GPU_TEVOP_A_SRC_ALPHA, GPU_TEVOP_A_SRC_ALPHA*/);

	// Create texEnv for text.
	N3D_C3D_TexEnvInit(&envText);
	N3D_C3D_TexEnvFunc(&envText, C3D_Both, GPU_MODULATE);
	N3D_C3D_TexEnvSrc(&envText, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR/*, 0*/);
	N3D_C3D_TexEnvOpRgb(&envText, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_COLOR/*, GPU_TEVOP_RGB_SRC_ALPHA*/);
	N3D_C3D_TexEnvOpAlpha(&envText, GPU_TEVOP_A_SRC_ALPHA, GPU_TEVOP_A_SRC_ALPHA/*, GPU_TEVOP_A_SRC_ALPHA*/);

	type = Graphics::RendererType::kRendererTypeN3DS;

	// Citro3D doesn't set the initial "size" value of C3D_Tex-es to 0, so we have to do that ourselves.
	_smushTex.size = 0;
}

GfxN3DS::~GfxN3DS() {
	releaseMovieFrame();

	N3D_C3D_RenderTargetDelete(_gameScreenTarget);

	N3D_FreeBuffer(_blankVBO);
	N3D_FreeBuffer(_smushVBO);
	N3D_FreeBuffer(_quadEBO);

	N3D_FreeBuffer(_blastVBO);

	bool isEMI = g_grim->getGameType() == GType_MONKEY4;

	delete _backgroundShader;

	DECOMPOSE_SHADER(_smush)
	DECOMPOSE_SHADER(_text)

	DECOMPOSE_SHADER(_manualClear)

	N3DS_3D::destroyNative3D();
}

void GfxN3DS::setupZBuffer() {
}

void GfxN3DS::setupQuadEBO() {
	// FIXME: Probably way too big...
	unsigned short quad_indices[6 * 1000];

	unsigned short start = 0;
	for (unsigned short *p = quad_indices; p < &quad_indices[6 * 1000]; p += 6) {
		p[0] = p[3] = start++;
		p[1] = start++;
		p[2] = p[4] = start++;
		p[5] = start++;
	}

	_quadEBO = N3D_CreateBuffer(sizeof(quad_indices), quad_indices, 0x4);
}

void GfxN3DS::setupUntexturedQuad() {
	_blankVBO = N3D_CreateBuffer(sizeof(float) * 2 * 4, untextured_quad, 0x4);
	_manualClearShader->addAttrLoader(0, GPU_FLOAT, 2);						// v0 = position
	_manualClearShader->addBufInfo(_blankVBO, 2 * sizeof(float), 1, 0x0);

}

void GfxN3DS::setupTexturedQuad() {
	_smushVBO = N3D_CreateBuffer(sizeof(textured_quad), textured_quad, 0x4);
	_smushShader->addAttrLoader(0, GPU_FLOAT, 2);						// v0 = position
	_smushShader->addAttrLoader(1, GPU_FLOAT, 2);						// v1 = texcoord
	_smushShader->addBufInfo(_smushVBO, 4 * sizeof(float), 2, 0x10);

	_textShader->addAttrLoader(0, GPU_FLOAT, 2);						// v0 = position
	_textShader->addAttrLoader(1, GPU_FLOAT, 2);						// v1 = texcoord



	if (g_grim->getGameType() == GType_GRIM) {
		_backgroundShader->addAttrLoader(0, GPU_FLOAT, 2);				// v0 = position
		_backgroundShader->addAttrLoader(1, GPU_FLOAT, 2);				// v1 = texcoord
		_backgroundShader->addBufInfo(_smushVBO, 4 * sizeof(float), 2, 0x10);
//	} else {
//		_dimPlaneProgram->enableVertexAttribute("position", _smushVBO, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	}
}

void GfxN3DS::setupTexturedCenteredQuad() {
}

void GfxN3DS::setupPrimitives() {
}

//GLuint GfxOpenGLS::nextPrimitive() {
//	GLuint ret = _primitiveVBOs[_currentPrimitive];
void *GfxN3DS::nextPrimitive() {
	void *ret = _primitiveVBOs[_currentPrimitive];
	_lastPrimitive = _currentPrimitive;
	_currentPrimitive = (_currentPrimitive + 1) % ARRAYSIZE(_primitiveVBOs);
	return ret;
}

void GfxN3DS::setupShaders() {
	bool isEMI = g_grim->getGameType() == GType_MONKEY4;

	CONSTRUCT_SHADER(common, _manualClear, 0)

	CONSTRUCT_SHADER(grim, _smush, 0)
	CONSTRUCT_SHADER(grim, _text, 0)

	if (isEMI) {
		CONSTRUCT_SHADER(emi, _background, 0)
	} else {
		_backgroundShader = new N3DS_3D::ShaderObj(&_smushProgram, _smushProgramFlags);
	}

	setupQuadEBO();
	setupUntexturedQuad();
	setupTexturedQuad();
	setupTexturedCenteredQuad();
	setupPrimitives();

	if (!isEMI) {
//		_blastVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, 128 * 16 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
		_blastVBO = N3D_CreateBuffer(size_t(128 * 16) * sizeof(float), nullptr, 0x4);
	}
}

void GfxN3DS::setupScreen(int screenW, int screenH) {
	_screenWidth = screenW;											// 640
	_screenHeight = screenH;										// 480
	_scaleW = _screenWidth / (float)_gameWidth;						// 1.0f
	_scaleH = _screenHeight / (float)_gameHeight;					// 1.0f
	_screenTexWidth = nextHigher2(_screenWidth);
	_screenTexHeight = nextHigher2(_screenHeight);

	g_system->showMouse(false);

	// Clear the render target.
	// Color: black, full opacity = 0x000000FF; color currently set to 0xABABABFF for testing purposes
	// Stencil: 0 = 0x00
	// Depth: 1.f converted to 24-bit unsigned int = 0xFFFFFF
	// Stencil + Depth: 0x00 << 24 | 0xFFFFFF = 0x00FFFFFF
	//N3D_C3D_RenderTargetClear(_gameScreenTarget, C3D_CLEAR_ALL, 0x00ff00FF, 0x00 << 24 | 0xFFFFFF);			// THIS WORKS
	N3D_C3D_RenderTargetClear(_gameScreenTarget, C3D_CLEAR_ALL, 0, 0x00 << 24 | 0xFFFFFF);

	setupZBuffer();
	setupShaders();

}

void GfxN3DS::setupCameraFrustum(float fov, float nclip, float fclip) {
}

void GfxN3DS::positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest, float roll) {
}

void GfxN3DS::positionCamera(const Math::Vector3d &pos, const Math::Matrix4 &rot) {
}


Math::Matrix4 GfxN3DS::getModelView() {
	if (g_grim->getGameType() == GType_MONKEY4) {
		Math::Matrix4 invertZ;
		invertZ(2, 2) = -1.0f;

		Math::Matrix4 viewMatrix = _currentRot;
		viewMatrix.transpose();

		Math::Matrix4 camPos;
		camPos(0, 3) = -_currentPos.x();
		camPos(1, 3) = -_currentPos.y();
		camPos(2, 3) = -_currentPos.z();

		Math::Matrix4 modelView = invertZ * viewMatrix * camPos;
		return modelView;
	} else {
		return _mvpMatrix;
	}
}

Math::Matrix4 GfxN3DS::getProjection() {
	Math::Matrix4 proj = _projMatrix;
	proj.transpose();
	return proj;
}

void GfxN3DS::clearScreen() {
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Depth and stencil buffers are interleaved on 3DS. C3D_RenderTargetClear uses GX_MemoryFill,
	//     which is unaffected by masking.
	// This means that, when using C3D_RenderTargetClear to clear buffers, both depth AND sencil
	//     buffers must be cleared simultaneously; you can't choose to clear one and preserve the other.
	// Solution: Draw a blank texture to the screen with depth writing ENabled and stencil writing DISabled.

	// we want to preserve _grimContext's settings
	// use a temporary context cloned from _grimContext; tmpContext is automatically set to be the active context
	N3DS_3D::ContextHandle *tmpContext = N3DS_3D::createContext(_grimContext);

	// begin frame if not already in one
	N3D_C3D_FrameBegin(0);
	N3D_C3D_FrameDrawOn(_gameScreenTarget);
	N3D_C3D_SetViewport(0, 0, 640, 480);
	//N3D_C3D_RenderTargetClear(_gameScreenTarget, C3D_CLEAR_COLOR, 0x00000000, 0xFFFFFFFF);
	N3D_C3D_SetTexEnv(0, &envGRIMDefault);

	// enable stencil testing
	N3D_StencilTestEnabled(true);
	// do not write to stencil component
	N3D_StencilMask(0x00);
	// Keep old value
	N3D_StencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, GPU_STENCIL_KEEP);
	// enable depth testing
	N3D_DepthTestEnabled(true);
	// write to all color components
	N3D_ColorMask(true, true, true, true);
	// write to depth component
	N3D_DepthMask(true);
	// make depth test always pass
	N3D_DepthFunc(GPU_ALWAYS);

	// unbind any textures at position 0
	N3D_C3D_TexBind(0, NULL);


	// change the current shader (see "graphics/3ds/z3d.cpp")
	N3DS_3D::getActiveContext()->changeShader(_manualClearShader);
	// draw
	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 0, 4);
	// end frame
	N3D_C3D_FrameEnd(0);

	//N3DS_3D::setContext(_backendContext);
	//g_system->updateScreen();

	// set _grimContext as the active context; its settings are automatically set
	N3DS_3D::setContext(_grimContext);
	// destroy tmpContext as it is no longer needed
	N3DS_3D::destroyContext(tmpContext);
	N3D_C3D_SetTexEnv(0, &envGRIMDefault);
}

void GfxN3DS::clearDepthBuffer() {
//	glClear(GL_DEPTH_BUFFER_BIT);																									// DEPTH AND STENCIL BUFFER ARE INTERLEAVED

	//N3DS_3D::ContextHandle *tmpContext = N3DS_3D::createContext(_grimContext);														// DEFINITE?
	//N3D_ColorMask(false, false, false, false);																						// DEFINITE?
	//N3D_StencilTestEnabled(true);																									// DEFINITE?
	//N3D_StencilMask(0x00);
	//N3D_StencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, GPU_STENCIL_KEEP);
	//N3D_DepthTestEnabled(true);																										// DEFINITE?
	//N3D_DepthMask(true);
	//N3D_DepthFunc(GPU_ALWAYS);																										// DEFINITE?
	//N3D_C3D_TexBind(0, nullptr);																									// DEFINITE?
	//N3DS_3D::getActiveContext()->changeShader(_manualClearShader);																		// DEFINITE?
	//N3D_C3D_FrameBegin(0);																											// DEFINITE? - ADDED
	//N3D_C3D_FrameDrawOn(_gameScreenTarget);																							// DEFINITE? - ADDED
	//N3D_C3D_SetViewport(0, 0, 640, 480);																							// DEFINITE? - ADDED
	//N3D_C3D_SetTexEnv(0, &envGRIMDefault);																							// DEFINITE? - ADDED
	//N3D_C3D_DrawElements(GPU_TRIANGLES, 6, C3D_UNSIGNED_SHORT, (void *)_quadEBO);													// DEFINITE?
	//N3D_C3D_FrameEnd(0);																											// DEFINITE? - ADDED
	//N3DS_3D::setContext(_grimContext);																								// DEFINITE?
	//N3DS_3D::destroyContext(tmpContext);																							// DEFINITE?
	//N3D_C3D_SetTexEnv(0, &envGRIMDefault);																							// DEFINITE? - ADDED
}

void GfxN3DS::flipBuffer(bool opportunistic) {
//	C3D_FrameSplit(0);
//	if (opportunistic) {
//		GLint fbo = 0;
//		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fbo);				// CHANGE THIS?
//		if (fbo == 0) {
//			// Don't flip if we are not rendering on FBO
//			// Flipping without any draw is undefined
//			// When using an FBO, the older texture will be used
//			return;
//		}
//	}

	drawEnd(0);
	N3D_C3D_TexBind(0, NULL);
	// set to 3DS backend settings
	N3DS_3D::setContext(_backendContext);
	// do final rendering ( see "backends/platform/3ds/osystem-graphics.cpp")
	g_system->updateScreen();
	// set to GRIM settings
	N3DS_3D::setContext(_grimContext);
	// set to GRIM texture environment
	N3D_C3D_SetTexEnv(0, &envGRIMDefault);

}

void GfxN3DS::getScreenBoundingBox(const Mesh *model, int *x1, int *y1, int *x2, int *y2) {
}

void GfxN3DS::getScreenBoundingBox(const EMIModel *model, int *x1, int *y1, int *x2, int *y2) {
}

void GfxN3DS::getActorScreenBBox(const Actor *actor, Common::Point &p1, Common::Point &p2) {
}

void GfxN3DS::startActorDraw(const Actor *actor) {
}

void GfxN3DS::finishActorDraw() {
}

void GfxN3DS::setShadow(Shadow *shadow) {
}

void GfxN3DS::drawShadowPlanes() {
}

void GfxN3DS::setShadowMode() {
}

void GfxN3DS::clearShadowMode() {
}

bool GfxN3DS::isShadowModeActive() {
	return false;
}

void GfxN3DS::setShadowColor(byte r, byte g, byte b) {
}

void GfxN3DS::getShadowColor(byte *r, byte *g, byte *b) {
}

void GfxN3DS::destroyShadow(Shadow *shadow) {
}

void GfxN3DS::set3DMode() {
	debug("set3DMode");
}

void GfxN3DS::translateViewpointStart() {
}

void GfxN3DS::translateViewpoint(const Math::Vector3d &vec) {
}

void GfxN3DS::rotateViewpoint(const Math::Angle &angle, const Math::Vector3d &axis_) {
}

void GfxN3DS::rotateViewpoint(const Math::Matrix4 &rot) {
}

void GfxN3DS::translateViewpointFinish() {
}

void GfxN3DS::updateEMIModel(const EMIModel* model) {
}

void GfxN3DS::drawEMIModelFace(const EMIModel* model, const EMIMeshFace* face) {
}

void GfxN3DS::drawMesh(const Mesh *mesh) {
}

void GfxN3DS::drawDimPlane() {
}

void GfxN3DS::drawModelFace(const Mesh *mesh, const MeshFace *face) {
}

void GfxN3DS::drawSprite(const Sprite *sprite) {
}

void GfxN3DS::enableLights() {
}

void GfxN3DS::disableLights() {
}

void GfxN3DS::setupLight(Grim::Light *light, int lightId) {
}

void GfxN3DS::turnOffLight(int lightId) {
}


void GfxN3DS::createTexture(Texture *texture, const uint8 *data, const CMap *cmap, bool clamp) {
}

void GfxN3DS::selectTexture(const Texture *texture) {
}

void GfxN3DS::destroyTexture(Texture *texture) {
}

void GfxN3DS::createBitmap(BitmapData *bitmap) {
	if (bitmap->_format != 1) {
		for (int pic = 0; pic < bitmap->_numImages; pic++) {
			uint16 *zbufPtr = reinterpret_cast<uint16 *>(const_cast<void *>(bitmap->getImageData(pic).getPixels()));

			// On 3DS, the stencil buffer can only be used in an s8z24 combined buffer, and the buffer can only be
			// written to via DMA commands to VRAM. Change pixels to uint32, shift 16-bit depth vals 8 bits left for
			// their 24-bit equivalents before scaling

			uint32 *zbmUnload = (uint32 *)calloc(bitmap->_width * bitmap->_height, 4);												// DEFINITE?

			for (int i = 0; i < (bitmap->_width * bitmap->_height); i++) {
				uint16 val = zbufPtr[i];
				// fix the value if it is incorrectly set to the bitmap transparency color
				if (val == 0xf81f) {
					val = 0;
				}
				// This is later read as a LA pair when filling texture
				// with L being used as the LSB in fragment shader
//				zbufPtr[i] = TO_LE_16(0xffff - ((uint32)val) * 0x10000 / 100 / (0x10000 - val));
				zbmUnload[i] = TO_LE_32(0xffffff - ((uint64)val << 8) * 0x1000000 / 100 / (0x1000000 - (val << 8)));				// DEFINITE? - converts from 16-bit to 24-in-32-bit
			}

			Graphics::PixelFormat pixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);															// DEFINITE?
			const_cast<Graphics::Surface &>(bitmap->getImageData(pic)).create(bitmap->_width, bitmap->_height, pixelFormat);		// IS THIS RIGHT????????????????????????
			uint32 *newZbufPtr = (uint32 *)const_cast<void *>(bitmap->getImageData(pic).getPixels());								// DEFINITE?
			for (int i = 0; i < (bitmap->_width * bitmap->_height); i++) {															// DEFINITE?
				newZbufPtr[i] = zbmUnload[i];																						// DEFINITE?
			}																														// DEFINITE?
			free(zbmUnload);																										// DEFINITE?
		}
	}

	bitmap->_hasTransparency = false;
	if (bitmap->_format == 1) {
		bitmap->_numTex = 1;
		C3D_Tex *textures = new C3D_Tex[bitmap->_numTex * bitmap->_numImages];
		bitmap->_texIds = textures;

		byte *bmpData = nullptr;
		const byte *pixOut = nullptr;

		GPU_TEXCOLOR format = GPU_RGBA8;
		int bytes = 4;

		const Graphics::PixelFormat format_16bpp(2, 5, 6, 5, 0, 11, 5, 0, 0);
		const Graphics::PixelFormat format_32bpp = Graphics::PixelFormat::createFormatRGBA32();

		for (int pic = 0; pic < bitmap->_numImages; pic++) {
			const Graphics::Surface &imageData = bitmap->getImageData(pic);
			if (bitmap->_format == 1 && imageData.format == format_16bpp) {
				if (bmpData == nullptr)
					bmpData = new byte[bytes * bitmap->_width * bitmap->_height];
				// Convert data to 32-bit RGBA format
				byte *bmpDataPtr = bmpData;
				const uint16 *bitmapData = reinterpret_cast<const uint16 *>(imageData.getPixels());
				for (int i = 0; i < bitmap->_width * bitmap->_height; i++, bmpDataPtr += bytes, bitmapData++) {
					uint16 pixel = *bitmapData;
					int r = pixel >> 11;
					bmpDataPtr[0] = (r << 3) | (r >> 2);
					int g = (pixel >> 5) & 0x3f;
					bmpDataPtr[1] = (g << 2) | (g >> 4);
					int b = pixel & 0x1f;
					bmpDataPtr[2] = (b << 3) | (b >> 2);
					if (pixel == 0xf81f) { // transparent
						bmpDataPtr[3] = 0;
						bitmap->_hasTransparency = true;
					} else {
						bmpDataPtr[3] = 255;
					}
				}
				pixOut = bmpData;
			} else if (bitmap->_format == 1 && imageData.format != format_32bpp) {
				bitmap->convertToColorFormat(pic, format_32bpp);
				pixOut = (const byte *)imageData.getPixels();
			} else {
				pixOut = (const byte *)imageData.getPixels();
			}

			int actualWidth = nextHigher2(bitmap->_width);
			int actualHeight = nextHigher2(bitmap->_height);

			C3D_Tex *c3dTex = &textures[bitmap->_numTex * pic];
			N3D_C3D_TexInit(c3dTex, (u16)actualWidth, (u16)actualHeight, format);
			N3D_C3D_TexSetFilter(c3dTex, GPU_NEAREST, GPU_NEAREST);
			N3D_C3D_TexSetWrap(c3dTex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
			// Swizzle pixOut data into c3dTex.
			N3D_DataToBlockTex((u32 *)const_cast<uint8 *>(pixOut), (u32 *)c3dTex->data, 0, 0,
			                   bitmap->_width, bitmap->_height, actualWidth, actualHeight,
			                   format, false);
		}

		if (bmpData)
			delete[] bmpData;
		bitmap->freeData();

		// Make a clone of the _backgroundShader ShaderObj that has its own attribute and buffer info (see "graphics/3ds/z3d.cpp")
		N3DS_3D::ShaderObj *shader = new N3DS_3D::ShaderObj(_backgroundShader);
		bitmap->_userData = shader;

		shader->addAttrLoader(0, GPU_FLOAT, 2);				// v0 = position
		shader->addAttrLoader(1, GPU_FLOAT, 2);				// v1 = texcoord

		if (g_grim->getGameType() == GType_MONKEY4) {
//			GLuint vbo = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, bitmap->_numCoords * 4 * sizeof(float), bitmap->_texc, GL_STATIC_DRAW);
//			shader->enableVertexAttribute("position", vbo, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
//			shader->enableVertexAttribute("texcoord", vbo, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2*sizeof(float));
			void *vbo = N3D_CreateBuffer(bitmap->_numCoords * 4 * sizeof(float), bitmap->_texc, 0x4);
			BufInfo_Init(&shader->_bufInfo);
			shader->addBufInfo(vbo, 4 * sizeof(float), 2, 0x10);
		} else {
			shader->addBufInfo(_smushVBO, 4 * sizeof(float), 2, 0x10);
		}
	} else {
		bitmap->_numTex = 0;
		bitmap->_texIds = nullptr;
		bitmap->_userData = nullptr;
	}
}

void GfxN3DS::drawBitmap(const Bitmap *bitmap, int dx, int dy, uint32 layer) {
	if (g_grim->getGameType() == GType_MONKEY4 && bitmap->_data && bitmap->_data->_texc) {
		// Configure fragment operations.
		N3D_DepthTestEnabled(false);
		N3D_DepthMask(false);																										// ADDED
		N3D_BlendEnabled(true);
		N3D_BlendFunc(GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
		N3D_CullFaceEnabled(false);																									// ADDED

		// Get textures.
		BitmapData *data = bitmap->_data;
		N3DS_3D::ShaderObj *shader = (N3DS_3D::ShaderObj *)data->_userData;
		C3D_Tex *textures = static_cast<C3D_Tex *>(bitmap->getTexIds());

		// Bind shader program.
		N3DS_3D::getActiveContext()->changeShader(shader);

		// Start hardware frame.
		drawStart(0, 0, 0, 640, 480);
		// Use texture environment settings for bitmaps and movies.
		N3D_C3D_SetTexEnv(0, &envBG_Smush);

		assert(layer < data->_numLayers);
		uint32 offset = data->_layers[layer]._offset;
		for (uint32 i = offset; i < offset + data->_layers[layer]._numImages; ++i) {
			N3D_C3D_TexBind(0, textures + data->_verts[i]._texid);

			u16 startVertex = data->_verts[i]._pos / 4 * 6;
			int numVertices = data->_verts[i]._verts / 4 * 6;
			N3D_C3D_DrawElements(GPU_TRIANGLES, numVertices, C3D_UNSIGNED_SHORT, (void *)((u16 *)_quadEBO + startVertex));
		}
		//N3D_C3D_FrameEnd(0);

		// Keep the assumed "default" fragop state here in case I change stuff and forget it.
		//N3D_BlendEnabled(false);																									// DEFAULT STATE
		//N3D_DepthMask(true);																										// DEFAULT STATE
		//N3D_DepthTestEnabled(true);																								// DEFAULT STATE

		return;
	}

	int format = bitmap->getFormat();
	if ((format == 1 && !_renderBitmaps) || (format == 5 && !_renderZBitmaps)) {
		return;
	}

	if (format == 1) {
		// Configure fragment operations.
		N3D_DepthTestEnabled(false);
		N3D_DepthMask(false);
		if (bitmap->getFormat() == 1 && bitmap->getHasTransparency()) {
			N3D_BlendEnabled(true);
			N3D_BlendFunc(GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
		} else {
			N3D_BlendEnabled(false);
		}

		N3DS_3D::ShaderObj *shader = static_cast<N3DS_3D::ShaderObj *>(bitmap->_data->_userData);
		N3DS_3D::getActiveContext()->changeShader(shader);

		float width = bitmap->getWidth();
		float height = bitmap->getHeight();
		shader->setUniform("texcrop", GPU_VERTEX_SHADER, Math::Vector2d(width / float(nextHigher2((int)width)), height / float(nextHigher2((int)height))));
		shader->setUniform("scale", GPU_VERTEX_SHADER, Math::Vector2d(width / float(_gameWidth), height / float(_gameHeight)));
		shader->setUniform("offset", GPU_VERTEX_SHADER, Math::Vector2d(float(dx) / float(_gameWidth), float(dy) / float(_gameHeight)));

		C3D_Tex *textures = static_cast<C3D_Tex *>(bitmap->getTexIds());
		int cur_tex_idx = bitmap->getNumTex() * (bitmap->getActiveImage() - 1);
		N3D_C3D_TexBind(0, textures + cur_tex_idx);

		drawStart(0, 0, 0, 640, 480);
		N3D_C3D_SetTexEnv(0, &envBG_Smush);
		N3D_C3D_DrawElements(GPU_TRIANGLES, 6, C3D_UNSIGNED_SHORT, (void *)_quadEBO);
		//N3D_C3D_FrameEnd(0);

		// Keep the assumed "default" fragop state here in case I change stuff and forget it.
		//N3D_BlendEnabled(false);																								// DEFAULT STATE
		//N3D_DepthMask(true);																									// DEFAULT STATE
		//N3D_DepthTestEnabled(true);																							// DEFAULT STATE
	} else {
//		// Only draw the manual zbuffer when enabled
//		if (bitmap->getActiveImage() - 1 < bitmap->getNumImages()) {
//			drawDepthBitmap(bitmap->getId(), dx, dy, bitmap->getWidth(), bitmap->getHeight(),
//			                (char *)const_cast<void *>(bitmap->getData(bitmap->getActiveImage() - 1).getPixels()));
//		} else {
//			debug("zbuffer image has index out of bounds! %d/%d", bitmap->getActiveImage(), bitmap->getNumImages());
//		}
		return;
	}
}

void GfxN3DS::drawDepthBitmap(int bitmapId, int x, int y, int w, int h, char *data) {
//	static int prevId = -1;
//	static int prevX = -1, prevY = -1;
//	static int prevW = -1, prevH = -1;
//	static char *prevData = nullptr;
//
//	// Sometimes the data pointer is reused by the allocator between bitmaps
//	// Use the bitmap ID to ensure we don't prevent an expected update
//	if (bitmapId == prevId && prevX == x && prevY == y && prevW == w && prevH == h && data == prevData) {
//		return;
//	}
//
//	prevId = bitmapId;
//	prevX = x;
//	prevY = y;
//	prevW = w;
//	prevH = h;
//	prevData = data;
//
////	glActiveTexture(GL_TEXTURE1);
////	glBindTexture(GL_TEXTURE_2D, _zBufTex);
////	glPixelStorei(GL_UNPACK_ALIGNMENT, 2); // 16 bit Z depth bitmap
////	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data);
////	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
////	glActiveTexture(GL_TEXTURE0);
//	N3D_DataToBlockTex((u32 *)data, (u32 *)_zBuf, x, y, w, h, nextHigher2(w), nextHigher2(h), GPU_RGBA8, false);					// DEFINITE?
//	GSPGPU_FlushDataCache(_zBuf, nextHigher2(w) * nextHigher2(h) * 4);																// DEFINITE?
//	GX_RequestDma((u32 *)_zBuf, (u32 *)_gameScreenTarget->frameBuf.depthBuf, nextHigher2(w) * nextHigher2(h) * 4);					// DEFINITE?
//	gspWaitForDMA();																												// DEFINITE?
}

void GfxN3DS::destroyBitmap(BitmapData *bitmap) {
	C3D_Tex *textures = static_cast<C3D_Tex *>(bitmap->_texIds);
	if (textures) {
		for (int i = 0; i < bitmap->_numTex * bitmap->_numImages; i++) {
			N3D_C3D_TexDelete(textures + i);
		}
		delete[] textures;
		bitmap->_texIds = nullptr;
	}
	N3DS_3D::ShaderObj *shader = (N3DS_3D::ShaderObj *)bitmap->_userData;
	if (g_grim->getGameType() == GType_MONKEY4) {
		shader->freeAttachedBuffer(0);
	}
	delete shader;

	if (bitmap->_format != 1) {
		bitmap->freeData();
	}
}

void GfxN3DS::createFont(Font *f) {
	if (!f->is8Bit())
		error("non-8bit fonts are not supported in 3DS renderer");						// Is this true?
	BitmapFont *font = static_cast<BitmapFont *>(f);
	const byte *bitmapData = font->getFontData();
	uint dataSize = font->getDataSize();

	uint8 bpp = 4;
	uint8 charsWide = 16;
	uint8 charsHigh = 16;

	byte *fntData = new byte[dataSize * bpp];
	byte *fntDataPtr = fntData;

	for (uint i = 0; i < dataSize; i++, fntDataPtr += bpp, bitmapData++) {
		byte pixel = *bitmapData;
		// Reverse color component order.
		if (pixel == 0x00) {
			fntDataPtr[3] = fntDataPtr[2] = fntDataPtr[1] = fntDataPtr[0] = 0;
		} else if (pixel == 0x80) {
			fntDataPtr[3] = fntDataPtr[2] = fntDataPtr[1] = 0;
			fntDataPtr[0] = 255;
		} else if (pixel == 0xFF) {
			fntDataPtr[3] = fntDataPtr[2] = fntDataPtr[1] = fntDataPtr[0] = 255;
		}
	}
	int size = 0;
	for (int i = 0; i < 256; ++i) {
		int width = font->getCharBitmapWidth(i), height = font->getCharBitmapHeight(i);
		int m = MAX(width, height);
		if (m > size)
			size = m;
	}
	assert(size < 64);
	if (size < 8)
		size = 8;
	if (size < 16)
		size = 16;
	else if (size < 32)
		size = 32;
	else if (size < 64)
		size = 64;

	// New user data container.
	FontUserData *userData = new FontUserData;
	font->setUserData(userData);
	userData->size = size;
	// Allocate linear memory for subtexture coordinates.
	userData->subTextures = (Tex3DS_SubTexture *)N3D_CreateBuffer(sizeof(Tex3DS_SubTexture) * charsWide * charsHigh/*, nullptr, 0x4*/);

	// Allocate linear memory for texture data via texture initialization.
	u32 pixelsWide = charsWide * size;
	u32 pixelsHigh = charsHigh * size;
	N3D_C3D_TexInit(&userData->texture, (u16)pixelsWide, (u16)pixelsHigh, GPU_RGBA8);
	N3D_C3D_TexSetFilter(&userData->texture, GPU_NEAREST, GPU_NEAREST);
	N3D_C3D_TexSetWrap(&userData->texture, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);

	// Allocate linear memory for temporary holding of unswizzled texels.
	// To prevent fragmentation, linear memory for temporary data should be allocated AFTER data intended for long-term use.
	uint arraySize = size * size * bpp * charsWide * charsHigh;
	byte *temp = (byte *)N3D_CreateBuffer(sizeof(byte) * arraySize/*, nullptr, 0x4*/);

	for (int i = 0, row = 0; i < 256; ++i) {
		// Precalculate subtexture coordinates.
		u32 charX = (i != 0) ? ((i - 1) % 16) : 0;
		u32 charY = (i != 0) ? ((i - 1) / 16) : 0;
		userData->subTextures[i].width  = size;
		userData->subTextures[i].height = size;
		userData->subTextures[i].left   = float(charX)     / charsWide;
		userData->subTextures[i].top    = float(charY)     / charsHigh;
		userData->subTextures[i].right  = float(charX + 1) / charsWide;
		userData->subTextures[i].bottom = float(charY + 1) / charsHigh;

		int width = font->getCharBitmapWidth(i), height = font->getCharBitmapHeight(i);
		int32 d = font->getCharOffset(i);
		for (int x = 0; x < height; ++x) {
			// a is the offset to get to the correct row.
			// b is the offset to get to the correct line in the character.
			// c is the offset of the character from the start of the row.
			uint a = row * size * size * bpp * charsHigh;
			uint b = x * size * charsWide * bpp;
			uint c = 0;
			if (i != 0)
				c = ((i - 1) % 16) * size * bpp;

			uint pos = a + b + c;
			uint pos2 = d * bpp + x * width * bpp;
			assert(pos + width * bpp <= arraySize);
			assert(pos2 + width * bpp <= dataSize * bpp);
			memcpy(temp + pos, fntData + pos2, width * bpp);
		}
		if (i != 0 && i % charsWide == 0)
			++row;
	}

	// Swizzle texels into C3D_Tex.
	GSPGPU_FlushDataCache((void *)temp, sizeof(byte) * (u32)arraySize);
	// GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) = (1 << 0) | (1 << 1) = 0b01 | 0b10 = 0b11 = 3
	N3D_C3D_SyncDisplayTransfer((u32 *)temp,                   GX_BUFFER_DIM(pixelsWide, pixelsHigh),
	                            (u32 *)userData->texture.data, GX_BUFFER_DIM(pixelsWide, pixelsHigh), 3);
//	N3D_DataToBlockTex((u32 *)const_cast<uint8 *>(temp), (u32 *)userData->texture.data, 0, 0,
//	                   pixelsWide, pixelsHigh, pixelsWide, pixelsHigh, GPU_RGBA8, false);

	// Deallocate temporary linear memory.
	N3D_FreeBuffer(temp);
	delete[] fntData;
	temp = nullptr;
}

void GfxN3DS::destroyFont(Font *font) {
	if (font->is8Bit()) {
		const FontUserData *data = static_cast<const FontUserData *>(static_cast<const BitmapFont *>(font)->getUserData());
		if (data) {
			// Delete font atlas texture.
			N3D_C3D_TexDelete(const_cast<C3D_Tex *>(&data->texture));
			// Free the buffer containing subtexture coordinates.
			N3D_FreeBuffer(data->subTextures);
			// Delete font data.
			delete data;
		}
	}
}

void GfxN3DS::createTextObject(TextObject *text) {
	const Color &color = text->getFGColor();
	const Font *f = text->getFont();
	if (!f->is8Bit())
		error("non-8bit fonts are not supported in 3DS renderer");						// Is this true?
	const BitmapFont *font = static_cast<const BitmapFont *>(f);

	const FontUserData *userData = (const FontUserData *)font->getUserData();
	if (!userData)
		error("Could not get font userdata");

	float sizeW = float(userData->size) / _gameWidth;
	float sizeH = float(userData->size) / _gameHeight;
	const Common::String *lines = text->getLines();
	int numLines = text->getNumLines();

	int numCharacters = 0;
	for (int j = 0; j < numLines; ++j) {
		numCharacters += lines[j].size();
	}

	float *bufData = new float[numCharacters * 16];
	float *cur = bufData;

	const Tex3DS_SubTexture *subTexes = userData->subTextures;

	for (int j = 0; j < numLines; ++j) {
		const Common::String &line = lines[j];
		int x = text->getLineX(j);
		int y = text->getLineY(j);
		for (uint i = 0; i < line.size(); ++i) {
			uint8 character = line[i];
			float w = y + font->getCharStartingLine(character);
			if (g_grim->getGameType() == GType_GRIM)
				w += font->getBaseOffsetY();
			float z = x + font->getCharStartingCol(character);
			z /= _gameWidth;
			w /= _gameHeight;

			// Copy precalculated texture coordinates from *subTexes.
			float charData[] = {
				z,         w,         subTexes[character].left,  subTexes[character].top,
				z + sizeW, w,         subTexes[character].right, subTexes[character].top,
				z + sizeW, w + sizeH, subTexes[character].right, subTexes[character].bottom,
				z,         w + sizeH, subTexes[character].left,  subTexes[character].bottom
			};
			memcpy(cur, charData, 16 * sizeof(float));
			cur += 16;

			x += font->getCharKernedWidth(character);
		}
	}
	void *vbo;
	if (text->isBlastDraw()) {
		vbo = _blastVBO;
//		glBufferSubData(GL_ARRAY_BUFFER, 0, numCharacters * 16 * sizeof(float), bufData);
		memcpy(vbo, bufData, numCharacters * 16 * sizeof(float));
	} else {
		vbo = N3D_CreateBuffer(numCharacters * 16 * sizeof(float), bufData, 0x4);
	}

	TextUserData * td = new TextUserData;
	td->characters = numCharacters;
	td->vbo = vbo;
	td->color = color;
	td->texture = const_cast<C3D_Tex *>(&userData->texture);
	text->setUserData(td);
	delete[] bufData;
}

void GfxN3DS::drawTextObject(const TextObject *text) {
	// Configure fragment operations.
	N3D_BlendEnabled(true);
	N3D_BlendFunc(GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);																			// ADDED
	N3D_DepthTestEnabled(false);
	N3D_DepthMask(false);																											// ADDED

	// Get user data from text object.
	const TextUserData * td = (const TextUserData *) text->getUserData();
	assert(td);
	// Check if *text isn't the active TextObject.
	if (_activeTextObject != text) {
		// Set _textShader to use td's VBO.
		_textShader->BufInfo_AddOrModify(td->vbo, 4 * sizeof(float), 2, 0x10, 0);
		// Remember the new active TextObject.
		_activeTextObject = const_cast<TextObject *>(text);
	}
	// Bind _textShader program.
	N3DS_3D::getActiveContext()->changeShader(_textShader);

	// Set color uniform.
	Math::Vector3d colors(float(td->color.getRed()  ) / 255.0f,
	                      float(td->color.getGreen()) / 255.0f,
	                      float(td->color.getBlue() ) / 255.0f);
	_textShader->setUniform("color", GPU_VERTEX_SHADER, colors);
	// Bind the requested font atlas texture.
	N3D_C3D_TexBind(0, td->texture);

	// Start a hardware frame if we aren't already in one.
	drawStart(0, 0, 0, 640, 480);
	// Configure texture environment to use our settings for drawing text.
	N3D_C3D_SetTexEnv(0, &envText);
	// Draw the text.
	N3D_C3D_DrawElements(GPU_TRIANGLES, (int)td->characters * 6, C3D_UNSIGNED_SHORT, (void *)_quadEBO);
	//N3D_C3D_FrameEnd(0);

	// Keep the assumed "default" fragop state here in case I change stuff and forget it.
	//N3D_BlendEnabled(false);																										// DEFAULT STATE, ADDED
	//N3D_DepthTestEnabled(true);																									// DEFAULT STATE
	//N3D_DepthMask(true);																											// DEFAULT STATE, ADDED
}

void GfxN3DS::destroyTextObject(TextObject *text) {
	// Get user data from text object.
	TextUserData * td = reinterpret_cast<TextUserData *>(const_cast<void *>(text->getUserData()));
	// Check if text object uses its own VBO.
	if (!text->isBlastDraw()) {
		// Free VBO.
		N3D_FreeBuffer(td->vbo);
	}
	text->setUserData(nullptr);

	// Check if *text is the active TextObject.
	if(_activeTextObject == text) {
		// Set _activeTextObject to nullptr.
		_activeTextObject = nullptr;
	}
	// Delete user data.
	td->vbo = nullptr;
	delete td;
}

void GfxN3DS::storeDisplay() {
}

void GfxN3DS::copyStoredToDisplay() {
}

void GfxN3DS::dimScreen() {
}

void GfxN3DS::dimRegion(int xin, int yReal, int w, int h, float level) {
}

void GfxN3DS::irisAroundRegion(int x1, int y1, int x2, int y2) {
}

void GfxN3DS::drawEmergString(int x, int y, const char *text, const Color &fgColor) {
}

void GfxN3DS::loadEmergFont() {
}

void GfxN3DS::drawGenericPrimitive(const float *vertices, uint32 numVertices, const PrimitiveObject *primitive) {

#define PRIMITIVE_LENGTH 4																											// FINALIZED - ADDED
#define SET_PRIMITIVE_UNIFORMS(primProg) \
	primProg->setUniform("color", GPU_VERTEX_SHADER, colorV); \
	primProg->setUniform("scaleWH", GPU_VERTEX_SHADER, Math::Vector2d(1.f / _gameWidth, 1.f / _gameHeight))							// FINALIZED - ADDED


#undef PRIMITIVE_LENGTH																												// FINALIZED - ADDED
#undef SET_PRIMITIVE_UNIFORMS																										// FINALIZED - ADDED

}

void GfxN3DS::drawRectangle(const PrimitiveObject *primitive) {
}

void GfxN3DS::drawLine(const PrimitiveObject *primitive) {
}

void GfxN3DS::drawPolygon(const PrimitiveObject *primitive) {
}

const Graphics::PixelFormat GfxN3DS::getMovieFormat() const {
	return Graphics::PixelFormat::createFormatRGBA32();
}

void GfxN3DS::prepareMovieFrame(Graphics::Surface* frame) {
	int width = frame->w;
	int height = frame->h;
	const byte *bitmap = (const byte *)frame->getPixels();

	GPU_TEXCOLOR frameFormat;

	// Used by Bink, QuickTime, MPEG, Theora and paletted SMUSH
	if (frame->format == getMovieFormat()) {
		frameFormat = GPU_RGBA8;
	// Used by 16-bit SMUSH
	} else if (frame->format == Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0)) {
		frameFormat = GPU_RGB565;
	} else {
		error("Unknown pixelformat: Bpp: %d RBits: %d GBits: %d BBits: %d ABits: %d RShift: %d GShift: %d BShift: %d AShift: %d",
			frame->format.bytesPerPixel,
			-(frame->format.rLoss - 8),
			-(frame->format.gLoss - 8),
			-(frame->format.bLoss - 8),
			-(frame->format.aLoss - 8),
			frame->format.rShift,
			frame->format.gShift,
			frame->format.bShift,
			frame->format.aShift);
	}

	// Check if _smushTex has been initialized and its format matches frameFormat.
	if ((_smushTex.size == 0) || (_smushTex.fmt != frameFormat)) {
		// Deallocate existing _smushTex (if any)
		N3D_C3D_TexDelete(&_smushTex);
		// Allocate a single 1024 x 512 C3D_Tex to handle all sizes of "frameFormat" movies
		N3D_C3D_TexInit(&_smushTex, 1024, 512, frameFormat);
	}
	N3D_C3D_TexSetFilter(&_smushTex, GPU_NEAREST, GPU_NEAREST);
	N3D_C3D_TexSetWrap(&_smushTex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);

	N3D_DataToBlockTex((u32 *)const_cast<uint8 *>(bitmap), (u32 *)_smushTex.data, 0, 0,
	                   width, height, (int)_smushTex.width, (int)_smushTex.height, frameFormat, false);

	_smushWidth = (int)(width);
	_smushHeight = (int)(height);
}

void GfxN3DS::drawMovieFrame(int offsetX, int offsetY) {
	// Configure fragment operations.
	N3D_DepthTestEnabled(false);
	N3D_DepthMask(false);																											// ADDED
	N3D_BlendEnabled(true);																											// ADDED
	N3D_BlendFunc(GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);																			// ADDED


	N3DS_3D::getActiveContext()->changeShader(_smushShader);

	// Since we aren't changing the size of _smushTex, divide by 1024 and 512 instead of nextHigher2(_smushWidth) and nextHigher2(_smushHeight)
	_smushShader->setUniform("texcrop", GPU_VERTEX_SHADER, Math::Vector2d(float(_smushWidth) / 1024, float(_smushHeight) / 512));
	_smushShader->setUniform("scale", GPU_VERTEX_SHADER, Math::Vector2d(float(_smushWidth) / float(_gameWidth), float(_smushHeight) / float(_gameHeight)));
	_smushShader->setUniform("offset", GPU_VERTEX_SHADER, Math::Vector2d(float(offsetX) / float(_gameWidth), float(offsetY) / float(_gameHeight)));
	N3D_C3D_TexBind(0, &_smushTex);

	drawStart(0, 0, 0, 640, 480);
	N3D_C3D_SetTexEnv(0, &envBG_Smush);
	N3D_C3D_DrawElements(GPU_TRIANGLES, 6, C3D_UNSIGNED_SHORT, (void *)_quadEBO);
	//N3D_C3D_FrameEnd(0);

	//N3D_DepthTestEnabled(true);																									// DEFAULT STATE
	//N3D_DepthMask(true);																											// DEFAULT STATE
}

void GfxN3DS::releaseMovieFrame() {
	if (_smushTex.size > 0) {
		N3D_C3D_TexDelete(&_smushTex);
		_smushTex.size = 0;
	}
}


const char *GfxN3DS::getVideoDeviceName() {
	return "Nintendo 3DS 3D Renderer";
}

void GfxN3DS::renderBitmaps(bool render) {
}

void GfxN3DS::renderZBitmaps(bool render) {
}

void GfxN3DS::createEMIModel(EMIModel *model) {
}

void GfxN3DS::destroyEMIModel(EMIModel *model) {
}

void GfxN3DS::createMesh(Mesh *mesh) {
}

void GfxN3DS::destroyMesh(const Mesh *mesh) {
}

//// for each row in the defined area (starting with topmost), read row into *buffer
//// remember, GL native images are flipped; this puts the pixels from the defined area right-side up
//static void readPixels(int x, int y, int width, int height, byte *buffer) {
//	byte *p = buffer;
//	for (int i = y; i < y + height; i++) {
//		glReadPixels(x, 479 - i, width, 1, GL_RGBA, GL_UNSIGNED_BYTE, p);	// reads one row at a time
//		p += width * 4;
//	}
//}

#define SUBTEXTURE_WIDTH 256																										// DEFINITE? - ADDED
#define SUBTEX_FROM_SCREEN(_id, _srcbuf, _dstbuf, _x, _y, _w, _h) \
	for (i = _y; i < _y + _h; i++) { \
		memcpy((void *)(_dstbuf + (i * SUBTEXTURE_WIDTH * 4)), \
		       (void *)(_srcbuf + ((i * _screenWidth + _x) * 4)), \
		       _w * 4); \
	} \
	createSpecialtyTexture(_id, _dstbuf, _w, _h)																					// DEFINITE? - ADDED

void GfxN3DS::makeScreenTextures() {																								// DEFINITE? - ADDED
}																																	// DEFINITE? - ADDED

#undef SUBTEX_FROM_SCREEN																											// DEFINITE? - ADDED
#undef SUBTEXTURE_WIDTH																												// DEFINITE? - ADDED

Bitmap *GfxN3DS::getScreenshot(int w, int h, bool useStored) {
	Bitmap *bmp;
	return bmp;
}

//void GfxOpenGLS::createSpecialtyTextureFromScreen(uint id, uint8 *data, int x, int y, int width, int height) {
//	g_system->presentBuffer();
//	readPixels(x, y, width, height, data);
//	createSpecialtyTexture(id, data, width, height);
//}
void GfxN3DS::createSpecialtyTextureFromScreen(uint id, uint8 *data, int x, int y, int width, int height) {

}

void GfxN3DS::setBlendMode(bool additive) {
	debug("setBlendMode");
	if (additive) {
//		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		_srcBlend = GPU_SRC_ALPHA;
		_dstBlend = GPU_ONE;
	} else {
//		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		_srcBlend = GPU_SRC_ALPHA;
		_dstBlend = GPU_ONE_MINUS_SRC_ALPHA;
	}
}

#undef CONSTRUCT_SHADER
#undef DECOMPOSE_SHADER
}

#endif // __3DS__
