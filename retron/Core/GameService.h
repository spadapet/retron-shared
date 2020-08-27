#pragma once

namespace ff
{
	class IInputEvents;
	struct InputDevices;
}

namespace ReTron
{
	class IAppService;
	struct DifficultySpec;
	struct GameOptions;
	struct Player;

	class IGameService
	{
	public:
		virtual IAppService* GetAppService() const = 0;
		virtual const GameOptions& GetGameOptions() const = 0;
		virtual const DifficultySpec& GetDifficultySpec() const = 0;
		virtual const ff::IInputEvents* GetInputEvents(const Player& player) = 0;
		virtual const ff::InputDevices& GetInputDevices(const Player& player) = 0;
	};
}
