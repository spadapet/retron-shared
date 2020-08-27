#pragma once

namespace ReTron
{
	class IGameService;
	struct LevelSpec;
	struct Player;

	class ILevelService
	{
	public:
		virtual IGameService* GetGameService() const = 0;
		virtual const LevelSpec& GetLevelSpec() const = 0;
		virtual size_t GetPlayerCount() const = 0;
		virtual Player& GetPlayer(size_t index) const = 0;
		virtual Player& GetPlayerOrCoop(size_t index) const = 0;
	};
}
