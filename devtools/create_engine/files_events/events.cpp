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

#include "common/config-manager.h"
#include "graphics/screen.h"
#include "xyzzy/events.h"
#include "xyzzy/xyzzy.h"
#include "xyzzy/views.h"

namespace Xyzzy {

Events *g_events;

Events::Events() : UIElement("Root", nullptr) {
	g_events = this;
}

Events::~Events() {
	g_events = nullptr;
}

void Events::runGame() {
	uint currTime, nextFrameTime = 0;
	_screen = new Graphics::Screen();
	Views views;	// Loads all views in the structure

	// Run the game
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		g_engine->loadGameState(saveSlot);

	addView("View1");

	Common::Event e;
	while (!_views.empty() && !shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
			if (e.type == Common::EVENT_QUIT ||
					e.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				_views.clear();
				break;
			} else {
				processEvent(e);
			}
		}

		if (_views.empty())
			break;

		g_system->delayMillis(10);
		if ((currTime = g_system->getMillis()) >= nextFrameTime) {
			nextFrameTime = currTime + FRAME_DELAY;
			tick();
			drawElements();
			_screen->update();
		}
	}

	delete _screen;
}

void Events::processEvent(Common::Event &ev) {
	switch (ev.type) {
	case Common::EVENT_KEYDOWN:
		if (ev.kbd.keycode < Common::KEYCODE_NUMLOCK)
			msgKeypress(KeypressMessage(ev.kbd));
		break;
	case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
		msgAction(ActionMessage(ev.customType));
		break;
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_MBUTTONDOWN:
		msgMouseDown(MouseDownMessage(ev.type, ev.mouse));
		break;
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONUP:
	case Common::EVENT_MBUTTONUP:
		msgMouseUp(MouseUpMessage(ev.type, ev.mouse));
		break;
	case Common::EVENT_MOUSEMOVE:
		msgMouseMove(MouseMoveMessage(ev.type, ev.mouse));
		break;
	default:
		break;
	}
}

void Events::replaceView(UIElement *ui, bool replaceAllViews) {
	assert(ui);
	UIElement *priorView = focusedView();

	if (replaceAllViews) {
		clearViews();

	} else if (!_views.empty()) {
		priorView->msgUnfocus(UnfocusMessage());
		_views.pop();
	}

	_views.push(ui);
	ui->redraw();
	ui->msgFocus(FocusMessage(priorView));
}

void Events::replaceView(const Common::String &name, bool replaceAllViews) {
	replaceView(findView(name));
}

void Events::addView(UIElement *ui) {
	assert(ui);
	UIElement *priorView = focusedView();

	if (!_views.empty())
		priorView->msgUnfocus(UnfocusMessage());

	_views.push(ui);
	ui->redraw();
	ui->msgFocus(FocusMessage(priorView));
}

void Events::addView(const Common::String &name) {
	addView(findView(name));
}

void Events::popView() {
	UIElement *priorView = focusedView();
	priorView->msgUnfocus(UnfocusMessage());
	_views.pop();

	for (int i = 0; i < (int)_views.size() - 1; ++i) {
		_views[i]->redraw();
		_views[i]->draw();
	}

	if (!_views.empty()) {
		UIElement *view = focusedView();
		view->msgFocus(FocusMessage(priorView));
		view->redraw();
		view->draw();
	}
}

void Events::redrawViews() {
	for (uint i = 0; i < _views.size(); ++i) {
		_views[i]->redraw();
		_views[i]->draw();
	}
}

bool Events::isPresent(const Common::String &name) const {
	for (uint i = 0; i < _views.size(); ++i) {
		if (_views[i]->_name == name)
			return true;
	}

	return false;
}

void Events::clearViews() {
	if (!_views.empty())
		focusedView()->msgUnfocus(UnfocusMessage());

	_views.clear();
}

void Events::addKeypress(const Common::KeyCode &kc) {
	Common::KeyState ks;
	ks.keycode = kc;
	if (kc >= Common::KEYCODE_SPACE && kc <= Common::KEYCODE_TILDE)
		ks.ascii = kc;

	focusedView()->msgKeypress(KeypressMessage(ks));
}

/*------------------------------------------------------------------------*/

Bounds::Bounds(Common::Rect &innerBounds) :
		_bounds(0, 0, 320, 200),
		_innerBounds(innerBounds),
		left(_bounds.left), top(_bounds.top),
		right(_bounds.right), bottom(_bounds.bottom) {
}

Bounds &Bounds::operator=(const Common::Rect &r) {
	_bounds = r;
	_innerBounds = r;
	_innerBounds.grow(-_borderSize);
	return *this;
}

void Bounds::setBorderSize(size_t borderSize) {
	_borderSize = borderSize;
	_innerBounds = *this;
	_innerBounds.grow(-_borderSize);
}

/*------------------------------------------------------------------------*/

UIElement::UIElement(const Common::String &name) :
		_name(name), _parent(g_engine), _bounds(_innerBounds) {
	g_engine->_children.push_back(this);
}

UIElement::UIElement(const Common::String &name, UIElement *uiParent) :
		_name(name), _parent(uiParent),
		_bounds(_innerBounds) {
	if (_parent)
		_parent->_children.push_back(this);
}

void UIElement::redraw() {
	_needsRedraw = true;

	for (size_t i = 0; i < _children.size(); ++i)
		_children[i]->redraw();
}

void UIElement::drawElements() {
	if (_needsRedraw) {
		draw();
		_needsRedraw = false;
	}

	for (size_t i = 0; i < _children.size(); ++i)
		_children[i]->drawElements();
}

UIElement *UIElement::findViewGlobally(const Common::String &name) {
	return g_events->findView(name);
}

void UIElement::focus() {
	g_events->replaceView(this);
}

void UIElement::close() {
	assert(g_events->focusedView() == this);
	g_events->popView();
}

bool UIElement::isFocused() const {
	return g_events->focusedView() == this;
}

void UIElement::clearSurface() {
	Graphics::ManagedSurface s = getSurface();
	s.fillRect(Common::Rect(s.w, s.h), 0);
}

void UIElement::draw() {
	for (size_t i = 0; i < _children.size(); ++i) {
		_children[i]->draw();
	}
}

bool UIElement::tick() {
	if (_timeoutCtr && --_timeoutCtr == 0) {
		timeout();
	}

	for (size_t i = 0; i < _children.size(); ++i) {
		if (_children[i]->tick())
			return true;
	}

	return false;
}

UIElement *UIElement::findView(const Common::String &name) {
	if (_name.equalsIgnoreCase(name))
		return this;

	UIElement *result;
	for (size_t i = 0; i < _children.size(); ++i) {
		if ((result = _children[i]->findView(name)) != nullptr)
			return result;
	}

	return nullptr;
}

void UIElement::replaceView(UIElement *ui, bool replaceAllViews) {
	g_events->replaceView(ui, replaceAllViews);
}

void UIElement::replaceView(const Common::String &name, bool replaceAllViews) {
	g_events->replaceView(name, replaceAllViews);
}

void UIElement::addView(UIElement *ui) {
	g_events->addView(ui);
}

void UIElement::addView(const Common::String &name) {
	g_events->addView(name);
}

void UIElement::addView() {
	g_events->addView(this);
}

Graphics::ManagedSurface UIElement::getSurface() const {
	return Graphics::ManagedSurface(*g_events->getScreen(), _bounds);
}

int UIElement::getRandomNumber(int minNumber, int maxNumber) {
	return g_engine->getRandomNumber(maxNumber - minNumber + 1) + minNumber;
}

int UIElement::getRandomNumber(int maxNumber) {
	return g_engine->getRandomNumber(maxNumber);
}

void UIElement::delaySeconds(uint seconds) {
	_timeoutCtr = seconds * FRAME_RATE;
}

void UIElement::delayFrames(uint frames) {
	_timeoutCtr = frames;
}

void UIElement::timeout() {
	redraw();
}

} // namespace Xyzzy
