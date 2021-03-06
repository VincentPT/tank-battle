/******************************************************************
* File:        TankCommandsBuilder.cpp
* Description: implement TankCommandsBuilder class. This is a helper 
*              class responsible for building tank commands.
* Author:      Vincent Pham
*
* Copyright (c) 2018 VincentPT.
** Distributed under the MIT License (http://opensource.org/licenses/MIT)
**
*
**********************************************************************/

#include "TankCommandsBuilder.h"

TankCommandsBuilder::TankCommandsBuilder(TankOperations& initCommands) : _commands(initCommands)
{
}

void TankCommandsBuilder::freeze() {
	_commands = FREEZE_COMMAND;
}

void TankCommandsBuilder::fire() {
	_commands |= 0x01000000;
}

bool TankCommandsBuilder::hasFire() const {
	return (_commands & 0x01000000) != 0;
}

void TankCommandsBuilder::move(char moveDir) {
	TankOperations singleCommand = (unsigned char)moveDir;
	_commands = ((0xFFFFFF00 & _commands) | singleCommand);
}

void TankCommandsBuilder::turn(char turnDir) {
	TankOperations singleCommand = (unsigned char)turnDir;
	singleCommand <<= 8;

	_commands = ((0xFFFF00FF & _commands) | singleCommand);
}

void TankCommandsBuilder::spinGun(char spinDir) {
	TankOperations singleCommand = (unsigned char)spinDir;
	singleCommand <<= 16;

	_commands = ((0xFF00FFFF & _commands) | singleCommand);
}

char TankCommandsBuilder::getMovingDir() const {
	return (char)(_commands & 0xFF);
}

char TankCommandsBuilder::getTurnDir() const {
	return (char)((_commands >> 8) & 0xFF);
}

char TankCommandsBuilder::getSpinningGunDir() const {
	return (char)((_commands >> 16) & 0xFF);
}