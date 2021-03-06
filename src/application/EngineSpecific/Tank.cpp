/******************************************************************
* File:        Tank.cpp
* Description: implement Tank class. Instances of this class are
*              players in game.
* Author:      Vincent Pham
*
* Copyright (c) 2018 VincentPT.
** Distributed under the MIT License (http://opensource.org/licenses/MIT)
**
*
**********************************************************************/

#include "Tank.h"
#include "Bullet.h"
#include "engine/Scene.h"
#include "engine/GameEngine.h"
#include "GameController.h"
#include "engine/LifeTimeControlComponent.h"
#include "engine/GameResource.h"
#include "../battle/GameInterfaceImpl.h"

using namespace ci;

Tank::Tank() :
	_movingDir(0),
	_rotateDir(0),
	_rotateBarrelDir(0),
	_lastMovingAt(-1),
	_lastRotatingAt(-1),
	_lastRotatingBarrelAt(-1),
	_lastFireTime(-1),
	_color(1,1,1)
{
	_movingSpeed = GameInterface::getInstance()->getObjectSpeed(GAME_TYPE_TANK);
	_rotateSpeed = glm::pi<float>()/ 5;
	_fireSpeed = 60; // 60 bullets can be fired in one minute
	_rotateBarrelSpeed = glm::half_pi<float>();

	allowGoThrough(false);
	setObjectStaticFlag(false);
	setComponentTextures();

	setShield(5);
	setHealth(50);
	// default group id is object id
	_groupId = getId();
}

Tank::~Tank() {}

void Tank::setBound(const ci::Rectf& boundRect) {
	DrawableObject::setBound(boundRect);
}

void Tank::setSize(const ci::vec2& size) {
	// the expected sizes base on its images
	static const ci::vec2 baseSize(76, 88);
	static const ci::vec2 baseBodySize(76, 72);
	static const ci::vec2 baseBarrelSize(16, 52);
	static const ci::vec2 baseBarrelPivotLocal(baseBarrelSize.x/2, 8);

	// caculate diffent ratio of base size and real size
	float scaleX = size.x / baseSize.x;
	float scaleY = size.y / baseSize.y;

	// construct tank's components coordinates base on real size
	ci::vec2 bodySize(baseBodySize.x*scaleX, baseBodySize.y*scaleY);
	ci::vec2 barrelSize(baseBarrelSize.x*scaleX, baseBarrelSize.y*scaleY);
	ci::vec2 barrelPivotLocal(baseBarrelPivotLocal.x*scaleX, baseBarrelPivotLocal.y*scaleY);

	ci::Rectf tankBound(-size.x/2, -size.y/2, size.x/2, size.y/2);
	ci::vec2 barrelPivot((tankBound.x1 + tankBound.x2) / 2, tankBound.y2 - barrelSize.y + barrelPivotLocal.y);

	// tank boundary
	//setBound(tankBound);

	// body boundary
	ci::Rectf bodyBound(tankBound.x1, tankBound.y1, tankBound.x2, tankBound.y1 + bodySize.y);
	_body.setBound(bodyBound);

	// barrel boundary
	ci::Rectf barrelBound(barrelPivot.x - barrelSize.x/2, tankBound.y2 - barrelSize.y, barrelPivot.x + barrelSize.x / 2, tankBound.y2);
	_barrel.setBound(barrelBound);

	// set pivots
	//setPivot(ci::vec3(bodyBound.getCenter(), 0));
	setBound(bodyBound);
	_barrel.setPivot(ci::vec3(barrelPivot, 0));

	// initialize cached gun
	ci::vec2 topGun = (barrelBound.getLowerLeft() + barrelBound.getLowerRight()) / 2.0f;
	_cahedGun = vec4(barrelPivot.x, barrelPivot.y, topGun.x, topGun.y);
}

void Tank::updateInternal(float t) {
	// update tank components
	_body.update(t);
	_barrel.update(t);

	// update rotation
	if (_rotateDir) {
		auto delta = t - _lastRotatingAt;
		rotate(_rotateDir * _rotateSpeed * delta);
		_lastRotatingAt = t;
	}

	// update barrel rotation
	if (_rotateBarrelDir) {
		auto delta = t - _lastRotatingBarrelAt;
		_barrel.rotate(_rotateBarrelDir * _rotateBarrelSpeed * delta);
		_lastRotatingBarrelAt = t;
		
		// update gun geometry for external use
		auto& barelBound = _barrel.getBound();
		auto& pivot3 = _barrel.getPivot();

		ci::vec2 pivot(pivot3.x, pivot3.y);
		ci::vec2 topGun = (barelBound.getLowerLeft() + barelBound.getLowerRight()) / 2.0f;

		pivot = transform(pivot, _barrel.getTransformation());
		topGun = transform(topGun, _barrel.getTransformation());

		_cahedGun = vec4(pivot.x, pivot.y, topGun.x, topGun.y);
	}

	// update tank's position
	if (_movingDir) {
		auto delta = t - _lastMovingAt;
		ci::vec3 movingDir(0, 1, 0);
		movingDir *= _movingDir*_movingSpeed*delta;
		translate(movingDir);
		_lastMovingAt = t;
	}
}

void Tank::drawInternal() {
	_body.draw();

	auto pivot = _body.getPivot();

	auto bodyBound = _body.getBound();
	auto w = bodyBound.getWidth() / 2;
	auto h = bodyBound.getHeight();

	Rectf identityRect(pivot.x - w/2, pivot.y - h /2, pivot.x + w/2, pivot.y + h/2);
	{
		gl::ScopedColor scopeColor(_color);
		gl::drawSolidRect(identityRect);
	}

	_barrel.draw();
}

void Tank::setComponentTextures() {
	_body.setTexture(GameResource::getInstance()->getTexture(TEX_ID_TANKBODY));
	_barrel.setTexture(GameResource::getInstance()->getTexture(TEX_ID_TANKBARREL));
}

void Tank::move(char direction, float at) {
	if (abs((int)direction) > 1) return;

	if (_movingDir == direction) return;

	_movingDir = direction;
	if (direction) {
		_lastMovingAt = at;
	}
}

void Tank::turn(char direction, float at) {
	if (abs((int)direction) > 1) return;

	if (_rotateDir == direction) return;

	_rotateDir = direction;
	if (direction) {
		_lastRotatingAt = at;
	}
}

void Tank::spinBarrel(char direction, float at) {
	if (abs((int)direction) > 1) return;

	if (_rotateBarrelDir == direction) return;

	_rotateBarrelDir = direction;
	if (direction) {
		_lastRotatingBarrelAt = at;
	}
}

void Tank::fire(float at) {
	if (_lastFireTime < 0 || (at - _lastFireTime) >= _fireSpeed/60) {
		auto currentScene = Scene::getCurrentScene();
		if (currentScene) {
			auto bullet = std::make_shared<Bullet>(at);
			currentScene->addDrawbleObject(bullet);

			// ensure that the bullet destroy automatically after 5 seconds
			auto bulletLifeTimeControl = std::make_shared<LifeTimeControlComponent>(5.0f);
			bulletLifeTimeControl->startLifeTimeCountDown(at);
			bullet->addComponent(bulletLifeTimeControl);

			CollisionDetectedHandler onCollisionDetected = std::bind(&GameController::OnBulletCollisionDetected,
				GameController::getInstance(), bullet, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			bullet->getCollisionHandler() += std::move(onCollisionDetected);

			bullet->setOwner(currentScene->findObjectRef(this));

			auto& tankTransform = getTransformation();
			auto& barrelTransform = _barrel.getTransformation();

			// put bullet transformation at tank's barrel
			auto bulletTransform = tankTransform * barrelTransform;
			bullet->setTransformation(bulletTransform);

			auto& barelBound = _barrel.getBound(); 
			auto& pivot = _barrel.getPivot();
			// bullet out position is center of bottom edge of the bound rectangle
			auto bulletOutPosition = (barelBound.getLowerLeft() + barelBound.getLowerRight())/2.0f;

			// after put bullet transformation at tank's barrel
			// the bullet is put at pivot of barrel
			// now we move it to bullet out position by offset the transformation matrix by translation vector of
			// pivot and bullet out position
			auto v = ci::vec3(bulletOutPosition, 0) - pivot;

			// temporary fix the bug of bullet stuck in some cases after fired out
			v = ci::normalize(v) * 4.f;
			bullet->translate(v);

			// bullet speed is 5 times faster than tank's moving speed
			bullet->setSpeed(_movingSpeed * 5);

			// set bullet size, should be smaller than tank's gun, current is 0.75 compare to tank's gun
			// the width of tank's gun assume that is half of barrel's width
			auto& defaultBulletBound = bullet->getBound();
			auto bulletW = 0.75f * barelBound.getWidth()/2;
			auto bulletH = bulletW * defaultBulletBound.getHeight() / defaultBulletBound.getWidth();
			bullet->setSize(bulletW, bulletH);

			_barrel.fire(at);
			_lastFireTime = at;
		}
	}
}

void Tank::setColor(const ci::Colorf& color) {
	_color = color;
}

ci::vec4 Tank::getGun() const {
	return _cahedGun;
}

int Tank::getGroupId() const {
	return _groupId;
}

void Tank::setGroupId(int groupId) {
	_groupId = groupId;
}

TankOperations Tank::getCurrentOperations() const {
	TankOperations operations = TANK_NULL_OPERATION;
	TankCommandsBuilder commandBuilder(operations);
	commandBuilder.freeze();
	commandBuilder.move(_movingDir);
	commandBuilder.turn(_rotateDir);
	commandBuilder.spinGun(_rotateBarrelDir);

	return operations;
}

float Tank::getMaxTurningSpeed() const {
	return _rotateSpeed;
}