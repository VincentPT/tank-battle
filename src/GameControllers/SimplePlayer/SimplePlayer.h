#pragma once
#include "GameControllerInterfaces.h"
#include "TankCommandsBuilder.h"
#include "PlayerContext.h"
#include "TankController.h"

class SimplePlayer : public TankController
{
public:
	SimplePlayer();
	virtual ~SimplePlayer();
	TankOperations giveOperations(TankPlayerContext* player);
};
