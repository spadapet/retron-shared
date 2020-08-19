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
	InitInput();
	InitPlayers();
	InitLevel();
}

ReTron::GameState::~GameState()
{
}

std::shared_ptr<ff::State> ReTron::GameState::Advance(ff::AppGlobals* globals)
{
	_gameInput->Advance(_gameInputDevices, ff::SECONDS_PER_ADVANCE_D);

	for (size_t i = 0; i < _playerInput.size(); i++)
	{
		_playerInput[i]->Advance(_playerInputDevices[i], ff::SECONDS_PER_ADVANCE_D);
	}

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

ff::IInputEvents* ReTron::GameState::GetPlayerInputEvents(size_t index)
{
	return _playerInput[index].Flush();
}

void ReTron::GameState::RestartLevel()
{
	InitLevel();
}

void ReTron::GameState::InitInput()
{
	ff::AppGlobals& globals = _appService->GetAppGlobals();

	// Game-wide input and devices
	_gameInput.Init(L"gameControls");
	_gameInputDevices._keys.Push(globals.GetKeys());
	_gameInputDevices._mice.Push(globals.GetPointer());

	// Player-specific input
	for (size_t i = 0; i < _playerInput.size(); i++)
	{
		_playerInput[i].Init((i == 0 || !_gameOptions.IsCoop()) ? L"playerControls" : L"secondaryPlayerControls");
	}

	// Player-specific devices
	for (size_t i = 0; i < _playerInputDevices.size(); i++)
	{
		if (i == 0 || !_gameOptions.IsCoop())
		{
			_playerInputDevices[i]._keys.Push(globals.GetKeys());
			_playerInputDevices[i]._mice.Push(globals.GetPointer());
		}
	}

	// Add joysticks game-wide and player-specific
	for (size_t i = 0; i < globals.GetJoysticks()->GetCount(); i++)
	{
		ff::IJoystickDevice* joystick = globals.GetJoysticks()->GetJoystick(i);
		_gameInputDevices._joys.Push(joystick);

		if (_gameOptions.IsCoop())
		{
			if (i < _playerInputDevices.size())
			{
				_playerInputDevices[i]._joys.Push(joystick);
			}
		}
		else
		{
			for (ff::InputDevices& inputDevices : _playerInputDevices)
			{
				inputDevices._joys.Push(joystick);
			}
		}
	}
}

void ReTron::GameState::InitPlayers()
{
	std::memset(_players.data(), 0, _players.size() * sizeof(Player));

	for (size_t i = 0; i < _gameOptions.GetPlayerCount(); i++)
	{
		Player& player = _players[i];

		player._coop = _gameOptions.IsCoop() ? &_players.back() : nullptr;
		player._index = i;
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

	std::vector<Player*> players;
	if (_gameOptions.IsCoop())
	{
		for (size_t i = 0; i < _gameOptions.GetPlayerCount(); i++)
		{
			players.push_back(&_players[i]);
		}
	}
	else
	{
		// ...
	}

	*_levelState = std::make_shared<LevelState>(this, levelSpec, std::move(players));
}
