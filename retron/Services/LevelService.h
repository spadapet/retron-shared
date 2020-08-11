#pragma once

namespace ReTron
{
	class EntityFactory;
	class IGameService;
	struct DifficultySpec;
	struct LevelSpec;

	class ILevelService
	{
	public:
		virtual IGameService* GetGameService() const = 0;
		virtual const DifficultySpec& GetDifficultySpec() const = 0;
		virtual const LevelSpec& GetLevelSpec() const = 0;
		virtual EntityFactory& GetEntityFactory() = 0;
		virtual ff::EntityDomain& GetEntityDomain() = 0;
	};
}
