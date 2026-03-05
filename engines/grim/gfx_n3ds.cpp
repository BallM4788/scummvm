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

#include "graphics/surface.h"

#include "engines/grim/actor.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/colormap.h"
#include "engines/grim/emi/modelemi.h"
#include "engines/grim/font.h"

#include "engines/grim/gfx_n3ds.h"
#include "engines/grim/grim.h"
#include "engines/grim/material.h"
#include "engines/grim/model.h"
#include "engines/grim/primitives.h"
#include "engines/grim/set.h"
#include "engines/grim/sprite.h"

#include "engines/grim/shaders-3ds/emi_actor_shbin.h"
#include "engines/grim/shaders-3ds/emi_actorLights_shbin.h"
#include "engines/grim/shaders-3ds/emi_background_shbin.h"
#include "engines/grim/shaders-3ds/emi_sprite_shbin.h"
#include "engines/grim/shaders-3ds/grim_actor_shbin.h"
#include "engines/grim/shaders-3ds/grim_actorLights_shbin.h"
#include "engines/grim/shaders-3ds/grim_shadowPlane_shbin.h"
#include "engines/grim/shaders-3ds/grim_smush_shbin.h"
#include "engines/grim/shaders-3ds/grim_text_shbin.h"
#include "backends/platform/3ds/shaders/common_manualClear_shbin.h"

namespace Grim {

#define CONSTRUCT_SHADER(shaderName, binaryName, geomStride) \
	shaderName##Shader = new N3DS_3D::ShaderObj((u32*)const_cast<u8 *>(binaryName##_shbin), binaryName##_shbin_size, geomStride)
#define DECOMPOSE_SHADER(shaderName) \
	delete shaderName##Shader

void GfxN3DS::drawStart(u8 flags, u32 x, u32 y, u32 w, u32 h, C3D_TexEnv *texenv) {
	if (!_inFrame) {
		C3D_FrameBegin(flags);
		C3D_FrameDrawOn(_gameScreenTarget);
		// Due to the way Citro3D works, we must set the viewport ourselves for every draw session.
		C3D_SetViewport(x, y, w, h);
		if (texenv) C3D_SetTexEnv(0, texenv);
		_inFrame = true;
	}
}

void GfxN3DS::drawEnd(u8 flags) {
	if (_inFrame) {
		C3D_FrameEnd(flags);
		_inFrame = false;
	}
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
	-1.0f, -1.0f,
	 1.0f, -1.0f,
	 1.0f,  1.0f,
	-1.0f,  1.0f,
};

static float textured_quad[] = {
//	X   , Y   , S   , T
	0.0f, 0.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
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
	N3DS_3D::ShaderObj *shader;
	uint32   characters;
	Color    color;
	C3D_Tex *texture;
};

struct FontUserData {
	int size;
	C3D_Tex texture;
	Tex3DS_SubTexture *subTextures;
};

struct EMIModelUserData {
	N3DS_3D::ShaderObj *_shader;
	N3DS_3D::ShaderObj *_shaderLights;
	void *_texCoordsVBO;
	void *_colorMapVBO;
	void *_verticesVBO;
	void *_normalsVBO;
};

struct ModelUserData {
	N3DS_3D::ShaderObj *_shader;
	N3DS_3D::ShaderObj *_shaderLights;
	void *_meshInfoVBO;
};

struct ShadowUserData {
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

	return look;
}

Math::Matrix4 makeRotationMatrix(const Math::Angle& angle, Math::Vector3d axis) {
	float c = angle.getCosine();
	float s = angle.getSine();
	axis.normalize();
	Math::Vector3d temp = (1.f - c) * axis;
	Math::Matrix4 rotate;
	rotate(0, 0) = c + temp.x() * axis.x();
	rotate(0, 1) = 0 + temp.x() * axis.y() + s * axis.z();
	rotate(0, 2) = 0 + temp.x() * axis.z() - s * axis.y();
	rotate(0, 3) = 0;
	rotate(1, 0) = 0 + temp.y() * axis.x() - s * axis.z();
	rotate(1, 1) = c + temp.y() * axis.y();
	rotate(1, 2) = 0 + temp.y() * axis.z() + s * axis.x();
	rotate(1, 3) = 0;
	rotate(2, 0) = 0 + temp.z() * axis.x() + s * axis.y();
	rotate(2, 1) = 0 + temp.z() * axis.y() - s * axis.x();
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
	// For 3DS perspective matrices, flip Y coordinate value and scale depth range to [-1, 0]
	// https://www.wolframalpha.com/input?i={{1,0,0,0},{0,-1,0,0},{0,0,0.5,-0.5},{0,0,0,1}}{{(2n)/(r-l),0,(r%2Bl)/(r-l),0},{0,(2n)/(t-b),(t%2Bb)/(t-b),0},{0,0,-(f%2Bn)/(f-n),-(2fn)/(f-n)},{0,0,-1,0}}
	// https://www.wolframalpha.com/input?i=simplify%20(0.5(-f-n))/(f-n)%2B0.5
	proj(0, 0) = (2.0f * nclip) / (right - left);
	proj(1, 1) = (2.0f * nclip) / (bottom - top);
	proj(2, 0) = (right + left) / (right - left);
	proj(2, 1) = (bottom + top) / (bottom - top);
	proj(2, 2) = -(nclip) / (fclip - nclip);
	proj(2, 3) = -1.0f;
	proj(3, 2) = -(fclip * nclip) / (fclip - nclip);
	proj(3, 3) = 0.0f;

	return proj;
}

GfxBase *CreateGfxN3DS() {
	return new GfxN3DS();
}

GfxN3DS::GfxN3DS() {
	type = Graphics::RendererType::kRendererTypeN3DS;
//	_smushTexId = 0;
	_matrixStack.push(Math::Matrix4());
	_fov = -1.0;
	_nclip = -1;
	_fclip = -1;
	_selectedTexture = nullptr;
//	_emergTexture = 0;
	_maxLights = 8;
	_lights = new LightObj[_maxLights];
	_lightsEnabled = false;
	_hasAmbientLight = false;
	_backgroundShader = nullptr;
	_smushShader = nullptr;
	_textShader = nullptr;
	_emergShader = nullptr;
	_actorShader = nullptr;
	_actorLightsShader = nullptr;
	_spriteShader = nullptr;
	_primRectShader = nullptr;
	_primLinesShader = nullptr;
	_irisShader = nullptr;
	_shadowPlaneShader = nullptr;
	_dimShader = nullptr;
	_dimPlaneShader = nullptr;
	_dimRegionShader = nullptr;
	_manualClearShader = nullptr;
	_inFrame = false;

	// Create context that corresponds to the Citro3D setting of the 3DS backend.
	_backendContext = N3DS_3D::createContext();
	// Create context that corresponds to Open GL settings to be used for GRIM engine rendering.
	_grimContext = N3DS_3D::createOGLContext();

	_gameScreenTex = custom3DS_GetGameScreen();
	_gameScreenTarget = C3D_RenderTargetCreateFromTex(_gameScreenTex, GPU_TEXFACE_2D, 0, GPU_RB_DEPTH24_STENCIL8);

	// Create default texEnv corresponding to OpenGL's starting configuration.
	C3D_TexEnvInit(&envGRIMDefault);
	C3D_TexEnvFunc(&envGRIMDefault, C3D_Both, GPU_REPLACE);
	C3D_TexEnvSrc(&envGRIMDefault, C3D_Both, GPU_PRIMARY_COLOR/*, GPU_PRIMARY_COLOR, GPU_CONSTANT*/);
	C3D_TexEnvOpRgb(&envGRIMDefault, GPU_TEVOP_RGB_SRC_COLOR/*, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_ALPHA*/);
	C3D_TexEnvOpAlpha(&envGRIMDefault, GPU_TEVOP_A_SRC_ALPHA/*, GPU_TEVOP_A_SRC_ALPHA, GPU_TEVOP_A_SRC_ALPHA*/);

	// Create texEnv for backgrounds and videos.
	C3D_TexEnvInit(&envBG_Smush);
	C3D_TexEnvFunc(&envBG_Smush, C3D_Both, GPU_REPLACE);
	C3D_TexEnvSrc(&envBG_Smush, C3D_Both, GPU_TEXTURE0/*, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR*/);
	C3D_TexEnvOpRgb(&envBG_Smush, GPU_TEVOP_RGB_SRC_COLOR/*, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_ALPHA*/);
	C3D_TexEnvOpAlpha(&envBG_Smush, GPU_TEVOP_A_SRC_ALPHA/*, GPU_TEVOP_A_SRC_ALPHA, GPU_TEVOP_A_SRC_ALPHA*/);

	// Create texEnv for text.
	C3D_TexEnvInit(&envText);
	C3D_TexEnvFunc(&envText, C3D_Both, GPU_MODULATE);
	C3D_TexEnvSrc(&envText, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR/*, 0*/);
	C3D_TexEnvOpRgb(&envText, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_COLOR/*, GPU_TEVOP_RGB_SRC_ALPHA*/);
	C3D_TexEnvOpAlpha(&envText, GPU_TEVOP_A_SRC_ALPHA, GPU_TEVOP_A_SRC_ALPHA/*, GPU_TEVOP_A_SRC_ALPHA*/);

	// Create texEnv for actors that are textured.
	C3D_TexEnvInit(&envActorTex);
	C3D_TexEnvFunc(&envActorTex, C3D_Both, GPU_MODULATE);
	C3D_TexEnvSrc(&envActorTex, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR/*, 0*/);
	C3D_TexEnvOpRgb(&envActorTex, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_COLOR/*, GPU_TEVOP_RGB_SRC_ALPHA*/);
	C3D_TexEnvOpAlpha(&envActorTex, GPU_TEVOP_A_SRC_ALPHA, GPU_TEVOP_A_SRC_ALPHA/*, GPU_TEVOP_A_SRC_ALPHA*/);

	// Create texEnv for actors that are not textured.
	C3D_TexEnvInit(&envActorNoTex);
	C3D_TexEnvFunc(&envActorNoTex, C3D_Both, GPU_REPLACE);
	C3D_TexEnvSrc(&envActorNoTex, C3D_Both, GPU_PRIMARY_COLOR/*, GPU_PRIMARY_COLOR, GPU_CONSTANT*/);
	C3D_TexEnvOpRgb(&envActorNoTex, GPU_TEVOP_RGB_SRC_COLOR/*, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_ALPHA*/);
	C3D_TexEnvOpAlpha(&envActorNoTex, GPU_TEVOP_A_SRC_ALPHA/*, GPU_TEVOP_A_SRC_ALPHA, GPU_TEVOP_A_SRC_ALPHA*/);

	// Create texEnv for shadow planes.
	C3D_TexEnvInit(&envShadowPlane);
	C3D_TexEnvFunc(&envShadowPlane, C3D_Both, GPU_REPLACE);
	C3D_TexEnvSrc(&envShadowPlane, C3D_Both, GPU_PRIMARY_COLOR/*, GPU_PRIMARY_COLOR, GPU_CONSTANT*/);
	C3D_TexEnvOpRgb(&envShadowPlane, GPU_TEVOP_RGB_SRC_COLOR/*, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_ALPHA*/);
	C3D_TexEnvOpAlpha(&envShadowPlane, GPU_TEVOP_A_SRC_ALPHA/*, GPU_TEVOP_A_SRC_ALPHA, GPU_TEVOP_A_SRC_ALPHA*/);

	float div = 6.0f;
	_overworldProjMatrix = makeFrustumMatrix(-1.f / div, 1.f / div, -0.75f / div, 0.75f / div, 1.0f / div, 3276.8f);

	// Citro3D doesn't set the initial "size" value of C3D textures to 0, so we have to do that ourselves.
	_smushTex.size = 0;
}

GfxN3DS::~GfxN3DS() {
	releaseMovieFrame();

	delete[] _lights;

	C3D_RenderTargetDelete(_gameScreenTarget);

	custom3DS_FreeBuffer(_blankVBO);
	custom3DS_FreeBuffer(_smushVBO);
	custom3DS_FreeBuffer(_quadEBO);
	custom3DS_FreeBuffer(_spriteVBO);

	custom3DS_FreeBuffer(_blastVBO);



	custom3DS_FreeBuffer(_zBuffer);


	bool isEMI = g_grim->getGameType() == GType_MONKEY4;

	if (isEMI) {
	} else {
	}

	DECOMPOSE_SHADER(_background);

	DECOMPOSE_SHADER(_sprite);
	DECOMPOSE_SHADER(_actor);
	DECOMPOSE_SHADER(_actorLights);
	DECOMPOSE_SHADER(_smush);
	DECOMPOSE_SHADER(_text);

	DECOMPOSE_SHADER(_shadowPlane);

	DECOMPOSE_SHADER(_manualClear);

	// Set backend context settings; otherwise, launcher will be blank screen.
	N3DS_3D::setContext(_backendContext);
	N3DS_3D::destroyContext(_grimContext);
	N3DS_3D::destroyContext(_backendContext);
	N3DS_3D::destroyNative3D();
}

void GfxN3DS::setupZBuffer() {
	// Create buffer to hold values from depth bitmaps.
	_zBuffer = custom3DS_CreateBuffer(nextHigher2(_gameWidth) * nextHigher2(_gameHeight) * 4, nullptr, 0x4);
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

	_quadEBO = custom3DS_CreateBuffer(sizeof(quad_indices), quad_indices, 0x4);
}

void GfxN3DS::setupUntexturedQuad() {
	_blankVBO = custom3DS_CreateBuffer(sizeof(float) * 2 * 4, untextured_quad, 0x4);
	_manualClearShader->addAttrLoader(0 /*position*/, GPU_FLOAT, 2);
	_manualClearShader->addBufInfo(_blankVBO, 2 * sizeof(float), 1, 0x0);

}

void GfxN3DS::setupTexturedQuad() {
	_smushVBO = custom3DS_CreateBuffer(sizeof(textured_quad), textured_quad, 0x4);
	_smushShader->addAttrLoader(0 /*position*/, GPU_FLOAT, 2);
	_smushShader->addAttrLoader(1 /*texcoord*/, GPU_FLOAT, 2);
	_smushShader->addBufInfo(_smushVBO, 4 * sizeof(float), 2, 0x10);

	_textShader->addAttrLoader(0 /*position*/, GPU_FLOAT, 2);
	_textShader->addAttrLoader(1 /*texcoord*/, GPU_FLOAT, 2);



	if (g_grim->getGameType() == GType_GRIM) {
		_backgroundShader->addAttrLoader(0 /*position*/, GPU_FLOAT, 2);
		_backgroundShader->addAttrLoader(1 /*texcoord*/, GPU_FLOAT, 2);
		_backgroundShader->addBufInfo(_smushVBO, 4 * sizeof(float), 2, 0x10);
	}
}

void GfxN3DS::setupTexturedCenteredQuad() {
	float sprite_textured_quad[] = {
	//	 X   ,  Y   , Z   , S   , T
		-0.5f, +0.5f, 0.0f, 0, 0,
		+0.5f, +0.5f, 0.0f, 0, 0,
		+0.5f, -0.5f, 0.0f, 0, 0,
		-0.5f, -0.5f, 0.0f, 0, 0,
	};

	_spriteVBO = custom3DS_CreateBuffer(sizeof(float) * 20, sprite_textured_quad, 0x4);
	_spriteShader->addAttrLoader(0 /*position*/, GPU_FLOAT, 3);
	_spriteShader->addAttrLoader(1 /*texcoord*/, GPU_FLOAT, 2);
	// Fixed vertex attribute for color. Due to how 3DS and citro3D work, and because
	//	we're using more than one shader (the backend shader plus our engine shaders),
	//	the values for the fixed attribute must be specified at render time.
	// NOTE: Each fixed vertex attribute consists of four float components, and a single
	//	shader can have no more than 12 vertex attributes total (fixed and non-fixed).
	// TODO: For MONKEY4, store vertex colors in their own buffer to make this attribute
	//	non-fixed.
	_spriteShader->addFixedAttrLoader(2);
	// Fixed attributes don't contribute to the buffer stride, and are not taken into
	//	account for the buffer permutation.
	_spriteShader->addBufInfo(_spriteVBO, 5 * sizeof(float), 2, 0x10);
}

void GfxN3DS::setupPrimitives() {
}

void *GfxN3DS::nextPrimitive() {
	void *ret = _primitiveVBOs[_currentPrimitive];
	_lastPrimitive = _currentPrimitive;
	_currentPrimitive = (_currentPrimitive + 1) % ARRAYSIZE(_primitiveVBOs);
	return ret;
}

void GfxN3DS::setupShaders() {
	bool isEMI = g_grim->getGameType() == GType_MONKEY4;

	CONSTRUCT_SHADER(_manualClear, common_manualClear, 0);

	CONSTRUCT_SHADER(_smush, grim_smush, 0);
	CONSTRUCT_SHADER(_text, grim_text, 0);

	CONSTRUCT_SHADER(_shadowPlane, grim_shadowPlane, 0);
	_shadowPlaneShader->addAttrLoader(0 /*texcoord*/, GPU_FLOAT, 3);

	if (!isEMI) {
		CONSTRUCT_SHADER(_background, grim_smush, 0);
		CONSTRUCT_SHADER(_actor, grim_actor, 0);
		CONSTRUCT_SHADER(_actorLights, grim_actorLights, 0);
		CONSTRUCT_SHADER(_sprite, grim_actor, 0);
	} else {
		CONSTRUCT_SHADER(_background, emi_background, 0);
		CONSTRUCT_SHADER(_actor, emi_actor, 0);
		CONSTRUCT_SHADER(_actorLights, emi_actorLights, 0);
		CONSTRUCT_SHADER(_sprite, emi_sprite, 0);
	}

	setupQuadEBO();
	setupUntexturedQuad();
	setupTexturedQuad();
	setupTexturedCenteredQuad();
	setupPrimitives();

	if (!isEMI) {
		_blastVBO = custom3DS_CreateBuffer(size_t(128 * 16) * sizeof(float), nullptr, 0x4);
	}
}

void GfxN3DS::setupScreen(int screenW, int screenH) {
	_screenWidth = screenW;							// 640
	_screenHeight = screenH;						// 480
	_scaleW = _screenWidth / (float)_gameWidth;		// 1.0f
	_scaleH = _screenHeight / (float)_gameHeight;	// 1.0f
	_screenTexWidth = nextHigher2(_screenWidth);
	_screenTexHeight = nextHigher2(_screenHeight);

	g_system->showMouse(false);

	// Clear the render target.
	// Color: 0x00000000 (OpenGL default)
	// Stencil: 0.f (OpenGL default) converted to  8-bit unsigned int = 0x00
	// Depth:   1.f (OpenGL default) converted to 24-bit unsigned int = 0xFFFFFF
	// Stencil + Depth: 0x00 << 24 | 0xFFFFFF = 0x00FFFFFF
	C3D_RenderTargetClear(_gameScreenTarget, C3D_CLEAR_ALL, 0, 0x00FFFFFF);

	setupZBuffer();
	setupShaders();

	// Due to the way Citro3D works, we must set the viewport ourselves for every draw.

	glTo3DS_BlendFunc(GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
	if (g_grim->getGameType() == GType_MONKEY4) {
		// GPU_LEQUAL as glTo3DS_DepthFunc ensures that subsequent drawing attempts for
		// the same triangles are not ignored by the depth test.
		// That's necessary for EMI where some models have multiple faces which
		// refer to the same vertices. The first face is usually using the
		// color map and the following are using textures.
		glTo3DS_DepthFunc(GPU_LEQUAL);
	}
}

void GfxN3DS::setupCameraFrustum(float fov, float nclip, float fclip) {
	if (_fov == fov && _nclip == nclip && _fclip == fclip)
		return;

	_fov = fov; _nclip = nclip; _fclip = fclip;

	float right = nclip * tan(fov / 2 * ((float)M_PI / 180));
	float top = right * 0.75;

	_projMatrix = makeFrustumMatrix(-right, right, -top, top, nclip, fclip);
}

void GfxN3DS::positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest, float roll) {
	Math::Matrix4 viewMatrix = makeRotationMatrix(Math::Angle(roll), Math::Vector3d(0, 0, 1));
	Math::Vector3d up_vec(0, 0, 1);

	if (pos.x() == interest.x() && pos.y() == interest.y())
		up_vec = Math::Vector3d(0, 1, 0);

	Math::Matrix4 lookMatrix = makeLookMatrix(pos, interest, up_vec);

	_viewMatrix = viewMatrix * lookMatrix;
	_viewMatrix.transpose();
}

void GfxN3DS::positionCamera(const Math::Vector3d &pos, const Math::Matrix4 &rot) {
	Math::Matrix4 projMatrix = _projMatrix;
	projMatrix.transpose();

	_currentPos = pos;
	_currentRot = rot;

	Math::Matrix4 invertZ;
	invertZ(2, 2) = -1.0f;

	Math::Matrix4 viewMatrix = _currentRot;
	viewMatrix.transpose();

	Math::Matrix4 camPos;
	camPos(0, 3) = -_currentPos.x();
	camPos(1, 3) = -_currentPos.y();
	camPos(2, 3) = -_currentPos.z();

	_viewMatrix = invertZ * viewMatrix * camPos;
	_mvpMatrix = projMatrix * _viewMatrix;
	_viewMatrix.transpose();
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
	// In the 3DS hardware, stencil testing is only possible in a render target
	// that's utilizing an interleaved "24-bit depth + 8-bit stencil" buffer.
	// Consequentially, in this mode, there is no straightforward functionality
	// available to clear only depth values or only stencil values; if one is
	// being cleared, the other must be cleared as well.

	// To get around this, we must draw a texture covering the entire screen using
	// _manualClearShader, with either stencil writing or depth writing enabled.

	// Copy current context into a temporary context, switching to the copy and
	// preserving the original.
	N3DS_3D::ContextHandle *tmpContext = N3DS_3D::createContext(_grimContext);
	// Drawing to the color buffer is already enabled.
	// Preserve stencil values.
	glTo3DS_Disable(ENUM3DS_CAP_STENCIL_TEST);
	// Enable depth testing.
	glTo3DS_Enable(ENUM3DS_CAP_DEPTH_TEST);
	// Always pass depth test.
	glTo3DS_DepthFunc(GPU_ALWAYS);
	// Enable writing to depth buffer.
	glTo3DS_DepthMask(true);
	// Unbind any bound textures.
	glTo3DS_BindTexture(0, nullptr);
	// Set a clear value of 0.f for all color components.
	_manualClearShader->setUniform("colorClear", GPU_VERTEX_SHADER, 0.0f, 0.0f, 0.0f, 0.0f);
	// Set a depth clear value of 1.f, or 0xFFFFFF.
	_manualClearShader->setUniform("depthClear", GPU_VERTEX_SHADER, 1.0f);
	// Draw to the game screen, clearing the depth and color buffers.
	drawStart(0, 0, 0, 640, 480, &envGRIMDefault);
		N3DS_3D::changeShader(_manualClearShader);
		N3DS_3D::getActiveContext()->applyContextState();
		C3D_DrawElements(GPU_TRIANGLES, 6, C3D_UNSIGNED_SHORT, (void *)_quadEBO);
	drawEnd(0);
	// Switch back to the original context, reapplying its settings.
	N3DS_3D::setContext(_grimContext);
	// Destroy temporary context.
	N3DS_3D::destroyContext(tmpContext);
}

void GfxN3DS::clearDepthBuffer() {
//	glClear(GL_DEPTH_BUFFER_BIT);
	// In the 3DS hardware, stencil testing is only possible in a render target
	// that's utilizing an interleaved "24-bit depth + 8-bit stencil" buffer.
	// Consequentially, in this mode, there is no straightforward functionality
	// available to clear only depth values or only stencil values; if one is
	// being cleared, the other must be cleared as well.

	// To get around this, we must draw a texture covering the entire screen using
	// _manualClearShader, with either stencil writing or depth writing enabled.

	// Copy current context into a temporary context, switching to the copy and
	// preserving the original.
	N3DS_3D::ContextHandle *tmpContext = N3DS_3D::createContext(_grimContext);
	// Preserve color buffer.
	glTo3DS_ColorMask(false, false, false, false);
	// Preserve stencil values.
	glTo3DS_Disable(ENUM3DS_CAP_STENCIL_TEST);
	// Enable depth testing.
	glTo3DS_Enable(ENUM3DS_CAP_DEPTH_TEST);
	// Always pass depth test.
	glTo3DS_DepthFunc(GPU_ALWAYS);
	// Enable writing to depth buffer.
	glTo3DS_DepthMask(true);
	// Unbind any bound textures.
	glTo3DS_BindTexture(0, nullptr);
	// Set a depth clear value of 1.f, or 0xFFFFFF.
	_manualClearShader->setUniform("depthClear", GPU_VERTEX_SHADER, 1.f);
	// Draw to the game screen, clearing the depth buffer.
	drawStart(0, 0, 0, 640, 480, &envGRIMDefault);
		N3DS_3D::changeShader(_manualClearShader);
		N3DS_3D::getActiveContext()->applyContextState();
		C3D_DrawElements(GPU_TRIANGLES, 6, C3D_UNSIGNED_SHORT, (void *)_quadEBO);
	drawEnd(0);
	// Switch back to the original context, reapplying its settings.
	N3DS_3D::setContext(_grimContext);
	// Destroy temporary context.
	N3DS_3D::destroyContext(tmpContext);
}

void GfxN3DS::flipBuffer(bool opportunistic) {
//	if (opportunistic) {
//		GLint fbo = 0;
//		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);				// CHANGE THIS?
//		if (fbo == 0) {
//			// Don't flip if we are not rendering on FBO
//			// Flipping without any draw is undefined
//			// When using an FBO, the older texture will be used
//			return;
//		}
//	}

	// Unbind any textures.
	glTo3DS_BindTexture(0, NULL);
	// Do backend rendering.
	g_system->updateScreen();
	// Reassert engine context settings.
	N3DS_3D::setContext(_grimContext);
}

void GfxN3DS::getScreenBoundingBox(const Mesh *model, int *x1, int *y1, int *x2, int *y2) {
	if (_currentShadowArray) {
		*x1 = -1;
		*y1 = -1;
		*x2 = -1;
		*y2 = -1;
		return;
	}

	Math::Matrix4 modelMatrix = _currentActor->getFinalMatrix();
	Math::Matrix4 mvpMatrix = _mvpMatrix * modelMatrix;

	double top = 1000;
	double right = -1000;
	double left = 1000;
	double bottom = -1000;

	Math::Vector3d obj;
	float *pVertices = nullptr;

	for (int i = 0; i < model->_numFaces; i++) {
		for (int j = 0; j < model->_faces[i].getNumVertices(); j++) {
			pVertices = model->_vertices + 3 * model->_faces[i].getVertex(j);

			obj.set(*(pVertices), *(pVertices + 1), *(pVertices + 2));

			Math::Vector4d v = Math::Vector4d(obj.x(), obj.y(), obj.z(), 1.0f);
			v = mvpMatrix * v;
			v /= v.w();

			double winX = (1 + v.x()) / 2.0f * _gameWidth;
			double winY = (1 + v.y()) / 2.0f * _gameHeight;

			if (winX > right)
				right = winX;
			if (winX < left)
				left = winX;
			if (winY < top)
				top = winY;
			if (winY > bottom)
				bottom = winY;
		}
	}

	double t = bottom;
	bottom = _gameHeight - top;
	top = _gameHeight - t;

	if (left < 0)
		left = 0;
	if (right >= _gameWidth)
		right = _gameWidth - 1;
	if (top < 0)
		top = 0;
	if (bottom >= _gameHeight)
		bottom = _gameHeight - 1;

	if (top >= _gameHeight || left >= _gameWidth || bottom < 0 || right < 0) {
		*x1 = -1;
		*y1 = -1;
		*x2 = -1;
		*y2 = -1;
		return;
	}

	*x1 = (int)left;
	*y1 = (int)(_gameHeight - bottom);
	*x2 = (int)right;
	*y2 = (int)(_gameHeight - top);
}

void GfxN3DS::getScreenBoundingBox(const EMIModel *model, int *x1, int *y1, int *x2, int *y2) {
	if (_currentShadowArray) {
		*x1 = -1;
		*y1 = -1;
		*x2 = -1;
		*y2 = -1;
		return;
	}

	Math::Matrix4 modelMatrix = _currentActor->getFinalMatrix();
	Math::Matrix4 mvpMatrix = _mvpMatrix * modelMatrix;

	double top = 1000;
	double right = -1000;
	double left = 1000;
	double bottom = -1000;

	for (uint i = 0; i < model->_numFaces; i++) {
		uint16 *indices = (uint16 *)model->_faces[i]._indexes;

		for (uint j = 0; j < model->_faces[i]._faceLength * 3; j++) {
			uint16 index = indices[j];
			const Math::Vector3d &dv = model->_drawVertices[index];

			Math::Vector4d v = Math::Vector4d(dv.x(), dv.y(), dv.z(), 1.0f);
			v = mvpMatrix * v;
			v /= v.w();

			double winX = (1 + v.x()) / 2.0f * _gameWidth;
			double winY = (1 + v.y()) / 2.0f * _gameHeight;

			if (winX > right)
				right = winX;
			if (winX < left)
				left = winX;
			if (winY < top)
				top = winY;
			if (winY > bottom)
				bottom = winY;
		}
	}

	double t = bottom;
	bottom = _gameHeight - top;
	top = _gameHeight - t;

	if (left < 0)
		left = 0;
	if (right >= _gameWidth)
		right = _gameWidth - 1;
	if (top < 0)
		top = 0;
	if (bottom >= _gameHeight)
		bottom = _gameHeight - 1;

	if (top >= _gameHeight || left >= _gameWidth || bottom < 0 || right < 0) {
		*x1 = -1;
		*y1 = -1;
		*x2 = -1;
		*y2 = -1;
		return;
	}

	*x1 = (int)left;
	*y1 = (int)(_gameHeight - bottom);
	*x2 = (int)right;
	*y2 = (int)(_gameHeight - top);
}

void GfxN3DS::getActorScreenBBox(const Actor *actor, Common::Point &p1, Common::Point &p2) {
	// Get the actor's bounding box information (describes a 3D box)
	Math::Vector3d bboxPos, bboxSize;
	actor->getBBoxInfo(bboxPos, bboxSize);

	// Translate the bounding box to the actor's position
	Math::Matrix4 m = actor->getFinalMatrix();
	bboxPos = bboxPos + actor->getWorldPos();

	// Set up the camera coordinate system
	Math::Matrix4 modelView = _currentRot;
	Math::Matrix4 zScale;
	zScale.setValue(2, 2, -1.0);
	modelView = modelView * zScale;
	modelView.transpose();
	modelView.translate(-_currentPos);
	modelView.transpose();

	// Set values outside of the screen range
	p1.x = 1000;
	p1.y = 1000;
	p2.x = -1000;
	p2.y = -1000;

	// Project all of the points in the 3D bounding box
	Math::Vector3d p, projected;
	for (int x = 0; x < 2; x++) {
		for (int y = 0; y < 2; y++) {
			for (int z = 0; z < 2; z++) {
				Math::Vector3d added(bboxSize.x() * 0.5f * (x * 2 - 1), bboxSize.y() * 0.5f * (y * 2 - 1), bboxSize.z() * 0.5f * (z * 2 - 1));
				m.transform(&added, false);
				p = bboxPos + added;

				Math::Vector4d v = Math::Vector4d(p.x(), p.y(), p.z(), 1.0f);
				v = _projMatrix.transform(modelView.transform(v));
				if (v.w() == 0.0)
					return;
				v /= v.w();

				double winX = (1 + v.x()) / 2.0f * _gameWidth;
				double winY = (1 + v.y()) / 2.0f * _gameHeight;

				// Find the points
				if (winX < p1.x)
					p1.x = winX;
				if (winY < p1.y)
					p1.y = winY;
				if (winX > p2.x)
					p2.x = winX;
				if (winY > p2.y)
					p2.y = winY;
			}
		}
	}

	// Swap the p1/p2 y coorindates
	int16 tmp = p1.y;
	p1.y = 480 - p2.y;
	p2.y = 480 - tmp;
}

void GfxN3DS::startActorDraw(const Actor *actor) {
	_currentActor = actor;
	glTo3DS_Enable(ENUM3DS_CAP_DEPTH_TEST);											// NOT NEEDED?

	const Math::Vector3d &pos = actor->getWorldPos();
	const Math::Quaternion &quat = actor->getRotationQuat();
	//const float scale = actor->getScale();

	Math::Matrix4 viewMatrix = _viewMatrix;
	viewMatrix.transpose();

	N3DS_3D::ShaderObj *shaders[] = { _spriteShader, _actorShader, _actorLightsShader };

	if (g_grim->getGameType() == GType_MONKEY4) {
		glTo3DS_Enable(ENUM3DS_CAP_CULL_FACE);
		glTo3DS_FrontFace(ENUM3DS_FRONTFACE_CW);

		if (actor->isInOverworld())
			viewMatrix = Math::Matrix4();

		Math::Vector4d color(1.0f, 1.0f, 1.0f, actor->getEffectiveAlpha());

		const Math::Matrix4 &viewRot = _currentRot;
		Math::Matrix4 modelMatrix = actor->getFinalMatrix();

		Math::Matrix4 normalMatrix = viewMatrix * modelMatrix;
		normalMatrix.invertAffineOrthonormal();
		modelMatrix.transpose();

		for (int i = 0; i < 3; i++) {
			shaders[i]->setUniform("modelMatrix", GPU_VERTEX_SHADER, modelMatrix);
			if (actor->isInOverworld()) {
				shaders[i]->setUniform("viewMatrix", GPU_VERTEX_SHADER, viewMatrix);
				shaders[i]->setUniform("projMatrix", GPU_VERTEX_SHADER, _overworldProjMatrix);
				shaders[i]->setUniform("cameraPos", GPU_VERTEX_SHADER, Math::Vector3d(0,0,0));
			} else {
				shaders[i]->setUniform("viewMatrix", GPU_VERTEX_SHADER, viewRot);
				shaders[i]->setUniform("projMatrix", GPU_VERTEX_SHADER, _projMatrix);
				shaders[i]->setUniform("cameraPos", GPU_VERTEX_SHADER, _currentPos);
			}
			shaders[i]->setUniform("normalMatrix", GPU_VERTEX_SHADER, normalMatrix);

//			shaders[i]->setUniform("useVertexAlpha", GL_FALSE);
//			shaders[i]->setUniform("uniformColor", color);
//			shaders[i]->setUniform1f("alphaRef", 0.0f);			// glAlphaFunc "ref" EQUIVALENT (IMPLEMENTED THIS ANOTHER WAY)
//			shaders[i]->setUniform1f("meshAlpha", 1.0f);		// FRAGMENT SHADER UNIFORM
		}
	} else {
		Math::Matrix4 modelMatrix = quat.toMatrix();
		Math::Matrix4 extraMatrix;
		_matrixStack.top() = extraMatrix;

		modelMatrix.transpose();
		modelMatrix.setPosition(pos);
		modelMatrix.transpose();

		for (int i = 0; i < 3; i++) {
			shaders[i]->setUniform("modelMatrix", GPU_VERTEX_SHADER, modelMatrix);
			shaders[i]->setUniform("viewMatrix", GPU_VERTEX_SHADER, _viewMatrix);
			shaders[i]->setUniform("projMatrix", GPU_VERTEX_SHADER, _projMatrix);
			shaders[i]->setUniform("extraMatrix", GPU_VERTEX_SHADER, extraMatrix);
//			shaders[i]->setUniform1f("alphaRef", 0.5f);											// fragshader only - glAlphaFunc "ref" EQUIVALENT (IMPLEMENT ANOTHER WAY)
		}
	}

	if (_currentShadowArray) {
		const Sector *shadowSector = _currentShadowArray->planeList.front().sector;
		Math::Vector3d color;
		if (g_grim->getGameType() == GType_GRIM) {
			color = Math::Vector3d(_shadowColorR, _shadowColorG, _shadowColorB) / 255.f;
		} else {
			color = Math::Vector3d(_currentShadowArray->color.getRed(), _currentShadowArray->color.getGreen(), _currentShadowArray->color.getBlue()) / 255.f;	// CONVERTED FROM UNSIGNED BYTE TO FLOAT
		}
		Math::Vector3d normal = shadowSector->getNormal();
		if (!_currentShadowArray->dontNegate)
			normal = -normal;

		for (int i = 0; i < 3; i++) {
			shaders[i]->setUniform("shadowActive", GPU_VERTEX_SHADER, true);
			shaders[i]->setUniform("shadowColor", GPU_VERTEX_SHADER, color);
			shaders[i]->setUniform("shadowLight", GPU_VERTEX_SHADER, _currentShadowArray->pos);			// shadowProjection, PARAM 1   OF 4
			shaders[i]->setUniform("shadowPoint", GPU_VERTEX_SHADER, shadowSector->getVertices()[0]);	// shadowProjection, PARAM 2   OF 4
			shaders[i]->setUniform("shadowNormal", GPU_VERTEX_SHADER, normal);							// shadowProjection, PARAM 3+4 OF 4
		}

		glTo3DS_DepthMask(false);
		glTo3DS_Disable(ENUM3DS_CAP_BLEND);
		glTo3DS_Enable(ENUM3DS_CAP_POLYGON_OFFSET);
	}
	else {
		for (int i = 0; i < 3; i++) {
			shaders[i]->setUniform("shadowActive", GPU_VERTEX_SHADER, false);
		}
	}

	_actorLightsShader->setUniform("hasAmbient", GPU_VERTEX_SHADER, _hasAmbientLight);
	if (_lightsEnabled) {
		// Allocate all variables in one chunk
		static const unsigned int numUniforms = 4;
		static const unsigned int uniformSize = _maxLights * 4;
		float *lightsData = new float[numUniforms * uniformSize];
		for (int i = 0; i < _maxLights; ++i) {
			const LightObj &l = _lights[i];

			// lightsPos
			Math::Vector4d tmp = viewMatrix * l._position;

			lightsData[0 * uniformSize + 4 * i + 0] = tmp.x();
			lightsData[0 * uniformSize + 4 * i + 1] = tmp.y();
			lightsData[0 * uniformSize + 4 * i + 2] = tmp.z();
			lightsData[0 * uniformSize + 4 * i + 3] = tmp.w();

			// lightsDir
			Math::Vector4d direction = l._direction;
			direction.w() = 0.0;
			viewMatrix.transformVector(&direction);
			direction.w() = l._direction.w();

			lightsData[1 * uniformSize + 4 * i + 0] = direction.x();
			lightsData[1 * uniformSize + 4 * i + 1] = direction.y();
			lightsData[1 * uniformSize + 4 * i + 2] = direction.z();
			lightsData[1 * uniformSize + 4 * i + 3] = direction.w();

			// lightsClr
			lightsData[2 * uniformSize + 4 * i + 0] = l._color.x();
			lightsData[2 * uniformSize + 4 * i + 1] = l._color.y();
			lightsData[2 * uniformSize + 4 * i + 2] = l._color.z();
			lightsData[2 * uniformSize + 4 * i + 3] = l._color.w();

			// lightsPar
			lightsData[3 * uniformSize + 4 * i + 0] = l._params.x();
			lightsData[3 * uniformSize + 4 * i + 1] = l._params.y();
			lightsData[3 * uniformSize + 4 * i + 2] = l._params.z();
			lightsData[3 * uniformSize + 4 * i + 3] = l._params.w();
		}

		if (!(_actorLightsShader->setUniform4fv("lightsPos", GPU_VERTEX_SHADER, &lightsData[0 * uniformSize], _maxLights))) {
			error("No uniform named 'lightsPos'");
		}

		if (!(_actorLightsShader->setUniform4fv("lightsDir", GPU_VERTEX_SHADER, &lightsData[1 * uniformSize], _maxLights))) {
			error("No uniform named 'lightsDir'");
		}

		if (!(_actorLightsShader->setUniform4fv("lightsClr", GPU_VERTEX_SHADER, &lightsData[2 * uniformSize], _maxLights))) {
			error("No uniform named 'lightsClr'");
		}

		if (!(_actorLightsShader->setUniform4fv("lightsPar", GPU_VERTEX_SHADER, &lightsData[3 * uniformSize], _maxLights))) {
			error("No uniform named 'lightsPar'");
		}

		delete[] lightsData;
	}
}

void GfxN3DS::finishActorDraw() {
	drawEnd(0);
	_currentActor = nullptr;
	glTo3DS_Disable(ENUM3DS_CAP_POLYGON_OFFSET);
	if (g_grim->getGameType() == GType_MONKEY4) {
		glTo3DS_Disable(ENUM3DS_CAP_CULL_FACE);
	}
}

void GfxN3DS::setShadow(Shadow *shadow) {
	_currentShadowArray = shadow;
}

void GfxN3DS::drawShadowPlanes() {
	// Preserve color values.
	glTo3DS_ColorMask(false, false, false, false);
	// Preserve depth values.
	glTo3DS_DepthMask(false);

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	// In the 3DS hardware, stencil testing is only possible in a render target
	// that's utilizing an interleaved "24-bit depth + 8-bit stencil" buffer.
	// Consequentially, in this mode, there is no straightforward functionality
	// available to clear only depth values or only stencil values; if one is
	// being cleared, the other must be cleared as well.

	// To get around this, we must draw a texture covering the entire screen using
	// _manualClearShader, with either stencil writing or depth writing enabled.

	// Copy current context into a temporary context, switching to the copy and
	//	preserving the original.
	N3DS_3D::ContextHandle *tmpContext = N3DS_3D::createContext(_grimContext);
	// Write to stencil values by enabling stencil testing.
	glTo3DS_Enable(ENUM3DS_CAP_STENCIL_TEST);
	// Set stencil test ref value to ~0, and set stencil test to always pass.
	// NOTE: 0xFF is 0b11111111, which for an 8-bit stencil is the same as ~0.
	glTo3DS_StencilFunc(GPU_ALWAYS, 0xFF, (u8)~0);
	// Write to all stencil value bits.
	glTo3DS_StencilMask(0xFF);
	// Replace existing stencil values.
	glTo3DS_StencilOp(GPU_STENCIL_REPLACE, GPU_STENCIL_REPLACE, GPU_STENCIL_REPLACE);
	// Unbind any bound textures.
	glTo3DS_BindTexture(0, nullptr);
	// Draw to the game screen, clearing the depth buffer.
	drawStart(0, 0, 0, 640, 480, &envGRIMDefault);
		N3DS_3D::changeShader(_manualClearShader);
		N3DS_3D::getActiveContext()->applyContextState();
		C3D_DrawElements(GPU_TRIANGLES, 6, C3D_UNSIGNED_SHORT, (void *)_quadEBO);
	drawEnd(0);
	// Switch back to the original context, reapplying its settings.
	N3DS_3D::setContext(_grimContext);
	// Destroy temporary context.
	N3DS_3D::destroyContext(tmpContext);

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	glTo3DS_Enable(ENUM3DS_CAP_STENCIL_TEST);
	glTo3DS_StencilFunc(GPU_ALWAYS, 1, (u8)~0);
	glTo3DS_StencilOp(GPU_STENCIL_REPLACE, GPU_STENCIL_REPLACE, GPU_STENCIL_REPLACE);

	if (!_currentShadowArray->userData) {
		uint32 numVertices = 0;
		uint32 numTriangles = 0;
		for (SectorListType::iterator i = _currentShadowArray->planeList.begin(); i != _currentShadowArray->planeList.end(); ++i) {
			numVertices += i->sector->getNumVertices();
			numTriangles += i->sector->getNumVertices() - 2;
		}

		float *vertBuf = new float[3 * numVertices];
		uint16 *idxBuf = new uint16[3 * numTriangles];

		float *vert = vertBuf;
		uint16 *idx = idxBuf;

		for (SectorListType::iterator i = _currentShadowArray->planeList.begin(); i != _currentShadowArray->planeList.end(); ++i) {
			Sector *shadowSector = i->sector;
			memcpy(vert, shadowSector->getVertices(), 3 * shadowSector->getNumVertices() * sizeof(float));
			uint16 first = (vert - vertBuf) / 3;
			for (uint16 j = 2; j < shadowSector->getNumVertices(); ++j) {
				*idx++ = first;
				*idx++ = first + j - 1;
				*idx++ = first + j;
			}
			vert += 3 * shadowSector->getNumVertices();
		}

		ShadowUserData *sud = new ShadowUserData;
		_currentShadowArray->userData = sud;
		sud->_numTriangles = numTriangles;
		sud->_verticesVBO = custom3DS_CreateBuffer(3 * numVertices * sizeof(float), vertBuf, 0x4);
		sud->_indicesVBO = custom3DS_CreateBuffer(3 * numTriangles * sizeof(uint16), idxBuf, 0x4);

		delete[] vertBuf;
		delete[] idxBuf;
	}

	const ShadowUserData *sud = (ShadowUserData *)_currentShadowArray->userData;
	// Modify _shadowPlaneShader->_bufInfo to use sud->_verticesVBO.
	_shadowPlaneShader->BufInfo_AddOrModify(sud->_verticesVBO, 3 * sizeof(float), 1, 0x0, 0);
	_shadowPlaneShader->setUniform("projMatrix", GPU_VERTEX_SHADER, _projMatrix);
	_shadowPlaneShader->setUniform("viewMatrix", GPU_VERTEX_SHADER, _viewMatrix);

	drawStart(0, 0, 0, 640, 480, &envShadowPlane);
		N3DS_3D::changeShader(_shadowPlaneShader);
		N3DS_3D::getActiveContext()->applyContextState();
		C3D_DrawElements(GPU_TRIANGLES, 3 * sud->_numTriangles, C3D_UNSIGNED_SHORT, (void *)sud->_indicesVBO);
	drawEnd(0);

	glTo3DS_ColorMask(true, true, true, true);

	glTo3DS_StencilFunc(GPU_EQUAL, 1, (u8)~0);
	glTo3DS_StencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, GPU_STENCIL_KEEP);
}

void GfxN3DS::setShadowMode() {
	GfxBase::setShadowMode();
}

void GfxN3DS::clearShadowMode() {
	GfxBase::clearShadowMode();

	glTo3DS_Disable(ENUM3DS_CAP_STENCIL_TEST);
	glTo3DS_DepthMask(true);
}

void GfxN3DS::setShadowColor(byte r, byte g, byte b) {
	_shadowColorR = r;
	_shadowColorG = g;
	_shadowColorB = b;
}

void GfxN3DS::getShadowColor(byte *r, byte *g, byte *b) {
	*r = _shadowColorR;
	*g = _shadowColorG;
	*b = _shadowColorB;
}

void GfxN3DS::destroyShadow(Shadow *shadow) {
	ShadowUserData *sud = static_cast<ShadowUserData *>(shadow->userData);
	if (sud) {
		custom3DS_FreeBuffer(sud->_verticesVBO);
		custom3DS_FreeBuffer(sud->_indicesVBO);
		sud->_verticesVBO = nullptr;
		sud->_indicesVBO = nullptr;
		delete sud;
	}

	shadow->userData = nullptr;
}

void GfxN3DS::set3DMode() {
}

void GfxN3DS::translateViewpointStart() {
	_matrixStack.push(_matrixStack.top());
}

void GfxN3DS::translateViewpoint(const Math::Vector3d &vec) {
	Math::Matrix4 temp;
	temp.setPosition(vec);
	temp.transpose();
	_matrixStack.top() = temp * _matrixStack.top();
}

void GfxN3DS::rotateViewpoint(const Math::Angle &angle, const Math::Vector3d &axis_) {
	Math::Matrix4 temp = makeRotationMatrix(angle, axis_) * _matrixStack.top();
	_matrixStack.top() = temp;
}

void GfxN3DS::rotateViewpoint(const Math::Matrix4 &rot) {
	Math::Matrix4 temp = rot * _matrixStack.top();
	_matrixStack.top() = temp;
}

void GfxN3DS::translateViewpointFinish() {
	_matrixStack.pop();
}

void GfxN3DS::updateEMIModel(const EMIModel* model) {
}

void GfxN3DS::drawEMIModelFace(const EMIModel* model, const EMIMeshFace* face) {
}

void GfxN3DS::drawMesh(const Mesh *mesh) {
	glTo3DS_AlphaFunc(GPU_GREATER, 128);		// ADDED from "gfx_opengl.cpp"	128 = 0.5f converted to 0-255 int
	glTo3DS_Enable(ENUM3DS_CAP_ALPHA_TEST);		// ADDED from "gfx_opengl.cpp"

	const ModelUserData *mud = (const ModelUserData *)mesh->_userData;
	if (!mud)
		return;
	N3DS_3D::ShaderObj *actorShader;
	if (_lightsEnabled && !isShadowModeActive())
		actorShader = mud->_shaderLights;
	else
		actorShader = mud->_shader;

	N3DS_3D::changeShader(actorShader);
	actorShader->setUniform("extraMatrix", GPU_VERTEX_SHADER, _matrixStack.top());

	const Material *curMaterial = nullptr;
	for (int i = 0; i < mesh->_numFaces;) {
		const MeshFace *face = &mesh->_faces[i];

		curMaterial = face->getMaterial();
		curMaterial->select();

		int faces = 0;
		for (; i < mesh->_numFaces; ++i) {
			if (mesh->_faces[i].getMaterial() != curMaterial)
				break;
			faces += 3 * (mesh->_faces[i].getNumVertices() - 2);
		}

		bool textured = face->hasTexture() && !_currentShadowArray;
		actorShader->setUniform("textured", GPU_VERTEX_SHADER, textured);
		actorShader->setUniform("texScale", GPU_VERTEX_SHADER, Math::Vector2d(_selectedTexture->_width, _selectedTexture->_height));

		// Start a new frame if we aren't already in one.
		drawStart(0, 0, 0, 640, 480);
			C3D_SetTexEnv(0, textured ? &envActorTex : &envActorNoTex);
			N3DS_3D::getActiveContext()->applyContextState();
			C3D_DrawArrays(GPU_TRIANGLES, *(int *)face->_userData, faces);
			// Draw session continues for all other meshes of the actor
			//	(unless a texture needs to be loaded).
	}

	glTo3DS_Disable(ENUM3DS_CAP_ALPHA_TEST);
}

void GfxN3DS::drawDimPlane() {
}

void GfxN3DS::drawModelFace(const Mesh *mesh, const MeshFace *face) {
}

void GfxN3DS::drawSprite(const Sprite *sprite) {
}

void GfxN3DS::enableLights() {
	_lightsEnabled = true;
}

void GfxN3DS::disableLights() {
	_lightsEnabled = false;
}

void GfxN3DS::setupLight(Grim::Light *light, int lightId) {
	_lightsEnabled = true;

	if (lightId >= _maxLights) {
		return;
	}

	// Disable previous lights.
	if (lightId == 0) {
		_hasAmbientLight = false;
		for (int id = 0; id < _maxLights; ++id)
			_lights[id]._color.w() = 0.0;
	}

	Math::Vector4d &lightColor  = _lights[lightId]._color;
	Math::Vector4d &lightPos    = _lights[lightId]._position;
	Math::Vector4d &lightDir    = _lights[lightId]._direction;
	Math::Vector4d &lightParams = _lights[lightId]._params;

	lightColor.x() = (float)light->_color.getRed();
	lightColor.y() = (float)light->_color.getGreen();
	lightColor.z() = (float)light->_color.getBlue();
	lightColor.w() = light->_scaledintensity;

	if (light->_type == Grim::Light::Omni) {
		lightPos = Math::Vector4d(light->_pos.x(), light->_pos.y(), light->_pos.z(), 1.0f);
		lightDir = Math::Vector4d(0.0f, 0.0f, 0.0f, -1.0f);
		lightParams = Math::Vector4d(light->_falloffNear, light->_falloffFar, 0.0f, 0.0f);
	} else if (light->_type == Grim::Light::Direct) {
		lightPos = Math::Vector4d(-light->_dir.x(), -light->_dir.y(), -light->_dir.z(), 0.0f);
		lightDir = Math::Vector4d(0.0f, 0.0f, 0.0f, -1.0f);
	} else if (light->_type == Grim::Light::Spot) {
		lightPos = Math::Vector4d(light->_pos.x(), light->_pos.y(), light->_pos.z(), 1.0f);
		lightDir = Math::Vector4d(light->_dir.x(), light->_dir.y(), light->_dir.z(), 1.0f);
		lightParams = Math::Vector4d(light->_falloffNear, light->_falloffFar, light->_cospenumbraangle, light->_cosumbraangle);
	} else if (light->_type == Grim::Light::Ambient) {
		lightPos = Math::Vector4d(0.0f, 0.0f, 0.0f, -1.0f);
		lightDir = Math::Vector4d(0.0f, 0.0f, 0.0f, -1.0f);
		_hasAmbientLight = true;
	}
}

void GfxN3DS::turnOffLight(int lightId) {
	if (lightId >= _maxLights) {
		return;
	}

	_lights[lightId]._color = Math::Vector4d(0.0f, 0.0f, 0.0f, 0.0f);
	_lights[lightId]._position = Math::Vector4d(0.0f, 0.0f, 0.0f, 0.0f);
	_lights[lightId]._direction = Math::Vector4d(0.0f, 0.0f, 0.0f, 0.0f);
}


void GfxN3DS::createTexture(Texture *texture, const uint8 *data, const CMap *cmap, bool clamp) {
	texture->_texture = new C3D_Tex[1];

	C3D_Tex *textures = static_cast<C3D_Tex *>(texture->_texture);

	// C3D_TexInit resets texture wrap+filter settings, so
	//	we're forced to do that after texture initialization.

	char *texdata = nullptr;
	char *texdatapos = nullptr;

	if (cmap != nullptr) { // EMI doesn't have colour-maps
		int bytes = 4;

		texdata = (char *)linearAlloc(texture->_width * texture->_height * 4);
		texdatapos = texdata;

		for (int y = 0; y < texture->_height; y++) {
			for (int x = 0; x < texture->_width; x++) {
				uint8 col = *(const uint8 *)(data);
				if (col == 0) {
					memset(texdatapos, 0, bytes); // transparent
					if (!texture->_hasAlpha) {
						// C3D_SyncDisplayTransfer requires ABGR-order input.
						texdatapos[0] = '\xff'; // fully opaque
					}
				} else {
					// C3D_SyncDisplayTransfer requires ABGR-order input.
					const char *components = cmap->_colors + 3 * (col);
					texdatapos[3] = components[0];			// R
					texdatapos[2] = components[1];			// G
					texdatapos[1] = components[2];			// B
					texdatapos[0] = '\xff'; // fully opaque
				}
				texdatapos += bytes;
				data++;
			}
		}

		C3D_TexInit(textures, (u16)texture->_width, (u16)texture->_height, GPU_RGBA8);
		if ((texture->_width < 64) || (texture->_height < 64)) {
			// C3D_SyncDisplayTransfer causes a thread hang if either dimension is less than 64 pixels.
			// "false" instructs NOT to reorder pixel components (we already did).
			custom3DS_DataToBlockTex((u32 *)texdata,          0, 0, texture->_width, texture->_height,
			                         (u32 *)textures[0].data, 0, 0, texture->_width, texture->_height,
			                         texture->_width, texture->_height, GPU_RGBA8, false);
		} else {
			// If Citro3D is not in the middle of a frame, C3D_SyncDisplayTransfer immediately processes a
			//	GX_DisplayTransfer request. If Citro3D -IS- in a frame, however, C3D_SyncDisplayTransfer adds
			//	the GX request to its internal GX command queue, which will only be run once the frame has ended.
			// Since the memory pointed to by texdata is only guaranteed to have the correct data during this
			//	fucntion call, we must force Citro3D to process the GX request immediately.
			// To do so, we'll call drawEnd() to end the frame (if we are in one).
			drawEnd();
			GSPGPU_FlushDataCache(texdata, texture->_width * texture->_height * bytes);
			// GX_TRANSFER_FMT_RGBA8 is already 0
			// GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) = (1 << 0) | (1 << 1) = 0b01 | 0b10 = 0b11 = 3
			C3D_SyncDisplayTransfer((u32 *)texdata,          GX_BUFFER_DIM(texture->_width, texture->_height),
			                        (u32 *)textures[0].data, GX_BUFFER_DIM(texture->_width, texture->_height), 3);
		}
	} else {
		int bytes = texture->_bpp;

		GPU_TEXCOLOR format;
		GX_TRANSFER_FORMAT transFmt;
		if (bytes == 4) {
			format = GPU_RGBA8;
			transFmt = GX_TRANSFER_FMT_RGBA8;
		} else {
			format = GPU_RGB8;
			transFmt = GX_TRANSFER_FMT_RGB8;
		}

		texdata = (char *)linearAlloc(texture->_width * texture->_height * bytes);
		texdatapos = texdata;

		// C3D_SyncDisplayTransfer requires ABGR-order input.
		for (int y = 0; y < texture->_height; y++) {
			for (int x = 0; x < texture->_width; x++) {
				for (int i = 0; i < bytes; i++) {
					texdatapos[i] = data[(bytes - 1) - i];
				}
				texdatapos += bytes;
				data += bytes;
			}
		}

		C3D_TexInit(textures, (u16)texture->_width, (u16)texture->_height, format);
		if ((texture->_width < 64) || (texture->_height < 64)) {
			// C3D_SyncDisplayTransfer causes a thread hang if either dimension is less than 64 pixels.
			// "false" instructs NOT to reorder pixel components (we already did).
			custom3DS_DataToBlockTex((u32 *)texdata,          0, 0, texture->_width, texture->_height,
			                         (u32 *)textures[0].data, 0, 0, texture->_width, texture->_height,
			                         texture->_width, texture->_height, format, false);
		} else {
			// If Citro3D is not in the middle of a frame, C3D_SyncDisplayTransfer immediately processes a
			//	GX_DisplayTransfer request. If Citro3D -IS- in a frame, however, C3D_SyncDisplayTransfer adds
			//	the GX request to its internal GX command queue, which will only be run once the frame has ended.
			// Since the memory pointed to by texdata is only guaranteed to have the correct data during this
			//	fucntion call, we must force Citro3D to process the GX request immediately.
			// To do so, we'll call drawEnd() to end the frame (if we are in one).
			drawEnd();
			GSPGPU_FlushDataCache(texdata, texture->_width * texture->_height * bytes);
			// GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) = (1 << 0) | (1 << 1) = 0b01 | 0b10 = 0b11 = 3
			C3D_SyncDisplayTransfer((u32 *)texdata,          GX_BUFFER_DIM(texture->_width, texture->_height),
			                        (u32 *)textures[0].data, GX_BUFFER_DIM(texture->_width, texture->_height),
			                        GX_TRANSFER_IN_FORMAT(transFmt) | GX_TRANSFER_OUT_FORMAT(transFmt) | 3);
		}
	}

	linearFree(texdata);

	// Remove darkened lines in EMI intro
	if (g_grim->getGameType() == GType_MONKEY4 && clamp) {
		C3D_TexSetWrap(textures, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
	} else {
		C3D_TexSetWrap(textures, GPU_REPEAT, GPU_REPEAT);
	}

	C3D_TexSetFilter(textures, GPU_LINEAR, GPU_LINEAR);
}

void GfxN3DS::selectTexture(const Texture *texture) {
	C3D_Tex *textures = static_cast<C3D_Tex *>(texture->_texture);
	glTo3DS_BindTexture(0, textures);

	if (texture->_hasAlpha && g_grim->getGameType() == GType_MONKEY4) {
		glTo3DS_Enable(ENUM3DS_CAP_BLEND);
	}

	_selectedTexture = const_cast<Texture *>(texture);
}

void GfxN3DS::destroyTexture(Texture *texture) {
	C3D_Tex *textures = static_cast<C3D_Tex *>(texture->_texture);
	if (textures) {
		C3D_TexDelete(textures);
		delete[] textures;
	}
}

void GfxN3DS::createBitmap(BitmapData *bitmap) {
	if (bitmap->_format != 1) {
		// Work area to temporarily store converted depth values.
		uint32 *zbm16to24in32 = (uint32 *)calloc(bitmap->_width * bitmap->_height, 4);
		for (int pic = 0; pic < bitmap->_numImages; pic++) {
			uint16 *zbufPtr = reinterpret_cast<uint16 *>(const_cast<void *>(bitmap->getImageData(pic).getPixels()));

			// On 3DS, the stencil buffer can only be used in an s8z24 combined buffer, and the buffer can only be
			// written to via DMA commands to VRAM. Change pixels to uint32, convert 16-bit depth vals to their 24-
			// bit equivalents before scaling.

			for (int i = 0; i < (bitmap->_width * bitmap->_height); i++) {
				uint32 val = (uint32)((float)zbufPtr[i] / 0xffff * 0xffffff);						// scales from 16-bit to 24-in-32-bit
				// fix the value if it is incorrectly set to the bitmap transparency color
				if (val == 0xf81ff7) {																// 0xf81f scaled to 24-bit = 0xf81ff7
					val = 0;
				}
				zbm16to24in32[i] = 0xffffff - ((uint64)val) * 0x1000000 / 100 / (0x1000000 - val);	// flips and compresses depth curve
			}

			// Re-init bitmap->getImageData(pic) with 32-bit pixels.
			Graphics::PixelFormat pixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
			const_cast<Graphics::Surface &>(bitmap->getImageData(pic)).create(bitmap->_width, bitmap->_height, pixelFormat);
			// Copy pixels from zbm16to24in32 to bitmap->getImageData(pic)'s pixels.
			uint32 *newZbufPtr = (uint32 *)const_cast<void *>(bitmap->getImageData(pic).getPixels());
			memcpy(newZbufPtr, zbm16to24in32, bitmap->_width * bitmap->_height * 4);
		}
		// Free work area.
		free(zbm16to24in32);
	}

	bitmap->_hasTransparency = false;
	if (bitmap->_format == 1) {
		bitmap->_numTex = 1;
		C3D_Tex *textures = new C3D_Tex[bitmap->_numTex * bitmap->_numImages];
		bitmap->_texIds = textures;

		byte *bmpData = nullptr;

		GPU_TEXCOLOR format = GPU_RGBA8;
		int bytes = 4;

		const Graphics::PixelFormat format_16bpp(2, 5, 6, 5, 0, 11, 5, 0, 0);
		const Graphics::PixelFormat format_32bpp = Graphics::PixelFormat::createFormatRGBA32();

		// C3D texture measurements MUST be powers of two.
		int actualWidth = nextHigher2(bitmap->_width);
		int actualHeight = nextHigher2(bitmap->_height);

		for (int pic = 0; pic < bitmap->_numImages; pic++) {
			// Create work area in linear memory.
			if (bmpData == nullptr)
				bmpData = (byte *)linearAlloc(bytes * actualWidth * actualHeight);
			byte *bmpDataPtr = bmpData;

			const Graphics::Surface &imageData = bitmap->getImageData(pic);
			if (bitmap->_format == 1 && imageData.format == format_16bpp) {
				// Convert data to 32-bit RGBA format
				const uint16 *bitmapData16 = reinterpret_cast<const uint16 *>(imageData.getPixels());
				for (int i = 0; i < bitmap->_height; i++, bmpDataPtr += ((actualWidth - bitmap->_width) * bytes)) {
					for (int j = 0; j < bitmap->_width; j++, bmpDataPtr += bytes, bitmapData16++) {
						// Convert pixel components, put them in ABGR order.
						uint16 pixel = *bitmapData16;
						int r = pixel >> 11;
						bmpDataPtr[3] = (r << 3) | (r >> 2);
						int g = (pixel >> 5) & 0x3f;
						bmpDataPtr[2] = (g << 2) | (g >> 4);
						int b = pixel & 0x1f;
						bmpDataPtr[1] = (b << 3) | (b >> 2);
						if (pixel == 0xf81f) { // transparent
							bmpDataPtr[0] = 0;
							bitmap->_hasTransparency = true;
						} else {
							bmpDataPtr[0] = 255;
						}
					}
				}
			} else {
				if (bitmap->_format == 1 && imageData.format != format_32bpp)
					bitmap->convertToColorFormat(pic, format_32bpp);
				const uint32 *bitmapData32 = reinterpret_cast<const uint32 *>(imageData.getPixels());
				for (int i = 0; i < bitmap->_height; i++, bmpDataPtr += ((actualWidth - bitmap->_width) * bytes)) {
					for (int j = 0; j < bitmap->_width; j++, bmpDataPtr += bytes, bitmapData32++) {
						// Put pixel components in ABGR order.
						uint32 pixel = *bitmapData32;
						uint32 *cvtPixPtr8to32 = ((uint32 *)bmpDataPtr);
						*cvtPixPtr8to32 = SWAP_BYTES_32(pixel);
					}
				}
			}

			C3D_Tex *c3dTex = &textures[bitmap->_numTex * pic];
			C3D_TexInit(c3dTex, (u16)actualWidth, (u16)actualHeight, format);
			C3D_TexSetFilter(c3dTex, GPU_NEAREST, GPU_NEAREST);
			C3D_TexSetWrap(c3dTex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
			// Copy bitmap pixels into c3dTex in hardware-required sequence.
			if ((actualWidth < 64) && (actualHeight < 64)) {
				// C3D_SyncDisplayTransfer causes a thread hang if either dimension is less than 64 pixels.
				// "false" instructs NOT to reorder pixel components (we already did).
				custom3DS_DataToBlockTex((u32 *)bmpData,      0, 0, bitmap->_width, bitmap->_height,
				                         (u32 *)c3dTex->data, 0, 0, actualWidth,    actualHeight,
				                         bitmap->_width, bitmap->_height, format, false);
			} else {
				GSPGPU_FlushDataCache(bmpData, bytes * actualWidth * actualHeight);
				// GX_TRANSFER_FMT_RGBA8 is already 0
				// GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) = (1 << 0) | (1 << 1) = 0b01 | 0b10 = 0b11 = 3
				C3D_SyncDisplayTransfer((u32 *)bmpData, GX_BUFFER_DIM(actualWidth, actualHeight),
				                        (u32 *)c3dTex->data, GX_BUFFER_DIM(actualWidth, actualHeight), 3);
			}
		}

		linearFree(bmpData);
		bitmap->freeData();

		// Clone _backgroundShader and assign instance-specific attribute and buffer info.
		N3DS_3D::ShaderObj *shader = new N3DS_3D::ShaderObj(_backgroundShader);
		bitmap->_userData = shader;

		shader->addAttrLoader(0 /*position*/, GPU_FLOAT, 2);
		shader->addAttrLoader(1 /*texcoord*/, GPU_FLOAT, 2);
		BufInfo_Init(&shader->_bufInfo);
		if (g_grim->getGameType() == GType_MONKEY4) {
			void *vbo = custom3DS_CreateBuffer(bitmap->_numCoords * 4 * sizeof(float), bitmap->_texc, 0x4);
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
		BitmapData *data = bitmap->_data;
		N3DS_3D::ShaderObj *shader = (N3DS_3D::ShaderObj *)data->_userData;
		C3D_Tex *textures = (C3D_Tex *)bitmap->getTexIds();

		glTo3DS_Disable(ENUM3DS_CAP_DEPTH_TEST);

		glTo3DS_Enable(ENUM3DS_CAP_BLEND);
		glTo3DS_BlendFunc(GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);

		N3DS_3D::changeShader(shader);
		assert(layer < data->_numLayers);
		uint32 offset = data->_layers[layer]._offset;
		drawStart(0, 0, 0, 640, 480, &envBG_Smush);
			for (uint32 i = offset; i < offset + data->_layers[layer]._numImages; ++i) {
				glTo3DS_BindTexture(0, textures + data->_verts[i]._texid);

				u16 startVertex = data->_verts[i]._pos / 4 * 6;
				int numVertices = data->_verts[i]._verts / 4 * 6;
				N3DS_3D::getActiveContext()->applyContextState();
				C3D_DrawElements(GPU_TRIANGLES, numVertices, C3D_UNSIGNED_SHORT, (void *)((u16 *)_quadEBO + startVertex));
			}
		drawEnd(0);
		return;
	}

	int format = bitmap->getFormat();
	if ((format == 1 && !_renderBitmaps) || (format == 5 && !_renderZBitmaps)) {
		return;
	}

	if (format == 1) {
		C3D_Tex *textures = (C3D_Tex *)bitmap->getTexIds();
		if (bitmap->getFormat() == 1 && bitmap->getHasTransparency()) {
			glTo3DS_Enable(ENUM3DS_CAP_BLEND);
			glTo3DS_BlendFunc(GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
		} else {
			glTo3DS_Disable(ENUM3DS_CAP_BLEND);
		}

		N3DS_3D::ShaderObj *shader = (N3DS_3D::ShaderObj *)bitmap->_data->_userData;
		N3DS_3D::changeShader(shader);
		glTo3DS_Disable(ENUM3DS_CAP_DEPTH_TEST);
		glTo3DS_DepthMask(false);

		int cur_tex_idx = bitmap->getNumTex() * (bitmap->getActiveImage() - 1);
		glTo3DS_BindTexture(0, textures + cur_tex_idx);
		float width = bitmap->getWidth();
		float height = bitmap->getHeight();
		shader->setUniform("offset", GPU_VERTEX_SHADER, Math::Vector2d(float(dx) / float(_gameWidth), float(dy) / float(_gameHeight)));
		shader->setUniform("scale", GPU_VERTEX_SHADER, Math::Vector2d(width / float(_gameWidth), height / float(_gameHeight)));
		shader->setUniform("texcrop", GPU_VERTEX_SHADER, Math::Vector2d(width / float(nextHigher2((int)width)), height / float(nextHigher2((int)height))));
		drawStart(0, 0, 0, 640, 480, &envBG_Smush);
			N3DS_3D::getActiveContext()->applyContextState();
			C3D_DrawElements(GPU_TRIANGLES, 6, C3D_UNSIGNED_SHORT, (void *)_quadEBO);
		drawEnd(0);

		glTo3DS_Disable(ENUM3DS_CAP_BLEND);
		glTo3DS_DepthMask(true);
		glTo3DS_Enable(ENUM3DS_CAP_DEPTH_TEST);
	} else {
		// Only draw the manual zbuffer when enabled
		if (bitmap->getActiveImage() - 1 < bitmap->getNumImages()) {
			drawDepthBitmap(bitmap->getId(), dx, dy, bitmap->getWidth(), bitmap->getHeight(),
			                (char *)const_cast<void *>(bitmap->getData(bitmap->getActiveImage() - 1).getPixels()));
		} else {
			warning("zbuffer image has index out of bounds! %d/%d", bitmap->getActiveImage(), bitmap->getNumImages());
		}
		return;
	}
}

void GfxN3DS::drawDepthBitmap(int bitmapId, int x, int y, int w, int h, char *data) {
	static int prevId = -1;
	static int prevX = -1, prevY = -1;
	static int prevW = -1, prevH = -1;
	static char *prevData = nullptr;

	// Sometimes the data pointer is reused by the allocator between bitmaps
	// Use the bitmap ID to ensure we don't prevent an expected update
	if (bitmapId == prevId && prevX == x && prevY == y && prevW == w && prevH == h && data == prevData) {
		return;
	}

	prevId = bitmapId;
	prevX = x;
	prevY = y;
	prevW = w;
	prevH = h;
	prevData = data;

	// Swizzle the depth data into _zBuffer at the appropriate location, taking into account the dimensions of the buffer (1024 by 512).
	// NOTE: the "false" here is an indicator to NOT reverse the byte order in each pixel.
	custom3DS_DataToBlockTex((u32 *)data,     0, 0, w, h,
	                         (u32 *)_zBuffer, x, y, nextHigher2(_gameWidth), nextHigher2(_gameHeight),
	                         w, h, GPU_RGBA8, false);
}

void GfxN3DS::sendBitmapDepthVals() {
	// DMA into _gameScreenTarget's depth buffer, replacing its data with _zBuffer's data.
	GSPGPU_FlushDataCache(_zBuffer, nextHigher2(_gameWidth) * nextHigher2(_gameHeight) * 4);
	GX_RequestDma((u32 *)_zBuffer, (u32 *)_gameScreenTarget->frameBuf.depthBuf, nextHigher2(_gameWidth) * nextHigher2(_gameHeight) * 4);
	gspWaitForDMA();
}

void GfxN3DS::destroyBitmap(BitmapData *bitmap) {
	C3D_Tex *textures = (C3D_Tex *)bitmap->_texIds;
	if (textures) {
		for (int i = 0; i < bitmap->_numTex * bitmap->_numImages; i++) {
			C3D_TexDelete(textures + i);
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
		if (pixel == 0x00) {
			fntDataPtr[0] = fntDataPtr[1] = fntDataPtr[2] = fntDataPtr[3] = 0;
		} else if (pixel == 0x80) {
			fntDataPtr[0] = fntDataPtr[1] = fntDataPtr[2] = 0;
			fntDataPtr[3] = 255;
		} else if (pixel == 0xFF) {
			fntDataPtr[0] = fntDataPtr[1] = fntDataPtr[2] = fntDataPtr[3] = 255;
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

	FontUserData *userData = new FontUserData;
	font->setUserData(userData);
	userData->size = size;
	// Allocate linear memory for subtexture coordinates.
	userData->subTextures = (Tex3DS_SubTexture *)custom3DS_CreateBuffer(sizeof(Tex3DS_SubTexture) * charsWide * charsHigh, nullptr, 0x4);

	// Allocate linear memory for texture data via texture initialization.
	u32 pixelsWide = charsWide * size;
	u32 pixelsHigh = charsHigh * size;
	C3D_TexInit(&userData->texture, (u16)pixelsWide, (u16)pixelsHigh, GPU_RGBA8);
	C3D_TexSetFilter(&userData->texture, GPU_NEAREST, GPU_NEAREST);
	C3D_TexSetWrap(&userData->texture, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);

	// Allocate temp in linear memory AFTER subTextures and texture, to prevent fragmentation of unoccupied space.
	uint arraySize = size * size * bpp * charsWide * charsHigh;
	byte *temp = (byte *)custom3DS_CreateBuffer(sizeof(byte) * arraySize/*, nullptr, 0x4*/);

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
//	GSPGPU_FlushDataCache((void *)temp, sizeof(byte) * (u32)arraySize);
//	// GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) = (1 << 0) | (1 << 1) = 0b01 | 0b10 = 0b11 = 3
//	C3D_SyncDisplayTransfer((u32 *)temp,                   GX_BUFFER_DIM(pixelsWide, pixelsHigh),
//	                        (u32 *)userData->texture.data, GX_BUFFER_DIM(pixelsWide, pixelsHigh), 3);
	custom3DS_DataToBlockTex((u32 *)const_cast<uint8 *>(temp), 0, 0, pixelsWide, pixelsHigh,
	                         (u32 *)userData->texture.data,    0, 0, pixelsWide, pixelsHigh,
	                         pixelsWide, pixelsHigh, GPU_RGBA8);

	delete[] fntData;
	custom3DS_FreeBuffer(temp);
}

void GfxN3DS::destroyFont(Font *font) {
	if (font->is8Bit()) {
		const FontUserData *data = static_cast<const FontUserData *>(static_cast<const BitmapFont *>(font)->getUserData());
		if (data) {
			C3D_TexDelete(const_cast<C3D_Tex *>(&data->texture));
			// Free the buffer containing subtexture coordinates.
			custom3DS_FreeBuffer(data->subTextures);
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
		memcpy(vbo, bufData, numCharacters * 16 * sizeof(float));
	} else {
		vbo = custom3DS_CreateBuffer(numCharacters * 16 * sizeof(float), bufData, 0x4);
	}

	N3DS_3D::ShaderObj *textShader = new N3DS_3D::ShaderObj(_textShader);

	textShader->addAttrLoader(0 /*position*/, GPU_FLOAT, 2);
	textShader->addAttrLoader(1 /*texcoord*/, GPU_FLOAT, 2);
	textShader->addBufInfo(vbo, 4 * sizeof(float), 2, 0x10);

	TextUserData * td = new TextUserData;
	td->characters = numCharacters;
	td->shader = textShader;
	td->color = color;
	td->texture = const_cast<C3D_Tex *>(&userData->texture);
	text->setUserData(td);
	delete[] bufData;
}

void GfxN3DS::drawTextObject(const TextObject *text) {
	glTo3DS_Enable(ENUM3DS_CAP_BLEND);
	glTo3DS_Disable(ENUM3DS_CAP_DEPTH_TEST);

	const TextUserData * td = (const TextUserData *) text->getUserData();
	assert(td);
	N3DS_3D::changeShader(td->shader);

	Math::Vector3d colors(float(td->color.getRed()  ) / 255.0f,
	                      float(td->color.getGreen()) / 255.0f,
	                      float(td->color.getBlue() ) / 255.0f);
	td->shader->setUniform("color", GPU_VERTEX_SHADER, colors);
	glTo3DS_BindTexture(0, td->texture);

	drawStart(0, 0, 0, 640, 480, &envText);
	// Draw the text.
		N3DS_3D::getActiveContext()->applyContextState();
		C3D_DrawElements(GPU_TRIANGLES, (int)td->characters * 6, C3D_UNSIGNED_SHORT, (void *)_quadEBO);
	drawEnd(0);

	glTo3DS_Enable(ENUM3DS_CAP_DEPTH_TEST);
}

void GfxN3DS::destroyTextObject(TextObject *text) {
	TextUserData * td = reinterpret_cast<TextUserData *>(const_cast<void *>(text->getUserData()));
	if (!text->isBlastDraw()) {
		td->shader->freeAttachedBuffer(0);
	}
	text->setUserData(nullptr);

	delete td->shader;
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

#define PRIMITIVE_LENGTH 4																						// FINALIZED - ADDED
#define SET_PRIMITIVE_UNIFORMS(primProg) \
	primProg->setUniform("color", GPU_VERTEX_SHADER, colorV); \
	primProg->setUniform("scaleWH", GPU_VERTEX_SHADER, Math::Vector2d(1.f / _gameWidth, 1.f / _gameHeight))		// FINALIZED - ADDED


#undef PRIMITIVE_LENGTH																							// FINALIZED - ADDED
#undef SET_PRIMITIVE_UNIFORMS																					// FINALIZED - ADDED

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
		// Deallocate existing _smushTex (if any).
		C3D_TexDelete(&_smushTex);
		// Allocate a single 1024 x 512 C3D_Tex to handle all sizes of "frameFormat" movies.
		C3D_TexInit(&_smushTex, 1024, 512, frameFormat);
	}
	C3D_TexSetFilter(&_smushTex, GPU_NEAREST, GPU_NEAREST);
	C3D_TexSetWrap(&_smushTex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);

	// Copy frame pixels into _smushTex in hardware-required sequence.
	custom3DS_DataToBlockTex((u32 *)const_cast<uint8 *>(bitmap), 0, 0,                width,                height,
	                         (u32 *)_smushTex.data,              0, 0, (int)_smushTex.width, (int)_smushTex.height,
	                         width, height, frameFormat);

	_smushWidth = (int)(width);
	_smushHeight = (int)(height);
}

void GfxN3DS::drawMovieFrame(int offsetX, int offsetY) {
	N3DS_3D::changeShader(_smushShader);
	glTo3DS_Disable(ENUM3DS_CAP_DEPTH_TEST);

	// We specify the EBO in the draw command.
	// Since we aren't changing the size of _smushTex, divide by 1024 and 512 instead of nextHigher2(_smushWidth) and nextHigher2(_smushHeight).
	_smushShader->setUniform("texcrop", GPU_VERTEX_SHADER, Math::Vector2d(float(_smushWidth) / 1024, float(_smushHeight) / 512));
	_smushShader->setUniform("scale", GPU_VERTEX_SHADER, Math::Vector2d(float(_smushWidth) / float(_gameWidth), float(_smushHeight) / float(_gameHeight)));
	_smushShader->setUniform("offset", GPU_VERTEX_SHADER, Math::Vector2d(float(offsetX) / float(_gameWidth), float(offsetY) / float(_gameHeight)));
	glTo3DS_BindTexture(0, &_smushTex);

	drawStart(0, 0, 0, 640, 480, &envBG_Smush);
		N3DS_3D::getActiveContext()->applyContextState();
		C3D_DrawElements(GPU_TRIANGLES, 6, C3D_UNSIGNED_SHORT, (void *)_quadEBO);
	drawEnd(0);

	// EBOs aren't bound, so they don't need to be unbound.
	glTo3DS_Enable(ENUM3DS_CAP_DEPTH_TEST);
}

void GfxN3DS::releaseMovieFrame() {
	if (_smushTex.size > 0) {
		C3D_TexDelete(&_smushTex);
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
	Common::Array<GrimVertex> meshInfo;
	meshInfo.reserve(mesh->_numVertices * 5);
	for (int i = 0; i < mesh->_numFaces; ++i) {
		MeshFace *face = &mesh->_faces[i];
		face->_userData = new uint32;
		*(uint32 *)face->_userData = meshInfo.size();

		if (face->getNumVertices() < 3)
			continue;

#define VERT(j) (&mesh->_vertices[3 * face->getVertex(j)])
#define TEXVERT(j) (face->hasTexture() ? &mesh->_textureVerts[2 * face->getTextureVertex(j)] : zero_texVerts)
#define NORMAL(j) (&mesh->_vertNormals[3 * face->getVertex(j)])

		for (int j = 2; j < face->getNumVertices(); ++j) {
			meshInfo.push_back(GrimVertex(VERT(0), TEXVERT(0), NORMAL(0)));
			meshInfo.push_back(GrimVertex(VERT(j - 1), TEXVERT(j - 1), NORMAL(j - 1)));
			meshInfo.push_back(GrimVertex(VERT(j), TEXVERT(j), NORMAL(j)));
		}

#undef VERT
#undef TEXVERT
#undef NORMAL

	}

	if (meshInfo.empty()) {
		mesh->_userData = nullptr;
		return;
	}

	ModelUserData *mud = new ModelUserData;
	mesh->_userData = mud;

	mud->_meshInfoVBO = custom3DS_CreateBuffer(meshInfo.size() * sizeof(GrimVertex), &meshInfo[0], 0x4);

	N3DS_3D::ShaderObj *actorShader = new N3DS_3D::ShaderObj(_actorShader);
	actorShader->addAttrLoader(0 /*position*/, GPU_FLOAT, 3);
	actorShader->addAttrLoader(1 /*texcoord*/, GPU_FLOAT, 2);
	actorShader->addAttrLoader(2 /* normal */, GPU_FLOAT, 3);
//	actorShader->disableVertexAttribute("color", Math::Vector4d(1.f, 1.f, 1.f, 1.f));
	actorShader->addBufInfo(mud->_meshInfoVBO, 8 * sizeof(float), 3, 0x210);
	mud->_shader = actorShader;

	actorShader = new N3DS_3D::ShaderObj(_actorLightsShader);
	actorShader->addAttrLoader(0 /*position*/, GPU_FLOAT, 3);
	actorShader->addAttrLoader(1 /*texcoord*/, GPU_FLOAT, 2);
	actorShader->addAttrLoader(2 /* normal */, GPU_FLOAT, 3);
//	actorShader->disableVertexAttribute("color", Math::Vector4d(1.f, 1.f, 1.f, 1.f));
	actorShader->addBufInfo(mud->_meshInfoVBO, 8 * sizeof(float), 3, 0x210);
	mud->_shaderLights = actorShader;
}

void GfxN3DS::destroyMesh(const Mesh *mesh) {
	ModelUserData *mud = static_cast<ModelUserData *>(mesh->_userData);

	for (int i = 0; i < mesh->_numFaces; ++i) {
		MeshFace *face = &mesh->_faces[i];
		if (face->_userData) {
			uint32 *data = static_cast<uint32 *>(face->_userData);
			delete data;
		}
	}

	if (!mud)
		return;

	custom3DS_FreeBuffer(mud->_meshInfoVBO);

	delete mud->_shader;
	delete mud->_shaderLights;
	delete mud;
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

#define SUBTEXTURE_WIDTH 256											// DEFINITE? - ADDED
#define SUBTEX_FROM_SCREEN(_id, _srcbuf, _dstbuf, _x, _y, _w, _h) \
	for (i = _y; i < _y + _h; i++) { \
		memcpy((void *)(_dstbuf + (i * SUBTEXTURE_WIDTH * 4)), \
		       (void *)(_srcbuf + ((i * _screenWidth + _x) * 4)), \
		       _w * 4); \
	} \
	createSpecialtyTexture(_id, _dstbuf, _w, _h)						// DEFINITE? - ADDED

void GfxN3DS::makeScreenTextures() {									// DEFINITE? - ADDED
}																		// DEFINITE? - ADDED

#undef SUBTEX_FROM_SCREEN												// DEFINITE? - ADDED
#undef SUBTEXTURE_WIDTH													// DEFINITE? - ADDED

Bitmap *GfxN3DS::getScreenshot(int w, int h, bool useStored) {
	// Temporary hack until this function is properly implemented.
	Bitmap *bmp;
	return bmp;
}

void GfxN3DS::createSpecialtyTextureFromScreen(uint id, uint8 *data, int x, int y, int width, int height) {
}

void GfxN3DS::setBlendMode(bool additive) {
	if (additive) {
		glTo3DS_BlendFunc(GPU_SRC_ALPHA, GPU_ONE);
	} else {
		glTo3DS_BlendFunc(GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
	}
}

#undef CONSTRUCT_SHADER
#undef DECOMPOSE_SHADER
}

#endif // __3DS__
