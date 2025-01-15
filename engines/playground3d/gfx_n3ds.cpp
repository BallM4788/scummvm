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

//#if defined(USE_OPENGL_SHADERS)
#if defined(__3DS__)
#include "graphics/3ds/n3d.h"
#include "graphics/3ds/z3d.h"

#include "common/rect.h"
#include "common/textconsole.h"

#include "graphics/surface.h"

#include "math/glmath.h"
#include "math/vector2d.h"
#include "math/rect2d.h"
#include "math/quat.h"

//#include "graphics/opengl/shader.h"

#include "engines/playground3d/gfx.h"
//#include "engines/playground3d/gfx_opengl_shaders.h"
#include "engines/playground3d/shaders-3ds/playground3d_cube_shbin.h"
#include "engines/playground3d/gfx_n3ds.h"

namespace Playground3d {

//static const GLfloat dimRegionVertices[] = {
//	//  X      Y
//	-0.5f,  0.5f,
//	 0.5f,  0.5f,
//	-0.5f, -0.5f,
//	 0.5f, -0.5f,
//};
static const Vtx dimRegionVertices[] = {
	//    X      Y
	{{-0.5f,  0.5f}},
	{{ 0.5f,  0.5f}},
	{{-0.5f, -0.5f}},
	{{ 0.5f, -0.5f}},
};

//static const GLfloat bitmapVertices[] = {
//	//  X      Y     S     T
//	-0.2f,  0.2f, 0.0f, 0.0f,
//	 0.2f,  0.2f, 1.0f, 0.0f,
//	-0.2f, -0.2f, 0.0f, 1.0f,
//	 0.2f, -0.2f, 1.0f, 1.0f,
//};
static const VtxTex bitmapVertices[] = {
	//    X      Y       S     T
	{{-0.2f,  0.2f}, {0.0f, 0.0f}},
	{{ 0.2f,  0.2f}, {1.0f, 0.0f}},
	{{-0.2f, -0.2f}, {0.0f, 1.0f}},
	{{ 0.2f, -0.2f}, {1.0f, 1.0f}},
};

Renderer *CreateGfxN3DS(OSystem *system) {
	return new N3DSRenderer(system);
}

//ShaderRenderer::ShaderRenderer(OSystem *system) :
//		Renderer(system),
//		_currentViewport(kOriginalWidth, kOriginalHeight),
//		_cubeShader(nullptr),
//		_fadeShader(nullptr),
//		_bitmapShader(nullptr),
//		_cubeVBO(0),
//		_fadeVBO(0),
//		_bitmapVBO(0) {
//}
N3DSRenderer::N3DSRenderer(OSystem *system) :
		Renderer(system),
		_currentViewport(kOriginalWidth, kOriginalHeight),
		_cubeShader(nullptr)/*,
		_fadeShader(nullptr),
		_bitmapShader(nullptr)*/ {
}

//ShaderRenderer::~ShaderRenderer() {
//	OpenGL::Shader::freeBuffer(_cubeVBO);
//	OpenGL::Shader::freeBuffer(_fadeVBO);
//	OpenGL::Shader::freeBuffer(_bitmapVBO);
//
//	delete _cubeShader;
//	delete _fadeShader;
//	delete _bitmapShader;
//}
N3DSRenderer::~N3DSRenderer() {
	N3DS_3D::freeBuffer(_cubeVBO);
	// N3DS_3D::freeBuffer(_fadeVBO);
	// N3DS_3D::freeBuffer(_bitmapVBO);

	delete _cubeShader;
	// delete _fadeShader;
	// delete _bitmapShader;
}

//void ShaderRenderer::init() {
void N3DSRenderer::init() {
//	debug("Initializing OpenGL Renderer with shaders");
	debug("Initializing Nintendo 3DS renderer");

	// Create context that corresponds to the Citro3D setting of the 3DS backend.
	_backendContext = N3DS_3D::createContext();
	// Create context that corresponds to Open GL settings to be used for Playground3D engine rendering.
	_p3dContext = N3DS_3D::createOGLContext();

	_gameScreenTex = N3D_GetGameScreen();
	_gameScreenTarget = N3D_C3D_RenderTargetCreateFromTex(_gameScreenTex, GPU_TEXFACE_2D, 0, GPU_RB_DEPTH24_STENCIL8);

	computeScreenViewport();

//	glEnable(GL_DEPTH_TEST);
	N3D_DepthTestEnabled(true);

//	static const char *cubeAttributes[] = { "position", "normal", "color", "texcoord", nullptr };
//	_cubeShader = OpenGL::Shader::fromFiles("playground3d_cube", cubeAttributes);
//	_cubeVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices);
//	_cubeShader->enableVertexAttribute("texcoord", _cubeVBO, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 0);
//	_cubeShader->enableVertexAttribute("position", _cubeVBO, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 8);
//	_cubeShader->enableVertexAttribute("normal", _cubeVBO, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 20);
//	_cubeShader->enableVertexAttribute("color", _cubeVBO, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 32);
	// SHOULDN'T BE NORMALIZED
	_cubeShader = new N3DS_3D::ShaderObj(playground3d_cube_shbin, playground3d_cube_shbin_size);
	_cubeVBO = N3DS_3D::createBuffer(sizeof(cubeVertices3DS), cubeVertices3DS);
	_cubeShader->addAttrLoader(0, GPU_FLOAT, 2);
	_cubeShader->addAttrLoader(1, GPU_FLOAT, 3);
	_cubeShader->addAttrLoader(2, GPU_FLOAT, 3);
	_cubeShader->addAttrLoader(3, GPU_FLOAT, 3);
	_cubeShader->addBufInfo(_cubeVBO, 11 * sizeof(float), 4, 0x3210);

//	static const char *fadeAttributes[] = { "position", nullptr };
//	_fadeShader = OpenGL::Shader::fromFiles("playground3d_fade", fadeAttributes);
//	_fadeVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(dimRegionVertices), dimRegionVertices);
//	_fadeShader->enableVertexAttribute("position", _fadeVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);
	// // SHOULD BE NORMALIZED
	// _fadeShader = new N3DS_3D::ShaderObj(playground3d_fade_shbin, playground3d_fade_shbin_size);
	// _fadeVBO = N3DS_3D::createBuffer(sizeof(dimRegionVertices), dimRegionVertices);
	// _fadeShader->addAttrLoader(0, GPU_FLOAT, 2);
	// _fadeShader->addBufInfo(_fadeVBO, sizeof(Vtx), 1, 0x0);

//	static const char *bitmapAttributes[] = { "position", "texcoord", nullptr };
//	_bitmapShader = OpenGL::Shader::fromFiles("playground3d_bitmap", bitmapAttributes);
//	_bitmapVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(bitmapVertices), bitmapVertices);
//	_bitmapShader->enableVertexAttribute("position", _bitmapVBO, 2, GL_FLOAT, GL_TRUE, 4 * sizeof(float), 0);
//	_bitmapShader->enableVertexAttribute("texcoord", _bitmapVBO, 2, GL_FLOAT, GL_TRUE, 4 * sizeof(float), 8);
	// // SHOULD BE NORMALIZED
	// _bitmapShader = new N3DS_3D::ShaderObj(playground3d_bitmap_shbin, playground3d_bitmap_shbin_size);
	// _bitmapVBO = N3DS_3D::createBuffer(sizeof(bitmapVertices), bitmapVertices);
	// _bitmapShader->addAttrLoader(0, GPU_FLOAT, 2);
	// _bitmapShader->addAttrLoader(1, GPU_FLOAT, 2);
	// _bitmapShader->addBufInfo(_bitmapVBO, sizeof(VtxTex), 2, 0x10);

//	glGenTextures(5, _textureRgbaId);
//	glGenTextures(5, _textureRgbId);
//	glGenTextures(2, _textureRgb565Id);
//	glGenTextures(2, _textureRgba5551Id);
//	glGenTextures(2, _textureRgba4444Id);
}

//void ShaderRenderer::deinit() {
void N3DSRenderer::deinit() {
//	glDeleteTextures(5, _textureRgbaId);
//	glDeleteTextures(5, _textureRgbId);
//	glDeleteTextures(2, _textureRgb565Id);
//	glDeleteTextures(2, _textureRgba5551Id);
//	glDeleteTextures(2, _textureRgba4444Id);
}

#define FLOAT_TO_8BIT(fnum)		((u32)(0.5f + (fnum) * (float)255))

//void ShaderRenderer::clear(const Math::Vector4d &clearColor) {
void N3DSRenderer::clear(const Math::Vector4d &clearColor) {
	u32 clearcolorint = FLOAT_TO_8BIT(clearColor.x()) << 24
	                  | FLOAT_TO_8BIT(clearColor.y()) << 16
	                  | FLOAT_TO_8BIT(clearColor.z()) << 8
	                  | FLOAT_TO_8BIT(clearColor.w());

	N3DS_3D::setContext(_p3dContext);
	N3D_C3D_FrameBegin(0);
	N3D_C3D_FrameDrawOn(_gameScreenTarget);
	N3D_C3D_SetViewport(0, 0, 640, 480);
	//N3D_C3D_RenderTargetClear(_gameScreenTarget, C3D_CLEAR_COLOR, clearcolorint, 0xFFFFFFFF);
	N3D_C3D_RenderTargetClear(_gameScreenTarget, C3D_CLEAR_ALL, 0x68B0D8FF, 0xFFFFFFFF);
//	glClearColor(clearColor.x(), clearColor.y(), clearColor.z(), clearColor.w());
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

#undef FLOAT_TO_8BIT

//void ShaderRenderer::loadTextureRGBA(Graphics::Surface *texture) {
void N3DSRenderer::loadTextureRGBA(Graphics::Surface *texture) {
//	glBindTexture(GL_TEXTURE_2D, _textureRgbaId[0]);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->getPixels());
//	glBindTexture(GL_TEXTURE_2D, _textureRgbaId[1]);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, texture->getPixels());
}

//void ShaderRenderer::loadTextureRGB(Graphics::Surface *texture) {
void N3DSRenderer::loadTextureRGB(Graphics::Surface *texture) {
//	glBindTexture(GL_TEXTURE_2D, _textureRgbId[0]);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_RGB, GL_UNSIGNED_BYTE, texture->getPixels());
}

//void ShaderRenderer::loadTextureRGB565(Graphics::Surface *texture) {
void N3DSRenderer::loadTextureRGB565(Graphics::Surface *texture) {
//	glBindTexture(GL_TEXTURE_2D, _textureRgb565Id[0]);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, texture->getPixels());
}

//void ShaderRenderer::loadTextureRGBA5551(Graphics::Surface *texture) {
void N3DSRenderer::loadTextureRGBA5551(Graphics::Surface *texture) {
//	glBindTexture(GL_TEXTURE_2D, _textureRgba5551Id[0]);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, texture->getPixels());
}

//void ShaderRenderer::loadTextureRGBA4444(Graphics::Surface *texture) {
void N3DSRenderer::loadTextureRGBA4444(Graphics::Surface *texture) {
//	glBindTexture(GL_TEXTURE_2D, _textureRgba4444Id[0]);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, texture->getPixels());
}

//void ShaderRenderer::setupViewport(int x, int y, int width, int height) {
void N3DSRenderer::setupViewport(int x, int y, int width, int height) {
//	glViewport(x, y, width, height);
	// DO VIEWPORT THINGS?
}

//void ShaderRenderer::enableFog(const Math::Vector4d &fogColor) {
void N3DSRenderer::enableFog(const Math::Vector4d &fogColor) {
}

//void ShaderRenderer::drawCube(const Math::Vector3d &pos, const Math::Vector3d &roll) {
void N3DSRenderer::drawCube(const Math::Vector3d &pos, const Math::Vector3d &roll) {
	auto rotateMatrix = (Math::Quaternion::fromEuler(roll.x(), roll.y(), roll.z(), Math::EO_XYZ)).inverse().toMatrix();
//	_cubeShader->use();
//	_cubeShader->setUniform("textured", false);
//	_cubeShader->setUniform("mvpMatrix", _mvpMatrix);
//	_cubeShader->setUniform("rotateMatrix", rotateMatrix);
//	_cubeShader->setUniform("modelPos", pos);
	N3DS_3D::getActiveContext()->changeShader(_cubeShader);
	_cubeShader->setUniform("textured", GPU_VERTEX_SHADER, 0.0f);
	_cubeShader->setUniform("mvpMatrix", GPU_VERTEX_SHADER, _mvpMatrix);
	_cubeShader->setUniform("rotateMatrix", GPU_VERTEX_SHADER, rotateMatrix);
	_cubeShader->setUniform("modelPos", GPU_VERTEX_SHADER, pos);

//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//	glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
//	glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
//	glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);
//	glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);
//	glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);
	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 0, 4);
	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 4, 4);
	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 8, 4);
	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 12, 4);
	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 16, 4);
	N3D_C3D_DrawArrays(GPU_TRIANGLE_STRIP, 20, 4);

}

//void ShaderRenderer::drawPolyOffsetTest(const Math::Vector3d &pos, const Math::Vector3d &roll) {
void N3DSRenderer::drawPolyOffsetTest(const Math::Vector3d &pos, const Math::Vector3d &roll) {
//	error("Polygon offset test not implemented yet");
}

void N3DSRenderer::flipBuffer() {
	N3D_C3D_FrameEnd(0);
	N3DS_3D::setContext(_backendContext);
}

//void ShaderRenderer::dimRegionInOut(float fade) {
void N3DSRenderer::dimRegionInOut(float fade) {
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//	glDisable(GL_DEPTH_TEST);
//	glDepthMask(GL_FALSE);
//
//	_fadeShader->use();
//	_fadeShader->setUniform1f("alphaLevel", 1.0 - fade);
//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//	_fadeShader->unbind();
}

//void ShaderRenderer::drawInViewport() {
void N3DSRenderer::drawInViewport() {
//	error("Draw in viewport test not implemented yet");
}

//void ShaderRenderer::drawRgbaTexture() {
void N3DSRenderer::drawRgbaTexture() {
//	Math::Vector2d offset;
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//	glDisable(GL_DEPTH_TEST);
//	glDepthMask(GL_FALSE);
//
//	_bitmapShader->use();
//
//	offset.setX(-0.8f);
//	offset.setY(0.8f);
//	_bitmapShader->setUniform("offsetXY", offset);
//	glBindTexture(GL_TEXTURE_2D, _textureRgbaId[0]);
//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//
//	offset.setX(-0.3f);
//	offset.setY(0.8f);
//	_bitmapShader->setUniform("offsetXY", offset);
//	glBindTexture(GL_TEXTURE_2D, _textureRgbId[0]);
//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//
//	offset.setX(0.2f);
//	offset.setY(0.8f);
//	_bitmapShader->setUniform("offsetXY", offset);
//	glBindTexture(GL_TEXTURE_2D, _textureRgb565Id[0]);
//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//
//	offset.setX(0.7f);
//	offset.setY(0.8f);
//	_bitmapShader->setUniform("offsetXY", offset);
//	glBindTexture(GL_TEXTURE_2D, _textureRgba5551Id[0]);
//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//
//	offset.setX(-0.8f);
//	offset.setY(0.2f);
//	_bitmapShader->setUniform("offsetXY", offset);
//	glBindTexture(GL_TEXTURE_2D, _textureRgba4444Id[0]);
//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//
//	_bitmapShader->unbind();
}

} // End of namespace Playground3d

#endif
