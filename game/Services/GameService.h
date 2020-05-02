#pragma once

namespace Game
{
	class IAppService;
	struct GameOptions;

	class IGameService
	{
	public:
		virtual IAppService* GetAppService() const = 0;
		virtual const GameOptions& GetGameOptions() const = 0;
	};
}
