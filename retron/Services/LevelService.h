#pragma once

namespace ReTron
{
	class IGameService;
	class Level;
	struct DifficultySpec;
	struct LevelSpec;

	class ILevelService
	{
	public:
		virtual IGameService* GetGameService() const = 0;
		virtual Level& GetLevel() = 0;
	};
}
