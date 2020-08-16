#pragma once

namespace ReTron
{
	class IGameService;
	struct LevelSpec;

	class ILevelService
	{
	public:
		virtual IGameService* GetGameService() const = 0;
		virtual const LevelSpec& GetLevelSpec() const = 0;
	};
}
