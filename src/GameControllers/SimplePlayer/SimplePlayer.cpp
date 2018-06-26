#include "SimplePlayer.h"
#include "../GameControllers.h"
#include "GameInterface.h"

extern "C" {
	GAME_CONTROLLER_API TankController* createController() {
		return new SimplePlayer();
	}
}

SimplePlayer::SimplePlayer()
{
}

SimplePlayer::~SimplePlayer()
{
}

TankOperations SimplePlayer::giveOperations(TankPlayerContext* player) {
	GameInterface* gameInterface = GameInterface::getInstance();
	if (gameInterface == nullptr) {
		return TANK_NULL_OPERATION;
	}

	auto gunRay = player->getMyGun();
	auto cameraViewObjects = player->getCameraSnapshot();

	return TANK_NULL_OPERATION;
}