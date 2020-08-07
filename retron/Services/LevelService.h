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
		virtual const DifficultySpec& GetDifficultySpec() const = 0;
		virtual const LevelSpec& GetLevelSpec() const = 0;
		virtual const Level& GetLevel() const = 0;
		virtual Level& GetLevel() = 0;
	};
}
