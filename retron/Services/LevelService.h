#pragma once

namespace Game
{
	class IGameService;

	class ILevelService
	{
	public:
		virtual IGameService* GetGameService() const = 0;
	};
}
