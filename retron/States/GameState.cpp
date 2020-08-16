#include "pch.h"
#include "Core/GameSpec.h"
#include "Dict/Dict.h"
#include "Globals/AppGlobals.h"
#include "Input/Joystick/JoystickDevice.h"
#include "Input/Joystick/JoystickInput.h"
#include "Input/Keyboard/KeyboardDevice.h"
#include "Input/Pointer/PointerDevice.h"
#include "Resource/Resources.h"
#include "Services/AppService.h"
#include "State/StateWrapper.h"
#include "States/GameState.h"
#include "States/LevelState.h"

ReTron::GameState::GameState(IAppService* appService)
	: _appService(appService)
	, _gameOptions(appService->GetDefaultGameOptions())
	, _diffSpec(_appService->GetGameSpec()._difficulties.GetKey(_gameOptions.GetDifficultyId())->GetValue())
	, _levelState(std::make_shared<ff::StateWrapper>())
	, _level(0)
{
	InitPlayers();
	InitLevel();
	InitInput();
}

ReTron::GameState::~GameState()
{
}

std::shared_ptr<ff::State> ReTron::GameState::Advance(ff::AppGlobals* globals)
{
	AdvanceInput();

	return ff::State::Advance(globals);
}

void ReTron::GameState::Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
	return ff::State::Render(globals, target, depth);
}

size_t ReTron::GameState::GetChildStateCount()
{
	return 1;
}

ff::State* ReTron::GameState::GetChildState(size_t index)
{
	return _levelState.get();
}

ReTron::IAppService* ReTron::GameState::GetAppService() const
{
	return _appService;
}

const ReTron::GameOptions& ReTron::GameState::GetGameOptions() const
{
	return _gameOptions;
}

const ReTron::DifficultySpec& ReTron::GameState::GetDifficultySpec() const
{
	return _diffSpec;
}

const ff::IInputEvents* ReTron::GameState::GetGameInputEvents()
{
	return _gameInput.Flush();
}

const ff::IInputEvents* ReTron::GameState::GetPlayerInputEvents(size_t player)
{
	assertRetVal(player < _playerInput.size(), nullptr);
	return _playerInput[player].Flush();
}

ReTron::Player& ReTron::GameState::GetPlayer(size_t player)
{
	assertRetVal(player < _playerInput.size(), _players[0]);
	return _players[player];
}

size_t ReTron::GameState::GetPlayerCount() const
{
	return _gameOptions.GetPlayerCount();
}

void ReTron::GameState::RestartLevel()
{
	InitLevel();
}

void ReTron::GameState::InitPlayers()
{
	for (size_t i = 0; i < GetPlayerCount(); i++)
	{
		Player& player = _players[i];
		player._active = true;
		player._lives = _diffSpec._lives;
	}
}

void ReTron::GameState::InitLevel()
{
	const GameSpec& gameSpec = _appService->GetGameSpec();
	const LevelSetSpec& levelSetSpec = gameSpec._levelSets.GetKey(_diffSpec._levelSet)->GetValue();

	size_t level = _level % levelSetSpec._levels.Size();
	ff::String levelId = levelSetSpec._levels[level];
	const LevelSpec& levelSpec = gameSpec._levels.GetKey(levelId)->GetValue();

	*_levelState = std::make_shared<LevelState>(this, levelSpec);
}

void ReTron::GameState::InitInput()
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

void ReTron::GameState::AdvanceInput()
{
	const double deltaTime = ff::SECONDS_PER_ADVANCE_D;

	_gameInput->Advance(_gameInputDevices, deltaTime);

	for (size_t i = 0; i < _playerInput.size(); i++)
	{
		_playerInput[i]->Advance(_playerInputDevices[i], deltaTime);
	}
}
