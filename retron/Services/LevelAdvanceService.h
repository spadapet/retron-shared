#pragma once

namespace ReTron
{
	class ILevelService;

	class LevelAdvanceService
	{
	public:
		LevelAdvanceService(ILevelService* levelService);

	private:
		ILevelService* _levelService;
	};
}
