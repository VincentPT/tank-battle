#include "PlayerControllerUI.h"
#include "../Engine/GameEngine.h"
#include "EngineSpecific/Tank.h"
#include "../common/ILogger.h"
using namespace ci;

static int instanceCount = 0;

PlayerControllerUI::PlayerControllerUI(ci::app::WindowRef inputWindow) {
	_keyDown = inputWindow->getSignalKeyDown().connect(std::bind(&PlayerControllerUI::onKeyDown, this, std::placeholders::_1));
	_keyUp = inputWindow->getSignalKeyUp().connect(std::bind(&PlayerControllerUI::onKeyUp, this, std::placeholders::_1));
	instanceCount++;
}

PlayerControllerUI::~PlayerControllerUI() {
	_keyDown.disconnect();
	_keyUp.disconnect();
    ILogger::getInstance()->log(LogLevel::Info, "PlayerControllerUI::~PlayerControllerUI is called \n");
	if (instanceCount > 1) {
		ILogger::getInstance()->logV(LogLevel::Info, "[!!!]warning: PlayerControllerUI is not clean up: %d \n", instanceCount);
	}
	instanceCount--;
}

void PlayerControllerUI::onKeyDown(ci::app::KeyEvent& e) {
	auto pTank = dynamic_cast<Tank*>(_owner);
	if (pTank == nullptr || pTank->isAvailable() == false) return;

	auto gameEngine = GameEngine::getInstance();
	if (gameEngine->isPausing()) {
		return;
	}
	float t = gameEngine->getCurrentTime();

	if (e.getCode() == app::KeyEvent::KEY_DOWN) {
		_keyDownDown = true;
	}
	else if(e.getCode() == app::KeyEvent::KEY_UP) {
		_keyUpDown = true;
	}
	else if (e.getCode() == app::KeyEvent::KEY_LEFT) {
		_keyLeftDown = true;
	}
	else if (e.getCode() == app::KeyEvent::KEY_RIGHT) {
		_keyRightDown = true;
	}
	else if (e.getCode() == app::KeyEvent::KEY_z) {
		_keyZDown = true;
	}
	else if (e.getCode() == app::KeyEvent::KEY_x) {
		_keyXDown = true;
	}
	else if (e.getCode() == app::KeyEvent::KEY_SPACE) {
		pTank->fire(t);
	}
	makeAction(t);
}

void PlayerControllerUI::onKeyUp(ci::app::KeyEvent& e) {
	auto gameEngine = GameEngine::getInstance();
	if (gameEngine->isPausing()) {
		return;
	}
	float t = gameEngine->getCurrentTime();

	if (e.getCode() == app::KeyEvent::KEY_DOWN) {
		_keyDownDown = false;
	}
	else if (e.getCode() == app::KeyEvent::KEY_UP) {
		_keyUpDown = false;
	}
	else if (e.getCode() == app::KeyEvent::KEY_LEFT) {
		_keyLeftDown = false;
	}
	else if (e.getCode() == app::KeyEvent::KEY_RIGHT) {
		_keyRightDown = false;
	}
	else if (e.getCode() == app::KeyEvent::KEY_z) {
		_keyZDown = false;
	}
	else if (e.getCode() == app::KeyEvent::KEY_x) {
		_keyXDown = false;
	}
	makeAction(t);
}


void PlayerControllerUI::makeAction(float t) {
	auto pTank = dynamic_cast<Tank*>(_owner);
	if (pTank == nullptr) return;

	char movingDir = 0;
	char rotateDir = 0;
	char spinBarrelDir = 0;

	if (_keyDownDown) {
		movingDir--;
	}
	if (_keyUpDown) {
		movingDir++;
	}
	if (_keyLeftDown) {
		rotateDir++;
	}
	if (_keyRightDown) {
		rotateDir--;
	}
	if (_keyZDown) {
		spinBarrelDir++;
	}
	if (_keyXDown) {
		spinBarrelDir--;
	}

	pTank->move(movingDir, t);
	pTank->turn(rotateDir, t);
	pTank->spinBarrel(spinBarrelDir, t);
}

void PlayerControllerUI::update(float t) {
}
