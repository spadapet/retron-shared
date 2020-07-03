#pragma once

namespace ReTron
{
	class IGameService;

	class ILevelService
	{
	public:
		virtual IGameService* GetGameService() const = 0;
	};
}
