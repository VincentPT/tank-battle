#include "WxGameView.h"

#include "cinder/gl/gl.h"
#include "cinder/app/App.h"

using namespace ci;
using namespace std;

WxGameView::WxGameView(ci::app::WindowRef parent) : _expectedRatio(1.0f), _parent(parent) {
}
WxGameView::~WxGameView(){}

bool WxGameView::updateViewPort() {
	auto& viewRatio = _expectedRatio;

	if (getWidth() == 0 || getHeight() == 0) {
		return false;
	}

	ci::ivec2 center(getX() + getWidth() / 2, getY() + getHeight() / 2);
	auto windowRatio = getWidth() * 1.0f / getHeight();

	int viewWidth, viewHeight;

	if (viewRatio < windowRatio) {
		viewHeight = (int) getHeight();
		viewWidth = (int)(viewHeight * viewRatio);
	}
	else {
		viewWidth = (int)getWidth();
		viewHeight = (int)(viewWidth / viewRatio);
	}

	_viewPort.set(center.x - viewWidth/2, center.y - viewHeight/2, center.x + viewWidth / 2, center.y + viewHeight / 2);

	return true;
}

void WxGameView::setSize(float w, float h) {
	CiWidget::setSize(w, h);
	updateViewPort();
}

void WxGameView::setPos(float x, float y) {
	CiWidget::setPos(x, y);
	updateViewPort();
}

void WxGameView::setSceneViewRatio(float ratio) {
	_expectedRatio = ratio;
	updateViewPort();
}

float WxGameView::getSenceViewRatio() const {
	return _expectedRatio;
}

void WxGameView::setScene(std::shared_ptr<Scene> gameScene) {
	_gameScene = gameScene;
}

void WxGameView::update() {
	if (_gameScene) {
		float t = (float)ci::app::App::get()->getElapsedSeconds();
		_gameScene->update(t);
	}
}

void WxGameView::draw() {
	//// draw view port area
	//{
	//	gl::ScopedColor lineColorScope(1, 0, 0);
	//	gl::drawStrokedRect(Rectf(_viewPort), 3);
	//}

	auto h = _parent->getHeight();
	// mapping scene to view port area
	gl::ScopedViewport scopeViewPort(_viewPort.x1, h - _viewPort.y1 - _viewPort.getHeight(), _viewPort.getWidth(), _viewPort.getHeight());

	if (_gameScene) {
		_gameScene->draw();
	}
}