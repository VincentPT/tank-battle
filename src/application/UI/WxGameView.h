/******************************************************************
* File:        WxGameView.h
* Description: declare WxGameView class. This class is responsible
*              for showing a game sence real time.
* Author:      Vincent Pham
*
* Copyright (c) 2018 VincentPT.
** Distributed under the MIT License (http://opensource.org/licenses/MIT)
**
*
**********************************************************************/

#pragma once
#include "CiWidget.h"
#include <memory>
#include "../Engine/Scene.h"
#include "cinder/app/Renderer.h"
#include "../common/SpinArray.hpp"

class WxGameView : public CiWidget
{
protected:
	ci::Area _viewPort;
	ci::Area _displayRange;
	float _expectedRatio;
	std::shared_ptr<Scene> _gameScene;
	ci::app::WindowRef _parent;
	std::shared_ptr<Widget> _tankViews[2];

	float _lastTime;
	SpinArray<float, 100> _frameCounter;
protected:
	bool updateViewPort();
public:
	WxGameView(ci::app::WindowRef parent);
	virtual ~WxGameView();

	void update();
	void draw();

	void setSize(float w, float h);
	void setPos(float x, float y);

	void setScene(std::shared_ptr<Scene> gameScene);
	void setTankView(const std::shared_ptr<Widget>& tankView);
	// scene view ratio width per height
	// default is 1:1
	void setSceneViewRatio(float ratio);
	float getSenceViewRatio() const;
};

