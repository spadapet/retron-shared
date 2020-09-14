#include "pch.h"
#include "Core/AppService.h"
#include "Core/GameSpec.h"
#include "Dict/Dict.h"
#include "Globals/AppGlobals.h"
#include "Input/Joystick/JoystickDevice.h"
#include "Input/Joystick/JoystickInput.h"
#include "Input/Keyboard/KeyboardDevice.h"
#include "Input/Pointer/PointerDevice.h"
#include "Resource/Resources.h"
#include "State/StateWrapper.h"
#include "States/GameState.h"
#include "States/LevelState.h"

ReTron::GameState::GameState(IAppService* appService)
	: _appService(appService)
	, _gameOptions(appService->GetDefaultGameOptions())
	, _diffSpec(appService->GetGameSpec()._difficulties.GetKey(_gameOptions.GetDifficultyId())->GetValue())
	, _levelSetSpec(appService->GetGameSpec()._levelSets.GetKey(_diffSpec._levelSet)->GetValue())
	, _playingLevelState(0)
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
	return (_playingLevelState < _levelStates.size()) ? 1 : 0;
}

ff::State* ReTron::GameState::GetChildState(size_t index)
{
	return _levelStates[_playingLevelState].get();
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

const ff::IInputEvents* ReTron::GameState::GetInputEvents(const Player& player)
{
	return _playerInput[player._index].Flush();
}

const ff::InputDevices& ReTron::GameState::GetInputDevices(const Player& player)
{
	return _playerInputDevices[player._index];
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
		_playerInput[i].Init(L"playerControls");
	}

	// Player-specific devices
	for (size_t i = 0; i < _playerInputDevices.size(); i++)
	{
		// In coop, the first player must be controlled by a joystick
		if (!_gameOptions.IsCoop() || i == 1)
		{
			_playerInputDevices[i]._keys.Push(globals.GetKeys());
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

		player._coop = _gameOptions.IsCoop() ? &GetCoopPlayer() : nullptr;
		player._index = i;
		player._lives = _diffSpec._lives;
	}
}

void ReTron::GameState::InitLevel()
{
	_levelStates.clear();
	_playingLevelState = 0;

	if (_gameOptions.IsCoop())
	{
		std::vector<Player*> players;

		for (size_t i = 0; i < _gameOptions.GetPlayerCount(); i++)
		{
			players.push_back(&_players[i]);
		}

		const LevelSpec& levelSpec = GetLevelSpec(GetCoopPlayer()._level);
		std::shared_ptr<LevelState> levelState = std::make_shared<LevelState>(this, levelSpec, std::move(players));
		std::shared_ptr<ff::StateWrapper> levelWrapper = std::make_shared<ff::StateWrapper>(levelState);
		_levelStates.push_back(levelWrapper);
	}
	else
	{
		for (size_t i = 0; i < _gameOptions.GetPlayerCount(); i++)
		{
			Player& player = _players[i];
			std::vector<Player*> players;
			players.push_back(&player);

			const LevelSpec& levelSpec = GetLevelSpec(player._level);
			std::shared_ptr<LevelState> levelState = std::make_shared<LevelState>(this, levelSpec, std::move(players));
			std::shared_ptr<ff::StateWrapper> levelWrapper = std::make_shared<ff::StateWrapper>(levelState);
			_levelStates.push_back(levelWrapper);
		}
	}
}

ReTron::Player& ReTron::GameState::GetCoopPlayer()
{
	return _players.back();
}

const ReTron::LevelSpec& ReTron::GameState::GetLevelSpec(size_t index) const
{
	size_t level = index % _levelSetSpec._levels.size();
	ff::StringRef levelId = _levelSetSpec._levels[level];
	return _appService->GetGameSpec()._levels.GetKey(levelId)->GetValue();
}
