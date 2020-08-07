#pragma once

namespace ReTron
{
	class ILevelService;

	class LevelRenderService
	{
	public:
		LevelRenderService(ILevelService* levelService);

	private:
		ILevelService* _levelService;
	};
}
