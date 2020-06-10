#include "pch.h"
#include "Globals/AppGlobals.h"
#include "Input/Joystick/JoystickDevice.h"
#include "Input/Joystick/JoystickInput.h"
#include "Input/Keyboard/KeyboardDevice.h"
#include "Input/Pointer/PointerDevice.h"
#include "Services/AppService.h"
#include "State/StateWrapper.h"
#include "States/GameState.h"
#include "States/LevelState.h"

Game::GameState::GameState(IAppService* appService, GameOptions gameOptions)
	: _appService(appService)
	, _gameOptions(gameOptions)
{
	InitInput();
	InitLevel();
}

Game::GameState::~GameState()
{
}

std::shared_ptr<ff::State> Game::GameState::Advance(ff::AppGlobals* globals)
{
	AdvanceInput();

	return ff::State::Advance(globals);
}

void Game::GameState::Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
	return ff::State::Render(globals, target, depth);
}

size_t Game::GameState::GetChildStateCount()
{
	return (_levelState != nullptr) ? 1 : 0;
}

ff::State* Game::GameState::GetChildState(size_t index)
{
	return _levelState.get();
}

Game::IAppService* Game::GameState::GetAppService() const
{
	return _appService;
}

const Game::GameOptions& Game::GameState::GetGameOptions() const
{
	return _gameOptions;
}

const ff::IInputEvents* Game::GameState::GetGameInputEvents()
{
	return _gameInput.Flush();
}

const ff::IInputEvents* Game::GameState::GetPlayerInputEvents(size_t player)
{
	assertRetVal(player < _playerInput.size(), nullptr);
	return _playerInput[player].Flush();
}

void Game::GameState::InitInput()
{
	ff::AppGlobals& globals = _appService->GetAppGlobals();

	_gameInput.Init(L"gameControls");
	_gameInputDevices._keys.Push(globals.GetKeys());
	_gameInputDevices._mice.Push(globals.GetPointer());

	_playerInputDevices[0]._keys.Push(globals.GetKeys());
	_playerInputDevices[0]._mice.Push(globals.GetPointer());
	_playerInput[0].Init(L"playerControls");

	for (size_t i = 1; i < _playerInput.size(); i++)
	{
		_playerInput[i].Init(L"secondaryPlayerControls");
	}

	for (size_t i = 0; i < globals.GetJoysticks()->GetCount(); i++)
	{
		ff::IJoystickDevice* joystick = globals.GetJoysticks()->GetJoystick(i);
		_gameInputDevices._joys.Push(joystick);

		if (i < _playerInputDevices.size())
		{
			_playerInputDevices[i]._joys.Push(joystick);
		}
	}
}

void Game::GameState::InitLevel()
{
	*_levelState = std::make_shared<LevelState>(this);
}

void Game::GameState::AdvanceInput()
{
	const double deltaTime = _appService->GetAppGlobals().GetGlobalTime()._secondsPerAdvance;

	ff::IInputMapping* gameInput = _gameInput.GetObject();
	if (gameInput)
	{
		gameInput->Advance(_gameInputDevices, deltaTime);
	}

	for (size_t i = 0; i < _playerInput.size(); i++)
	{
		ff::IInputMapping* playerInput = _playerInput[i].GetObject();
		if (playerInput)
		{
			playerInput->Advance(_playerInputDevices[i], deltaTime);
		}
	}
}
