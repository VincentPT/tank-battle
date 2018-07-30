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
#include <RawStringLib.h>

#include <memory>

using namespace ffscript;
using namespace std;

class ScriptedPlayer::ScriptedPlayerImpl {
	GlobalScopeRef rootScope;
	Program* _program;
	int _functionIdOfMainFunction;
	int _setupFunctionId = -1;
	int _cleanupFunctionId = -1;

	shared_ptr<ScriptTask> _scriptTask;
	TankPlayerContext* _temporaryPlayerContex;
	ScriptingLib::PlayerContextSciptingLibrary _myScriptLib;
	CompilerSuite _compiler;
	string _errorMessage;
public:
	ScriptedPlayerImpl() : _functionIdOfMainFunction(-1) {
		//wstring scriptWstr = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes(script);
		_compiler.initialize(1024);

		rootScope = _compiler.getGlobalScope();
		auto scriptCompiler = rootScope->getCompiler();
		includeRawStringToCompiler(scriptCompiler);

		_myScriptLib.loadLibrary(scriptCompiler);		
		
		scriptCompiler->beginUserLib();
		_compiler.setPreprocessor(std::make_shared<DefaultPreprocessor>());
	}

	const char* setProgramScipt(const wchar_t* scriptStart, const wchar_t* scriptEnd) {
		auto scriptCompiler = rootScope->getCompiler();
		scriptCompiler->setErrorText("");

		_functionIdOfMainFunction = -1;
		_program = _compiler.compileProgram(scriptStart, scriptEnd);
		if (_program) {
			_functionIdOfMainFunction = scriptCompiler->findFunction("update", "float");
			if (_functionIdOfMainFunction >= 0) {
				auto functionFactory = scriptCompiler->getFunctionFactory(_functionIdOfMainFunction);
				_scriptTask = make_shared<ScriptTask>(_program);
			}
			else {
				_errorMessage = "function 'void update(float)' is not found";
			}

			_setupFunctionId = scriptCompiler->findFunction("setup", "");
			_cleanupFunctionId = scriptCompiler->findFunction("cleanup", "");

			if(_setupFunctionId < 0) {
				GameInterface::getInstance()->printMessage("function 'void setup()' is not found\n");
			}
			if (_cleanupFunctionId < 0) {
				GameInterface::getInstance()->printMessage("function 'void cleanup()' is not found\n");
			}
		}
		else {
			_errorMessage = scriptCompiler->getLastError();
		}

		if (_functionIdOfMainFunction < 0) {
			return _errorMessage.c_str();
		}

		return nullptr;
	}

	~ScriptedPlayerImpl(){
		if (_program) {
			delete _program;
		}
	}

	void setup(TankPlayerContext* player){
		rootScope->runGlobalCode();
		_myScriptLib.resetCommand();

		if (_setupFunctionId >= 0) {
			try {
				_scriptTask->runFunction(5 * 1024 * 1024, _setupFunctionId, nullptr);
			}
			catch (std::exception&e) {
				string message = "setup funtion failed to execute:";
				message += e.what();
				message += "\n";
				GameInterface::getInstance()->printMessage(message.c_str());
			}
		}
	}

	void cleanup(TankPlayerContext* player) {
		if (_cleanupFunctionId >= 0) {
			try {
				_scriptTask->runFunction(5 * 1024 * 1024, _cleanupFunctionId, nullptr);
			}
			catch (std::exception&e) {
				string message = "clean funtion failed to execute:";
				message += e.what();
				message += "\n";
				GameInterface::getInstance()->printMessage(message.c_str());
			}
		}
	}

	TankOperations giveOperations(TankPlayerContext* player) {
		if (_program && _functionIdOfMainFunction >= 0) {
			// run function and allow maxium 5mb stack size
			_temporaryPlayerContex = player;
			float t = GameInterface::getInstance()->getTime();
			try {
				ScriptParamBuffer paramsBuffer(t);
				_scriptTask->runFunction(5 * 1024 * 1024, _functionIdOfMainFunction, paramsBuffer);
				return _myScriptLib.getOperations();
			}
			catch (std::exception&e) {
				string message = "update funtion failed to execute:";
				message += e.what();
				message += "\n";
				GameInterface::getInstance()->printMessage(message.c_str());
			}
		}

		return TANK_NULL_OPERATION;
	}
};

ScriptedPlayer::ScriptedPlayer() : _pImpl(nullptr)
{
	_pImpl = new ScriptedPlayerImpl();
}

ScriptedPlayer::~ScriptedPlayer()
{
	if (_pImpl) {
		delete _pImpl;
	}
}

const char* ScriptedPlayer::setProgramScript(const wchar_t* scriptStart, const wchar_t* scriptEnd) {
	return _pImpl->setProgramScipt(scriptStart, scriptEnd);
}

const char* ScriptedPlayer::setProgramScript(const char* file) {
	auto wstr = readCodeFromUtf8File(file);

	return setProgramScript(wstr.c_str(), wstr.c_str() + wstr.size());
}

void ScriptedPlayer::setup(TankPlayerContext* player) {
	_pImpl->setup(player);
}

void ScriptedPlayer::cleanup(TankPlayerContext* player) {
	_pImpl->cleanup(player);
}

TankOperations ScriptedPlayer::giveOperations(TankPlayerContext* player) {
	return _pImpl->giveOperations(player);
	//return TANK_NULL_OPERATION;
}