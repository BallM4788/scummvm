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

//#include <3ds.h>
//#include <citro3d.h>
//#include <tex3ds.h>
#include "graphics/3ds/n3d.h"
//#include "graphics/3ds/z3d.h"


#include "common/singleton.h"
#include "common/array.h"

namespace N3DS_3D {

class Native3D : public Common::Singleton<Native3D> {
private:
	Common::Array<N3DContext *> _N3DContextArray;

public:
	N3DContext *createContext() {
		N3DContext *ctx = new N3DContext;
		_N3DContextArray.push_back(ctx);
		return ctx;
	}

	bool existsContexts() {
		return _N3DContextArray.size() != 0;
	}

	N3DContext *getContext(ContextHandle *handle) {
		for (Common::Array<N3DContext *>::iterator it = _N3DContextArray.begin(); it != _N3DContextArray.end(); it++) {
			if ((*it) == (N3DContext *)handle) {
				return *it;
			}
		}
		return nullptr;
	}

	void destroyContext(ContextHandle *handle) {
		for (Common::Array<N3DContext *>::iterator it = _N3DContextArray.begin(); it != _N3DContextArray.end(); it++) {
			if (*it == (N3DContext *)handle) {
				(*it)->deinit();
				delete *it;
				_N3DContextArray.erase(it);
				break;
			}
		}
	}

	void destroyContexts() {
		for (Common::Array<N3DContext *>::iterator it = _N3DContextArray.begin(); it != _N3DContextArray.end(); it++) {
			if (*it != nullptr) {
				(*it)->deinit();
				delete *it;
			}
		}
		_N3DContextArray.clear();
	}

};

} // end of namespace N3DS_3D

namespace Common {
	DECLARE_SINGLETON(N3DS_3D::Native3D);
}


namespace N3DS_3D {

N3DContext *activeContext;

N3DContext *getActiveContext() {
	assert(activeContext);
	return activeContext;
}



ContextHandle *createContext() {
	activeContext = Native3D::instance().createContext();
	activeContext->init();
	activeContext->applyContextState();
	return (ContextHandle *)activeContext;
}

ContextHandle *createOGLContext() {
	activeContext = Native3D::instance().createContext();
	activeContext->initOGL();
	activeContext->applyContextState();
	return (ContextHandle *)activeContext;
}

ContextHandle *createContext(ContextHandle *source) {
	activeContext = Native3D::instance().createContext();
	activeContext->init((N3DContext *)source);
	activeContext->applyContextState();
	return (ContextHandle *)activeContext;
}

void setContext(ContextHandle *handle) {
	N3DContext *ctx = Native3D::instance().getContext(handle);
	if (ctx == nullptr) {
		error("N3DS_3D: Context not found");
	}
	activeContext = ctx;
	//activeContext->activeShaderObj = nullptr;
	activeShaderObj = nullptr;
	activeContext->applyContextState(true);
}

N3DContext *getContext(ContextHandle *handle) {
	N3DContext *ctx = Native3D::instance().getContext(handle);
	if (ctx == nullptr) {
		error("N3DS_3D: Context not found");
	}
	return ctx;
}

void destroyContext(ContextHandle *handle) {
	Native3D::instance().destroyContext(handle);
	if ((N3DContext *)handle == activeContext)
		activeContext = nullptr;
	if (!Native3D::instance().existsContexts())
		Native3D::destroy();
}

void destroyNative3D() {
	Native3D::instance().destroyContexts();
	Native3D::destroy();
	activeContext = nullptr;
}


} // end of namespace N3DS_3D
