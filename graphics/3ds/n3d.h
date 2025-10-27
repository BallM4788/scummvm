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

/*
 * This file is based on, or a modified version of code from TinyGL (C) 1997-2022 Fabrice Bellard,
 * which is licensed under the MIT license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */

#ifndef GRAPHICS_3DS_NATIVE3D_H
#define GRAPHICS_3DS_NATIVE3D_H

//struct Tex3DS_Texture_s
//{
//	u16 numSubTextures;               // Number of subtextures
//	u16 width;                        // Texture width
//	u16 height;                       // Texture height
//	u8  format;                       // Texture format
//	u8  mipmapLevels;                 // Number of mipmaps
//	Tex3DS_SubTexture *subTextures;   // Subtextures
//};


//#include "graphics/3ds/ngl.h"
#include "graphics/3ds/z3d.h"

namespace N3DS_3D {

typedef void *ContextHandle;

// Create a new context with values corresponding to the ScummVM 3DS backend environment;
// 	this new context is now the active context, and a pointer to the context's handle is returned.
ContextHandle *createContext();
// Create a new context with values corresponding to OpenGL's default environment;
// 	this new context is now the active context, and a pointer to the context's handle is returned.
ContextHandle *createOGLContext();
// Clone an existing context; this new context is now the active context, and a pointer to the
// 	context's handle is returned.
ContextHandle *createContext(ContextHandle *source);
// Set the context accociated with a given handle pointer as the active context.
void setContext(ContextHandle *handle);
// Return a pointer to the context accociated with a given handle pointer.
N3DContext *getContext(ContextHandle *handle);
// Destroy the context accociated with a given handle pointer.
void destroyContext(ContextHandle *handle);
// Destroy the Native3D singleton along with any contexts.
void destroyNative3D();


} // end of namespace N3DS_3D

#endif // GRAPHICS_N3DS_NATIVE3D_H
