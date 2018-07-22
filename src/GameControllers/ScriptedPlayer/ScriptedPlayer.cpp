#include "ScriptedPlayer.h"
#include "../GameControllers.h"
#include "../common/GameUtil.hpp"
#include "GameInterface.h"
#include "TankCommandsBuilder.h"
#include "PlayerSciptingLibrary.h"

#include <string>
#include <locale>
#include <codecvt>

#include <CompilerSuite.h>
#include <ScriptTask.h>
#include <Utils.h>
#include <DefaultPreprocessor.h>

#include <memory>

using namespace ffscript;
using namespace std;

extern "C" {
	GAME_CONTROLLER_API ScriptedPlayer* createController() {
		return new ScriptedPlayer();
	}
}

class ScriptedPlayer::ScriptedPlayerImpl {
	GlobalScopeRef rootScope;
	Program* _program;
	int _functionIdOfMainFunction;
	shared_ptr<ScriptTask> _scriptTask;
	TankPlayerContext* _temporaryPlayerContex;
	PlayerContextSciptingLibrary _myScriptLib;
public:
	ScriptedPlayerImpl(const wchar_t* scriptStart, const wchar_t* scriptEnd) : _functionIdOfMainFunction(-1) {
		//wstring scriptWstr = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes(script);
		CompilerSuite compiler;
		compiler.initialize(1024);

		rootScope = compiler.getGlobalScope();
		auto scriptCompiler = rootScope->getCompiler();

		_myScriptLib.loadLibrary(scriptCompiler);		
		
		scriptCompiler->beginUserLib();
		compiler.setPreprocessor(std::make_shared<DefaultPreprocessor>());
		_program = compiler.compileProgram(scriptStart, scriptEnd);

		if (_program) {
			_functionIdOfMainFunction = scriptCompiler->findFunction("update", "float");
			if (_functionIdOfMainFunction >= 0) {
				auto functionFactory = scriptCompiler->getFunctionFactory(_functionIdOfMainFunction);
				_scriptTask = make_shared<ScriptTask>(_program);
			}
		}
	}

	bool isValidProgram() {
		return _functionIdOfMainFunction >= 0;
	}

	~ScriptedPlayerImpl(){
		if (_program) {
			delete _program;
		}
	}

	TankOperations giveOperations(TankPlayerContext* player) {
		// run function and allow maxium 5mb stack size
		_temporaryPlayerContex = player;
		_myScriptLib.resetCommand();
		float t = GameInterface::getInstance()->getTime();
		try {
			ScriptParamBuffer paramsBuffer(&t);
			_scriptTask->runFunction(5 * 1024 * 1024, _functionIdOfMainFunction, paramsBuffer);
			return _myScriptLib.getOperations();
		}
		catch (std::exception&e) {			
		}

		return TANK_NULL_OPERATION;
	}
};

ScriptedPlayer::ScriptedPlayer() : _pImpl(nullptr)
{
}

ScriptedPlayer::~ScriptedPlayer()
{
	if (_pImpl) {
		delete _pImpl;
	}
}

bool ScriptedPlayer::setProgramScript(const wchar_t* scriptStart, const wchar_t* scriptEnd) {
	if (_pImpl) {
		delete _pImpl;
	}
	
	_pImpl = new ScriptedPlayerImpl(scriptStart, scriptEnd);
	if (_pImpl->isValidProgram() == false) {
		return false;
	}

	return false;
}

void ScriptedPlayer::setup(TankPlayerContext*) {
	wstring script = L"void update(float t) {"\
		L"    freeze();"\
		L"    setMove(MOVE_FORWARD);"\
		L"}";

	setProgramScript(
		script.c_str(), script.c_str() + script.size()
	);
}

TankOperations ScriptedPlayer::giveOperations(TankPlayerContext* player) {
	return _pImpl->giveOperations(player);
	//return TANK_NULL_OPERATION;
}