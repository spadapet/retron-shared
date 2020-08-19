#pragma once

namespace ff
{
	class IInputEvents;
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
		virtual ff::IInputEvents* GetPlayerInputEvents(size_t index) = 0;
	};
}
