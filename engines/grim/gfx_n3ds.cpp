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

//#include <3ds.h>
//#include <citro3d.h>

#include "graphics/3ds/n3d.h"
#include "graphics/3ds/z3d.h"

#include "common/endian.h"
#include "common/file.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"

// #if defined(USE_OPENGL_SHADERS)
#if defined(__3DS__)

#include "graphics/surface.h"
// #include "graphics/opengl/context.h"

#include "engines/grim/actor.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/colormap.h"
#include "engines/grim/emi/modelemi.h"
#include "engines/grim/font.h"

#include "engines/grim/shaders-3ds/emi_actor_shbin.h"																				// FINALIZED - ADDED
#include "engines/grim/shaders-3ds/emi_actorlights_shbin.h"																			// FINALIZED - ADDED
#include "engines/grim/shaders-3ds/emi_background_shbin.h"																			// FINALIZED - ADDED
#include "engines/grim/shaders-3ds/emi_dimplane_shbin.h"																			// FINALIZED - ADDED
#include "engines/grim/shaders-3ds/emi_sprite_shbin.h"																				// FINALIZED - ADDED
#include "engines/grim/shaders-3ds/grim_actor_shbin.h"																				// FINALIZED - ADDED
#include "engines/grim/shaders-3ds/grim_actorlights_shbin.h"																		// FINALIZED - ADDED
#include "engines/grim/shaders-3ds/grim_background_shbin.h"																			// FINALIZED - ADDED
#include "engines/grim/shaders-3ds/grim_dim_shbin.h"																				// FINALIZED - ADDED
#include "engines/grim/shaders-3ds/grim_emerg_shbin.h"																				// FINALIZED - ADDED
#include "engines/grim/shaders-3ds/grim_primRect_shbin.h"																			// FINALIZED - ADDED
#include "engines/grim/shaders-3ds/grim_primLines_shbin.h"																			// FINALIZED - ADDED
#include "engines/grim/shaders-3ds/grim_shadowplane_shbin.h"																		// FINALIZED - ADDED
#include "engines/grim/shaders-3ds/grim_smush_shbin.h"																				// FINALIZED - ADDED
#include "engines/grim/shaders-3ds/grim_text_shbin.h"																				// FINALIZED - ADDED
#include "engines/grim/shaders-3ds/manualclear_shbin.h"																				// FINALIZED - ADDED

// #include "engines/grim/gfx_opengl_shaders.h"
#include "engines/grim/gfx_n3ds.h"																									// FINALIZED
#include "engines/grim/grim.h"
#include "engines/grim/material.h"
#include "engines/grim/model.h"
#include "engines/grim/primitives.h"
#include "engines/grim/set.h"
#include "engines/grim/sprite.h"

namespace Grim {

template<class T>
static T nextHigher2(T k) {
	if (k == 0)
		return 1;
	--k;

	for (uint i = 1; i < sizeof(T) * 8; i <<= 1)
		k = k | k >> i;

	return k + 1;
}

static float untextured_quad[] = {																									// DEFINITE? - ADDED
//	X   , Y																															// DEFINITE? - ADDED
	0.0f, 0.0f,																														// DEFINITE? - ADDED
	1.0f, 0.0f,																														// DEFINITE? - ADDED
	1.0f, 1.0f,																														// DEFINITE? - ADDED
	0.0f, 1.0f,																														// DEFINITE? - ADDED
};																																	// DEFINITE? - ADDED

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
//	OpenGL::Shader * shader;
	N3DS_3D::ShaderObj *shader;																										// FINALIZED
	uint32 characters;
	Color  color;

//	GLuint texture;
	C3D_Tex *texture;																												// FINALIZED
};

struct FontUserData {
	int size;
//	GLuint texture;
	C3D_Tex *texture;																												// FINALIZED
	// int numSubTextures;																												// DEFINITE?
	Tex3DS_SubTexture *subTextures;																									// FINALIZED

};

struct EMIModelUserData {
//	OpenGL::Shader *_shader;
//	OpenGL::Shader *_shaderLights;
//	uint32 _texCoordsVBO;
//	uint32 _colorMapVBO;
//	uint32 _verticesVBO;
//	uint32 _normalsVBO;
	N3DS_3D::ShaderObj *_shader;																									// FINALIZED
	N3DS_3D::ShaderObj *_shaderLights;																								// FINALIZED
	void  *_texCoordsVBO;																											// FINALIZED
	void  *_colorMapVBO;																											// FINALIZED
	void  *_verticesVBO;																											// FINALIZED
	void  *_normalsVBO;																												// FINALIZED
};

struct ModelUserData {
//	OpenGL::Shader *_shader;
//	OpenGL::Shader *_shaderLights;
//	uint32 _meshInfoVBO;
	N3DS_3D::ShaderObj *_shader;																									// FINALIZED
	N3DS_3D::ShaderObj *_shaderLights;																								// FINALIZED
	void *_meshInfoVBO;																												// FINALIZED
};

struct ShadowUserData {
//	uint32 _verticesVBO;
//	uint32 _indicesVBO;
	void *_verticesVBO;																												// FINALIZED
	void *_indicesVBO;																												// FINALIZED
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

GfxN3DS::GfxN3DS() {
	_backendContext = (N3DS_3D::N3DContext *)N3DS_3D::createContext();																// DEFINITE? - ADDED
	_grimContext = (N3DS_3D::N3DContext *)N3DS_3D::createOGLContext();																// DEFINITE? - ADDED
	_gameScreenTex = N3D_GetGameScreen();																							// DEFINITE? - ADDED
	debug("_gstex addr: %lx", (u32)_gameScreenTex);
	u32 vaddr = (u32)_gameScreenTex->data;
	debug("_gstex->data addr: %lx", vaddr);
	debug("_gstex is in vram: %u", (vaddr >= OS_VRAM_VADDR && vaddr < OS_VRAM_VADDR + OS_VRAM_SIZE));




	_gameScreenTarget = N3D_C3D_RenderTargetCreateFromTex(_gameScreenTex, GPU_TEXFACE_2D, 0, GPU_RB_DEPTH24_STENCIL8);				// DEFINITE? - ADDED
	debug("_gstarg is null: %d", (_gameScreenTarget == NULL));

	debug("GfxN3DS::GfxN3DS - Creating linear alloc (_screenCopySpace)");
	_screenCopySpace = N3DS_3D::createBuffer(640 * 480 * 4);																		// DEFINITE? - ADDED
	debug("GfxN3DS::GfxN3DS - Linear alloc created: %u bytes (_screenCopySpace)", linearGetSize(_screenCopySpace));

	N3D_C3D_TexEnvInit(&envNormal);																									// DEFINITE? - ADDED
	N3D_C3D_TexEnvFunc(&envNormal, C3D_Both, GPU_MODULATE);																			// DEFINITE? - ADDED
	N3D_C3D_TexEnvColor(&envNormal, 0x00000000);																					// DEFINITE? - ADDED
	N3D_C3D_TexEnvSrc(&envNormal, C3D_Both, GPU_TEXTURE0, GPU_PREVIOUS, GPU_CONSTANT);												// DEFINITE? - ADDED
	N3D_C3D_TexEnvOpRgb(&envNormal, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_ALPHA);						// DEFINITE? - ADDED
	N3D_C3D_TexEnvOpAlpha(&envNormal, GPU_TEVOP_A_SRC_ALPHA, GPU_TEVOP_A_SRC_ALPHA, GPU_TEVOP_A_SRC_ALPHA);							// DEFINITE? - ADDED

	N3D_C3D_TexEnvInit(&envSmush);																									// DEFINITE? - ADDED
	N3D_C3D_TexEnvFunc(&envSmush, C3D_Both, GPU_REPLACE);																			// DEFINITE? - ADDED
	N3D_C3D_TexEnvColor(&envSmush, 0x00000000);																						// DEFINITE? - ADDED
	N3D_C3D_TexEnvSrc(&envSmush, C3D_RGB, GPU_TEXTURE0/*, 0, 0*/);																	// DEFINITE? - ADDED
	N3D_C3D_TexEnvSrc(&envSmush, C3D_Alpha, GPU_PRIMARY_COLOR/*, 0, 0*/);															// DEFINITE? - ADDED
	N3D_C3D_TexEnvOpRgb(&envSmush, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_ALPHA);						// DEFINITE? - ADDED
	N3D_C3D_TexEnvOpAlpha(&envSmush, GPU_TEVOP_A_SRC_ALPHA, GPU_TEVOP_A_SRC_ALPHA, GPU_TEVOP_A_SRC_ALPHA);							// DEFINITE? - ADDED

	N3D_C3D_TexEnvInit(&envText);																									// DEFINITE? - ADDED
	N3D_C3D_TexEnvFunc(&envText, C3D_Both, GPU_MODULATE);																			// DEFINITE? - ADDED
	N3D_C3D_TexEnvColor(&envText, 0x00000000);																						// DEFINITE? - ADDED
	N3D_C3D_TexEnvSrc(&envText, C3D_RGB, GPU_PRIMARY_COLOR, GPU_TEXTURE0/*, 0*/);													// DEFINITE? - ADDED
	N3D_C3D_TexEnvSrc(&envText, C3D_Alpha, GPU_PRIMARY_COLOR, GPU_TEXTURE0/*, 0*/);													// DEFINITE? - ADDED
	N3D_C3D_TexEnvOpRgb(&envText, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_ALPHA);						// DEFINITE? - ADDED
	N3D_C3D_TexEnvOpAlpha(&envText, GPU_TEVOP_A_SRC_ALPHA, GPU_TEVOP_A_SRC_ALPHA, GPU_TEVOP_A_SRC_ALPHA);							// DEFINITE? - ADDED

	N3D_C3D_TexEnvInit(&envEmerg);																									// DEFINITE? - ADDED
	N3D_C3D_TexEnvFunc(&envEmerg, C3D_Both, GPU_REPLACE);																			// DEFINITE? - ADDED
	N3D_C3D_TexEnvColor(&envEmerg, 0x00000000);																						// DEFINITE? - ADDED
	N3D_C3D_TexEnvSrc(&envEmerg, C3D_RGB, GPU_PRIMARY_COLOR/*, 0, 0*/);																// DEFINITE? - ADDED
	N3D_C3D_TexEnvSrc(&envEmerg, C3D_Alpha, GPU_TEXTURE0/*, 0, 0*/);																// DEFINITE? - ADDED
	N3D_C3D_TexEnvOpRgb(&envEmerg, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_ALPHA);						// DEFINITE? - ADDED
	N3D_C3D_TexEnvOpAlpha(&envEmerg, GPU_TEVOP_A_SRC_ALPHA, GPU_TEVOP_A_SRC_ALPHA, GPU_TEVOP_A_SRC_ALPHA);							// DEFINITE? - ADDED




//	type = Graphics::RendererType::kRendererTypeOpenGLShaders;
	type = Graphics::RendererType::kRendererTypeN3DS;																				// DEFINITE?
//	_smushTexId = 0;
	_matrixStack.push(Math::Matrix4());
	_fov = -1.0;
	_nclip = -1;
	_fclip = -1;
	_selectedTexture = nullptr;
//	_emergTexture = 0;
	//_emergTexture = nullptr;
	_maxLights = 8;
//	_lights = new GLSLight[_maxLights];
	_lights = new LightObj[_maxLights];																								// DEFINITE?
	_lightsEnabled = false;
	_hasAmbientLight = false;
//	_backgroundProgram = nullptr;
//	_smushProgram = nullptr;
//	_textProgram = nullptr;
//	_emergProgram = nullptr;
//	_actorProgram = nullptr;
//	_actorLightsProgram = nullptr;
//	_spriteProgram = nullptr;
//	_primitiveProgram = nullptr;
//	_irisProgram = nullptr;
//	_shadowPlaneProgram = nullptr;
//	_dimProgram = nullptr;
//	_dimPlaneProgram = nullptr;
//	_dimRegionProgram = nullptr;
	_programBackground = nullptr;																									// DEFINITE?
	_programSmush = nullptr;																										// DEFINITE?
	_programText = nullptr;																											// DEFINITE?
	_programEmerg = nullptr;																										// DEFINITE?
	_programActor = nullptr;																										// DEFINITE?
	_programActorLights = nullptr;																									// DEFINITE?
	_programSprite = nullptr;																										// DEFINITE?
	_programPrimRect = nullptr;																										// DEFINITE? - REIMPLEMENTED
	_programPrimLines = nullptr;																									// DEFINITE? - REIMPLEMENTED
	_programIris = nullptr;																											// DEFINITE?
	_programShadowPlane = nullptr;																									// DEFINITE?
	_programDim = nullptr;																											// DEFINITE?
	_programDimPlane = nullptr;																										// DEFINITE?
	_programDimRegion = nullptr;																									// DEFINITE?
	_programClear = nullptr;																										// DEFINITE? - ADDED

	float div = 6.0f;
	_overworldProjMatrix = makeFrustumMatrix(-1.f / div, 1.f / div, -0.75f / div, 0.75f / div, 1.0f / div, 3276.8f);

	// GL_LEQUAL as glDepthFunc ensures that subsequent drawing attempts for														// DEFINITE? - MOVED from setupScreen()
	// the same triangles are not ignored by the depth test.																		// DEFINITE? - MOVED from setupScreen()
	// That's necessary for EMI where some models have multiple faces which															// DEFINITE? - MOVED from setupScreen()
	// refer to the same vertices. The first face is usually using the																// DEFINITE? - MOVED from setupScreen()
	// color map and the following are using textures.																				// DEFINITE? - MOVED from setupScreen()
	_depthFunc = (g_grim->getGameType() == GType_MONKEY4) ? GPU_LEQUAL : GPU_LESS;													// DEFINITE? - MOVED from setupScreen(), MODIFIED

}

GfxN3DS::~GfxN3DS() {
	releaseMovieFrame();
	for (unsigned int i = 0; i < _numSpecialtyTextures; i++) {
		destroyTexture(&_specialtyTextures[i]);
	}
	delete[] _lights;

	N3D_C3D_RenderTargetDelete(_gameScreenTarget);																					// DEFINITE? - ADDED

	debug("GfxN3DS::~GfxN3DS - Deleting linear alloc: -%u bytes (_blankVBO)", linearGetSize(_blankVBO));
	N3DS_3D::freeBuffer(_blankVBO);
	debug("GfxN3DS::~GfxN3DS - Linear alloc deleted (_blankVBO).");
	debug("GfxN3DS::~GfxN3DS - Deleting linear alloc: -%u bytes (_smushVBO)", linearGetSize(_smushVBO));
	N3DS_3D::freeBuffer(_smushVBO);
	debug("GfxN3DS::~GfxN3DS - Linear alloc deleted (_smushVBO).");
	debug("GfxN3DS::~GfxN3DS - Deleting linear alloc: -%u bytes (_quadEBO)", linearGetSize(_quadEBO));
	N3DS_3D::freeBuffer(_quadEBO);
	debug("GfxN3DS::~GfxN3DS - Linear alloc deleted (_quadEBO).");
	debug("GfxN3DS::~GfxN3DS - Deleting linear alloc: -%u bytes (_spriteVBO)", linearGetSize(_spriteVBO));
	N3DS_3D::freeBuffer(_spriteVBO);
	debug("GfxN3DS::~GfxN3DS - Linear alloc deleted (_spriteVBO).");
	debug("GfxN3DS::~GfxN3DS - Deleting linear alloc: -%u bytes (_zBuf)", linearGetSize(_zBuf));
	N3DS_3D::freeBuffer(_zBuf);
	debug("GfxN3DS::~GfxN3DS - Linear alloc deleted (_zBuf).");

	debug("GfxN3DS::~GfxN3DS - Deleting linear alloc: -%u bytes (_irisVBO)", linearGetSize(_irisVBO));
	N3DS_3D::freeBuffer(_irisVBO);
	debug("GfxN3DS::~GfxN3DS - Linear alloc deleted (_irisVBO).");
	debug("GfxN3DS::~GfxN3DS - Deleting linear alloc: -%u bytes (_dimVBO)", linearGetSize(_dimVBO));
	N3DS_3D::freeBuffer(_dimVBO);
	debug("GfxN3DS::~GfxN3DS - Linear alloc deleted (_dimVBO).");
	debug("GfxN3DS::~GfxN3DS - Deleting linear alloc: -%u bytes (_dimRegionVBO)", linearGetSize(_dimRegionVBO));
	N3DS_3D::freeBuffer(_dimRegionVBO);
	debug("GfxN3DS::~GfxN3DS - Linear alloc deleted (_dimRegionVBO).");
	debug("GfxN3DS::~GfxN3DS - Deleting linear alloc: -%u bytes (_blastVBO)", linearGetSize(_blastVBO));
	N3DS_3D::freeBuffer(_blastVBO);
	debug("GfxN3DS::~GfxN3DS - Linear alloc deleted (_blastVBO).");

//	delete _backgroundProgram;
//	delete _smushProgram;
//	delete _textProgram;
//	delete _emergProgram;
//	delete _actorProgram;
//	delete _actorLightsProgram;
//	delete _spriteProgram;
//	delete _primitiveProgram;
//	delete _irisProgram;
//	delete _shadowPlaneProgram;
//	delete _dimProgram;
//	delete _dimPlaneProgram;
//	delete _dimRegionProgram;
//	glDeleteTextures(1, &_storedDisplay);
//	glDeleteTextures(1, &_emergTexture);
	delete _programBackground;																										// DEFINITE?
	delete _programSmush;																											// DEFINITE?
	delete _programText;																											// DEFINITE?
	delete _programEmerg;																											// DEFINITE?
	delete _programActor;																											// DEFINITE?
	delete _programActorLights;																										// DEFINITE?
	delete _programSprite;																											// DEFINITE?
	delete _programIris;																											// DEFINITE?
	debug("GfxN3DS::~GfxN3DS - Deleting linear alloc: -%u bytes (_primVBOChunk)", linearGetSize(_primVBOChunk));
	N3DS_3D::freeBuffer(_primVBOChunk);																								// DEFINITE? - ADDED
	debug("GfxN3DS::~GfxN3DS - Linear alloc deleted (_primVBOChunk).");
	delete _programPrimRect;																										// DEFINITE? - REIMPLEMENTED
	delete _programPrimLines;																										// DEFINITE? - REIMPLEMENTED
	delete _programShadowPlane;																										// DEFINITE?
	delete _programDimRegion;																										// DEFINITE?
	delete _programDim;																												// DEFINITE?
	delete _programDimPlane;																										// DEFINITE?
	delete _programClear;																											// DEFINITE? - ADDED
	debug("GfxN3DS::~GfxN3DS - Deleting tex: -%u bytes (_storedDisplay)", _storedDisplay.size);
	debug("GfxN3DS::~GfxN3DS - Deleting tex: -%u bytes (_emergTexture)", _emergTexture.size);
	N3D_C3D_TexDelete(&_storedDisplay);																								// DEFINITE?
	debug("GfxN3DS::~GfxN3DS - Tex deleted (_storedDisplay)");
	N3D_C3D_TexDelete(&_emergTexture);																								// DEFINITE?
	debug("GfxN3DS::~GfxN3DS - Tex deleted (_emergTexture)");

	debug("GfxN3DS::~GfxN3DS - Deleting linear alloc: -%u bytes (_screenCopySpace)", linearGetSize(_screenCopySpace));
	N3DS_3D::freeBuffer(_screenCopySpace);																							// DEFINITE? - ADDED
	debug("GfxN3DS::~GfxN3DS - Linear alloc deleted (_screenCopySpace).");

	N3DS_3D::destroyNative3D();																										// DEFINITE? - ADDED
}

void GfxN3DS::setupZBuffer() {
//	GLint format = GL_LUMINANCE_ALPHA;
//	GLenum ztype = GL_UNSIGNED_BYTE;
	float width = _gameWidth;
	float height = _gameHeight;

//	glGenTextures(1, (GLuint *)&_zBufTex);
//	glActiveTexture(GL_TEXTURE1);
//	glBindTexture(GL_TEXTURE_2D, _zBufTex);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexImage2D(GL_TEXTURE_2D, 0, format, nextHigher2((int)width), nextHigher2((int)height), 0, format, ztype, nullptr);	// nullptr means blank tex is created
//	glActiveTexture(GL_TEXTURE0);
	debug("GfxN3DS::setupZBuffer - Creating linear alloc (_zBuf)");
	_zBuf = N3DS_3D::createBuffer(nextHigher2((int)width) * nextHigher2((int)height) * 4);											// DEFINITE
	debug("GfxN3DS::setupZBuffer - Linear alloc created: %u bytes (_zBuf)", linearGetSize(_zBuf));

	//STORE AND WRITE Z-BUFFER ARRAY BEFORE SENDING TO ACTUAL BUFFER

//	_zBufTexCrop = Math::Vector2d(width / nextHigher2((int)width), height / nextHigher2((int)height));								// (WE DON'T NEED THIS)
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

//	_quadEBO = OpenGL::Shader::createBuffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW);
	debug("GfxN3DS::setupQuadEBO - Creating linear alloc (_quadEBO)");
	_quadEBO = N3DS_3D::createBuffer(sizeof(quad_indices), quad_indices);															// DEFINITE?
	debug("GfxN3DS::setupQuadEBO - Linear alloc created: %u bytes (_quadEBO)", linearGetSize(_quadEBO));
}

void GfxN3DS::setupUntexturedQuad() {																								// DEFINITE? - ADDED
	debug("GfxN3DS::setupUntexturedQuad - Creating linear alloc (_blankVBO)");
	_blankVBO = N3DS_3D::createBuffer(sizeof(untextured_quad), untextured_quad);													// DEFINITE? - ADDED
	debug("GfxN3DS::setupUntexturedQuad - Linear alloc created: %u bytes (_blankVBO)", linearGetSize(_blankVBO));
	_programClear->addAttrLoader(0, GPU_FLOAT, 2);						// v0 = position											// DEFINITE? - ADDED
	_programClear->addBufInfo(_blankVBO, 2 * sizeof(float), 1, 0x0);																// DEFINITE? - ADDED

	if (g_grim->getGameType() == GType_MONKEY4) {																					// DEFINITE? - MOVED from setupTexturedQuad()
		_programDimPlane->addAttrLoader(0, GPU_FLOAT, 2);				// v0 = position											// DEFINITE? - MOVED from setupTexturedQuad()
		_programDimPlane->addBufInfo(_blankVBO, 2 * sizeof(float), 2, 0x0);															// DEFINITE? - MOVED from setupTexturedQuad()
	}																																// DEFINITE? - MOVED from setupTexturedQuad()
}																																	// DEFINITE? - ADDED

void GfxN3DS::setupTexturedQuad() {
//	_smushVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(textured_quad), textured_quad, GL_STATIC_DRAW);
//	_smushProgram->enableVertexAttribute("position", _smushVBO, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
//	_smushProgram->enableVertexAttribute("texcoord", _smushVBO, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
	debug("GfxN3DS::setupTexturedQuad - Creating linear alloc (_smushVBO)");
	_smushVBO = N3DS_3D::createBuffer(sizeof(textured_quad), textured_quad);														// DEFINITE?
	debug("GfxN3DS::setupTexturedQuad - Linear alloc created: %u bytes (_smushVBO)", linearGetSize(_smushVBO));
	_programSmush->addAttrLoader(0, GPU_FLOAT, 2);						// v0 = position											// DEFINITE?
	_programSmush->addAttrLoader(1, GPU_FLOAT, 2);						// v1 = texcoord											// DEFINITE?
	_programSmush->addBufInfo(_smushVBO, 4 * sizeof(float), 2, 0x10);																// DEFINITE?

//	_emergProgram->enableVertexAttribute("position", _smushVBO, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
//	_emergProgram->enableVertexAttribute("texcoord", _smushVBO, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
	_programEmerg->addAttrLoader(0, GPU_FLOAT, 2);						// v0 = position											// DEFINITE?
	_programEmerg->addAttrLoader(1, GPU_FLOAT, 2);						// v1 = texcoord											// DEFINITE?
	_programEmerg->addBufInfo(_smushVBO, 4 * sizeof(float), 2, 0x10);																// DEFINITE?

	if (g_grim->getGameType() == GType_GRIM) {
//		_backgroundProgram->enableVertexAttribute("position", _smushVBO, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
//		_backgroundProgram->enableVertexAttribute("texcoord", _smushVBO, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
		_programBackground->addAttrLoader(0, GPU_FLOAT, 2);				// v0 = position											// DEFINITE?
		_programBackground->addAttrLoader(1, GPU_FLOAT, 2);				// v1 = texcoord											// DEFINITE?
		_programBackground->addBufInfo(_smushVBO, 4 * sizeof(float), 2, 0x10);														// DEFINITE?
//	} else {
//		_dimPlaneProgram->enableVertexAttribute("position", _smushVBO, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	}
}

void GfxN3DS::setupTexturedCenteredQuad() {
//	_spriteVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(textured_quad_centered), textured_quad_centered, GL_STATIC_DRAW);
//	_spriteProgram->enableVertexAttribute("position", _spriteVBO, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
//	_spriteProgram->enableVertexAttribute("texcoord", _spriteVBO, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
//	_spriteProgram->disableVertexAttribute("color", Math::Vector4d(1.0f, 1.0f, 1.0f, 1.0f));
	debug("GfxN3DS::setupTexturedCenteredQuad - Creating linear alloc (_spriteVBO)");
	_spriteVBO = N3DS_3D::createBuffer(sizeof(textured_quad_centered), textured_quad_centered);										// DEFINITE?
	debug("GfxN3DS::setupTexturedCenteredQuad - Linear alloc created: %u bytes (_spriteVBO)", linearGetSize(_spriteVBO));
	_programSprite->addAttrLoader(0, GPU_FLOAT, 3);						// v0 = position											// DEFINITE?
	_programSprite->addAttrLoader(1, GPU_FLOAT, 2);						// v1 = texcoord											// DEFINITE?
	_programSprite->addBufInfo(_spriteVBO, 5 * sizeof(float), 2, 0x10);																// DEFINITE?
}

void GfxN3DS::setupPrimitives() {
#define SIZEOF_PRIMVBO sizeof(float) * 8
	uint32 numVBOs = ARRAYSIZE(_primitiveVBOs);
//	glGenBuffers(numVBOs, _primitiveVBOs);
	debug("GfxN3DS::setupPrimitives - Creating linear alloc (_primVBOChunk)");
	_primVBOChunk = (float *)N3DS_3D::createBuffer(numVBOs * SIZEOF_PRIMVBO);														// DEFINITE?
	debug("GfxN3DS::setupPrimitives - Linear alloc created: %u bytes (_primVBOChunk)", linearGetSize(_primVBOChunk));
	_lastPrimitive = 0;																												// DEFINITE? - ADDED
	_currentPrimitive = 0;
#undef SIZEOF_PRIMVBO

	for (uint32 i = 0; i < numVBOs; ++i) {
//		glBindBuffer(GL_ARRAY_BUFFER, _primitiveVBOs[i]);
//		glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
		//_primitiveVBOs[i] = (void *)&_primVBOChunk[i * 8];							// OLD
		_primitiveVBOs[i] = (float *)_primVBOChunk + (i * 8);																		// DEFINITE?

	}

	// from GfxN3DS::drawGenericPrimitive: _primitiveProgram->enableVertexAttribute("position", prim, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	_programPrimRect->addAttrLoader(0, GPU_FLOAT, 2);					// v0 = position											// DEFINITE?
	_programPrimRect->addBufInfo(_primVBOChunk, 2 * sizeof(float), 1, 0x0);															// DEFINITE? - ADDED
	_programPrimLines->addAttrLoader(0, GPU_FLOAT, 2);					// v0 = position											// DEFINITE?
	_programPrimLines->addBufInfo(_primVBOChunk, 2 * sizeof(float), 1, 0x0);														// DEFINITE? - ADDED

	if (g_grim->getGameType() == GType_MONKEY4)
		return;

//	glGenBuffers(1, &_irisVBO);
//	glBindBuffer(GL_ARRAY_BUFFER, _irisVBO);
//	glBufferData(GL_ARRAY_BUFFER, 20 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	debug("GfxN3DS::setupPrimitives - Creating linear alloc (_irisVBO)");
	_irisVBO = N3DS_3D::createBuffer(20 * sizeof(float));																			// DEFINITE?
	debug("GfxN3DS::setupPrimitives - Linear alloc created: %u bytes (_irisVBO)", linearGetSize(_irisVBO));

//	_irisProgram->enableVertexAttribute("position", _irisVBO, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	_programIris->addAttrLoader(0, GPU_FLOAT, 2);						// v0 = position											// DEFINITE?
	_programIris->addBufInfo(_irisVBO, 2 * sizeof(float), 1, 0x0);																	// DEFINITE?

//	glGenBuffers(1, &_dimVBO);
//	glBindBuffer(GL_ARRAY_BUFFER, _dimVBO);
	debug("GfxN3DS::setupPrimitives - Creating linear alloc (_dimVBO)");
	_dimVBO = N3DS_3D::createBuffer(12 * sizeof(float));																			// DEFINITE?
	debug("GfxN3DS::setupPrimitives - Linear alloc created: %u bytes (_dimVBO)", linearGetSize(_dimVBO));

	float points[12] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
	};

//	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), points, GL_DYNAMIC_DRAW);
	memcpy(_dimVBO, points, 12 * sizeof(float));																					// DEFINITE?

//	_dimProgram->enableVertexAttribute("position", _dimVBO, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
//	_dimProgram->enableVertexAttribute("texcoord", _dimVBO, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	_programDim->addAttrLoader(0, GPU_FLOAT, 2);						// v0 = position											// DEFINITE?
	_programDim->addAttrLoader(1, GPU_FLOAT, 2);						// v1 = texcoord											// DEFINITE?
	_programDim->addBufInfo(_dimVBO, 2 * sizeof(float), 1, 0x0);		// separate buffers											// DEFINITE?
	_programDim->addBufInfo(_dimVBO, 2 * sizeof(float), 1, 0x1);		// separate buffers											// DEFINITE?

//	glGenBuffers(1, &_dimRegionVBO);
//	glBindBuffer(GL_ARRAY_BUFFER, _dimRegionVBO);
	debug("GfxN3DS::setupPrimitives - Creating linear alloc (_dimRegionVBO)");
	_dimRegionVBO = N3DS_3D::createBuffer(24 * sizeof(float));																		// DEFINITE?
	debug("GfxN3DS::setupPrimitives - Linear alloc created: %u bytes (_dimRegionVBO)", linearGetSize(_dimRegionVBO));

//	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

//	_dimRegionProgram->enableVertexAttribute("position", _dimRegionVBO, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
//	_dimRegionProgram->enableVertexAttribute("texcoord", _dimRegionVBO, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
	_programDimRegion->addAttrLoader(0, GPU_FLOAT, 2);					// v0 = position											// DEFINITE?
	_programDimRegion->addAttrLoader(1, GPU_FLOAT, 2);					// v1 = texcoord											// DEFINITE?
	_programDimRegion->addBufInfo(_dimRegionVBO, 4 * sizeof(float), 2, 0x10);														// DEFINITE?

//	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//GLuint GfxOpenGLS::nextPrimitive() {
//	GLuint ret = _primitiveVBOs[_currentPrimitive];
void *GfxN3DS::nextPrimitive() {																									// DEFINITE?
	void *ret = _primitiveVBOs[_currentPrimitive];																					// DEFINITE?
	_lastPrimitive = _currentPrimitive;																								// DEFINITE? - ADDED
	_currentPrimitive = (_currentPrimitive + 1) % ARRAYSIZE(_primitiveVBOs);
	return ret;
}

void GfxN3DS::setupShaders() {
	bool isEMI = g_grim->getGameType() == GType_MONKEY4;

//	static const char* commonAttributes[] = {"position", "texcoord", nullptr};
//	_backgroundProgram = OpenGL::Shader::fromFiles(isEMI ? "emi_background" : "grim_background", commonAttributes);
//	_smushProgram = OpenGL::Shader::fromFiles("grim_smush", commonAttributes);
//	_textProgram = OpenGL::Shader::fromFiles("grim_text", commonAttributes);
//	_emergProgram = OpenGL::Shader::fromFiles("grim_emerg", commonAttributes);
	_programSmush           = new N3DS_3D::ShaderObj(      grim_smush_shbin,       grim_smush_shbin_size);							// DEFINITE?
	_programText            = new N3DS_3D::ShaderObj(       grim_text_shbin,        grim_text_shbin_size);							// DEFINITE?
	_programEmerg           = new N3DS_3D::ShaderObj(      grim_emerg_shbin,       grim_emerg_shbin_size);							// DEFINITE?

//	static const char* actorAttributes[] = {"position", "texcoord", "color", "normal", nullptr};
//	_actorProgram = OpenGL::Shader::fromFiles(isEMI ? "emi_actor" : "grim_actor", actorAttributes);
//	_actorLightsProgram = OpenGL::Shader::fromFiles(isEMI ? "emi_actorlights" : "grim_actorlights", actorAttributes);
//	_spriteProgram = OpenGL::Shader::fromFiles(isEMI ? "emi_sprite" : "grim_actor", actorAttributes);
	if (isEMI) {																													// DEFINITE?
		_programBackground  = new N3DS_3D::ShaderObj(  emi_background_shbin,   emi_background_shbin_size);							// DEFINITE?
		_programActor       = new N3DS_3D::ShaderObj(       emi_actor_shbin,        emi_actor_shbin_size);							// DEFINITE?
		_programActorLights = new N3DS_3D::ShaderObj( emi_actorlights_shbin,  emi_actorlights_shbin_size);							// DEFINITE?
		_programSprite      = new N3DS_3D::ShaderObj(      emi_sprite_shbin,       emi_sprite_shbin_size);							// DEFINITE?
	} else {																														// DEFINITE?
		_programBackground  = new N3DS_3D::ShaderObj( grim_background_shbin,  grim_background_shbin_size);							// DEFINITE?
		_programActor       = new N3DS_3D::ShaderObj(      grim_actor_shbin,       grim_actor_shbin_size);							// DEFINITE?
		_programActorLights = new N3DS_3D::ShaderObj(grim_actorlights_shbin, grim_actorlights_shbin_size);							// DEFINITE?
		_programSprite      = new N3DS_3D::ShaderObj(      grim_actor_shbin,       grim_actor_shbin_size);							// DEFINITE?
	}

//	static const char* primAttributes[] = { "position", nullptr };
//	_shadowPlaneProgram = OpenGL::Shader::fromFiles("grim_shadowplane", primAttributes);
//	_primitiveProgram = OpenGL::Shader::fromFiles("grim_primitive", primAttributes);
	_programShadowPlane     = new N3DS_3D::ShaderObj(grim_shadowplane_shbin, grim_shadowplane_shbin_size);							// DEFINITE?
	_programShadowPlane->addAttrLoader(0, GPU_FLOAT, 3);				// v0 = position											// DEFINITE?
	_programPrimRect        = new N3DS_3D::ShaderObj(grim_primRect_shbin, grim_primRect_shbin_size);								// DEFINITE? - REIMPLEMENTED
	_programPrimLines       = new N3DS_3D::ShaderObj(grim_primLines_shbin, grim_primLines_shbin_size, 2);							// DEFINITE? - REIMPLEMENTED

	if (!isEMI) {
//		_irisProgram = _primitiveProgram->clone();
		_programIris = new N3DS_3D::ShaderObj(_programPrimRect);																	// DEFINITE?

//		_dimProgram = OpenGL::Shader::fromFiles("grim_dim", commonAttributes);
//		_dimRegionProgram   = _dimProgram->clone();
		_programDim         = new N3DS_3D::ShaderObj(        grim_dim_shbin,         grim_dim_shbin_size);							// DEFINITE?
		_programDimRegion   = new N3DS_3D::ShaderObj(_programDim);																	// DEFINITE?
	} else {
//		_dimPlaneProgram = OpenGL::Shader::fromFiles("emi_dimplane", primAttributes);
		_programDimPlane    = new N3DS_3D::ShaderObj(    emi_dimplane_shbin,     emi_dimplane_shbin_size);							// DEFINITE?
	}

	_programClear           = new N3DS_3D::ShaderObj(     manualclear_shbin,      manualclear_shbin_size);							// DEFINITE? - ADDED

	setupQuadEBO();
	setupUntexturedQuad();																											// DEFINITE? - ADDED
	setupTexturedQuad();
	setupTexturedCenteredQuad();
	setupPrimitives();

	if (!isEMI) {
//		_blastVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, 128 * 16 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
		debug("GfxN3DS::setupShaders - Creating linear alloc (_blastVBO)");
		_blastVBO = N3DS_3D::createBuffer(size_t(128 * 16) * sizeof(float));									// DEFINITE?
		debug("GfxN3DS::setupShaders - Linear alloc created: %u bytes (_blastVBO)", linearGetSize(_blastVBO));
	}
}

void GfxN3DS::setupScreen(int screenW, int screenH) {
	_screenWidth = screenW;											// 640
	_screenHeight = screenH;										// 480
	_scaleW = _screenWidth / (float)_gameWidth;						// 1.0f
	_scaleH = _screenHeight / (float)_gameHeight;					// 1.0f
	_screenTexWidth = nextHigher2(_screenWidth);																					// DEFINITE? - ADDED
	_screenTexHeight = nextHigher2(_screenHeight);																					// DEFINITE? - ADDED


	g_system->showMouse(false);

	setupZBuffer();
	setupShaders();

//	glViewport(0, 0, _screenWidth, _screenHeight);
	// C3D_FrameDrawOn SETS VIEWPORT AUTOMATICALLY

//	glGenTextures(1, &_storedDisplay);

//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	if (g_grim->getGameType() == GType_MONKEY4) {
//		// GL_LEQUAL as glDepthFunc ensures that subsequent drawing attempts for
//		// the same triangles are not ignored by the depth test.
//		// That's necessary for EMI where some models have multiple faces which
//		// refer to the same vertices. The first face is usually using the
//		// color map and the following are using textures.
//		glDepthFunc(GL_LEQUAL);
//	}
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
	Math::Matrix4 viewMatrix = makeRotationMatrix(Math::Angle(roll), Math::Vector3d(0, 0, 1));							// viewMatrix is COLUMN MAJOR
	Math::Vector3d up_vec(0, 0, 1);

	if (pos.x() == interest.x() && pos.y() == interest.y())
		up_vec = Math::Vector3d(0, 1, 0);

	Math::Matrix4 lookMatrix = makeLookMatrix(pos, interest, up_vec);													// lookMatrix is COLUMN MAJOR

	_viewMatrix = viewMatrix * lookMatrix;
	_viewMatrix.transpose();
}

void GfxN3DS::positionCamera(const Math::Vector3d &pos, const Math::Matrix4 &rot) {
	Math::Matrix4 projMatrix = _projMatrix;																				// projMatrix is COLUMN MAJOR
	projMatrix.transpose();																								// transpose for left side multiplication

	_currentPos = pos;
	_currentRot = rot;

	Math::Matrix4 invertZ;
	invertZ(2, 2) = -1.0f;

	Math::Matrix4 viewMatrix = _currentRot;
	viewMatrix.transpose();																								// rotation inverse is its transposition

	Math::Matrix4 camPos;
	camPos(0, 3) = -_currentPos.x();																					// translation inverse is the negative of its XYZ components
	camPos(1, 3) = -_currentPos.y();																					// translation inverse is the negative of its XYZ components
	camPos(2, 3) = -_currentPos.z();																					// translation inverse is the negative of its XYZ components

	_viewMatrix = invertZ * viewMatrix * camPos;																		// invZ * R^-1 * T^-1 = camera view
	_mvpMatrix = projMatrix * _viewMatrix;																				// projection * view; model will be multiplied in later
	_viewMatrix.transpose();																							// _viewMatrix TRANSPOSED TO COL MAJOR
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
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);																				// DEPTH AND STENCIL BUFFER ARE INTERLEAVED
	N3DS_3D::ContextHandle tmpContext = N3DS_3D::createContext(&_grimContext);														// DEFINITE?
	N3D_StencilTestEnabled(false);																									// DEFINITE?
	N3D_DepthTestEnabled(true);																										// DEFINITE?
	N3D_DepthFunc(GPU_ALWAYS);																										// DEFINITE?
	debug("GfxN3DS::clearScreen");
	N3D_C3D_TexBind(0, nullptr);																							// DEFINITE?
	debug("GfxN3DS::clearScreen N3DCONTEXT_FROM_HANDLE");
	N3DCONTEXT_FROM_HANDLE(tmpContext)->changeShader(_programClear);																// DEFINITE?
	N3D_C3D_FrameBegin(0);																											// DEFINITE? - ADDED
	N3D_C3D_FrameDrawOn(_gameScreenTarget);																							// DEFINITE? - ADDED
	N3D_C3D_SetTexEnv(0, &envNormal);																								// DEFINITE? - ADDED
	N3D_C3D_DrawElements(GPU_TRIANGLES, 6, C3D_UNSIGNED_SHORT, (void *)_quadEBO);													// DEFINITE?
	N3D_C3D_FrameSplit(0);																											// DEFINITE? - ADDED
	_gameScreenDirty = true;																										// DEFINITE? - ADDED
	N3DS_3D::destroyContext(&tmpContext);																							// DEFINITE?
	N3DS_3D::setContext(&_grimContext);																								// DEFINITE?
}

void GfxN3DS::clearDepthBuffer() {
//	glClear(GL_DEPTH_BUFFER_BIT);																									// DEPTH AND STENCIL BUFFER ARE INTERLEAVED
	N3DS_3D::ContextHandle tmpContext = N3DS_3D::createContext(&_grimContext);														// DEFINITE?
	N3D_ColorMask(false, false, false, false);																						// DEFINITE?
	N3D_StencilTestEnabled(false);																									// DEFINITE?
	N3D_DepthTestEnabled(true);																										// DEFINITE?
	N3D_DepthFunc(GPU_ALWAYS);																										// DEFINITE?
	debug("GfxN3DS::clearDepthBuffer");
	N3D_C3D_TexBind(0, nullptr);																							// DEFINITE?
	N3DCONTEXT_FROM_HANDLE(tmpContext)->changeShader(_programClear);																// DEFINITE?
	N3D_C3D_FrameBegin(0);																											// DEFINITE? - ADDED
	N3D_C3D_FrameDrawOn(_gameScreenTarget);																							// DEFINITE? - ADDED
	N3D_C3D_SetTexEnv(0, &envNormal);																								// DEFINITE? - ADDED
	N3D_C3D_DrawElements(GPU_TRIANGLES, 6, C3D_UNSIGNED_SHORT, (void *)_quadEBO);													// DEFINITE?
	N3D_C3D_FrameSplit(0);																											// DEFINITE? - ADDED
	N3DS_3D::destroyContext(&tmpContext);																							// DEFINITE?
	N3DS_3D::setContext(&_grimContext);																								// DEFINITE?
}

void GfxN3DS::flipBuffer(bool opportunistic) {
//	if (opportunistic) {
//		GLint fbo = 0;
//		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fbo);																			// CHANGE THIS!!!!!!!!!!!!!!!!
//		if (fbo == 0) {
//			// Don't flip if we are not rendering on FBO
//			// Flipping without any draw is undefined
//			// When using an FBO, the older texture will be used
//			return;
//		}
//	}

	N3DS_3D::setContext(&_backendContext);																							// DEFINITE?
	g_system->updateScreen();
	N3DS_3D::setContext(&_grimContext);																								// DEFINITE?
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
////	glEnable(GL_DEPTH_TEST);
//	N3D_DepthTestEnabled(true);																											// NOT NEEDED?

	const Math::Vector3d &pos = actor->getWorldPos();
	const Math::Quaternion &quat = actor->getRotationQuat();
	//const float scale = actor->getScale();

	Math::Matrix4 viewMatrix = _viewMatrix;
	viewMatrix.transpose();

//	OpenGL::Shader *shaders[] = { _spriteProgram, _actorProgram, _actorLightsProgram };
	N3DS_3D::ShaderObj *shaders[] = { _programSprite, _programActor, _programActorLights };											// DEFINITE?

	if (g_grim->getGameType() == GType_MONKEY4) {
//		glEnable(GL_CULL_FACE);
//		glFrontFace(GL_CW);
		N3D_CullFaceEnabled(true);																									// FINALIZED
		N3D_FrontFace(N3D_FRONTFACE_CW);																							// FINALIZED

		if (actor->isInOverworld())
			viewMatrix = Math::Matrix4();

//		Math::Vector4d color(1.0f, 1.0f, 1.0f, actor->getEffectiveAlpha());																									// !!!!! DEFER DOING THIS !!!!!
		_color.x() = _color.y() = _color.z() = 1.0f;																				// DEFINITE? - ADDED
		const float alpha = actor->getEffectiveAlpha();																				// DEFINITE? - ADDED from "gfx_opengl.cpp"
		if (alpha < 1.f) {																											// DEFINITE? - ADDED from "gfx_opengl.cpp"
			_alpha = alpha;																											// DEFINITE? - ADDED from "gfx_opengl.cpp"
			N3D_BlendEnabled(true);																									// DEFINITE? - ADDED from "gfx_opengl.cpp"
			N3D_BlendFunc(GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);																	// DEFINITE? - ADDED from "gfx_opengl.cpp"
		}																															// DEFINITE? - ADDED from "gfx_opengl.cpp"

		const Math::Matrix4 &viewRot = _currentRot;
		Math::Matrix4 modelMatrix = actor->getFinalMatrix();

		Math::Matrix4 normalMatrix = viewMatrix * modelMatrix;
		normalMatrix.invertAffineOrthonormal();
		modelMatrix.transpose();

		for (int i = 0; i < 3; i++) {
//			shaders[i]->use();
//			shaders[i]->setUniform("modelMatrix", modelMatrix);
			shaders[i]->setUniform("modelMatrix", GPU_VERTEX_SHADER, modelMatrix);													// DEFINITE?
			if (actor->isInOverworld()) {
//				shaders[i]->setUniform("viewMatrix", viewMatrix);
//				shaders[i]->setUniform("projMatrix", _overworldProjMatrix);
//				shaders[i]->setUniform("cameraPos", Math::Vector3d(0,0,0));
				shaders[i]->setUniform("viewMatrix", GPU_VERTEX_SHADER, viewMatrix);												// DEFINITE?
				shaders[i]->setUniform("projMatrix", GPU_VERTEX_SHADER, _overworldProjMatrix);										// DEFINITE?
				shaders[i]->setUniform("cameraPos", GPU_VERTEX_SHADER, Math::Vector3d(0,0,0));										// DEFINITE?
			} else {
//				shaders[i]->setUniform("viewMatrix", viewRot);
//				shaders[i]->setUniform("projMatrix", _projMatrix);
//				shaders[i]->setUniform("cameraPos", _currentPos);
				shaders[i]->setUniform("viewMatrix", GPU_VERTEX_SHADER, viewRot);													// DEFINITE?
				shaders[i]->setUniform("projMatrix", GPU_VERTEX_SHADER, _projMatrix);												// DEFINITE?
				shaders[i]->setUniform("cameraPos", GPU_VERTEX_SHADER, _currentPos);												// DEFINITE?
			}
//			shaders[i]->setUniform("normalMatrix", normalMatrix);
			shaders[i]->setUniform("normalMatrix", GPU_VERTEX_SHADER, normalMatrix);												// DEFINITE?

//			shaders[i]->setUniform("useVertexAlpha", GL_FALSE);																		// MOVED to drawEMIModelFace() and drawSprite()
//			shaders[i]->setUniform("uniformColor", color);																			// MOVED to drawEMIModelFace() and drawSprite()
//			shaders[i]->setUniform1f("alphaRef", 0.0f);																				// glAlphaFunc "ref" EQUIVALENT (WE DON'T NEED THIS)
//			shaders[i]->setUniform1f("meshAlpha", 1.0f);																// FRAGMENT SHADER UNIFORM
		}
	} else {
		Math::Matrix4 modelMatrix = quat.toMatrix();
//		bool hasZBuffer = g_grim->getCurrSet()->getCurrSetup()->_bkgndZBm;
		Math::Matrix4 extraMatrix;
		_matrixStack.top() = extraMatrix;

		modelMatrix.transpose();
		modelMatrix.setPosition(pos);
		modelMatrix.transpose();

		for (int i = 0; i < 3; i++) {
//			shaders[i]->use();
//			shaders[i]->setUniform("modelMatrix", modelMatrix);
//			shaders[i]->setUniform("viewMatrix", _viewMatrix);
//			shaders[i]->setUniform("projMatrix", _projMatrix);
//			shaders[i]->setUniform("extraMatrix", extraMatrix);
//			shaders[i]->setUniform("tex", 0);																						// SAMPLES MODEL TEX            (WE DON'T NEED THIS)
//			shaders[i]->setUniform("texZBuf", 1);																					// SAMPLES ZBUF TEX             (WE DON'T NEED THIS)
//			shaders[i]->setUniform("hasZBuffer", hasZBuffer);																		// WHETHER TO RUN checkZBuffer  (WE DON'T NEED THIS)
//			shaders[i]->setUniform("texcropZBuf", _zBufTexCrop);																	// CROPS ZBUF TEX FOR SAMPLING  (WE DON'T NEED THIS)
//			shaders[i]->setUniform("screenSize", Math::Vector2d(_screenWidth, _screenHeight));										// USED IN checkZBuffer         (WE DON'T NEED THIS)
//			shaders[i]->setUniform1f("alphaRef", 0.5f);																				// glAlphaFunc "ref" EQUIVALENT (WE DON'T NEED THIS)
			shaders[i]->setUniform("modelMatrix", GPU_VERTEX_SHADER, modelMatrix);													// DEFINITE?
			shaders[i]->setUniform("viewMatrix", GPU_VERTEX_SHADER, _viewMatrix);													// DEFINITE?
			shaders[i]->setUniform("projMatrix", GPU_VERTEX_SHADER, _projMatrix);													// DEFINITE?
			shaders[i]->setUniform("extraMatrix", GPU_VERTEX_SHADER, extraMatrix);													// DEFINITE?
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
//			shaders[i]->use();
//			shaders[i]->setUniform("shadow._active", true);
//			shaders[i]->setUniform("shadow._color", color);
//			shaders[i]->setUniform("shadow._light", _currentShadowArray->pos);
//			shaders[i]->setUniform("shadow._point", shadowSector->getVertices()[0]);
//			shaders[i]->setUniform("shadow._normal", normal);
			// avoid using bools																									// FINALIZED
			shaders[i]->setUniform("shadowActive", GPU_VERTEX_SHADER, 1.0f);														// FINALIZED
			shaders[i]->setUniform("shadowColor", GPU_VERTEX_SHADER, color);														// FINALIZED
			shaders[i]->setUniform("shadowLight", GPU_VERTEX_SHADER, _currentShadowArray->pos);										// FINALIZED - shadowProjection, PARAM 1   OF 4
			shaders[i]->setUniform("shadowPoint", GPU_VERTEX_SHADER, shadowSector->getVertices()[0]);								// FINALIZED - shadowProjection, PARAM 2   OF 4
			shaders[i]->setUniform("shadowNormal", GPU_VERTEX_SHADER, normal);														// FINALIZED - shadowProjection, PARAM 3+4 OF 4
		}

//		glDepthMask(GL_FALSE);
//		glDisable(GL_BLEND);
//		glEnable(GL_POLYGON_OFFSET_FILL);
		N3D_DepthMask(false);																										// FINALIZED
		// N3D_BlendEnabled(false);																										// NOT NEEDED?
		N3D_PolygonOffsetEnabled(true);																								// FINALIZED

	}
	else {
		for (int i = 0; i < 3; i++) {
//			shaders[i]->use();
//			shaders[i]->setUniform("shadow._active", false);
			// avoid using bools																									// FINALIZED
			shaders[i]->setUniform("shadowActive", GPU_VERTEX_SHADER, 0.0f);														// FINALIZED
		}
	}

//	_actorLightsProgram->setUniform("hasAmbient", _hasAmbientLight);
	_programActorLights->setUniform("hasAmbient", GPU_VERTEX_SHADER, _hasAmbientLight);												// DEFINITE?

	if (_lightsEnabled) {
		// Allocate all variables in one chunk
		static const unsigned int numUniforms = 4;
		static const unsigned int uniformSize = _maxLights * 4;
		float *lightsData = new float[numUniforms * uniformSize];
		for (int i = 0; i < _maxLights; ++i) {
//			const GLSLight &l = _lights[i];
			const LightObj &l = _lights[i];																							// DEFINITE?

			// lightsPosition
			Math::Vector4d tmp = viewMatrix * l._position;

			lightsData[0 * uniformSize + 4 * i + 0] = tmp.x();
			lightsData[0 * uniformSize + 4 * i + 1] = tmp.y();
			lightsData[0 * uniformSize + 4 * i + 2] = tmp.z();
			lightsData[0 * uniformSize + 4 * i + 3] = tmp.w();

			// lightsDirection
			Math::Vector4d direction = l._direction;
			direction.w() = 0.0;
			viewMatrix.transformVector(&direction);
			direction.w() = l._direction.w();

			lightsData[1 * uniformSize + 4 * i + 0] = direction.x();
			lightsData[1 * uniformSize + 4 * i + 1] = direction.y();
			lightsData[1 * uniformSize + 4 * i + 2] = direction.z();
			lightsData[1 * uniformSize + 4 * i + 3] = direction.w();

			// lightsColor
			lightsData[2 * uniformSize + 4 * i + 0] = l._color.x();
			lightsData[2 * uniformSize + 4 * i + 1] = l._color.y();
			lightsData[2 * uniformSize + 4 * i + 2] = l._color.z();
			lightsData[2 * uniformSize + 4 * i + 3] = l._color.w();

			// lightsParams
			lightsData[3 * uniformSize + 4 * i + 0] = l._params.x();
			lightsData[3 * uniformSize + 4 * i + 1] = l._params.y();
			lightsData[3 * uniformSize + 4 * i + 2] = l._params.z();
			lightsData[3 * uniformSize + 4 * i + 3] = l._params.w();
		}

		Common::String uniform;
//		GLint uniformPos;

//		uniform = Common::String::format("lightsPosition");
//		uniformPos = _actorLightsProgram->getUniformLocation(uniform.c_str());
//		if (uniformPos == -1) {
//			error("No uniform named '%s'", uniform.c_str());
//		}
//		glUniform4fv(uniformPos, _maxLights, &lightsData[0 * uniformSize]);
		if (!(_programActorLights->setUniform4fv("lightsPosition", GPU_VERTEX_SHADER, &lightsData[0 * uniformSize], _maxLights))) {	// DEFINITE?
			error("No uniform named 'lightsPosition'");																				// DEFINITE?
		}																															// DEFINITE?

//		uniform = Common::String::format("lightsDirection");
//		uniformPos = _actorLightsProgram->getUniformLocation(uniform.c_str());
//		if (uniformPos == -1) {
//			error("No uniform named '%s'", uniform.c_str());
//		}
//		glUniform4fv(uniformPos, _maxLights, &lightsData[1 * uniformSize]);
		if (!(_programActorLights->setUniform4fv("lightsDirection", GPU_VERTEX_SHADER, &lightsData[1 * uniformSize], _maxLights))) {// DEFINITE?
			error("No uniform named 'lightsDirection'");																			// DEFINITE?
		}																															// DEFINITE?

//		uniform = Common::String::format("lightsColor");
//		uniformPos = _actorLightsProgram->getUniformLocation(uniform.c_str());
//		if (uniformPos == -1) {
//			error("No uniform named '%s'", uniform.c_str());
//		}
//		glUniform4fv(uniformPos, _maxLights, &lightsData[2 * uniformSize]);
		if (!(_programActorLights->setUniform4fv("lightsColor", GPU_VERTEX_SHADER, &lightsData[2 * uniformSize], _maxLights))) {	// DEFINITE?
			error("No uniform named 'lightsColor'");																				// DEFINITE?
		}																															// DEFINITE?

//		uniform = Common::String::format("lightsParams");
//		uniformPos = _actorLightsProgram->getUniformLocation(uniform.c_str());
//		if (uniformPos == -1) {
//			error("No uniform named '%s'", uniform.c_str());
//		}
//		glUniform4fv(uniformPos, _maxLights, &lightsData[3 * uniformSize]);
		if (!(_programActorLights->setUniform4fv("lightsParams", GPU_VERTEX_SHADER, &lightsData[3 * uniformSize], _maxLights))) {	// DEFINITE?
			error("No uniform named 'lightsParams'");																				// DEFINITE?
		}																															// DEFINITE?

		delete[] lightsData;
	}
}

void GfxN3DS::finishActorDraw() {
	_currentActor = nullptr;

	if (_alpha < 1.f) {																												// DEFINITE? - ADDED from "gfx_opengl.cpp"
		N3D_BlendEnabled(false);																									// DEFINITE? - ADDED from "gfx_opengl.cpp"
		_alpha = 1.f;																												// DEFINITE? - ADDED from "gfx_opengl.cpp"
	}																																// DEFINITE? - ADDED from "gfx_opengl.cpp"

//	glDisable(GL_POLYGON_OFFSET_FILL);
	if (_currentShadowArray)																										// DEFINITE? - ADDED from "gfx_opengl.cpp"
		N3D_PolygonOffsetEnabled(false);																							// FINALIZED

	if (g_grim->getGameType() == GType_MONKEY4) {
//		glDisable(GL_CULL_FACE);
		N3D_CullFaceEnabled(false);																									// FINALIZED
	}
}

void GfxN3DS::setShadow(Shadow *shadow) {
	_currentShadowArray = shadow;
}

void GfxN3DS::drawShadowPlanes() {
//	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);																			// MOVED DOWN
//	glDepthMask(GL_FALSE);																											// MOVED DOWN
//	glClearStencil(~0);																												// MOVED DOWN
//	glClear(GL_STENCIL_BUFFER_BIT);																									// MOVED DOWN

//	glEnable(GL_STENCIL_TEST);																										// MOVED DOWN
//	glStencilFunc(GL_ALWAYS, 1, (GLuint)~0);																						// MOVED DOWN
//	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);																				// MOVED DOWN
																														// IN 3DS, CLEARING ISN'T AFFECTED BY MASKING

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
//		sud->_verticesVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, 3 * numVertices * sizeof(float), vertBuf, GL_STATIC_DRAW);
//		sud->_indicesVBO = OpenGL::Shader::createBuffer(GL_ELEMENT_ARRAY_BUFFER, 3 * numTriangles * sizeof(uint16), idxBuf, GL_STATIC_DRAW);
		debug("GfxN3DS::drawShadowPlanes - Creating linear alloc (sud->_verticesVBO)");
		sud->_verticesVBO = N3DS_3D::createBuffer(3 * numVertices * sizeof(float), vertBuf);										// DEFINITE?
		debug("GfxN3DS::drawShadowPlanes - Linear alloc created: %u bytes (sud->_verticesVBO)", linearGetSize(sud->_verticesVBO));
		debug("GfxN3DS::drawShadowPlanes - Creating linear alloc (sud->_indicesVBO )");
		sud->_indicesVBO  = N3DS_3D::createBuffer(3 * numTriangles * sizeof(uint16), idxBuf);										// DEFINITE?
		debug("GfxN3DS::drawShadowPlanes - Linear alloc created: %u bytes (sud->_indicesVBO )", linearGetSize(sud->_indicesVBO ));

		delete[] vertBuf;
		delete[] idxBuf;
	}

	const ShadowUserData *sud = (ShadowUserData *)_currentShadowArray->userData;
	_programShadowPlane->BufInfo_AddOrModify(sud->_verticesVBO, 3 * sizeof(float), 1, 0x0, 0);										// DEFINITE?
//	_shadowPlaneProgram->use();
//	_shadowPlaneProgram->setUniform("projMatrix", _projMatrix);
//	_shadowPlaneProgram->setUniform("viewMatrix", _viewMatrix);
	_programShadowPlane->setUniform("projMatrix", GPU_VERTEX_SHADER, _projMatrix);													// DEFINITE?
	_programShadowPlane->setUniform("viewMatrix", GPU_VERTEX_SHADER, _viewMatrix);													// DEFINITE?

//	glBindBuffer(GL_ARRAY_BUFFER, sud->_verticesVBO);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sud->_indicesVBO);
//	const uint32 attribPos = _shadowPlaneProgram->getAttribute("position")._idx;
//	glEnableVertexAttribArray(attribPos);
//	glVertexAttribPointer(attribPos, 3, GL_FLOAT, GL_TRUE, 3 * sizeof(float), nullptr);
	// See GfxN3DS::setupShaders()																									// DEFINITE?
//	glDrawElements(GL_TRIANGLES, 3 * sud->_numTriangles, GL_UNSIGNED_SHORT, nullptr);
	N3D_C3D_FrameBegin(0);																											// DEFINITE? - ADDED
	N3D_C3D_FrameDrawOn(_gameScreenTarget);																							// DEFINITE? - ADDED
	N3D_C3D_SetTexEnv(0, &envNormal);																								// DEFINITE? - ADDED
		//~~~clearing stencil buffer - start~~~																						// DEFINITE? - REIMPLEMENTED
	debug("GfxN3DS::DrawShadowPlanes");
	N3D_C3D_TexBind(0, nullptr);																							// DEFINITE? - REIMPLEMENTED
	N3DCONTEXT_FROM_HANDLE(_grimContext)->changeShader(_programClear);																// DEFINITE? - REIMPLEMENTED
	N3D_ColorMask(false, false, false, false);																						// DEFINITE? - REIMPLEMENTED
	N3D_DepthMask(false);																											// DEFINITE? - REIMPLEMENTED
	N3D_StencilTestEnabled(true);																									// FINALIZED - REIMPLEMENTED
	N3D_StencilOp(GPU_STENCIL_REPLACE, GPU_STENCIL_REPLACE, GPU_STENCIL_REPLACE);													// FINALIZED - REIMPLEMENTED
	N3D_StencilFunc(GPU_ALWAYS, (int)~0, (int)~0);																					// FINALIZED - REIMPLEMENTED - ref VALUE ~0 (all "1"s) IS WRITTEN TO STENCIL BUFFER
	N3D_C3D_DrawElements(GPU_TRIANGLES, 6, C3D_UNSIGNED_SHORT, (void *)_quadEBO);													// DEFINITE? - REIMPLEMENTED
	//~~~clearing stencil buffer - end~~~																							// DEFINITE? - REIMPLEMENTED
	N3DCONTEXT_FROM_HANDLE(_grimContext)->changeShader(_programShadowPlane);														// DEFINITE?
	N3D_StencilFunc(GPU_ALWAYS, 1, (int)~0);																						// FINALIZED - ref VALUE 1 ("1" at bit 0) IS WRITTEN TO STENCIL BUFFER
	N3D_C3D_DrawElements(GPU_TRIANGLES, 3 * sud->_numTriangles, C3D_UNSIGNED_SHORT, (void *)sud->_indicesVBO);						// DEFINITE?
	N3D_C3D_FrameSplit(0);																											// DEFINITE? - ADDED
	_gameScreenDirty = true;																										// DEFINITE? - ADDED


//	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	N3D_ColorMask(true, true, true, true);																							// FINALIZED

//	glStencilFunc(GL_EQUAL, 1, (GLuint)~0);
//	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	N3D_StencilFunc(GPU_EQUAL, 1, (int)~0);																							// FINALIZED
	N3D_StencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, GPU_STENCIL_KEEP);															// FINALIZED
}

void GfxN3DS::setShadowMode() {
	GfxBase::setShadowMode();
}

void GfxN3DS::clearShadowMode() {
	GfxBase::clearShadowMode();

//	glDisable(GL_STENCIL_TEST);
//	glDepthMask(GL_TRUE);
	N3D_StencilTestEnabled(false);																									// DEFINITE?
	N3D_DepthMask(true);																											// DEFINITE?
}

bool GfxN3DS::isShadowModeActive() {
	return false;
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
//		OpenGL::Shader::freeBuffer(sud->_verticesVBO);
//		OpenGL::Shader::freeBuffer(sud->_indicesVBO);
		debug("GfxN3DS::destroyShadow - Deleting linear alloc: -%u bytes (sud->_verticesVBO)", linearGetSize(sud->_verticesVBO));
		N3DS_3D::freeBuffer(sud->_verticesVBO);																						// DEFINITE?
		debug("GfxN3DS::destroyShadow - Linear alloc deleted (sud->_verticesVBO).");
		debug("GfxN3DS::destroyShadow - Deleting linear alloc: -%u bytes (sud->_indicesVBO)", linearGetSize(sud->_indicesVBO));
		N3DS_3D::freeBuffer(sud->_indicesVBO);																						// DEFINITE?
		debug("GfxN3DS::destroyShadow - Linear alloc deleted (sud->_indicesVBO).");
		sud->_verticesVBO = nullptr;
		sud->_indicesVBO = nullptr;
		delete sud;
	}

	shadow->userData = nullptr;
}

void GfxN3DS::set3DMode() {
	// USE MODEL VIEW MATRIX?
	N3D_DepthTestEnabled(true);																										// DEFINITE? - ADDED from "gfx_opengl.cpp"
	N3D_DepthFunc(_depthFunc);																										// DEFINITE? - ADDED from "gfx_opengl.cpp"
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
	const EMIModelUserData *mud = (const EMIModelUserData *)model->_userData;
//	glBindBuffer(GL_ARRAY_BUFFER, mud->_verticesVBO);
//	glBufferSubData(GL_ARRAY_BUFFER, 0, model->_numVertices * 3 * sizeof(float), model->_drawVertices);
//	glBindBuffer(GL_ARRAY_BUFFER, mud->_normalsVBO);
//	glBufferSubData(GL_ARRAY_BUFFER, 0, model->_numVertices * 3 * sizeof(float), model->_drawNormals);
	memcpy(mud->_verticesVBO, model->_drawVertices, model->_numVertices * 3 * sizeof(float));										// DEFINITE?
	memcpy(mud->_normalsVBO,  model->_drawNormals,  model->_numVertices * 3 * sizeof(float));										// DEFINITE?
}

void GfxN3DS::drawEMIModelFace(const EMIModel* model, const EMIMeshFace* face) {
	N3D_DepthTestEnabled(true);																										// DEFINITE? - ADDED from "gfx_opengl.cpp"
	N3D_AlphaTestEnabled(false);																									// DEFINITE? - ADDED from "gfx_opengl.cpp"

	float alpha = _alpha;																											// DEFINITE? - ADDED from "gfx_opengl.cpp"
	if (model->_meshAlphaMode == Actor::AlphaReplace) {																				// DEFINITE? - ADDED from "gfx_opengl.cpp"
		alpha *= model->_meshAlpha;																									// DEFINITE? - ADDED from "gfx_opengl.cpp"
	}																																// DEFINITE? - ADDED from "gfx_opengl.cpp"

//	if (face->_flags & EMIMeshFace::kAlphaBlend || face->_flags & EMIMeshFace::kUnknownBlend)
//		glEnable(GL_BLEND);
	if (face->_flags & EMIMeshFace::kAlphaBlend || face->_flags & EMIMeshFace::kUnknownBlend ||										// DEFINITE?
	    _currentActor->hasLocalAlpha() || _alpha < 1.0f)																			// DEFINITE? - ADDED from "gfx_opengl.cpp"
		N3D_BlendEnabled(true);																										// DEFINITE?
	const EMIModelUserData *mud = (const EMIModelUserData *)model->_userData;
//	OpenGL::Shader *actorShader;
	N3DS_3D::ShaderObj *actorShader;																								// DEFINITE?
	if ((face->_flags & EMIMeshFace::kNoLighting) ? false : _lightsEnabled)
		actorShader = mud->_shaderLights;
	else
		actorShader = mud->_shader;
//	actorShader->use();
	N3DCONTEXT_FROM_HANDLE(_grimContext)->changeShader(actorShader);																// DEFINITE?

	_color.w() = alpha;																												// DEFINITE? - MOVED from StartActorDraw()
	actorShader->setUniform("uniformColor", GPU_VERTEX_SHADER, _color);																// DEFINITE? - MOVED from StartActorDraw()

	bool textured = face->_hasTexture && !_currentShadowArray;
//	actorShader->setUniform("textured", textured ? GL_TRUE : GL_FALSE);
//	actorShader->setUniform("useVertexAlpha", _selectedTexture->_hasAlpha);
//	actorShader->setUniform1f("meshAlpha", (model->_meshAlphaMode == Actor::AlphaReplace) ? model->_meshAlpha : 1.0f);				// FRAGMENT SHADER UNIFORM
	// avoid using bools																											// DEFINITE?
	actorShader->setUniform("textured", GPU_VERTEX_SHADER, textured ? 1.0f : 0.0f);													// DEFINITE?
	actorShader->setUniform("useVertexAlpha", GPU_VERTEX_SHADER, _selectedTexture->_hasAlpha ? 1.0f : 0.0f);						// DEFINITE?
	// actorShader->setUniform("meshAlpha", (model->_meshAlphaMode == Actor::AlphaReplace) ? model->_meshAlpha : 1.0f);	// N3D	avoid using bool uniforms as much as possible

//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face->_indicesEBO);

//	glDrawElements(GL_TRIANGLES, 3 * face->_faceLength, GL_UNSIGNED_SHORT, nullptr);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	N3D_C3D_FrameBegin(0);																											// DEFINITE? - ADDED
	N3D_C3D_FrameDrawOn(_gameScreenTarget);																							// DEFINITE? - ADDED
	N3D_C3D_SetTexEnv(0, &envNormal);																								// DEFINITE? - ADDED
	N3D_C3D_DrawElements(GPU_TRIANGLES, 3 * face->_faceLength, C3D_UNSIGNED_SHORT, face->_indicesEBO);								// DEFINITE?
	N3D_C3D_FrameSplit(0);																											// DEFINITE? - ADDED
	_gameScreenDirty = true;																										// DEFINITE? - ADDED

	N3D_DepthTestEnabled(true);																										// DEFINITE? - ADDED from "gfx_opengl.cpp"
	N3D_AlphaTestEnabled(true);																										// DEFINITE? - ADDED from "gfx_opengl.cpp"
	N3D_BlendEnabled(false);																										// DEFINITE? - ADDED from "gfx_opengl.cpp"

	if (!_currentShadowArray)																										// DEFINITE? - ADDED from "gfx_opengl.cpp"
		N3D_DepthMask(true);																										// DEFINITE? - ADDED from "gfx_opengl.cpp"

}

void GfxN3DS::drawMesh(const Mesh *mesh) {
	N3D_AlphaFunc(GPU_GREATER, 128);																								// FINALIZED - ADDED from "gfx_opengl.cpp"	128 = 0.5f converted to 0-255 int
	N3D_AlphaTestEnabled(true);																										// FINALIZED - ADDED from "gfx_opengl.cpp"

	const ModelUserData *mud = (const ModelUserData *)mesh->_userData;
	if (!mud)
		return;
//	OpenGL::Shader *actorShader;
	N3DS_3D::ShaderObj *actorShader;																								// DEFINITE?
	if (_lightsEnabled && !isShadowModeActive())
		actorShader = mud->_shaderLights;
	else
		actorShader = mud->_shader;

//	actorShader->use();
//	actorShader->setUniform("extraMatrix", _matrixStack.top());
	N3DCONTEXT_FROM_HANDLE(_grimContext)->changeShader(actorShader);																// DEFINITE?
	actorShader->setUniform("extraMatrix", GPU_VERTEX_SHADER, _matrixStack.top());													// DEFINITE?

	N3D_C3D_FrameBegin(0);																											// DEFINITE? - ADDED
	N3D_C3D_FrameDrawOn(_gameScreenTarget);																							// DEFINITE? - ADDED
	N3D_C3D_SetTexEnv(0, &envNormal);																								// DEFINITE? - ADDED
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
//		actorShader->setUniform("textured", textured ? GL_TRUE : GL_FALSE);
//		actorShader->setUniform("texScale", Math::Vector2d(_selectedTexture->_width, _selectedTexture->_height));
		// avoid using bools																										// DEFINITE?
		actorShader->setUniform("texturedBool", GPU_VERTEX_SHADER, textured ? 1.0f : 0.0f);											// DEFINITE?
		actorShader->setUniform("texScale", GPU_VERTEX_SHADER, Math::Vector2d(_selectedTexture->_width, _selectedTexture->_height));// DEFINITE?

//		glDrawArrays(GL_TRIANGLES, *(int *)face->_userData, faces);
		N3D_C3D_DrawArrays(GPU_TRIANGLES, *(int *)face->_userData, faces);															// DEFINITE?
	}
	N3D_C3D_FrameSplit(0);																											// DEFINITE? - ADDED
	_gameScreenDirty = true;																										// DEFINITE? - ADDED

	N3D_AlphaTestEnabled(false);																									// FINALIZED - ADDED from "gfx_opengl.cpp"
}

void GfxN3DS::drawDimPlane() {
	if (_dimLevel == 0.0f)
		return;

//	glDisable(GL_DEPTH_TEST);
//	glDepthMask(GL_FALSE);
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	N3D_DepthTestEnabled(false);																									// FINALIZED
	N3D_DepthMask(false);																											// FINALIZED
	N3D_BlendEnabled(true);																											// FINALIZED
	N3D_BlendFunc(GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);																			// FINALIZED

//	_dimPlaneProgram->use();
//	_dimPlaneProgram->setUniform1f("dim", _dimLevel);
	N3DCONTEXT_FROM_HANDLE(_grimContext)->changeShader(_programDimPlane);															// DEFINITE?
	_programDimPlane->setUniform("dim", GPU_VERTEX_SHADER, _dimLevel);																// DEFINITE?

//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quadEBO);
//	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	N3D_C3D_FrameBegin(0);																											// DEFINITE? - ADDED
	N3D_C3D_SetTexEnv(0, &envNormal);																								// DEFINITE? - ADDED
	N3D_C3D_FrameDrawOn(_gameScreenTarget);																							// DEFINITE? - ADDED
	N3D_C3D_DrawElements(GPU_TRIANGLES, 6, C3D_UNSIGNED_SHORT, (void *)_quadEBO);													// DEFINITE?
	N3D_C3D_FrameSplit(0);																											// DEFINITE? - ADDED
	_gameScreenDirty = true;																										// DEFINITE? - ADDED

//	glEnable(GL_DEPTH_TEST);
//	glDepthMask(GL_TRUE);
	N3D_DepthTestEnabled(true);																										// FINALIZED
	N3D_DepthMask(true);																											// FINALIZED
	N3D_BlendEnabled(false);																										// DEFINITE? - ADDED from "gfx_opengl.cpp"
}

void GfxN3DS::drawModelFace(const Mesh *mesh, const MeshFace *face) {
}

void GfxN3DS::drawSprite(const Sprite *sprite) {
	if (g_grim->getGameType() == GType_MONKEY4) {
//		glDepthMask(GL_TRUE);
		N3D_DepthMask(true);																										// DEFINTIE?

		// avoid using bools																										// DEFINITE? - MOVED from startActorDraw()
		_programSprite->setUniform("useVertexAlpha", GPU_VERTEX_SHADER, 0.0f);														// DEFINITE? - MOVED from startActorDraw()
		// FIXME: Currently vertex-specific colors are not supported for sprites.													// DEFINITE? - MOVED from lower down in this function
		// It is unknown at this time if this is really needed anywhere.															// DEFINITE? - MOVED from lower down in this function
																																	// FIGURE OUT HOW TO TO DO THIS AS VTXBUF
		Math::Vector4d color(sprite->_red[0]   / 255.0f,																			// DEFINITE? - MOVED from lower down in this function
		                     sprite->_green[0] / 255.0f,																			// DEFINITE? - MOVED from lower down in this function
		                     sprite->_blue[0]  / 255.0f,																			// DEFINITE? - MOVED from lower down in this function
		                     sprite->_alpha[0] * _alpha / 255.0f);																	// DEFINITE? - MOVED from lower down in this function, MODIFIED from "gfx_opengl.cpp"
		_programSprite->setUniform("uniformColor", GPU_VERTEX_SHADER, color);														// DEFINITE? - MOVED from lower down in this function
	} else {
//		glDepthMask(GL_FALSE);
		N3D_DepthMask(false);																										// DEFINTIE?
	}

	if (sprite->_flags1 & Sprite::BlendAdditive) {
//		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		N3D_BlendFunc(GPU_SRC_ALPHA, GPU_ONE);																						// DEFINTIE?
	} else {
//		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		N3D_BlendFunc(GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);																		// DEFINTIE?
	}

	// FIXME: depth test does not work yet because final z coordinates
	//        for Sprites and actor textures are inconsistently calculated
//	if (sprite->_flags2 & Sprite::DepthTest || _currentActor->isInOverworld()) {
//		glEnable(GL_DEPTH_TEST);
	if (sprite->_flags2 & Sprite::DepthTest) {																						// DEFINITE? - ADDED from "gfx_opengl.cpp"
		N3D_DepthTestEnabled(true);																									// DEFINITE?
	} else {
//		glDisable(GL_DEPTH_TEST);
		N3D_DepthTestEnabled(false);																								// DEFINITE?
	}

//	_spriteProgram->use();
	N3DCONTEXT_FROM_HANDLE(_grimContext)->changeShader(_programSprite);																// DEFINITE?

	Math::Matrix4 rotateMatrix;
	rotateMatrix.buildAroundZ(_currentActor->getYaw());

	Math::Matrix4 extraMatrix;
	extraMatrix.setPosition(sprite->_pos);
	extraMatrix(0, 0) = sprite->_width;
	extraMatrix(1, 1) = sprite->_height;

	extraMatrix = rotateMatrix * extraMatrix;
	extraMatrix.transpose();
//	_spriteProgram->setUniform("extraMatrix", extraMatrix);
//	_spriteProgram->setUniform("textured", GL_TRUE);
	_programSprite->setUniform("extraMatrix", GPU_VERTEX_SHADER, extraMatrix);														// DEFINITE?
	// avoid using bools																											// DEFINITE?
	_programSprite->setUniform("textured", GPU_VERTEX_SHADER, 1.0f);																// DEFINITE?
	if (g_grim->getGameType() == GType_GRIM) {
//		_spriteProgram->setUniform1f("alphaRef", 0.5f);
		// 0.5f converted to 0-255 int = 128
		N3D_AlphaTestEnabled(true);																									// DEFINITE? - ADDED from "gfx_opengl.cpp"
		N3D_AlphaFunc(GPU_GEQUAL, 128);																								// DEFINITE? - ADDED from "gfx_opengl.cpp"
	} else if (sprite->_flags2 & Sprite::AlphaTest) {
//		_spriteProgram->setUniform1f("alphaRef", 0.1f);
		// 0.1f converted to 0-255 int = 26
		N3D_AlphaTestEnabled(true);																									// DEFINITE? - ADDED from "gfx_opengl.cpp"
		N3D_AlphaFunc(GPU_GEQUAL, 26);																								// DEFINITE? - ADDED from "gfx_opengl.cpp"
	} else {
//		_spriteProgram->setUniform1f("alphaRef", 0.0f);
		N3D_AlphaTestEnabled(false);																								// DEFINITE? - ADDED from "gfx_opengl.cpp"
	}

//	// FIXME: Currently vertex-specific colors are not supported for sprites.															// MOVED UP
//	// It is unknown at this time if this is really needed anywhere.																	// MOVED UP
//	Math::Vector4d color(sprite->_red[0] / 255.0f, sprite->_green[0] / 255.0f, sprite->_blue[0] / 255.0f, sprite->_alpha[0] / 255.0f);	// MOVED UP
//	_spriteProgram->setUniform("uniformColor", color);																					// MOVED UP

//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quadEBO);
//	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//	glEnable(GL_DEPTH_TEST);
//	glDepthMask(GL_TRUE);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	N3D_C3D_FrameBegin(0);																											// DEFINITE? - ADDED
	N3D_C3D_FrameDrawOn(_gameScreenTarget);																							// DEFINITE? - ADDED
	N3D_C3D_SetTexEnv(0, &envNormal);																								// DEFINITE? - ADDED
	N3D_C3D_DrawElements(GPU_TRIANGLES, 6, C3D_UNSIGNED_SHORT, (void *)_quadEBO);													// DEFINITE?
	N3D_C3D_FrameSplit(0);																											// DEFINITE? - ADDED
	_gameScreenDirty = true;																										// DEFINITE? - ADDED
	N3D_DepthTestEnabled(true);																										// DEFINITE?
	N3D_DepthMask(true);																											// DEFINITE?
	N3D_BlendFunc(GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);																			// DEFINITE?

	N3D_AlphaTestEnabled(false);																									// DEFINITE? - ADDED from "gfx_opengl.cpp"
	N3D_BlendEnabled(false);																										// DEFINITE? - ADDED from "gfx_opengl.cpp"
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
//	texture->_texture = new GLuint[1];
//	glGenTextures(1, (GLuint *)texture->_texture);
	texture->_texture = new C3D_Tex[1];																								// DEFINITE?

//	GLuint *textures = (GLuint *)texture->_texture;
//	glBindTexture(GL_TEXTURE_2D, textures[0]);
	C3D_Tex *textures = static_cast<C3D_Tex *>(texture->_texture);																	// DEFINITE?

	// Remove darkened lines in EMI intro
	if (g_grim->getGameType() == GType_MONKEY4 && clamp) {
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		N3D_C3D_TexSetWrap(textures, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);															// DEFINITE?
	} else {
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		N3D_C3D_TexSetWrap(textures, GPU_REPEAT, GPU_REPEAT);																		// DEFINITE?
	}

//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	N3D_C3D_TexSetFilter(textures, GPU_LINEAR, GPU_LINEAR);																			// DEFINITE?

	if (cmap != nullptr) { // EMI doesn't have colour-maps
		int bytes = 4;

		char *texdata = new char[texture->_width * texture->_height * bytes];
		char *texdatapos = texdata;

		for (int y = 0; y < texture->_height; y++) {
			for (int x = 0; x < texture->_width; x++) {
				uint8 col = *(const uint8 *)(data);
				if (col == 0) {
					memset(texdatapos, 0, bytes); // transparent
					if (!texture->_hasAlpha) {
						texdatapos[3] = '\xff'; // fully opaque
					}
				} else {
					memcpy(texdatapos, cmap->_colors + 3 * (col), 3);
					texdatapos[3] = '\xff'; // fully opaque
				}
				texdatapos += bytes;
				data++;
			}
		}

//		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->_width, texture->_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texdata);
		debug("GfxN3DS::createTexture - Creating tex");
		N3D_C3D_TexInit(textures, (u16)texture->_width, (u16)texture->_height, GPU_RGBA8);											// DEFINITE?
		debug("GfxN3DS::createTexture - Tex created: %u bytes", textures->size);
		GSPGPU_FlushDataCache((void *)texdata, texture->_width * texture->_height * bytes);											// DEFINITE?
		// GX_TRANSFER_FMT_RGBA8 is already 0																						// DEFINITE?
		// GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) = (1 << 0) | (1 << 1) = 0b01 | 0b10 = 0b11 = 3						// DEFINITE?
		N3D_C3D_SyncDisplayTransfer((u32 *)texdata,          (u32)GX_BUFFER_DIM(texture->_width, texture->_height),					// DEFINITE?
		                            (u32 *)textures[0].data, (u32)GX_BUFFER_DIM(texture->_width, texture->_height), 3);				// DEFINITE?
		delete[] texdata;
	} else {
//		GLint format = (texture->_bpp == 4) ? GL_RGBA : GL_RGB;
//		glTexImage2D(GL_TEXTURE_2D, 0, format, texture->_width, texture->_height, 0, format, GL_UNSIGNED_BYTE, data);
		GPU_TEXCOLOR format;
		GX_TRANSFER_FORMAT transFmt;																								// DEFINITE?
		if (texture->_bpp == 4) {																									// DEFINITE?
			format = GPU_RGBA8;																										// DEFINITE?
			transFmt = GX_TRANSFER_FMT_RGBA8;																						// DEFINITE?
		} else {																													// DEFINITE?
			format = GPU_RGB8;																										// DEFINITE?
			transFmt = GX_TRANSFER_FMT_RGB8;																						// DEFINITE?
		}																															// DEFINITE?
		debug("GfxN3DS::createTexture - Creating tex");
		N3D_C3D_TexInit(textures, (u16)texture->_width, (u16)texture->_height, format);												// DEFINITE?
		debug("GfxN3DS::createTexture - Tex created: %u bytes", textures->size);
		GSPGPU_FlushDataCache(static_cast<const void *>(data), texture->_width * texture->_height * texture->_bpp);					// DEFINTIE?
		// GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) = (1 << 0) | (1 << 1) = 0b01 | 0b10 = 0b11 = 3						// DEFINITE?
		N3D_C3D_SyncDisplayTransfer((u32 *)const_cast<uint8 *>(data), (u32)GX_BUFFER_DIM(texture->_width, texture->_height),		// DEFINITE?
		                            (u32 *)textures[0].data,          (u32)GX_BUFFER_DIM(texture->_width, texture->_height),		// DEFINITE?
		                            GX_TRANSFER_IN_FORMAT(transFmt) | GX_TRANSFER_OUT_FORMAT(transFmt) | 3);						// DEFINITE?
	}
}

void GfxN3DS::selectTexture(const Texture *texture) {
//	GLuint *textures = (GLuint *)texture->_texture;
//	glBindTexture(GL_TEXTURE_2D, textures[0]);
	C3D_Tex *textures = static_cast<C3D_Tex *>(texture->_texture);																	// DEFINITE?
	debug("GfxN3DS::selectTexture");
	N3D_C3D_TexBind(0, textures);																						// DEFINITE?

	if (texture->_hasAlpha && g_grim->getGameType() == GType_MONKEY4) {
//		glEnable(GL_BLEND);
		N3D_BlendEnabled(true);																										// DEFINITE?
	}

	_selectedTexture = const_cast<Texture *>(texture);
}

void GfxN3DS::destroyTexture(Texture *texture) {
//	GLuint *textures = static_cast<GLuint *>(texture->_texture);
	C3D_Tex *textures = static_cast<C3D_Tex *>(texture->_texture);																	// DEFINITE?
	if (textures) {
//		glDeleteTextures(1, textures);
		debug("GfxN3DS::destroyTexture - Deleting tex: -%u bytes", textures->size);
		N3D_C3D_TexDelete(textures);																								// DEFINITE?
		debug("GfxN3DS::destroyTexture - Tex deleted");
		delete[] textures;
	}
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
//		GLuint *textures = new GLuint[bitmap->_numTex * bitmap->_numImages];
		C3D_Tex *textures = new C3D_Tex[bitmap->_numTex * bitmap->_numImages];														// DEFINITE?
		bitmap->_texIds = textures;
//		glGenTextures(bitmap->_numTex * bitmap->_numImages, textures);

		byte *texData = nullptr;
		const byte *texOut = nullptr;

//		GLint format = GL_RGBA;
//		GLint btype = GL_UNSIGNED_BYTE;
		GPU_TEXCOLOR format = GPU_RGBA8;																							// DEFINITE?
		int bytes = 4;

//		glPixelStorei(GL_UNPACK_ALIGNMENT, bytes);

		const Graphics::PixelFormat format_16bpp(2, 5, 6, 5, 0, 11, 5, 0, 0);
// #ifdef SCUMM_BIG_ENDIAN
		const Graphics::PixelFormat format_32bpp(4, 8, 8, 8, 8, 24, 16, 8, 0);
// #else
//		const Graphics::PixelFormat format_32bpp(4, 8, 8, 8, 8, 0, 8, 16, 24);
// #endif

		for (int pic = 0; pic < bitmap->_numImages; pic++) {
			const Graphics::Surface &imageData = bitmap->getImageData(pic);
			if (bitmap->_format == 1 && imageData.format == format_16bpp) {
				if (texData == nullptr)
					texData = new byte[bytes * bitmap->_width * bitmap->_height];
				// Convert data to 32-bit RGBA format
				byte *texDataPtr = texData;
				const uint16 *bitmapData = reinterpret_cast<const uint16 *>(imageData.getPixels());
				for (int i = 0; i < bitmap->_width * bitmap->_height; i++, texDataPtr += bytes, bitmapData++) {
					uint16 pixel = *bitmapData;
					int r = pixel >> 11;
					texDataPtr[0] = (r << 3) | (r >> 2);
					int g = (pixel >> 5) & 0x3f;
					texDataPtr[1] = (g << 2) | (g >> 4);
					int b = pixel & 0x1f;
					texDataPtr[2] = (b << 3) | (b >> 2);
					if (pixel == 0xf81f) { // transparent
						texDataPtr[3] = 0;
						bitmap->_hasTransparency = true;
					} else {
						texDataPtr[3] = 255;
					}
				}
				texOut = texData;
			} else if (bitmap->_format == 1 && imageData.format != format_32bpp) {
				bitmap->convertToColorFormat(pic, format_32bpp);
				texOut = (const byte *)imageData.getPixels();
			} else {
				texOut = (const byte *)imageData.getPixels();
			}

			int actualWidth = nextHigher2(bitmap->_width);
			int actualHeight = nextHigher2(bitmap->_height);

//			glBindTexture(GL_TEXTURE_2D, textures[bitmap->_numTex * pic]);
//			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//			glTexImage2D(GL_TEXTURE_2D, 0, format, actualWidth, actualHeight, 0, format, btype, nullptr);
//			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bitmap->_width, bitmap->_height, format, btype, texOut);
			C3D_Tex *c3dTex = &textures[bitmap->_numTex * pic];																		// DEFINITE?
			debug("GfxN3DS::createBitmap - Creating tex");
			N3D_C3D_TexInit(c3dTex, (u16)actualWidth, (u16)actualHeight, format);													// DEFINITE?
			debug("GfxN3DS::createBitmap - Tex created: %u bytes", c3dTex->size);
			N3D_C3D_TexSetFilter(c3dTex, GPU_NEAREST, GPU_NEAREST);																	// DEFINITE?
			N3D_C3D_TexSetWrap(c3dTex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);														// DEFINITE?
			N3D_DataToBlockTex((u32 *)const_cast<uint8 *>(texOut), (u32 *)c3dTex->data, 0, 0,										// DEFINITE?
			                   bitmap->_width, bitmap->_height, actualWidth, actualHeight,											// DEFINITE?
			                   format, 0, false);																					// DEFINITE?
		}

		if (texData)
			delete[] texData;
		bitmap->freeData();

//		OpenGL::Shader *shader = _backgroundProgram->clone();
		N3DS_3D::ShaderObj *shader = new N3DS_3D::ShaderObj(_programBackground);													// DEFINITE?
		bitmap->_userData = shader;

		if (g_grim->getGameType() == GType_MONKEY4) {
//			GLuint vbo = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, bitmap->_numCoords * 4 * sizeof(float), bitmap->_texc, GL_STATIC_DRAW);
//			shader->enableVertexAttribute("position", vbo, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
//			shader->enableVertexAttribute("texcoord", vbo, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2*sizeof(float));
			debug("GfxN3DS::createBitmap - Creating linear alloc (void *vbo)");
			void *vbo = N3DS_3D::createBuffer(bitmap->_numCoords * 4 * sizeof(float), bitmap->_texc);								// DEFINITE?
			debug("GfxN3DS::createBitmap - Linear alloc created: %u bytes (void *vbo)", linearGetSize(vbo));
			BufInfo_Init(&shader->_bufInfo);																						// DEFINITE?
			shader->addBufInfo(vbo, 4 * sizeof(float), 2, 0x10);																	// DEFINITE?
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
//		OpenGL::Shader *shader = (OpenGL::Shader *)data->_userData;
//		GLuint *textures = (GLuint *)bitmap->getTexIds();
		N3DS_3D::ShaderObj *shader = (N3DS_3D::ShaderObj *)data->_userData;															// DEFINITE? - CLONE OF _programBackground
		C3D_Tex *textures = static_cast<C3D_Tex *>(bitmap->getTexIds());															// DEFINITE?

//		glDisable(GL_DEPTH_TEST);
		N3D_DepthTestEnabled(false);																								// DEFINITE?
		N3D_DepthMask(false);																										// DEFINITE? - ADDED from "gfx_opengl.cpp"

//		glEnable(GL_BLEND);
//		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		N3D_BlendEnabled(true);																										// DEFINITE?
		N3D_BlendFunc(GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);																		// DEFINITE?

//		shader->use();
//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quadEBO);
		N3DCONTEXT_FROM_HANDLE(_grimContext)->changeShader(shader);																	// DEFINITE?
		N3D_C3D_FrameBegin(0);																										// DEFINITE? - ADDED
		N3D_C3D_SetTexEnv(0, &envNormal);																							// DEFINITE? - ADDED
		N3D_C3D_FrameDrawOn(_gameScreenTarget);																						// DEFINITE? - ADDED
		assert(layer < data->_numLayers);
		uint32 offset = data->_layers[layer]._offset;
		for (uint32 i = offset; i < offset + data->_layers[layer]._numImages; ++i) {
//			glBindTexture(GL_TEXTURE_2D, textures[data->_verts[i]._texid]);
			debug("GfxN3DS::drawBitmap");
			N3D_C3D_TexBind(0, textures + data->_verts[i]._texid);														// DEFINITE?

//			unsigned short startVertex = data->_verts[i]._pos / 4 * 6;
//			unsigned short numVertices = data->_verts[i]._verts / 4 * 6;
//			glDrawElements(GL_TRIANGLES, numVertices, GL_UNSIGNED_SHORT, (void *)(startVertex * sizeof(unsigned short)));
			u16 startVertex = data->_verts[i]._pos / 4 * 6;																			// DEFINITE?
			int numVertices = data->_verts[i]._verts / 4 * 6;																		// DEFINITE?
			N3D_C3D_DrawElements(GPU_TRIANGLES, numVertices, C3D_UNSIGNED_SHORT, (void *)((u16 *)_quadEBO + startVertex));			// DEFINITE?
		}
		N3D_C3D_FrameSplit(0);																										// DEFINITE? - ADDED
		_gameScreenDirty = true;																									// DEFINITE? - ADDED

		N3D_BlendEnabled(false);																									// DEFINITE? - ADDED from "gfx_opengl.cpp"
		N3D_DepthMask(true);																										// DEFINITE? - ADDED from "gfx_opengl.cpp"
		N3D_DepthTestEnabled(true);																									// DEFINITE? - ADDED from "gfx_opengl.cpp"

		return;
	}

	int format = bitmap->getFormat();
	if ((format == 1 && !_renderBitmaps) || (format == 5 && !_renderZBitmaps)) {
		return;
	}

	if (format == 1) {
//		GLuint *textures = (GLuint *)bitmap->getTexIds();
		C3D_Tex *textures = static_cast<C3D_Tex *>(bitmap->getTexIds());															// DEFINITE?
		if (bitmap->getFormat() == 1 && bitmap->getHasTransparency()) {
//			glEnable(GL_BLEND);
//			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			N3D_BlendEnabled(true);																									// DEFINITE?
			N3D_BlendFunc(GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);																	// DEFINITE?
		} else {
//			glDisable(GL_BLEND);
			N3D_BlendEnabled(false);																								// DEFINITE?
		}

//		OpenGL::Shader *shader = (OpenGL::Shader *)bitmap->_data->_userData;
//		shader->use();
//		glDisable(GL_DEPTH_TEST);
//		glDepthMask(GL_FALSE);
		N3DS_3D::ShaderObj *shader = static_cast<N3DS_3D::ShaderObj *>(bitmap->_data->_userData);									// DEFINITE?
		N3DCONTEXT_FROM_HANDLE(_grimContext)->changeShader(shader);																	// DEFINITE?
		N3D_DepthTestEnabled(false);																								// DEFINITE?
		N3D_DepthMask(false);																										// DEFINITE?

//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quadEBO);
		int cur_tex_idx = bitmap->getNumTex() * (bitmap->getActiveImage() - 1);
//		glBindTexture(GL_TEXTURE_2D, textures[cur_tex_idx]);
		debug("GfxN3DS::drawBitmap2");
		N3D_C3D_TexBind(0, textures + cur_tex_idx);																		// DEFINITE?
		float width = bitmap->getWidth();
		float height = bitmap->getHeight();
//		shader->setUniform("offsetXY", Math::Vector2d(float(dx) / _gameWidth, float(dy) / _gameHeight));
//		shader->setUniform("sizeWH", Math::Vector2d(width / _gameWidth, height / _gameHeight));
//		shader->setUniform("texcrop", Math::Vector2d(width / nextHigher2((int)width), height / nextHigher2((int)height)));
//		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
		shader->setUniform("offsetXY", GPU_VERTEX_SHADER, Math::Vector2d(float(dx) / _gameWidth, float(dy) / _gameHeight));			// DEFINITE?
		shader->setUniform("sizeWH",   GPU_VERTEX_SHADER, Math::Vector2d(    width / _gameWidth,    height / _gameHeight));			// DEFINITE?
		shader->setUniform("texcrop",  GPU_VERTEX_SHADER, Math::Vector2d(width  / nextHigher2((int)width),							// DEFINITE?
		                                                                 height / nextHigher2((int)height)));						// DEFINITE?
		N3D_C3D_FrameBegin(0);																										// DEFINITE? - ADDED
		N3D_C3D_FrameDrawOn(_gameScreenTarget);																						// DEFINITE? - ADDED
		N3D_C3D_SetTexEnv(0, &envNormal);																							// DEFINITE? - ADDED
		N3D_C3D_DrawElements(GPU_TRIANGLES, 6, C3D_UNSIGNED_SHORT, (void *)_quadEBO);												// DEFINITE?
		N3D_C3D_FrameSplit(0);																										// DEFINITE? - ADDED
		_gameScreenDirty = true;																									// DEFINITE? - ADDED

//		glDisable(GL_BLEND);
//		glDepthMask(GL_TRUE);
//		glEnable(GL_DEPTH_TEST);
		N3D_BlendEnabled(false);																									// DEFINITE?
		N3D_DepthMask(true);																										// DEFINITE?
		N3D_DepthTestEnabled(true);																									// DEFINITE?
	} else {
		// Only draw the manual zbuffer when enabled
		if (bitmap->getActiveImage() - 1 < bitmap->getNumImages()) {
			drawDepthBitmap(bitmap->getId(), dx, dy, bitmap->getWidth(), bitmap->getHeight(),
			                (char *)const_cast<void *>(bitmap->getData(bitmap->getActiveImage() - 1).getPixels()));
		} else {
			debug("zbuffer image has index out of bounds! %d/%d", bitmap->getActiveImage(), bitmap->getNumImages());
		}
		return;
	}
}

void GfxN3DS::drawDepthBitmap(int bitmapId, int x, int y, int w, int h, char *data) {
	N3D_C3D_FrameEnd(0);																											// DEFINITE? - ADDED

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

//	glActiveTexture(GL_TEXTURE1);
//	glBindTexture(GL_TEXTURE_2D, _zBufTex);
//	glPixelStorei(GL_UNPACK_ALIGNMENT, 2); // 16 bit Z depth bitmap
//	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data);
//	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
//	glActiveTexture(GL_TEXTURE0);
	N3D_DataToBlockTex((u32 *)data, (u32 *)_zBuf, x, y, w, h, nextHigher2(w), nextHigher2(h), GPU_RGBA8, 0, false);					// DEFINITE?
	GSPGPU_FlushDataCache(_zBuf, nextHigher2(w) * nextHigher2(h) * 4);																// DEFINITE?
	GX_RequestDma((u32 *)_zBuf, (u32 *)_gameScreenTarget->frameBuf.depthBuf, nextHigher2(w) * nextHigher2(h) * 4);					// DEFINITE?
	gspWaitForDMA();																												// DEFINITE?
	////_gameScreenDirty = true;																									// DEFINITE? - ADDED
}

void GfxN3DS::destroyBitmap(BitmapData *bitmap) {
//	GLuint *textures = (GLuint *)bitmap->_texIds;
	C3D_Tex *textures = static_cast<C3D_Tex *>(bitmap->_texIds);																	// DEFINITE?
	if (textures) {
//		glDeleteTextures(bitmap->_numTex * bitmap->_numImages, textures);
		for (int i = 0; i < bitmap->_numTex * bitmap->_numImages; i++) {															// DEFINITE?
			debug("GfxN3DS::destroyBitmap - Deleting tex[%d]: -%u bytes", i, (textures + i)->size);
			N3D_C3D_TexDelete(textures + i);																						// DEFINITE?
			debug("GfxN3DS::destroyBitmap - Tex deleted");
		}																															// DEFINITE?
		delete[] textures;
		bitmap->_texIds = nullptr;
	}
//	OpenGL::Shader *shader = (OpenGL::Shader *)bitmap->_userData;
	N3DS_3D::ShaderObj *shader = (N3DS_3D::ShaderObj *)bitmap->_userData;															// DEFINITE?
	if (g_grim->getGameType() == GType_MONKEY4) {
//		glDeleteBuffers(1, &shader->getAttributeAt(0)._vbo);
		shader->freeAttachedBuffer(0);																								// DEFINITE?
	}
	delete shader;

	if (bitmap->_format != 1) {
		bitmap->freeData();
	}
}

void GfxN3DS::createFont(Font *f) {
	if (!f->is8Bit())
		error("non-8bit fonts are not supported in GL shaders renderer");
	BitmapFont *font = static_cast<BitmapFont *>(f);
	const byte *bitmapData = font->getFontData();
	uint dataSize = font->getDataSize();

	uint8 bpp = 4;
	uint8 charsWide = 16;
	uint8 charsHigh = 16;

	byte *texDataPtr = new byte[dataSize * bpp];
	byte *data = texDataPtr;

	// DO NOT NEED TO FLIP COLOR COMPONENT ORDER
	for (uint i = 0; i < dataSize; i++, texDataPtr += bpp, bitmapData++) {
		byte pixel = *bitmapData;
		if (pixel == 0x00) {
			texDataPtr[0] = texDataPtr[1] = texDataPtr[2] = texDataPtr[3] = 0;											// total transparency
		} else if (pixel == 0x80) {
			texDataPtr[0] = texDataPtr[1] = texDataPtr[2] = 0;															// RGB = 00 00 00
			texDataPtr[3] = 255;																						// total opacity
		} else if (pixel == 0xFF) {
			texDataPtr[0] = texDataPtr[1] = texDataPtr[2] = texDataPtr[3] = 255;										// RGB = FF FF FF, total opacity
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

	uint arraySize = size * size * bpp * charsWide * charsHigh;
	byte *temp = new byte[arraySize]();

	FontUserData *userData = new FontUserData;
	font->setUserData(userData);
//	userData->texture = 0;
	userData->size = size;
	userData->subTextures = (Tex3DS_SubTexture *)malloc(sizeof(Tex3DS_SubTexture) * charsWide * charsHigh);							// DEFINITE? - ADDED

//	GLuint *texture = &(userData->texture);
//	glGenTextures(1, texture);
	for (int i = 0, row = 0; i < 256; ++i) {
		u32 charX = (i != 0) ? ((i - 1) % 16) : 0;																					// DEFINITE? - ADDED
		u32 charY = (i != 0) ? ((i - 1) / 16) : 0;																					// DEFINITE? - ADDED
																																	// DEFINITE? - ADDED
		userData->subTextures[i].width  = size;																						// DEFINITE? - ADDED
		userData->subTextures[i].height = size;																						// DEFINITE? - ADDED
		userData->subTextures[i].left   =          float(charX)     / charsWide;													// DEFINITE? - ADDED
		userData->subTextures[i].top    = 1.0f - ( float(charY)     / charsHigh );													// DEFINITE? - ADDED
		userData->subTextures[i].right  =          float(charX + 1) / charsWide;													// DEFINITE? - ADDED
		userData->subTextures[i].bottom = 1.0f - ( float(charY + 1) / charsHigh );													// DEFINITE? - ADDED

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
			memcpy(temp + pos, data + pos2, width * bpp);
		}
		if (i != 0 && i % charsWide == 0)
			++row;
	}

//	glBindTexture(GL_TEXTURE_2D, texture[0]);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size * charsWide, size * charsHigh, 0, GL_RGBA, GL_UNSIGNED_BYTE, temp);
	u32 pixelsWide = charsWide * size;																								// DEFINITE?
	u32 pixelsHigh = charsHigh * size;																								// DEFINITE?
	debug("GfxN3DS::createFont - Creating tex");
	N3D_C3D_TexInit(userData->texture, (u16)pixelsWide, (u16)pixelsHigh, GPU_RGBA8);												// DEFINITE?
	debug("GfxN3DS::createFont - Tex created: %u bytes", userData->texture->size);
	N3D_C3D_TexSetFilter(userData->texture, GPU_NEAREST, GPU_NEAREST);																// DEFINITE?
	N3D_C3D_TexSetWrap(userData->texture, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);													// DEFINITE?
	GSPGPU_FlushDataCache((void *)temp, (u32)arraySize);																			// DEFINITE?
	// GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) = (1 << 0) | (1 << 1) = 0b01 | 0b10 = 0b11 = 3							// DEFINITE?
	N3D_C3D_SyncDisplayTransfer((u32 *)temp,                    GX_BUFFER_DIM(pixelsWide, pixelsHigh),								// DEFINITE?
	                            (u32 *)userData->texture->data, GX_BUFFER_DIM(pixelsWide, pixelsHigh), 3);							// DEFINITE?

	delete[] data;
	delete[] temp;
	temp = nullptr;
}

void GfxN3DS::destroyFont(Font *font) {
	if (font->is8Bit()) {
		const FontUserData *data = static_cast<const FontUserData *>(static_cast<const BitmapFont *>(font)->getUserData());
		if (data) {
//			glDeleteTextures(1, &(data->texture));
			debug("GfxN3DS::destroyFont - Deleting tex: -%u bytes", data->texture->size);
			N3D_C3D_TexDelete(data->texture);																						// DEFINITE?
			debug("GfxN3DS::destroyFont - Tex deleted");
			free(data->subTextures);																								// DEFINITE?
			delete data;
		}
	}
}

void GfxN3DS::createTextObject(TextObject *text) {
	const Color &color = text->getFGColor();
	const Font *f = text->getFont();
	if (!f->is8Bit())
		error("non-8bit fonts are not supported in GL shaders renderer");
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

	const Tex3DS_SubTexture *subTexes = userData->subTextures;																		// DEFINITE? - ADDED

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
//			float width = 1 / 16.f;
//			float cx = ((character - 1) % 16) / 16.0f;
//			float cy = ((character - 1) / 16) / 16.0f;

			float charData[] = {
//				z, w, cx, cy,
//				z + sizeW, w, cx + width, cy,
//				z + sizeW, w + sizeH, cx + width, cy + width,
//				z, w + sizeH, cx, cy + width
				z,         w,         subTexes[character].left,  subTexes[character].top,											// DEFINITE?	// position[2], texcoords[2]
				z + sizeW, w,         subTexes[character].right, subTexes[character].top,											// DEFINITE?	// position[2], texcoords[2]
				z + sizeW, w + sizeH, subTexes[character].right, subTexes[character].bottom,										// DEFINITE?	// position[2], texcoords[2]
				z,         w + sizeH, subTexes[character].left,  subTexes[character].bottom											// DEFINITE?	// position[2], texcoords[2]
			};
			memcpy(cur, charData, 16 * sizeof(float));
			cur += 16;

			x += font->getCharKernedWidth(character);
		}
	}
//	GLuint vbo;
	void *vbo;																														// DEFINITE?
	if (text->isBlastDraw()) {
		vbo = _blastVBO;
//		glBindBuffer(GL_ARRAY_BUFFER, vbo);
//		glBufferSubData(GL_ARRAY_BUFFER, 0, numCharacters * 16 * sizeof(float), bufData);
		memcpy(vbo, bufData, numCharacters * 16 * sizeof(float));																	// DEFINITE?
	} else {
//		vbo = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, numCharacters * 16 * sizeof(float), bufData, GL_STATIC_DRAW);
		debug("GfxN3DS::createTextObject - Creating linear alloc (vbo)");
		vbo = N3DS_3D::createBuffer(numCharacters * 16 * sizeof(float), bufData);													// DEFINITE?
		debug("GfxN3DS::createTextObject - Linear alloc created: %u bytes (vbo)", linearGetSize(vbo));
	}

//	OpenGL::Shader * textShader = _textProgram->clone();
//	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	N3DS_3D::ShaderObj *textShader = new N3DS_3D::ShaderObj(_programText);															// DEFINITE?

//	textShader->enableVertexAttribute("position", vbo, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
//	textShader->enableVertexAttribute("texcoord", vbo, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
	textShader->addAttrLoader(0, GPU_FLOAT, 2);							// v0 = position											// DEFINITE?
	textShader->addAttrLoader(1, GPU_FLOAT, 2);							// v1 = texcoord											// DEFINITE?
	textShader->addBufInfo(vbo, 4 * sizeof(float), 2, 0x10);																		// DEFINITE?

	TextUserData * td = new TextUserData;
	td->characters = numCharacters;
	td->shader = textShader;
	td->color = color;
	td->texture = userData->texture;
	text->setUserData(td);
	delete[] bufData;
}

void GfxN3DS::drawTextObject(const TextObject *text) {
//	glEnable(GL_BLEND);
//	glDisable(GL_DEPTH_TEST);
	N3D_BlendEnabled(true);																											// DEFINITE?
	N3D_DepthTestEnabled(false);																									// DEFINITE?
	const TextUserData * td = (const TextUserData *) text->getUserData();
	assert(td);
//	td->shader->use();
	N3DCONTEXT_FROM_HANDLE(_grimContext)->changeShader(td->shader);																	// DEFINITE?

	Math::Vector3d colors(float(td->color.getRed()) / 255.0f,
	                      float(td->color.getGreen()) / 255.0f,
	                      float(td->color.getBlue()) / 255.0f);
//	td->shader->setUniform("color", colors);
//	glBindTexture(GL_TEXTURE_2D, td->texture);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quadEBO);
//	glDrawElements(GL_TRIANGLES, td->characters * 6, GL_UNSIGNED_SHORT, nullptr);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//	glEnable(GL_DEPTH_TEST);
	td->shader->setUniform("color", GPU_VERTEX_SHADER, colors);																		// DEFINITE?
	debug("GfxN3DS::drawTextObject");
	N3D_C3D_TexBind(0, td->texture);																						// DEFINITE?
	N3D_C3D_FrameBegin(0);																											// DEFINITE? - ADDED
	N3D_C3D_FrameDrawOn(_gameScreenTarget);																							// DEFINITE? - ADDED
	N3D_C3D_SetTexEnv(0, &envText);																									// DEFINITE? - ADDED
	N3D_C3D_DrawElements(GPU_TRIANGLES, (int)td->characters * 6, C3D_UNSIGNED_SHORT, (void *)_quadEBO);								// DEFINITE?
	N3D_C3D_FrameSplit(0);																											// DEFINITE? - ADDED
	_gameScreenDirty = true;																										// DEFINITE? - ADDED
	N3D_DepthTestEnabled(true);																										// DEFINITE?
}

void GfxN3DS::destroyTextObject(TextObject *text) {
	const TextUserData * td = (const TextUserData *) text->getUserData();
	if (!text->isBlastDraw()) {
//		glDeleteBuffers(1, &td->shader->getAttributeAt(0)._vbo);
		td->shader->freeAttachedBuffer(0);																							// DEFINITE?
	}
	text->setUserData(nullptr);

	delete td->shader;
	delete td;
}

void GfxN3DS::storeDisplay() {
//	glBindTexture(GL_TEXTURE_2D, _storedDisplay);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _screenWidth, _screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);	// nullptr means blank tex is created
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, _screenWidth, _screenHeight, 0);
	debug("GfxN3DS::storeDisplay - Deleting tex: -%u bytes (_storedDisplay)", _storedDisplay.size);
	N3D_C3D_TexDelete(&_storedDisplay);																								// DEFINITE?
	debug("GfxN3DS::storeDisplay - Tex deleted (_storedDisplay)");
	debug("GfxN3DS::storeDisplay - Creating tex (_storedDisplay)");
	N3D_C3D_TexInit(&_storedDisplay, (u16)_screenTexWidth, (u16)_screenTexHeight, GPU_RGBA8);										// DEFINITE?
	debug("GfxN3DS::storeDisplay - Tex created: %u bytes (_storedDisplay)", _storedDisplay.size);
	N3D_C3D_TexSetFilter(&_storedDisplay, GPU_LINEAR, GPU_LINEAR);																	// DEFINITE?
	N3D_C3D_SyncTextureCopy(																										// DEFINITE?
		(u32 *)_gameScreenTex->data, GX_BUFFER_DIM(((u32)_screenTexWidth       * 8 * 4) >> 4, 0),									// DEFINITE?
		(u32 *)_storedDisplay.data, GX_BUFFER_DIM(((u32)_storedDisplay.width * 8 * 4) >> 4, 0),										// DEFINITE?
		_storedDisplay.width * _storedDisplay.height * 4, GX_TRANSFER_RAW_COPY(1)													// DEFINITE?
	);																																// DEFINITE?
}

void GfxN3DS::copyStoredToDisplay() {
//	if (!_dimProgram)
	if (!_programDim)																												// DEFINITE?
		return;

//	_dimProgram->use();
//	_dimProgram->setUniform("scaleWH", Math::Vector2d(1.f, 1.f));
//	_dimProgram->setUniform("tex", 0);																					// FRAGMENT SHADER UNIFORM, SAMPLER2D
	N3DCONTEXT_FROM_HANDLE(_grimContext)->changeShader(_programDim);																// DEFINITE?
	_programDim->setUniform("scaleWH", GPU_VERTEX_SHADER, Math::Vector2d(1.f, 1.f));												// DEFINITE?

//	glBindTexture(GL_TEXTURE_2D, _storedDisplay);
	debug("GfxN3DS::copyStoredToDisplay");
	N3D_C3D_TexBind(0, &_storedDisplay);																					// DEFINITE?

//	glDisable(GL_DEPTH_TEST);
//	glDepthMask(GL_FALSE);
	N3D_DepthTestEnabled(false);																									// FINALIZED
	N3D_DepthMask(false);																											// FINALIZED

//	glDrawArrays(GL_TRIANGLES, 0, 6);
	N3D_C3D_FrameBegin(0);																											// DEFINITE? - ADDED
	N3D_C3D_FrameDrawOn(_gameScreenTarget);																							// DEFINITE? - ADDED
	N3D_C3D_SetTexEnv(0, &envNormal);																								// DEFINITE? - ADDED
	N3D_C3D_DrawArrays(GPU_TRIANGLES, 0, 6);																						// DEFINITE?
	N3D_C3D_FrameSplit(0);																											// DEFINITE? - ADDED
	_gameScreenDirty = true;																										// DEFINITE? - ADDED

//	glEnable(GL_DEPTH_TEST);
//	glDepthMask(GL_TRUE);
	N3D_DepthTestEnabled(true);																										// FINALIZED
	N3D_DepthMask(true);																											// FINALIZED
}

void GfxN3DS::dimScreen() {
	u32 *data = (u32 *)_storedDisplay.data;																							// DEFINITE? - MODIFIED from "gfx_opengl.cpp"
	for (int l = 0; l < _screenTexWidth * _screenTexHeight; l++) {																	// DEFINITE? - MODIFIED from "gfx_opengl.cpp"
		u32 pixel = data[l];																										// DEFINITE? - ADDED from "gfx_opengl.cpp"
		u8 r = (pixel & 0x000000FF);																								// DEFINITE? - MODIFIED from "gfx_opengl.cpp"
		u8 g = (pixel & 0x0000FF00) >> 8;																							// DEFINITE? - MODIFIED from "gfx_opengl.cpp"
		u8 b = (pixel & 0x00FF0000) >> 16;																							// DEFINITE? - MODIFIED from "gfx_opengl.cpp"
		u32 color = (r + g + b) / 10;																								// DEFINITE? - ADDED from "gfx_opengl.cpp"
		data[l] = (pixel & 0xFF000000) | ((color & 0xFF) << 16) | ((color & 0xFF) << 8) | (color & 0xFF);							// DEFINITE? - MODIFIED from "gfx_opengl.cpp"
	}																																// DEFINITE? - ADDED from "gfx_opengl.cpp"
}

void GfxN3DS::dimRegion(int xin, int yReal, int w, int h, float level) {
	N3D_C3D_FrameEnd(0);																											// DEFINITE? - ADDED

	// _screenWidth = 640, _screenTexWidth = 1042
	// _screenHeight = 480, _screenTexHeight = 512
	xin = (int)(xin * _scaleW);
	yReal = (int)(yReal * _scaleH);
	w = (int)(w * _scaleW);
	h = (int)(h * _scaleH);
//	int yin = _screenHeight - yReal - h;
	int yin = _screenTexHeight - yReal - h;																							// DEFINITE?
	int wcoord = xin + w;																											// DEFINITE? - ADDED
	int hcoord = yin + h;																											// DEFINITE? - ADDED
	int xin8 = xin >> 3 << 3;	// xin lowered to 0 or multiple of 8 (if it isn't already a multiple of 8)							// DEFINITE? - ADDED
	int yin8 = yin >> 3 << 3;	// yin lowered to 0 or multiple of 8 (if it isn't already a multiple of 8)							// DEFINITE? - ADDED
	// w8 and h8 are wcoord and hcoord RAISED to multiples of 8 (if they aren't already multiples of 8),							// DEFINITE? - ADDED
	//     subtracted by xin8 and yin8, respectively.																				// DEFINITE? - ADDED
	int w8 = (wcoord % 8 == 0) ? (wcoord - xin8) : ((((wcoord >> 3) + 1) << 3) - xin8);												// DEFINITE? - ADDED
	int h8 = (hcoord % 8 == 0) ? (hcoord - yin8) : ((((hcoord >> 3) + 1) << 3) - yin8);												// DEFINITE? - ADDED

//	GLuint texture;
//	glGenTextures(1, &texture);
//	glBindTexture(GL_TEXTURE_2D, texture);
	C3D_Tex tmpTex, texture;																										// DEFINITE?

//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	debug("GfxN3DS::dimRegion - Creating tex (tmpTex)");
	N3D_C3D_TexInit(&tmpTex,  (u16)nextHigher2(w8), (u16)nextHigher2(h8), GPU_RGBA8);												// DEFINITE?
	debug("GfxN3DS::dimRegion - Tex created: %u bytes (tmpTex)", tmpTex.size);
	debug("GfxN3DS::dimRegion - Creating tex (texture)");
	N3D_C3D_TexInit(&texture, (u16)nextHigher2(w),  (u16)nextHigher2(h),  GPU_RGBA8);												// DEFINITE?
	debug("GfxN3DS::dimRegion - Tex created: %u bytes (texture)", texture.size);
	N3D_C3D_TexSetFilter(&texture, GPU_LINEAR, GPU_LINEAR);																			// DEFINITE?

//	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, xin, yin, w, h, 0);
	//???    u8* _dstAddr = ((u8*)dstAddr) + (((destPosX * 8 + destPosY * dstWidth) * pixelSize) >> 3);
	//???    u8* _scrAddr = ((u8*)srcAddr) + (((sourcePosX * 8 + sourcePosY * srcWidth) * pixelSize) >> 3);
	N3D_C3D_SyncTextureCopy(																										// DEFINITE? - REIMPLEMENTED
		(u32 *)_gameScreenTarget->frameBuf.colorBuf + (yin8 * _screenTexWidth + xin8),												// DEFINITE? - REIMPLEMENTED
		(u32)GX_BUFFER_DIM((w8 * 8 * 4) >> 4, ((_screenTexWidth - w8) * 8 * 4) >> 4),												// DEFINITE? - REIMPLEMENTED
		(u32 *)tmpTex.data,																											// DEFINITE? - REIMPLEMENTED
		(u32)GX_BUFFER_DIM((w8 * 8 * 4) >> 4, ((tmpTex.width    - w8) * 8 * 4) >> 4),												// DEFINITE? - REIMPLEMENTED
		w8 * h8 * 4, GX_TRANSFER_RAW_COPY(1)																						// DEFINITE? - REIMPLEMENTED
	);																																// DEFINITE? - REIMPLEMENTED

//	glBindBuffer(GL_ARRAY_BUFFER, _dimRegionVBO);

	u32 *data = (u32 *)tmpTex.data;																									// DEFINITE? - MODIFIED from "gfx_opengl.cpp"
	for (int ly = 0; ly < tmpTex.height; ly++) {																					// DEFINITE? - MODIFIED from "gfx_opengl.cpp"
		for (int lx = 0; lx < tmpTex.width; lx++) {																					// DEFINITE? - MODIFIED from "gfx_opengl.cpp"
			u32 pixel = data[ly * tmpTex.width + lx];																				// DEFINITE? - MODIFIED from "gfx_opengl.cpp"
			u8 r = (pixel & 0x000000FF);																							// DEFINITE? - MODIFIED from "gfx_opengl.cpp"
			u8 g = (pixel & 0x0000FF00) >> 8;																						// DEFINITE? - MODIFIED from "gfx_opengl.cpp"
			u8 b = (pixel & 0x00FF0000) >> 16;																						// DEFINITE? - MODIFIED from "gfx_opengl.cpp"
			u32 color = (u32)(((r + g + b) / 3) * level);																			// DEFINITE? - ADDED from "gfx_opengl.cpp"
			data[ly * tmpTex.width + lx] = (pixel & 0xFF000000) | ((color & 0xFF) << 16) | ((color & 0xFF) << 8) | (color & 0xFF);	// DEFINITE? - MODIFIED from "gfx_opengl.cpp"
		}																															// DEFINITE? - ADDED from "gfx_opengl.cpp"
	}																																// DEFINITE? - ADDED from "gfx_opengl.cpp"
	if ((xin != xin8) | (yin != yin8) | (w != w8) | (h != h8)) {																	// DEFINITE? - ADDED
		N3D_ArbDataToArbBlockTexOffset((u32 *)tmpTex.data, (u32 *)texture.data, w, h, (xin - xin8), (yin - yin8),					// DEFINITE? - ADDED
		                               tmpTex.width, tmpTex.height, 0, 0, texture.width, texture.height, GPU_RGBA8,					// DEFINITE? - ADDED
		                               0, true);																					// DEFINITE? - ADDED
	} else {																														// DEFINITE? - ADDED
		memcpy(texture.data, tmpTex.data, texture.width * texture.height * 4);														// DEFINITE? - ADDED
	}																																// DEFINITE? - ADDED
	debug("GfxN3DS::dimRegion - Deleting tex: -%u bytes (tmptex)", tmpTex.size);
	N3D_C3D_TexDelete(&tmpTex);																										// DEFINITE? - ADDED
	debug("GfxN3DS::dimRegion - Tex deleted (tmptex)");

//	float width = w;
//	float height = h;
//	float x = xin;
//	float y = yin;
	float width = texture.width;																									// DEFINITE? - MODIFIED
	float height = texture.height;																									// DEFINITE? - MODIFIED
	float x = xin8;																													// DEFINITE? - MODIFIED
	float y = yin8;																													// DEFINITE? - MODIFIED
	float points[24] = {	// xy, uv
		// triangle 1
		x,         y,          0.0f, 0.0f,
		x + width, y,          1.0f, 0.0f,
		x + width, y + height, 1.0f, 1.0f,
		// triangle 2
		x + width, y + height, 1.0f, 1.0f,
		x,         y + height, 0.0f, 1.0f,
		x,         y,          0.0f, 0.0f,
	};

//	glBufferSubData(GL_ARRAY_BUFFER, 0, 24 * sizeof(float), points);
	memcpy(_dimRegionVBO, points, 24 * sizeof(float));																				// DEFINITE?

//	_dimRegionProgram->use();
//	_dimRegionProgram->setUniform("scaleWH", Math::Vector2d(1.f / _screenWidth, 1.f / _screenHeight));
//	_dimRegionProgram->setUniform("tex", 0);
	N3DCONTEXT_FROM_HANDLE(_grimContext)->changeShader(_programDimRegion);															// DEFINITE?
	_programDimRegion->setUniform("scaleWH", GPU_VERTEX_SHADER,																		// DEFINITE?
	                              Math::Vector2d(1.f / _screenWidth, 1.f / _screenHeight));											// DEFINITE?

//	glDisable(GL_DEPTH_TEST);
//	glDepthMask(GL_FALSE);
	N3D_DepthTestEnabled(false);																									// DEFINITE?
	N3D_DepthMask(false);																											// DEFINITE?

//	glDrawArrays(GL_TRIANGLES, 0, 6);
	debug("GfxN3DS::dimRegion");
	N3D_C3D_TexBind(0, &texture);																						// DEFINITE?
	N3D_C3D_FrameBegin(0);																											// DEFINITE? - ADDED
	N3D_C3D_FrameDrawOn(_gameScreenTarget);																							// DEFINITE? - ADDED
	N3D_C3D_SetTexEnv(0, &envNormal);																								// DEFINITE? - ADDED
	N3D_C3D_DrawArrays(GPU_TRIANGLES, 0, 6);																						// DEFINITE?
	N3D_C3D_FrameSplit(0);																											// DEFINITE? - ADDED
	_gameScreenDirty = true;																										// DEFINITE? - ADDED

//	glEnable(GL_DEPTH_TEST);
//	glDepthMask(GL_TRUE);
	N3D_DepthTestEnabled(true);																										// DEFINITE?
	N3D_DepthMask(true);																											// DEFINITE?

//	glDeleteTextures(1, &texture);
	debug("GfxN3DS::dimRegion - Deleting tex: -%u bytes (texture)", texture.size);
	N3D_C3D_TexDelete(&texture);																									// DEFINITE?
	debug("GfxN3DS::dimRegion - Tex deleted (texture)");
}

void GfxN3DS::irisAroundRegion(int x1, int y1, int x2, int y2) {
//	_irisProgram->use();																								// CLONE OF PRIMITIVE PROGRAM
//	_irisProgram->setUniform("color", Math::Vector3d(0.0f, 0.0f, 0.0f));
//	_irisProgram->setUniform("scaleWH", Math::Vector2d(1.f / _gameWidth, 1.f / _gameHeight));


	// 																																use GPU_SCISSOR_INVERT?


	N3DCONTEXT_FROM_HANDLE(_grimContext)->changeShader(_programIris);																// DEFINITE? - CLONE OF PRIMRECT
	_programIris->setUniform("color", GPU_VERTEX_SHADER, Math::Vector3d(0.0f, 0.0f, 0.0f));											// DEFINITE?
	_programIris->setUniform("scaleWH", GPU_VERTEX_SHADER, Math::Vector2d(1.f / _gameWidth, 1.f / _gameHeight));					// DEFINITE?

	float fx1 = x1;
	float fx2 = x2;
	float fy1 = y1;
	float fy2 = y2;
	float width = _screenWidth;
	float height = _screenHeight;
	float points[20] = { // xy
		0.0f,  0.0f,
		0.0f,  fy1,					// fx1,   fy1 ????
		width, 0.0f,
		fx2,   fy1,
		width, height,
		fx2,   fy2,
		0.0f,  height,
		fx1,   fy2,
		0.0f,  fy1,					// 0.0f,  0.0f ???
		fx1,   fy1
	};

//	glBindBuffer(GL_ARRAY_BUFFER, _irisVBO);
//	glBufferSubData(GL_ARRAY_BUFFER, 0, 20 * sizeof(float), points);
	memcpy(_irisVBO, points, 20 * sizeof(float));																					// DEFINITE?

//	glDisable(GL_DEPTH_TEST);
//	glDepthMask(GL_FALSE);
	N3D_DepthTestEnabled(false);																									// DEFINITE?
	N3D_DepthMask(false);																											// DEFINITE?
	N3D_BlendEnabled(false);																										// from "gfx_opengl.cpp"

//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 10);
	N3D_C3D_FrameBegin(0);																											// DEFINITE? - ADDED
	N3D_C3D_FrameDrawOn(_gameScreenTarget);																							// DEFINITE? - ADDED
	N3D_C3D_SetTexEnv(0, &envNormal);																								// DEFINITE? - ADDED
	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 0, 10);																					// DEFINITE?
	N3D_C3D_FrameSplit(0);																											// DEFINITE? - ADDED
	_gameScreenDirty = true;																										// DEFINITE? - ADDED

//	glEnable(GL_DEPTH_TEST);
//	glDepthMask(GL_TRUE);
	N3D_DepthTestEnabled(true);																										// DEFINITE?
	N3D_DepthMask(true);																											// DEFINITE?
}

void GfxN3DS::drawEmergString(int x, int y, const char *text, const Color &fgColor) {
	if (!*text)
		return;

//	glEnable(GL_BLEND);
//	glDisable(GL_DEPTH_TEST);
//	glBindTexture(GL_TEXTURE_2D, _emergTexture);
//	_emergProgram->use();
	N3D_BlendEnabled(true);																											// DEFINITE?
	N3D_DepthTestEnabled(false);																									// DEFINITE?
	debug("GfxN3DS::drawEmergString");
	N3D_C3D_TexBind(0, &_emergTexture);																					// DEFINITE?
	N3DCONTEXT_FROM_HANDLE(_grimContext)->changeShader(_programEmerg);																// DEFINITE?
	Math::Vector3d colors(float(fgColor.getRed()) / 255.0f,
	                      float(fgColor.getGreen()) / 255.0f,
	                      float(fgColor.getBlue()) / 255.0f);
//	_emergProgram->setUniform("color", colors);
//	_emergProgram->setUniform("sizeWH", Math::Vector2d(float(8) / _gameWidth, float(16) / _gameHeight));
//	_emergProgram->setUniform("texScale", Math::Vector2d(float(8) / 128, float(16) / 128));
	_programEmerg->setUniform("color", GPU_VERTEX_SHADER, colors);																	// DEFINITE?
	_programEmerg->setUniform("sizeWHtexScale", GPU_VERTEX_SHADER,																	// DEFINITE?
	                          Math::Vector4d(float(8) / _gameWidth,																	// DEFINITE?
	                                         float(16) / _gameHeight,																// DEFINITE?
	                                         float(8) / 128,																		// DEFINITE?
	                                         float(16) / 128));																		// DEFINITE?

	N3D_C3D_FrameBegin(0);																											// DEFINITE? - ADDED
	N3D_C3D_FrameDrawOn(_gameScreenTarget);																							// DEFINITE? - ADDED
	N3D_C3D_SetTexEnv(0, &envEmerg);																								// DEFINITE? - ADDED
	for (; *text; ++text, x+=10) {
		int blockcol = *text & 0xf;
		int blockrow = *text / 16;
//		_emergProgram->setUniform("offsetXY", Math::Vector2d(float(x) / _gameWidth, float(y) / _gameHeight));
//		_emergProgram->setUniform("texOffsetXY", Math::Vector2d(float(blockcol * 8) / 128, float(blockrow * 16) / 128));
		_programEmerg->setUniform("offsetXYtexOffsetXY", GPU_VERTEX_SHADER,											// DEFINITE?
		                          Math::Vector4d(float(x) / _gameWidth,																// DEFINITE?
		                                         float(y) / _gameHeight,															// DEFINITE?
		                                         float(blockcol * 8) / 128,															// DEFINITE?
		                                         float(blockrow * 16) / 128));														// DEFINITE?
//		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		N3D_C3D_DrawArrays(GPU_TRIANGLE_FAN, 0, 4);																					// DEFINITE?
	}
	N3D_C3D_FrameSplit(0);																											// DEFINITE? - ADDED
	_gameScreenDirty = true;																										// DEFINITE? - ADDED
}

void GfxN3DS::loadEmergFont() {
	uint8 *atlas = new uint8[128 * 128]();
	uint8 *atlasSwizzle = new uint8[128 * 128]();																					// DEFINITE? - ADDED

	for (int c = 32; c < 128; ++c) {
		int blockrow = c / 16;
		int blockcol = c & 0xf;
		for (int row = 0; row < 13; ++row) {
			int base = 128 * (16 * blockrow + row) + 8 * blockcol;
			uint8 val = BitmapFont::emerFont[c - 32][row];
			atlas[base + 0] = (val & 0x80) ? 255 : 0;
			atlas[base + 1] = (val & 0x40) ? 255 : 0;
			atlas[base + 2] = (val & 0x20) ? 255 : 0;
			atlas[base + 3] = (val & 0x10) ? 255 : 0;
			atlas[base + 4] = (val & 0x08) ? 255 : 0;
			atlas[base + 5] = (val & 0x04) ? 255 : 0;
			atlas[base + 6] = (val & 0x02) ? 255 : 0;
			atlas[base + 7] = (val & 0x01) ? 255 : 0;
		}
	}

//	glGenTextures(1, &_emergTexture);
//	glBindTexture(GL_TEXTURE_2D, _emergTexture);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 128, 128, 0, GL_ALPHA, GL_UNSIGNED_BYTE, atlas);
	debug("free linear space: %lu bytes", linearSpaceFree());
	debug("free VRAM space: %lu bytes", vramSpaceFree());
	debug("GfxN3DS::loadEmergFont - Creating tex (_emergTexture)");
	N3D_C3D_TexInit(&_emergTexture, 128, 128, GPU_A8);																				// DEFINITE?
	debug("GfxN3DS::loadEmergFont - Tex created: %u bytes (_emergTexture)", _emergTexture.size);
	N3D_C3D_TexSetFilter(&_emergTexture, GPU_LINEAR, GPU_LINEAR);																	// DEFINITE?
	N3D_C3D_TexSetWrap(&_emergTexture, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);														// DEFINITE?
	N3D_DataToBlockTex((u32 *)atlas, (u32 *)atlasSwizzle, 0, 0, 128, 128, 128, 128, GPU_A8, 0, false);								// DEFINITE?
	N3D_C3D_SyncTextureCopy(																										// DEFINITE?
		(u32 *)atlasSwizzle,        GX_BUFFER_DIM((128 * 8) >> 4, 0),																// DEFINITE?
		(u32 *)_emergTexture.data, GX_BUFFER_DIM((128 * 8) >> 4, 0),																// DEFINITE?
		128 * 128, GX_TRANSFER_RAW_COPY(1)																							// DEFINITE?
	);																																// DEFINITE?

	delete[] atlasSwizzle;																											// DEFINITE? - ADDED
	delete[] atlas;
}

void GfxN3DS::drawGenericPrimitive(const float *vertices, uint32 numVertices, const PrimitiveObject *primitive) {
	N3D_C3D_FrameEnd(0);																											// DEFINITE? - ADDED

	const Color color(primitive->getColor());
	const Math::Vector3d colorV =
	  Math::Vector3d(color.getRed(), color.getGreen(), color.getBlue()) / 255.f;

//	GLuint prim = nextPrimitive();
//	glBindBuffer(GL_ARRAY_BUFFER, prim);
//	glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices * sizeof(float), vertices);
	void *prim = nextPrimitive();																									// DEFINITE?
	memcpy(prim, vertices, numVertices * sizeof(float));																			// DEFINITE?

//	glDisable(GL_DEPTH_TEST);
//	glDepthMask(GL_FALSE);
	N3D_DepthTestEnabled(false);																									// DEFINITE?
	N3D_DepthMask(false);																											// DEFINITE?

//	_primitiveProgram->enableVertexAttribute("position", prim, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
//	_primitiveProgram->use(true);
//	_primitiveProgram->setUniform("color", colorV);
//	_primitiveProgram->setUniform("scaleWH", Math::Vector2d(1.f / _gameWidth, 1.f / _gameHeight));

#define PRIMITIVE_LENGTH 4																											// FINALIZED - ADDED
#define SET_PRIMITIVE_UNIFORMS(primProg) \
	primProg->setUniform("color", GPU_VERTEX_SHADER, colorV); \
	primProg->setUniform("scaleWH", GPU_VERTEX_SHADER, Math::Vector2d(1.f / _gameWidth, 1.f / _gameHeight))							// FINALIZED - ADDED

	N3D_C3D_FrameBegin(0);																											// DEFINITE? - ADDED
	N3D_C3D_FrameDrawOn(_gameScreenTarget);																							// DEFINITE? - ADDED
	N3D_C3D_SetTexEnv(0, &envNormal);																								// DEFINITE? - ADDED
	switch (primitive->getType()) {
		case PrimitiveObject::RectangleType:
//			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			SET_PRIMITIVE_UNIFORMS(_programPrimRect);																				// FINALIZED - ADDED
			N3DCONTEXT_FROM_HANDLE(_grimContext)->changeShader(_programPrimRect);													// DEFINITE?
			N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, (_lastPrimitive * PRIMITIVE_LENGTH), 4);											// DEFINITE?
			break;
		case PrimitiveObject::LineType:
//			glDrawArrays(GL_LINES, 0, 2);
			SET_PRIMITIVE_UNIFORMS(_programPrimLines);																				// FINALIZED - ADDED
			N3DCONTEXT_FROM_HANDLE(_grimContext)->changeShader(_programPrimLines);													// DEFINITE?
			N3D_C3D_DrawArrays(GPU_GEOMETRY_PRIM, (_lastPrimitive * PRIMITIVE_LENGTH), 2);											// DEFINITE?
			break;
		case PrimitiveObject::PolygonType:
//			glDrawArrays(GL_LINES, 0, 4);
			SET_PRIMITIVE_UNIFORMS(_programPrimLines);																				// FINALIZED - ADDED
			N3DCONTEXT_FROM_HANDLE(_grimContext)->changeShader(_programPrimLines);													// DEFINITE?
			N3D_C3D_DrawArrays(GPU_GEOMETRY_PRIM, (_lastPrimitive * PRIMITIVE_LENGTH), 4);											// DEFINITE?
			break;
		default:
			// Impossible
			break;
	}
	N3D_C3D_FrameEnd(0);																											// DEFINITE? - ADDED

#undef PRIMITIVE_LENGTH																												// FINALIZED - ADDED
#undef SET_PRIMITIVE_UNIFORMS																										// FINALIZED - ADDED

//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	glDepthMask(GL_TRUE);
//	glEnable(GL_DEPTH_TEST);
	// BufInfo_Init(C3D_GetBufInfo());																						// Citro3D
	N3D_DepthMask(true);																											// DEFINITE?
	N3D_DepthTestEnabled(true);																										// DEFINITE?
}

void GfxN3DS::drawRectangle(const PrimitiveObject *primitive) {
	float x1 = primitive->getP1().x * _scaleW;
	float y1 = primitive->getP1().y * _scaleH;
	float x2 = primitive->getP2().x * _scaleW;
	float y2 = primitive->getP2().y * _scaleH;

	if (primitive->isFilled()) {
		float data[] = { x1, y1, x2 + 1, y1, x1, y2 + 1, x2 + 1, y2 + 1 };
		drawGenericPrimitive(data, 8, primitive);
	} else {
		float top[] =    { x1,     y1,
		                   x2 + 1, y1,
		                   x1,     y1 + 1,
		                   x2 + 1, y1 + 1 };
		float right[] =  { x2,     y1,
		                   x2 + 1, y1,
		                   x2,     y2 + 1,
		                   x2 + 1, y2 + 1 };
		float bottom[] = { x1,     y2,
		                   x2 + 1, y2,
		                   x1,     y2 + 1,
		                   x2 + 1, y2 + 1 };
		float left[] =   { x1,     y1,
		                   x1 + 1, y1,
		                   x1,     y2 + 1,
		                   x1 + 1, y2 + 1 };
		drawGenericPrimitive(top, 8, primitive);
		drawGenericPrimitive(right, 8, primitive);
		drawGenericPrimitive(bottom, 8, primitive);
		drawGenericPrimitive(left, 8, primitive);
	}
}

void GfxN3DS::drawLine(const PrimitiveObject *primitive) {
	float x1 = primitive->getP1().x * _scaleW;
	float y1 = primitive->getP1().y * _scaleH;
	float x2 = primitive->getP2().x * _scaleW;
	float y2 = primitive->getP2().y * _scaleH;

	float data[] = { x1, y1, x2, y2 };

	drawGenericPrimitive(data, 4, primitive);
}

void GfxN3DS::drawPolygon(const PrimitiveObject *primitive) {
	float x1 = primitive->getP1().x * _scaleW;
	float y1 = primitive->getP1().y * _scaleH;
	float x2 = primitive->getP2().x * _scaleW;
	float y2 = primitive->getP2().y * _scaleH;
	float x3 = primitive->getP3().x * _scaleW;
	float y3 = primitive->getP3().y * _scaleH;
	float x4 = primitive->getP4().x * _scaleW;
	float y4 = primitive->getP4().y * _scaleH;

	const float data[] = { x1,     y1,
	                       x2 + 1, y2 + 1,
	                       x3,     y3 + 1,
	                       x4 + 1, y4 };

	drawGenericPrimitive(data, 8, primitive);
}

const Graphics::PixelFormat GfxN3DS::getMovieFormat() const {
// #ifdef SCUMM_BIG_ENDIAN
	return Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
// #else
//	return Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
// #endif
}

void GfxN3DS::prepareMovieFrame(Graphics::Surface* frame) {
	int width = frame->w;
	int height = frame->h;
	const byte *bitmap = (const byte *)frame->getPixels();

//	GLenum frameType, frameFormat;
	GPU_TEXCOLOR frameFormat;																										// DEFINITE?

	// Used by Bink, QuickTime, MPEG, Theora and paletted SMUSH
	if (frame->format == getMovieFormat()) {
//		frameType = GL_UNSIGNED_BYTE;
//		frameFormat = GL_RGBA;
		frameFormat = GPU_RGBA8;																									// DEFINITE?
	// Used by 16-bit SMUSH
	} else if (frame->format == Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0)) {
//		frameType = GL_UNSIGNED_SHORT_5_6_5;
//		frameFormat = GL_RGB;
		frameFormat = GPU_RGB565;																									// DEFINITE?
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

	// create texture
//	if (_smushTexId == 0) {
//		glGenTextures(1, &_smushTexId);
	if (_smushTex.size == 0) {																										// DEFINITE?
		debug("GfxN3DS::prepareMovieFrame - Creating tex (_smushTex)");
		N3D_C3D_TexInit(&_smushTex, (u16)nextHigher2(width), (u16)nextHigher2(height), frameFormat);								// DEFINITE?
		debug("GfxN3DS::prepareMovieFrame - Tex created: %u bytes (_smushTex)", _smushTex.size);
	}
//	glBindTexture(GL_TEXTURE_2D, _smushTexId);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexImage2D(GL_TEXTURE_2D, 0, frameFormat, nextHigher2(width), nextHigher2(height), 0, frameFormat, frameType, nullptr);
	N3D_C3D_TexSetFilter(&_smushTex, GPU_NEAREST, GPU_NEAREST);																		// DEFINITE?
	N3D_C3D_TexSetWrap(&_smushTex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);															// DEFINITE?

//	glPixelStorei(GL_UNPACK_ALIGNMENT, frame->format.bytesPerPixel);													// !!!!!!!!!!!!
//	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, frameFormat, frameType, bitmap);								// !!!!!!!!!!!!
//	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);																				// !!!!!!!!!!!!
	N3D_DataToBlockTex((u32 *)const_cast<uint8 *>(bitmap), (u32 *)_smushTex.data, 0, 0,												// DEFINITE?
	                   width, height, (int)_smushTex.width, (int)_smushTex.height, frameFormat, 0, false);							// DEFINITE?

	_smushWidth = (int)(width);
	_smushHeight = (int)(height);
}

void GfxN3DS::drawMovieFrame(int offsetX, int offsetY) {
//	_smushProgram->use();
//	glDisable(GL_DEPTH_TEST);
	N3DCONTEXT_FROM_HANDLE(_grimContext)->changeShader(_programSmush);																// DEFINITE?
	N3D_DepthTestEnabled(false);																									// DEFINITE?

//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quadEBO);
//	_smushProgram->setUniform("texcrop", Math::Vector2d(float(_smushWidth) / nextHigher2(_smushWidth), float(_smushHeight) / nextHigher2(_smushHeight)));
//	_smushProgram->setUniform("scale", Math::Vector2d(float(_smushWidth)/ float(_gameWidth), float(_smushHeight) / float(_gameHeight)));
//	_smushProgram->setUniform("offset", Math::Vector2d(float(offsetX) / float(_gameWidth), float(offsetY) / float(_gameHeight)));
//	glBindTexture(GL_TEXTURE_2D, _smushTexId);
	_programSmush->setUniform("texcrop", GPU_VERTEX_SHADER,																			// DEFINITE?
	                          Math::Vector2d(float(_smushWidth)  / nextHigher2(_smushWidth),										// DEFINITE?
	                                         float(_smushHeight) / nextHigher2(_smushHeight)));										// DEFINITE?
	_programSmush->setUniform("scale", GPU_VERTEX_SHADER,																			// DEFINITE?
	                          Math::Vector2d(float(_smushWidth)  / float(_gameWidth),												// DEFINITE?
	                                         float(_smushHeight) / float(_gameHeight)));											// DEFINITE?
	_programSmush->setUniform("offset", GPU_VERTEX_SHADER,																			// DEFINITE?
	                          Math::Vector2d(float(offsetX) / float(_gameWidth),													// DEFINITE?
	                                         float(offsetY) / float(_gameHeight)));													// DEFINITE?
	debug("GfxN3DS::drawMovieFrame");
	N3D_C3D_TexBind(0, &_smushTex);																						// DEFINITE?

//	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
	N3D_C3D_FrameBegin(0);																											// DEFINITE? - ADDED
	N3D_C3D_FrameDrawOn(_gameScreenTarget);																							// DEFINITE? - ADDED
	N3D_C3D_SetTexEnv(0, &envSmush);																								// DEFINITE? - ADDED
	N3D_C3D_DrawElements(GPU_TRIANGLES, 6, C3D_UNSIGNED_SHORT, (void *)_quadEBO);													// DEFINITE?
	N3D_C3D_FrameSplit(0);																											// DEFINITE? - ADDED

//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//	glEnable(GL_DEPTH_TEST);
	N3D_DepthTestEnabled(true);																										// DEFINITE?
}

void GfxN3DS::releaseMovieFrame() {
	N3D_C3D_FrameEnd(0);																											// DEFINITE? - ADDED
//	if (_smushTexId > 0) {
//		glDeleteTextures(1, &_smushTexId);
	if (_smushTex.size > 0) {																										// DEFINITE?
		debug("GfxN3DS::releaseMovieFrame - Deleting tex: -%u bytes (_smushTex)", _smushTex.size);
		N3D_C3D_TexDelete(&_smushTex);																								// DEFINITE?
		debug("GfxN3DS::releaseMovieFrame - Tex deleted (_smushTex)");
//		_smushTexId = 0;
	}
}


const char *GfxN3DS::getVideoDeviceName() {
//	return "OpenGLS Renderer";
	return "Nintendo 3DS 3D Renderer";																								// DEFINITE?
}

void GfxN3DS::renderBitmaps(bool render) {
}

void GfxN3DS::renderZBitmaps(bool render) {
}

void GfxN3DS::createEMIModel(EMIModel *model) {
	EMIModelUserData *mud = new EMIModelUserData;
	model->_userData = mud;
//	mud->_verticesVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, model->_numVertices * 3 * sizeof(float), model->_vertices, GL_STREAM_DRAW);

//	mud->_normalsVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, model->_numVertices * 3 * sizeof(float), model->_normals, GL_STREAM_DRAW);

//	mud->_texCoordsVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, model->_numVertices * 2 * sizeof(float), model->_texVerts, GL_STATIC_DRAW);

//	mud->_colorMapVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, model->_numVertices * 4 * sizeof(byte), model->_colorMap, GL_STATIC_DRAW);

	debug("GfxN3DS::createEMIModel - Creating linear alloc (mud->_verticesVBO)");
	mud->_verticesVBO  = N3DS_3D::createBuffer(model->_numVertices * 3 * sizeof(float), model->_vertices);							// DEFINITE?
	debug("GfxN3DS::createEMIModel - Linear alloc created: %u bytes (mud->_verticesVBO)", linearGetSize(mud->_verticesVBO ));
	debug("GfxN3DS::createEMIModel - Creating linear alloc (mud->_normalsVBO)");
	mud->_normalsVBO   = N3DS_3D::createBuffer(model->_numVertices * 3 * sizeof(float), model->_normals);							// DEFINITE?
	debug("GfxN3DS::createEMIModel - Linear alloc created: %u bytes (mud->_normalsVBO)", linearGetSize(mud->_normalsVBO  ));
	debug("GfxN3DS::createEMIModel - Creating linear alloc (mud->_texCoordsVBO)");
	mud->_texCoordsVBO = N3DS_3D::createBuffer(model->_numVertices * 2 * sizeof(float), model->_texVerts);							// DEFINITE?
	debug("GfxN3DS::createEMIModel - Linear alloc created: %u bytes (mud->_texCoordsVBO)", linearGetSize(mud->_texCoordsVBO));
	debug("GfxN3DS::createEMIModel - Creating linear alloc (mud->_colorMapVBO)");
	mud->_colorMapVBO  = N3DS_3D::createBuffer(model->_numVertices * 4 * sizeof(byte), model->_colorMap);							// DEFINITE?	// needs to be pre-normalized?
	debug("GfxN3DS::createEMIModel - Linear alloc created: %u bytes (mud->_colorMapVBO)", linearGetSize(mud->_colorMapVBO ));

//	OpenGL::Shader *actorShader = _actorProgram->clone();
//	actorShader->enableVertexAttribute("position", mud->_verticesVBO, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
//	actorShader->enableVertexAttribute("normal", mud->_normalsVBO, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
//	actorShader->enableVertexAttribute("texcoord", mud->_texCoordsVBO, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
//	actorShader->enableVertexAttribute("color", mud->_colorMapVBO, 4, GL_UNSIGNED_BYTE, GL_TRUE, 4 * sizeof(byte), 0);
	N3DS_3D::ShaderObj *actorShader = new N3DS_3D::ShaderObj(_programActor);														// DEFINITE?
	actorShader->addAttrLoader(0, GPU_FLOAT, 3);							// v0 = position										// DEFINITE?
	actorShader->addAttrLoader(1, GPU_FLOAT, 3);							// v1 = normal											// DEFINITE?
	actorShader->addAttrLoader(2, GPU_FLOAT, 2);							// v2 = texcoord										// DEFINITE?
	actorShader->addAttrLoader(3, GPU_UNSIGNED_BYTE, 4);					// v3 = color											// DEFINITE?
	actorShader->addBufInfo(mud->_verticesVBO,  3 * sizeof(float), 1, 0x0);	// separate buffers										// DEFINITE?
	actorShader->addBufInfo(mud->_normalsVBO,   3 * sizeof(float), 1, 0x1);	// separate buffers										// DEFINITE?
	actorShader->addBufInfo(mud->_texCoordsVBO, 2 * sizeof(float), 1, 0x2);	// separate buffers										// DEFINITE?
	actorShader->addBufInfo(mud->_colorMapVBO,  4 * sizeof(byte),  1, 0x3);	// separate buffers										// DEFINITE?
	mud->_shader = actorShader;

//	actorShader = _actorLightsProgram->clone();
//	actorShader->enableVertexAttribute("position", mud->_verticesVBO, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
//	actorShader->enableVertexAttribute("normal", mud->_normalsVBO, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
//	actorShader->enableVertexAttribute("texcoord", mud->_texCoordsVBO, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
//	actorShader->enableVertexAttribute("color", mud->_colorMapVBO, 4, GL_UNSIGNED_BYTE, GL_TRUE, 4 * sizeof(byte), 0);
	actorShader = new N3DS_3D::ShaderObj(_programActorLights);																		// DEFINITE?
	actorShader->addAttrLoader(0, GPU_FLOAT, 3);							// v0 = position										// DEFINITE?
	actorShader->addAttrLoader(1, GPU_FLOAT, 3);							// v1 = normal											// DEFINITE?
	actorShader->addAttrLoader(2, GPU_FLOAT, 2);							// v2 = texcoord										// DEFINITE?
	actorShader->addAttrLoader(3, GPU_UNSIGNED_BYTE, 4);					// v3 = color											// DEFINITE?
	actorShader->addBufInfo(mud->_verticesVBO,  3 * sizeof(float), 1, 0x0);	// separate buffers										// DEFINITE?
	actorShader->addBufInfo(mud->_normalsVBO,   3 * sizeof(float), 1, 0x1);	// separate buffers										// DEFINITE?
	actorShader->addBufInfo(mud->_texCoordsVBO, 2 * sizeof(float), 1, 0x2);	// separate buffers										// DEFINITE?
	actorShader->addBufInfo(mud->_colorMapVBO,  4 * sizeof(byte),  1, 0x3);	// separate buffers										// DEFINITE?
	mud->_shaderLights = actorShader;

	for (uint32 i = 0; i < model->_numFaces; ++i) {
		EMIMeshFace * face = &model->_faces[i];
//		face->_indicesEBO = OpenGL::Shader::createBuffer(GL_ELEMENT_ARRAY_BUFFER, face->_faceLength * 3 * sizeof(uint16), face->_indexes, GL_STATIC_DRAW);
		debug("GfxN3DS::createEMIModel - Creating linear alloc (face->_indicesEBO)");
		face->_indicesEBO = N3DS_3D::createBuffer(face->_faceLength * 3 * sizeof(uint16), face->_indexes);							// DEFINITE?
		debug("GfxN3DS::createEMIModel - Linear alloc created: %u bytes (face->_indicesEBO)", linearGetSize(face->_indicesEBO));
	}

//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GfxN3DS::destroyEMIModel(EMIModel *model) {
	for (uint32 i = 0; i < model->_numFaces; ++i) {
		EMIMeshFace *face = &model->_faces[i];
//		OpenGL::Shader::freeBuffer(face->_indicesEBO);
//		face->_indicesEBO = 0;
		debug("GfxN3DS::destroyEMIModel - Deleting linear alloc: -%u bytes (face->_indicesEBO)", linearGetSize(face->_indicesEBO));
		N3DS_3D::freeBuffer(face->_indicesEBO);																						// DEFINITE?
		debug("GfxN3DS::destroyEMIModel - Linear alloc deleted (face->_indicesEBO).");
		face->_indicesEBO = nullptr;																								// DEFINITE?
	}

	EMIModelUserData *mud = static_cast<EMIModelUserData *>(model->_userData);

	if (mud) {
//		OpenGL::Shader::freeBuffer(mud->_verticesVBO);
//		OpenGL::Shader::freeBuffer(mud->_normalsVBO);
//		OpenGL::Shader::freeBuffer(mud->_texCoordsVBO);
//		OpenGL::Shader::freeBuffer(mud->_colorMapVBO);
		debug("GfxN3DS::destroyEMIModel - Deleting linear alloc: -%u bytes (mud->_verticesVBO)", linearGetSize(mud->_verticesVBO));
		N3DS_3D::freeBuffer(mud->_verticesVBO);																						// DEFINITE?
		debug("GfxN3DS::destroyEMIModel - Linear alloc deleted (mud->_verticesVBO).");
		debug("GfxN3DS::destroyEMIModel - Deleting linear alloc: -%u bytes (mud->_normalsVBO)", linearGetSize(mud->_normalsVBO));
		N3DS_3D::freeBuffer(mud->_normalsVBO);																						// DEFINITE?
		debug("GfxN3DS::destroyEMIModel - Linear alloc deleted (mud->_normalsVBO).");
		debug("GfxN3DS::destroyEMIModel - Deleting linear alloc: -%u bytes (mud->_texCoordsVBO)", linearGetSize(mud->_texCoordsVBO));
		N3DS_3D::freeBuffer(mud->_texCoordsVBO);																					// DEFINITE?
		debug("GfxN3DS::destroyEMIModel - Linear alloc deleted (mud->_texCoordsVBO).");
		debug("GfxN3DS::destroyEMIModel - Deleting linear alloc: -%u bytes (mud->_colorMapVBO)", linearGetSize(mud->_colorMapVBO));
		N3DS_3D::freeBuffer(mud->_colorMapVBO);																						// DEFINITE?
		debug("GfxN3DS::destroyEMIModel - Linear alloc deleted (mud->_colorMapVBO).");
		mud->_verticesVBO = nullptr;																								// DEFINITE?
		mud->_normalsVBO = nullptr;																									// DEFINITE?
		mud->_texCoordsVBO = nullptr;																								// DEFINITE?
		mud->_colorMapVBO = nullptr;																								// DEFINITE?

		delete mud->_shader;
		delete mud;
	}

	model->_userData = nullptr;
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

//	mud->_meshInfoVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, meshInfo.size() * sizeof(GrimVertex), &meshInfo[0], GL_STATIC_DRAW);
	debug("GfxN3DS::createMesh - Creating linear alloc (mud->_meshInfoVBO)");
	mud->_meshInfoVBO = N3DS_3D::createBuffer(meshInfo.size() * sizeof(GrimVertex), &meshInfo[0]);									// DEFINITE?
	debug("GfxN3DS::createMesh - Linear alloc created: %u bytes (mud->_meshInfoVBO)", linearGetSize(mud->_meshInfoVBO));

//	OpenGL::Shader *actorShader = _actorProgram->clone();
//	actorShader->enableVertexAttribute("position", mud->_meshInfoVBO, 3, GL_FLOAT, GL_FALSE, sizeof(GrimVertex), 0);
//	actorShader->enableVertexAttribute("texcoord", mud->_meshInfoVBO, 2, GL_FLOAT, GL_FALSE, sizeof(GrimVertex), 3 * sizeof(float));
//	actorShader->enableVertexAttribute("normal", mud->_meshInfoVBO, 3, GL_FLOAT, GL_FALSE, sizeof(GrimVertex), 5 * sizeof(float));
//	actorShader->disableVertexAttribute("color", Math::Vector4d(1.f, 1.f, 1.f, 1.f));
	N3DS_3D::ShaderObj *actorShader = new N3DS_3D::ShaderObj(_programActor);														// DEFINITE?
	actorShader->addAttrLoader(0, GPU_FLOAT, 3);								// v0 = position									// DEFINITE?
	actorShader->addAttrLoader(1, GPU_FLOAT, 2);								// v1 = texcoord									// DEFINITE?
	actorShader->addAttrLoader(2, GPU_FLOAT, 3);								// v2 = normal										// DEFINITE?
	actorShader->addBufInfo(mud->_meshInfoVBO, 8 * sizeof(float), 3, 0x210);	//													// DEFINITE?
	mud->_shader = actorShader;

//	actorShader = _actorLightsProgram->clone();
//	actorShader->enableVertexAttribute("position", mud->_meshInfoVBO, 3, GL_FLOAT, GL_FALSE, sizeof(GrimVertex), 0);
//	actorShader->enableVertexAttribute("texcoord", mud->_meshInfoVBO, 2, GL_FLOAT, GL_FALSE, sizeof(GrimVertex), 3 * sizeof(float));
//	actorShader->enableVertexAttribute("normal", mud->_meshInfoVBO, 3, GL_FLOAT, GL_FALSE, sizeof(GrimVertex), 5 * sizeof(float));
//	actorShader->disableVertexAttribute("color", Math::Vector4d(1.f, 1.f, 1.f, 1.f));
	actorShader = new N3DS_3D::ShaderObj(_programActorLights);																		// DEFINITE?
	actorShader->addAttrLoader(0, GPU_FLOAT, 3);								// v0 = position									// DEFINITE?
	actorShader->addAttrLoader(1, GPU_FLOAT, 2);								// v1 = texcoord									// DEFINITE?
	actorShader->addAttrLoader(2, GPU_FLOAT, 3);								// v2 = normal										// DEFINITE?
	actorShader->addBufInfo(mud->_meshInfoVBO, 8 * sizeof(float), 3, 0x210);	//													// DEFINITE?
	mud->_shaderLights = actorShader;
}

void GfxN3DS::destroyMesh(const Mesh *mesh) {
	ModelUserData *mud = static_cast<ModelUserData *>(mesh->_userData);

	debug("GfxN3DS::destroyMesh - Deleting linear alloc (mud->_meshInfoVBO): -%u bytes", linearGetSize(mud->_meshInfoVBO));
	N3DS_3D::freeBuffer(mud->_meshInfoVBO);																							// DEFINITE? - ADDED
	debug("GfxN3DS::destroyMesh - Linear alloc deleted (mud->_meshInfoVBO).");

	for (int i = 0; i < mesh->_numFaces; ++i) {
		MeshFace *face = &mesh->_faces[i];
		if (face->_userData) {
			uint32 *data = static_cast<uint32 *>(face->_userData);
			delete data;
		}
	}

	if (!mud)
		return;

	delete mud->_shader;
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

#define SUBTEXTURE_WIDTH 256																										// DEFINITE? - ADDED
#define SUBTEX_FROM_SCREEN(_id, _srcbuf, _dstbuf, _x, _y, _w, _h) \
	for (i = _y; i < _y + _h; i++) { \
		memcpy((void *)(_dstbuf + (i * SUBTEXTURE_WIDTH * 4)), \
		       (void *)(_srcbuf + ((i * _screenWidth + _x) * 4)), \
		       _w * 4); \
	} \
	createSpecialtyTexture(_id, _dstbuf, _w, _h)																					// DEFINITE? - ADDED

void GfxN3DS::makeScreenTextures() {																								// DEFINITE? - ADDED
	// Copy screen to a buffer																										// DEFINITE? - ADDED
	N3D_C3D_SyncDisplayTransfer((u32 *)_gameScreenTarget->frameBuf.colorBuf, GX_BUFFER_DIM(1024, 512),								// DEFINITE? - ADDED
	                            (u32 *)_screenCopySpace,                     GX_BUFFER_DIM( 640, 480), 5);							// DEFINITE? - ADDED

	// Make a buffer big enough to hold any of the subTextures																		// DEFINITE? - ADDED
	uint8 *buffer = new uint8[256 * 256 * 4];																						// DEFINITE? - ADDED
																																	// DEFINITE? - ADDED
	int i;																															// DEFINITE? - ADDED
	// TODO: Handle screen resolutions other than 640 x 480																			// DEFINITE? - ADDED
	SUBTEX_FROM_SCREEN(0, (uint8 *)_screenCopySpace, buffer,   0,   0, 256, 256);													// DEFINITE? - ADDED
	SUBTEX_FROM_SCREEN(1, (uint8 *)_screenCopySpace, buffer, 256,   0, 256, 256);													// DEFINITE? - ADDED
	SUBTEX_FROM_SCREEN(2, (uint8 *)_screenCopySpace, buffer, 512,   0, 128, 128);													// DEFINITE? - ADDED
	SUBTEX_FROM_SCREEN(3, (uint8 *)_screenCopySpace, buffer, 512, 128, 128, 128);													// DEFINITE? - ADDED
	SUBTEX_FROM_SCREEN(4, (uint8 *)_screenCopySpace, buffer,   0, 256, 256, 256);													// DEFINITE? - ADDED
	SUBTEX_FROM_SCREEN(5, (uint8 *)_screenCopySpace, buffer, 256, 256, 256, 256);													// DEFINITE? - ADDED
	SUBTEX_FROM_SCREEN(6, (uint8 *)_screenCopySpace, buffer, 512, 256, 128, 128);													// DEFINITE? - ADDED
	SUBTEX_FROM_SCREEN(7, (uint8 *)_screenCopySpace, buffer, 512, 384, 128, 128);													// DEFINITE? - ADDED

	delete[] buffer;																												// DEFINITE? - ADDED
}																																	// DEFINITE? - ADDED

#undef SUBTEX_FROM_SCREEN																											// DEFINITE? - ADDED
#undef SUBTEXTURE_WIDTH																												// DEFINITE? - ADDED

Bitmap *GfxN3DS::getScreenshot(int w, int h, bool useStored) {
	Graphics::Surface src;
// #ifdef SCUMM_BIG_ENDIAN
//	src.create(_screenWidth, _screenHeight, Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
	src.w = _screenWidth;																											// DEFINITE? - REIMPLEMENTED
	src.h = _screenHeight;																											// DEFINITE? - REIMPLEMENTED
	src.format = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);																// DEFINITE? - REIMPLEMENTED
	src.pitch = src.w * src.format.bytesPerPixel;																					// DEFINITE? - REIMPLEMENTED
	// void *srcLinMem = linearAlloc(_screenWidth * _screenHeight * 4);																// DEFINITE? - REIMPLEMENTED
	src.setPixels(_screenCopySpace);																								// DEFINITE? - REIMPLEMENTED
// #else
//	src.create(_screenWidth, _screenHeight, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
// #endif
	Bitmap *bmp;

	if (useStored) {
//		if (OpenGLContext.type == OpenGL::kContextGLES2) {
//			GLuint frameBuffer;
//			glGenFramebuffers(1, &frameBuffer);
//			glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
//			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _storedDisplay, 0);
//
//			readPixels(0, 0, _screenWidth, _screenHeight, (uint8 *)src.getPixels());
//
//			glBindFramebuffer(GL_FRAMEBUFFER, 0);
//			glDeleteFramebuffers(1, &frameBuffer);
//#if !USE_FORCED_GLES2
//		} else {
//			glBindTexture(GL_TEXTURE_2D, _storedDisplay);
//			char *buffer = new char[_screenWidth * _screenHeight * 4];
//
//			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
//			byte *rawBuf = (byte *)src.getPixels();
//			for (int i = 0; i < _screenHeight; i++) {
//				memcpy(&(rawBuf[(_screenHeight - i - 1) * _screenWidth * 4]), &buffer[4 * _screenWidth * i], _screenWidth * 4);
//			}
//			delete[] buffer;
//#endif // !USE_FORCED_GLES2
//		}
		// Cropping bit is at x << 2																								// DEFINITE? - REIMPLEMENTED
		// GX_TRANSFER_FLIP_VERT(1) | (1 << 2) = (1 << 0) | (1 << 2) = 0b0001 | 0b0100 = 0b0101 = 5									// DEFINITE? - REIMPLEMENTED
		N3D_C3D_SyncDisplayTransfer((u32 *)_storedDisplay.data, GX_BUFFER_DIM(1024, 512),											// DEFINITE? - REIMPLEMENTED
		                            (u32 *)_screenCopySpace,    GX_BUFFER_DIM( 640, 480), 5);										// DEFINITE? - REIMPLEMENTED
	} else {
//		readPixels(0, 0, _screenWidth, _screenHeight, (uint8 *)src.getPixels());
		// Cropping bit is at x << 2																								// DEFINITE? - REIMPLEMENTED
		// GX_TRANSFER_FLIP_VERT(1) | (1 << 2) = (1 << 0) | (1 << 2) = 0b0001 | 0b0100 = 0b0101 = 5									// DEFINITE? - REIMPLEMENTED
		N3D_C3D_SyncDisplayTransfer((u32 *)_gameScreenTarget->frameBuf.colorBuf, GX_BUFFER_DIM(1024, 512),							// DEFINITE? - REIMPLEMENTED
		                            (u32 *)_screenCopySpace,                     GX_BUFFER_DIM( 640, 480), 5);						// DEFINITE? - REIMPLEMENTED
	}
	bmp = createScreenshotBitmap(&src, w, h, true);
//	src.free();
	src.setPixels(0);																												// DEFINITE? - REIMPLEMENTED
	src.w = src.h = src.pitch = 0;																									// DEFINITE? - REIMPLEMENTED
	src.format = Graphics::PixelFormat();																							// DEFINITE? - REIMPLEMENTED
	return bmp;
}

//void GfxOpenGLS::createSpecialtyTextureFromScreen(uint id, uint8 *data, int x, int y, int width, int height) {
//	readPixels(x, y, width, height, data);
//	createSpecialtyTexture(id, data, width, height);
//}
void GfxN3DS::createSpecialtyTextureFromScreen(uint id, uint8 *data, int x, int y, int width, int height) {
}

void GfxN3DS::setBlendMode(bool additive) {
	if (additive) {
//		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		N3D_BlendFunc(GPU_SRC_ALPHA, GPU_ONE);																						// DEFINITE?
	} else {
//		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		N3D_BlendFunc(GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);																		// DEFINITE?
	}
}

}

#endif // __3DS__
